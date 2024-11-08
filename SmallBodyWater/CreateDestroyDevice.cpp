#include "main.h"

#include "DXUTgui.h"
#include "SDKmisc.h"

HWND DXUTgetWindow();

GraphicResources * G;

SceneState scene_state;
WaterDefinition water_definition;

BlurHandling blur_handling;

BlurParams blurParams;

std::unique_ptr<Keyboard> _keyboard;
std::unique_ptr<Mouse> _mouse;

CDXUTDialogResourceManager          g_DialogResourceManager;
CDXUTTextHelper*                    g_pTxtHelper = NULL;

#include <codecvt>
std::unique_ptr<SceneNode> loadSponza(ID3D11Device* device, ID3D11InputLayout** l, DirectX::IEffect *e);

inline float lerp(float x1, float x2, float t){
	return x1*(1.0 - t) + x2*t;
}

inline float nextFloat(float x1, float x2){
	return lerp(x1, x2, (float)std::rand() / (float)RAND_MAX);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
void FillGrid_Indexed(std::vector<VertexPositionNormalTexture> & _vertices, std::vector<UINT> & _indices, DWORD dwWidth, DWORD dwLength,
	_In_opt_ std::function<float __cdecl(SimpleMath::Vector3 )> setHeight)
{
	// Fill vertex buffer
	for (DWORD i = 0; i <= dwLength; ++i)
	{
		for (DWORD j = 0; j <= dwWidth; ++j)
		{
			VertexPositionNormalTexture    pVertex;
			pVertex.position.x = ((float)j / dwWidth);
			pVertex.position.z = ((float)i / dwLength);
			pVertex.position.y = setHeight(pVertex.position);
			_vertices.push_back(pVertex);
		}
	}
	VertexPositionNormalTexture    pVertex;

	// Fill index buffer
	int index = 0;
	for (DWORD i = 0; i < dwLength; ++i)
	{
		for (DWORD j = 0; j < dwWidth; ++j)
		{
			/*
			pVertex.position = SimpleMath::Vector3((float)j / dwWidth, 0, (float)i / dwLength); pVertex.position.y=setHeight(pVertex.position);
			pVertex.textureCoordinate = SimpleMath::Vector2(0, 0);
			_vertices.push_back(pVertex);
			pVertex.position = SimpleMath::Vector3((float)j / dwWidth, 0, (float)(i + 1) / dwLength); pVertex.position.y = setHeight(pVertex.position);
			pVertex.textureCoordinate = SimpleMath::Vector2(0, 1);
			_vertices.push_back(pVertex);
			pVertex.position = SimpleMath::Vector3((float)(j + 1) / dwWidth, 0, (float)i / dwLength); pVertex.position.y = setHeight(pVertex.position);
			pVertex.textureCoordinate = SimpleMath::Vector2(1, 0);
			_vertices.push_back(pVertex);

			_indices.push_back(index++);
			_indices.push_back(index++);
			_indices.push_back(index++);

			pVertex.position = SimpleMath::Vector3((float)(j + 1) / dwWidth, 0, (float)i / dwLength); pVertex.position.y = setHeight(pVertex.position);
			pVertex.textureCoordinate = SimpleMath::Vector2(1, 0);
			_vertices.push_back(pVertex);
			pVertex.position = SimpleMath::Vector3((float)j / dwWidth, 0, (float)(i + 1) / dwLength); pVertex.position.y = setHeight(pVertex.position);
			pVertex.textureCoordinate = SimpleMath::Vector2(0, 1);
			_vertices.push_back(pVertex);
			pVertex.position = SimpleMath::Vector3((float)(j + 1) / dwWidth, 0, (float)(i + 1) / dwLength); pVertex.position.y = setHeight(pVertex.position);
			pVertex.textureCoordinate = SimpleMath::Vector2(1, 1);
			_vertices.push_back(pVertex);

			_indices.push_back(index++);
			_indices.push_back(index++);
			_indices.push_back(index++);
			*/

			//_vertices[(DWORD)(i     * (dwWidth + 1) + j)].textureCoordinate = SimpleMath::Vector2(0, 0);
			//_vertices[(DWORD)((i + 1) * (dwWidth + 1) + j)].textureCoordinate = SimpleMath::Vector2(0, 1);
			//_vertices[(DWORD)(i     * (dwWidth + 1) + j + 1)].textureCoordinate = SimpleMath::Vector2(1, 0);
			//_vertices[(DWORD)((i + 1) * (dwWidth + 1) + j + 1)].textureCoordinate = SimpleMath::Vector2(1, 1);

			_indices.push_back((DWORD)(i     * (dwWidth + 1) + j));
			_indices.push_back((DWORD)((i + 1) * (dwWidth + 1) + j));
			_indices.push_back((DWORD)(i     * (dwWidth + 1) + j + 1));

			_indices.push_back((DWORD)(i     * (dwWidth + 1) + j + 1));
			_indices.push_back((DWORD)((i + 1) * (dwWidth + 1) + j));
			_indices.push_back((DWORD)((i + 1) * (dwWidth + 1) + j + 1));
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* device, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	std::srand(unsigned(std::time(0)));

	HRESULT hr;

	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();

	G = new GraphicResources();
	G->render_states = std::make_unique<CommonStates>(device);
	G->scene_constant_buffer = std::make_unique<ConstantBuffer<SceneState> >(device);
	G->scene_water_definition_constant_buffer = std::make_unique<ConstantBuffer<WaterDefinition> >(device);

	_keyboard = std::make_unique<Keyboard>();
	_mouse = std::make_unique<Mouse>();
	HWND hwnd = DXUTgetWindow();
	_mouse->SetWindow(hwnd);

	g_DialogResourceManager.OnD3D11CreateDevice(device, context);
	g_pTxtHelper = new CDXUTTextHelper(device, context, &g_DialogResourceManager, 15);

	//effects
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"ModelVS.hlsl", L"RENDER_TBN_VS", L"vs_5_0" };
		shaderDef[L"GS"] = { L"ModelVS.hlsl", L"RENDER_TBN_GS", L"gs_5_0" };
		shaderDef[L"PS"] = { L"ModelPS.hlsl", L"RENDER_TBN_PS", L"ps_5_0" };

		G->render_tbn_effect = createHlslEffect(device, shaderDef, false);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"ModelVS.hlsl", L"SCREEN_SPACE_QUAD_VS", L"vs_5_0" };
		shaderDef[L"GS"] = { L"ModelVS.hlsl", L"SCREEN_SPACE_QUAD_GS", L"gs_5_0" };
		shaderDef[L"PS"] = { L"ModelPS.hlsl", L"compute_ground_normal_stage1", L"ps_5_0" };

		G->compute_ground_normal_stage1 = createHlslEffect(device, shaderDef, false);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"ModelVS.hlsl", L"SCREEN_SPACE_QUAD_VS", L"vs_5_0" };
		shaderDef[L"GS"] = { L"ModelVS.hlsl", L"SCREEN_SPACE_QUAD_GS", L"gs_5_0" };
		shaderDef[L"PS"] = { L"ModelPS.hlsl", L"compute_ground_normal_stage2", L"ps_5_0" };

		G->compute_ground_normal_stage2 = createHlslEffect(device, shaderDef, false);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"ModelVS.hlsl", L"MAKE_GROUND_VS", L"vs_5_0" };
		shaderDef[L"GS"] = { L"ModelVS.hlsl", L"MAKE_GROUND_GS", L"gs_5_0" };

		G->make_ground_effect = createHlslEffect(device, shaderDef, true);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"ModelVS.hlsl", L"GROUND_VERTEX", L"vs_5_0" };
		shaderDef[L"PS"] = { L"ModelPS.hlsl", L"GROUND_FRAG", L"ps_5_0" };

		G->ground_effect = createHlslEffect(device, shaderDef, false);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"ModelVS.hlsl", L"SKYDOME_VERTEX", L"vs_5_0" };
		shaderDef[L"PS"] = { L"ModelPS.hlsl", L"SKYDOME_FRAG", L"ps_5_0" };

		G->skydome_effect = createHlslEffect(device, shaderDef, false);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"ModelVS.hlsl", L"SKYPLANE_VERTEX", L"vs_5_0" };
		shaderDef[L"PS"] = { L"ModelPS.hlsl", L"SKYPLANE_FRAG", L"ps_5_0" };

		G->skyplane_effect = createHlslEffect(device, shaderDef, false);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"ModelVS.hlsl", L"WATER_VERTEX", L"vs_5_0" };
		shaderDef[L"PS"] = { L"ModelPS.hlsl", L"WATER_FRAG", L"ps_5_0" };

		G->water_effect = createHlslEffect(device, shaderDef, false);
	}
	/*
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"blur.hlsl", L"VS", L"vs_5_0" };
		shaderDef[L"GS"] = { L"blur.hlsl", L"GS", L"gs_5_0" };
		shaderDef[L"PS"] = { L"blur.hlsl", L"HB", L"ps_5_0" };

		G->blur_h_effect = createHlslEffect(device, shaderDef);
	}
	{
		std::map<const WCHAR*, EffectShaderFileDef> shaderDef;
		shaderDef[L"VS"] = { L"blur.hlsl", L"VS", L"vs_5_0" };
		shaderDef[L"GS"] = { L"blur.hlsl", L"GS", L"gs_5_0" };
		shaderDef[L"PS"] = { L"blur.hlsl", L"VB", L"ps_5_0" };

		G->blur_v_effect = createHlslEffect(device, shaderDef);
	}
	*/
	//models
	{
		G->ground_model = CreateTBNModelMeshPart(device, G->out_buffer.ReleaseAndGetAddressOf(), [=](std::vector<PosTBNTex2d> & _vertices, std::vector<UINT> & _indices){
			for (DWORD j = 0; j < 511 * 511 * 2 * 3; ++j){
				_vertices.push_back(PosTBNTex2d());
				_indices.push_back(j);
			}
		});
		G->raw_ground_model = CreateModelMeshPart(device, [=](std::vector<VertexPositionNormalTexture> & _vertices, std::vector<UINT> & _indices){
			FillGrid_Indexed(_vertices, _indices, 511, 511, [=](SimpleMath::Vector3 p){
				return 0;
			});
		});
		G->skyDome_model = CreateModelMeshPart(device, [=](std::vector<VertexPositionNormalTexture> & _vertices, std::vector<UINT> & _indices){
			LoadModel("models\\skydome.txt", _vertices, _indices);
		});
		G->skyPlane_model = CreateModelMeshPart(device, [=](std::vector<VertexPositionNormalTexture> & _vertices, std::vector<UINT> & _indices){
			float constant = 4.0*(1.0 - 0.0);
			FillGrid_Indexed(_vertices, _indices, 50, 50, [&constant](SimpleMath::Vector3 p){
				float x = (p.x - 0.5);
				float z = (p.z - 0.5);
				return 1.0 - constant * ((x * x) + (z * z));
			});
		});
		G->water_model = CreateModelMeshPart(device, [=](std::vector<VertexPositionNormalTexture> & _vertices, std::vector<UINT> & _indices){
			FillGrid_Indexed(_vertices, _indices, 1, 1, [=](SimpleMath::Vector3 p){
				return 0;
			});
		});
		CreateSinglePointBuffer(G->single_point_buffer.ReleaseAndGetAddressOf(), device, G->compute_ground_normal_stage1.get(), G->single_point_layout.ReleaseAndGetAddressOf());


		G->raw_ground_model->CreateInputLayout(device, G->make_ground_effect.get(), G->common_input_layout.ReleaseAndGetAddressOf());

		G->ground_model->CreateInputLayout(device, G->ground_effect.get(), G->ground_model_input_layout.ReleaseAndGetAddressOf());

		hr = D3DX11CreateShaderResourceViewFromFile(device, L"models\\dirt.dds", NULL, NULL, G->ground_texture.ReleaseAndGetAddressOf(), NULL);
		hr = D3DX11CreateShaderResourceViewFromFile(device, L"models\\normal.dds", NULL, NULL, G->ground_normal_texture.ReleaseAndGetAddressOf(), NULL);
		hr = D3DX11CreateShaderResourceViewFromFile(device, L"models\\hm.bmp", NULL, NULL, G->height_texture.ReleaseAndGetAddressOf(), NULL);
		hr = D3DX11CreateShaderResourceViewFromFile(device, L"models\\cloud001.dds", NULL, NULL, G->sky_plane_texture.ReleaseAndGetAddressOf(), NULL);
		hr = D3DX11CreateShaderResourceViewFromFile(device, L"models\\perturb001.dds", NULL, NULL, G->sky_plane_perturb_texture.ReleaseAndGetAddressOf(), NULL);
		hr = D3DX11CreateShaderResourceViewFromFile(device, L"models\\waternormal.dds", NULL, NULL, G->water_texture.ReleaseAndGetAddressOf(), NULL);
		//CreateSinglePointBuffer(G->single_point_buffer.ReleaseAndGetAddressOf(), device, G->quad_effect.get(), G->single_point_layout.ReleaseAndGetAddressOf());
		//G->quad_mesh = CreateQuadModelMeshPart(device);
		//G->quad_mesh->CreateInputLayout(device, G->quad_effect.get(), G->quad_mesh_layout.ReleaseAndGetAddressOf());
	}

	scene_state.viewLightPos.x = 0;

	//blending blending
	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	hr = device->CreateBlendState(&blendStateDescription, G->sky_plane_blend_state.ReleaseAndGetAddressOf());

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	delete g_pTxtHelper;

	g_DialogResourceManager.OnD3D11DestroyDevice();

	_mouse = 0;

	_keyboard = 0;

	delete G;
}
