#include "GeometricPrimitive.h"
#include "Effects.h"
#include "DirectXHelpers.h"
#include "Model.h"
#include "CommonStates.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "SimpleMath.h"

#include "DirectXMath.h"

#include "DXUT.h"

#include <wrl.h>

#include <map>
#include <algorithm>
#include <array>
#include <memory>
#include <assert.h>
#include <malloc.h>
#include <Exception>

#include "ConstantBuffer.h"

#include "AppBuffers.h"

using namespace DirectX;

struct EffectShaderFileDef{
	WCHAR * name;
	WCHAR * entry_point;
	WCHAR * shader_ver;
};
class IPostProcess
{
public:
	virtual ~IPostProcess() { }

	virtual void __cdecl Process(_In_ ID3D11DeviceContext* deviceContext, _In_opt_ std::function<void __cdecl()> setCustomState = nullptr) = 0;
};

inline ID3D11RenderTargetView** renderTargetViewToArray(ID3D11RenderTargetView* rtv1, ID3D11RenderTargetView* rtv2 = 0, ID3D11RenderTargetView* rtv3 = 0){
	static ID3D11RenderTargetView* rtvs[10];
	rtvs[0] = rtv1;
	rtvs[1] = rtv2;
	rtvs[2] = rtv3;
	return rtvs;
};
inline ID3D11ShaderResourceView** shaderResourceViewToArray(ID3D11ShaderResourceView* rtv1, ID3D11ShaderResourceView* rtv2 = 0, ID3D11ShaderResourceView* rtv3 = 0, ID3D11ShaderResourceView* rtv4 = 0, ID3D11ShaderResourceView* rtv5 = 0){
	static ID3D11ShaderResourceView* srvs[10];
	srvs[0] = rtv1;
	srvs[1] = rtv2;
	srvs[2] = rtv3;
	srvs[3] = rtv4;
	srvs[4] = rtv5;
	return srvs;
};
inline ID3D11Buffer** constantBuffersToArray(ID3D11Buffer* c1, ID3D11Buffer* c2){
	static ID3D11Buffer* cbs[10];
	cbs[0] = c1;
	cbs[1] = c2;
	return cbs;
};
inline ID3D11Buffer** constantBuffersToArray(DirectX::ConstantBuffer<SceneState> &cb){
	static ID3D11Buffer* cbs[10];
	cbs[0] = cb.GetBuffer();
	return cbs;
};
inline ID3D11Buffer** constantBuffersToArray(DirectX::ConstantBuffer<BlurHandling> &cb){
	static ID3D11Buffer* cbs[10];
	cbs[0] = cb.GetBuffer();
	return cbs;
};
inline ID3D11Buffer** constantBuffersToArray(DirectX::ConstantBuffer<WaterDefinition> &cb){
	static ID3D11Buffer* cbs[10];
	cbs[0] = cb.GetBuffer();
	return cbs;
};
inline ID3D11Buffer** constantBuffersToArray(DirectX::ConstantBuffer<SceneState> &c1, DirectX::ConstantBuffer<BlurHandling> &c2){
	static ID3D11Buffer* cbs[10];
	cbs[0] = c1.GetBuffer();
	cbs[1] = c2.GetBuffer();
	return cbs;
};
inline ID3D11SamplerState** samplerStateToArray(ID3D11SamplerState* ss1, ID3D11SamplerState* ss2 = 0){
	static ID3D11SamplerState* sss[10];
	sss[0] = ss1;
	sss[1] = ss2;
	return sss;
};

namespace Camera{
	void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
}
std::unique_ptr<DirectX::IEffect> createHlslEffect(ID3D11Device* device, std::map<const WCHAR*, EffectShaderFileDef>& fileDef, bool withSo);

bool LoadModel(char* filename, std::vector<VertexPositionNormalTexture> & _vertices, std::vector<UINT> & _indices);
std::unique_ptr<DirectX::ModelMeshPart> CreateModelMeshPart(ID3D11Device* device, std::function<void(std::vector<VertexPositionNormalTexture> & _vertices, std::vector<UINT> & _indices)> createGeometry);
void CreateSinglePointBuffer(ID3D11Buffer ** vertexBuffer, ID3D11Device* device, DirectX::IEffect * effect, ID3D11InputLayout ** layout);
std::unique_ptr<DirectX::ModelMeshPart> CreateQuadModelMeshPart(ID3D11Device* device);

void set_scene_world_matrix(DirectX::XMFLOAT4X4 transformation);
void scene_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void(ID3D11ShaderResourceView * texture, DirectX::XMFLOAT4X4 transformation)> setCustomState = nullptr);

void post_proccess(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState);

void wall_set_world_matrix();
void wall_draw(ID3D11DeviceContext* pd3dImmediateContext, IEffect* effect, ID3D11InputLayout* inputLayout, _In_opt_ std::function<void __cdecl()> setCustomState = nullptr);

void DrawQuad(ID3D11DeviceContext* pd3dImmediateContext, _In_ IEffect* effect, _In_opt_ std::function<void __cdecl()> setCustomState);

struct SceneNode{
	DirectX::XMFLOAT4X4 transformation;
	std::vector<DirectX::ModelMeshPart*> mesh;
	std::vector<SceneNode*> children;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> > texture;
	~SceneNode();
	void draw(_In_ ID3D11DeviceContext* deviceContext, _In_ IEffect* ieffect, _In_ ID3D11InputLayout* iinputLayout,
		_In_opt_ std::function<void(ID3D11ShaderResourceView * texture, DirectX::XMFLOAT4X4 transformation)> setCustomState);
};

struct BlurParams
{
	int Radius;
	int WeightLength;
	float Weights[64 * 2 + 1];

	BlurParams(int radius = 0)
	{
		WeightLength = (Radius = radius) * 2 + 1;
	}
};
BlurParams GaussianBlur(int radius);

class GraphicResources {
public:
	std::unique_ptr<CommonStates> render_states;

	std::unique_ptr<DirectX::IEffect> compute_ground_normal_stage1;

	std::unique_ptr<DirectX::IEffect> render_tbn_effect;

	std::unique_ptr<DirectX::IEffect> compute_ground_normal_stage2;

	std::unique_ptr<DirectX::IEffect> make_ground_effect;

	std::unique_ptr<DirectX::IEffect> ground_effect;

	std::unique_ptr<DirectX::IEffect> skydome_effect;

	std::unique_ptr<DirectX::IEffect> skyplane_effect;

	std::unique_ptr<DirectX::IEffect> water_effect;

	std::unique_ptr<DirectX::ConstantBuffer<SceneState> > scene_constant_buffer;

	std::unique_ptr<DirectX::ConstantBuffer<WaterDefinition> > scene_water_definition_constant_buffer;

	std::unique_ptr<DirectX::ModelMeshPart> raw_ground_model;

	std::unique_ptr<DirectX::ModelMeshPart> ground_model;

	std::unique_ptr<DirectX::ModelMeshPart> skyDome_model;

	std::unique_ptr<DirectX::ModelMeshPart> skyPlane_model;

	std::unique_ptr<DirectX::ModelMeshPart> water_model;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> common_input_layout;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> ground_model_input_layout;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ground_texture;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ground_normal_texture;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> height_texture;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sky_plane_texture;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sky_plane_perturb_texture;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> water_texture;

	//std::unique_ptr<DirectX::ModelMeshPart> quad_mesh;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> quad_mesh_layout;

	Microsoft::WRL::ComPtr<ID3D11Buffer> single_point_buffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> out_buffer;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> single_point_layout;

	Microsoft::WRL::ComPtr<ID3D11BlendState> sky_plane_blend_state;
};

class SwapChainGraphicResources {
public:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depthStencilSRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilT;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> faseNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> faseNormalV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> faseNormalT;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> vertexNormalSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> vertexNormalV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> vertexNormalT;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> vertexTangentSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> vertexTangentV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> vertexTangentT;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> vertexBinormalSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> vertexBinormalV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> vertexBinormalT;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> refractionSRV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> refractionV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> refractionT;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> reflectionSRV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> reflectionV;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> reflectionT;
};
struct PosTBNTex2d{
	XMFLOAT4 position;
	
	XMFLOAT4 color;
	
	XMFLOAT2 textureCoordinate;

	XMFLOAT3 tangent;

	XMFLOAT3 binormal;

	XMFLOAT3 normal;

	static const int InputElementCount = 6;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};
std::unique_ptr<DirectX::ModelMeshPart> CreateTBNModelMeshPart(ID3D11Device* device, ID3D11Buffer** out_stream_buffer, std::function<void(std::vector<PosTBNTex2d> & _vertices, std::vector<UINT> & _indices)> createGeometry);

#include "DrawCalls.h"