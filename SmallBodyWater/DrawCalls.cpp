#include "main.h"

extern GraphicResources * G;

extern SceneState scene_state;

using namespace SimpleMath;

void loadMatrix_VP(Matrix & v, Matrix & p){
	v = Matrix(scene_state.mView).Transpose();
	p = Matrix(scene_state.mProjection).Transpose();
}
void loadMatrix_WP(Matrix & w, Matrix & p){
	w = Matrix(scene_state.mWorld).Transpose();
	p = Matrix(scene_state.mProjection).Transpose();
}
void storeMatrix(Matrix & w, Matrix & wv, Matrix & wvp){
	scene_state.mWorld = w.Transpose();
	scene_state.mWorldView = wv.Transpose();
	scene_state.mWorldViewProjection = wvp.Transpose();
}

void DrawQuad(ID3D11DeviceContext* pd3dImmediateContext,  _In_ IEffect* effect, 
	_In_opt_ std::function<void __cdecl()> setCustomState){
	//effect->Apply(pd3dImmediateContext);
	//setCustomState();

	//pd3dImmediateContext->IASetInputLayout(nullptr);
	//pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);// D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//pd3dImmediateContext->Draw(1, 0);
	pd3dImmediateContext->IASetInputLayout(G->single_point_layout.Get());

	effect->Apply(pd3dImmediateContext);

	auto vertexBuffer = G->single_point_buffer.Get();
	UINT vertexStride = sizeof(XMFLOAT3);
	UINT vertexOffset = 0;

	pd3dImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);

	setCustomState();

	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	pd3dImmediateContext->Draw(1, 0);
}


void set_scene_world_matrix(DirectX::XMFLOAT4X4 transformation){
	Matrix w, v, p;
	loadMatrix_VP(v, p);

	w = transformation;// .Translation(Vector3(0.0f, 6.0f, 8.0f));
	Matrix  wv = w * v;
	Matrix wvp = wv * p;

	storeMatrix(w, wv, wvp);
}
void scene_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void(ID3D11ShaderResourceView * texture, DirectX::XMFLOAT4X4 transformation)> setCustomState){
	//G->scene->draw(pd3dImmediateContext, effect, inputLayout, setCustomState);
}

void post_proccess(ID3D11DeviceContext* context, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState){
	/*
	context->IASetInputLayout(inputLayout);

	effect->Apply(context);

	auto vertexBuffer = G->single_point_buffer.Get();
	UINT vertexStride = sizeof(XMFLOAT3);
	UINT vertexOffset = 0;

	context->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &vertexOffset);

    setCustomState();

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->Draw(1, 0);
	*/
	//G->quad_mesh->Draw(context, effect, inputLayout, [=]
	//{
	//	setCustomState();
	//});
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void water_set_object1_matrix(SimpleMath::Matrix v){
	Matrix w, p;
	loadMatrix_WP(w, p);

	Matrix  wvp = w * v * p;

	scene_state.mObject1WorldViewProjection = wvp.Transpose();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
