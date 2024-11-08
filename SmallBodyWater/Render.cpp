#include "main.h"

#include "DXUTgui.h"
#include "SDKmisc.h"

extern GraphicResources * G;

extern SwapChainGraphicResources * SCG;

extern SceneState scene_state;

extern WaterDefinition water_definition;

extern BlurHandling blur_handling;

extern CDXUTTextHelper*                    g_pTxtHelper;

ID3D11ShaderResourceView* null[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

inline void set_scene_constant_buffer(ID3D11DeviceContext* context){
	G->scene_constant_buffer->SetData(context, scene_state);
};

inline void set_blur_constant_buffer(ID3D11DeviceContext* context){
	//G->blur_constant_buffer->SetData(context, blur_handling);
};

inline void set__water_definition_constant_buffer(ID3D11DeviceContext* context){
	G->scene_water_definition_constant_buffer->SetData(context, water_definition);
};

void RenderText()
{
	g_pTxtHelper->Begin();
	g_pTxtHelper->SetInsertionPos(2, 0);
	g_pTxtHelper->SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
	g_pTxtHelper->DrawTextLine(DXUTGetFrameStats(true && DXUTIsVsyncEnabled()));
	g_pTxtHelper->DrawTextLine(DXUTGetDeviceStats());

	g_pTxtHelper->End();
}

void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	Camera::OnFrameMove(fTime, fElapsedTime, pUserContext);
}

void renderScene(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, bool scene, bool refraction, bool reflection);
void postProccessGBuffer(ID3D11Device* pd3dDevice, ID3D11DeviceContext* context);
void postProccessBlur(ID3D11Device* pd3dDevice, ID3D11DeviceContext* context, _In_opt_ std::function<void __cdecl()> setHState, _In_opt_ std::function<void __cdecl()> setVState);

void clear(ID3D11DeviceContext* context, float ClearColor[], int n, ID3D11RenderTargetView** pRTV, ID3D11DepthStencilView* pDSV){
	for (int i = 0; i < n; i++)
		context->ClearRenderTargetView(pRTV[i], ClearColor);

	context->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
void clearAndSetRenderTarget(ID3D11DeviceContext* context, float ClearColor[], int n, ID3D11RenderTargetView** pRTV, ID3D11DepthStencilView* pDSV){
	for (int i = 0; i < n; i++)
		context->ClearRenderTargetView(pRTV[i], ClearColor);

	context->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	context->OMSetRenderTargets(n, pRTV, pDSV); //renderTargetViewToArray(pRTV) DXUTGetD3D11RenderTargetView
}
inline ID3D11UnorderedAccessView** UAVToArray(ID3D11UnorderedAccessView* rtv1, ID3D11UnorderedAccessView* rtv2 = 0, ID3D11UnorderedAccessView* rtv3 = 0){
	static ID3D11UnorderedAccessView* rtvs[10];
	rtvs[0] = rtv1;
	rtvs[1] = rtv2;
	rtvs[2] = rtv3;
	return rtvs;
};
////////////////
inline SimpleMath::Matrix make_reflection_view(DirectX::XMFLOAT4 plane){
	auto reflection = DirectX::XMMatrixReflect(XMLoadFloat4(&plane));

	auto invView = XMMatrixTranspose(XMLoadFloat4x4(&scene_state.mInvView));

	DirectX::XMMATRIX reflection_inv_view;
	reflection_inv_view.r[0] = -1 * XMVector4Transform(invView.r[0], reflection);
	reflection_inv_view.r[1] = XMVector4Transform(invView.r[1], reflection);
	reflection_inv_view.r[2] = XMVector4Transform(invView.r[2], reflection);
	reflection_inv_view.r[3] = XMVector4Transform(invView.r[3], reflection);

	auto reflection_view = DirectX::XMMatrixInverse(0, reflection_inv_view);

	return reflection_view;
};
inline SimpleMath::Matrix get_view(){
	return SimpleMath::Matrix(scene_state.mView).Transpose();
};
inline void set_view(SimpleMath::Matrix & view){
	scene_state.mView = view.Transpose();
	scene_state.mInvView = view.Invert().Transpose();
};
void water_set_object1_matrix(SimpleMath::Matrix v);
////////////////
auto reflectionPlane = SimpleMath::Vector4(0, 1, 0, -3.75);

auto refractionPlane = SimpleMath::Vector4(0, -1, 0, 3.75 + 0.1);

auto infinityPlane = SimpleMath::Vector4(0, 1, 0, 999.999);
///////////////
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* context,
	double fTime, float fElapsedTime, void* pUserContext)
{
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = scene_state.vFrustumParams.x;
	vp.Height = scene_state.vFrustumParams.y;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	/////////////
	static bool preCalc = false;

	clearAndSetRenderTarget(context, clearColor, 1, renderTargetViewToArray(DXUTGetD3D11RenderTargetView()), DXUTGetD3D11DepthStencilView());
	/////////////
	context->GSSetConstantBuffers(0, 1, constantBuffersToArray(*(G->scene_constant_buffer)));

	context->VSSetConstantBuffers(0, 1, constantBuffersToArray(*(G->scene_constant_buffer)));

	context->VSSetConstantBuffers(1, 1, constantBuffersToArray(*(G->scene_water_definition_constant_buffer)));

	context->PSSetConstantBuffers(1, 1, constantBuffersToArray(*(G->scene_water_definition_constant_buffer)));

	context->PSSetSamplers(0, 1, samplerStateToArray(G->render_states->AnisotropicWrap()));
	/////////////

	//расчитываем нормали GROUND
	if (!preCalc)
	{
		preCalc = true;

		vp.Width = 512;
		vp.Height = 512;
		context->RSSetViewports(1, &vp);

		context->OMSetRenderTargetsAndUnorderedAccessViews(
			0,
			0,
			SCG->depthStencilV.Get(),
			1,
			1,
			UAVToArray(SCG->faseNormalV.Get()),
			0
		);
		DrawQuad(context, G->compute_ground_normal_stage1.get(), [=]{
			context->PSSetShaderResources(0, 1, shaderResourceViewToArray(G->height_texture.Get()));

			context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
			context->RSSetState(G->render_states->CullNone());
			context->OMSetDepthStencilState(G->render_states->DepthNone(), 0);
		});

		context->OMSetRenderTargetsAndUnorderedAccessViews(
			0,
			0,
			SCG->depthStencilV.Get(),
			2,
			1,
			UAVToArray(SCG->vertexNormalV.Get()),
			0
		);
		DrawQuad(context, G->compute_ground_normal_stage2.get(), [=]{
			context->PSSetShaderResources(2, 1, shaderResourceViewToArray(SCG->faseNormalSRV.Get()));

			context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
			context->RSSetState(G->render_states->CullNone());
			context->OMSetDepthStencilState(G->render_states->DepthNone(), 0);
		});

		vp.Width = scene_state.vFrustumParams.x;
		vp.Height = scene_state.vFrustumParams.y;
		context->RSSetViewports(1, &vp);

		context->OMSetRenderTargets(0, 0, SCG->depthStencilV.Get());
		{
			UINT offset[1] = { 0 };
			ID3D11Buffer* soBuffers[1] = { G->ground_model->vertexBuffer.Get() };
			context->SOSetTargets(1, soBuffers, offset);
		}
		G->raw_ground_model->Draw(context, G->make_ground_effect.get(), G->common_input_layout.Get(), [=]{
			context->VSSetShaderResources(0, 1, shaderResourceViewToArray(G->height_texture.Get()));
			context->VSSetShaderResources(3, 1, shaderResourceViewToArray(SCG->vertexNormalSRV.Get()));

			context->RSSetState(G->render_states->CullNone());
			context->OMSetDepthStencilState(G->render_states->DepthNone(), 0);
		});
		{
			UINT offset[1] = { 0 };
			ID3D11Buffer* soBuffers[1] = { 0 };
			context->SOSetTargets(1, soBuffers, offset);
		}
	}

	{
		context->PSSetShaderResources(0, 5, null);
		context->VSSetShaderResources(0, 5, null);
		context->GSSetShaderResources(0, 5, null);
	}

	auto camera_view = get_view();

	auto refractionView = camera_view;

	auto reflectionView = make_reflection_view(reflectionPlane);

	//render scene refraction into water
	{
		clearAndSetRenderTarget(context, clearColor, 1, renderTargetViewToArray(SCG->refractionV.Get()), DXUTGetD3D11DepthStencilView());

		set_view(refractionView);
		water_definition.plane = refractionPlane;
		set__water_definition_constant_buffer(context);

		renderScene(pd3dDevice, context, false, true, false);
	}
	//render scene reflection from water
	{
		clearAndSetRenderTarget(context, clearColor, 1, renderTargetViewToArray(SCG->reflectionV.Get()), DXUTGetD3D11DepthStencilView());

		set_view(reflectionView);
		water_definition.plane = reflectionPlane;
		set__water_definition_constant_buffer(context);

		renderScene(pd3dDevice, context, false, false, true);
	}
	//render scene and water with refraction
	{
		clearAndSetRenderTarget(context, clearColor, 1, renderTargetViewToArray(DXUTGetD3D11RenderTargetView()), DXUTGetD3D11DepthStencilView());

		set_view(camera_view);
		water_definition.plane = infinityPlane;
		set__water_definition_constant_buffer(context);

		renderScene(pd3dDevice, context, true, false, false);
		
		water_set_world_matrix();
		water_set_object1_matrix(reflectionView);
		set_scene_constant_buffer(context);

		water_draw(context, G->water_effect.get(), G->common_input_layout.Get(), [=]{
			context->PSSetShaderResources(0, 1, shaderResourceViewToArray(SCG->refractionSRV.Get()));
			context->PSSetShaderResources(1, 1, shaderResourceViewToArray(SCG->reflectionSRV.Get()));
			context->PSSetShaderResources(4, 1, shaderResourceViewToArray(G->water_texture.Get()));

			context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
			context->RSSetState(G->render_states->CullCounterClockwise());
			context->OMSetDepthStencilState(G->render_states->DepthDefault(), 0);
		});
	}
	RenderText();
}

void renderScene(ID3D11Device* pd3dDevice, ID3D11DeviceContext* context, bool scene, bool refraction, bool reflection)
{
	//if (scene || reflection)
	{
		
		if (reflection){
			water_definition.plane = infinityPlane;
			set__water_definition_constant_buffer(context);
		}

		skydome_set_world_matrix();
		set_scene_constant_buffer(context);
		skydome_draw(context, G->skydome_effect.get(), G->common_input_layout.Get(), [=]{
			context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
			context->RSSetState(G->render_states->CullNone());
			context->OMSetDepthStencilState(G->render_states->DepthNone(), 0);
		});

		skyPlane_set_world_matrix();
		set_scene_constant_buffer(context);
		skyPlane_draw(context, G->skyplane_effect.get(), G->common_input_layout.Get(), [=]{
			context->PSSetShaderResources(0, 1, shaderResourceViewToArray(G->sky_plane_perturb_texture.Get()));
			context->PSSetShaderResources(1, 1, shaderResourceViewToArray(G->sky_plane_texture.Get()));

			float blendFactor[4];
			blendFactor[0] = 0.0f;
			blendFactor[1] = 0.0f;
			blendFactor[2] = 0.0f;
			blendFactor[3] = 0.0f;
			context->OMSetBlendState(G->sky_plane_blend_state.Get(), &blendFactor[0], 0xFFFFFFFF);
			context->RSSetState(G->render_states->CullClockwise());
			context->OMSetDepthStencilState(G->render_states->DepthNone(), 0);
		});
	}

	if (reflection){
		water_definition.plane = reflectionPlane;
		set__water_definition_constant_buffer(context);
	}

	ground_set_world_matrix();
	set_scene_constant_buffer(context);
	ground_draw(context, G->ground_effect.get(), G->ground_model_input_layout.Get(), [=]{
		context->PSSetShaderResources(0, 1, shaderResourceViewToArray(G->ground_normal_texture.Get()));
		context->PSSetShaderResources(1, 1, shaderResourceViewToArray(G->ground_texture.Get()));

		context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
		context->RSSetState(G->render_states->CullCounterClockwise());
		context->OMSetDepthStencilState(G->render_states->DepthDefault(), 0);
	});
	/*
	auto prevtopology = G->ground_model->primitiveType;
	G->ground_model->primitiveType = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	ground_draw(context, G->render_tbn_effect.get(), G->ground_model_input_layout.Get(), [=]{
		context->OMSetBlendState(G->render_states->Opaque(), Colors::Black, 0xFFFFFFFF);
		context->RSSetState(G->render_states->Wireframe());
		context->OMSetDepthStencilState(G->render_states->DepthDefault(), 0);
	});
	G->ground_model->primitiveType = prevtopology;
	*/
}