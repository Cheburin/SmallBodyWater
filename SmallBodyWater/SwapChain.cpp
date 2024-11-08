#include "main.h"

#include "DXUTgui.h"
#include "SDKmisc.h"

extern SceneState scene_state;

extern CDXUTTextHelper*                    g_pTxtHelper;
extern CDXUTDialogResourceManager          g_DialogResourceManager;

SwapChainGraphicResources * SCG;
//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* device, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* backBufferSurfaceDesc, void* pUserContext)
{
	float ratio = backBufferSurfaceDesc->Width / (FLOAT)backBufferSurfaceDesc->Height;

	float np = 0.1, fp =1000, fov = D3DX_PI / 4;

	////Some tests
	{
		SimpleMath::Matrix prm(DirectX::XMMatrixPerspectiveFovLH(fov, ratio, np, fp));

		auto t1 = SimpleMath::Vector4::Transform(SimpleMath::Vector4(0, 0, 0.05, 1), prm);
		t1 /= t1.w;

		auto t2 = SimpleMath::Vector4::Transform(SimpleMath::Vector4(0, 0, -1.99, 1), prm);
		t2 /= t2.w;

		auto t3 = SimpleMath::Vector4::Transform(SimpleMath::Vector4(0, 0, -0.05, 1), prm);
	}
	////
	HRESULT hr;

	g_DialogResourceManager.OnD3D11ResizedSwapChain(device, backBufferSurfaceDesc);

	DirectX::XMStoreFloat4x4(&scene_state.mProjection, DirectX::XMMatrixTranspose(DirectX::XMMatrixPerspectiveFovLH(fov, ratio, np, fp)));
	//DirectX::XMStoreFloat4x4(&scene_state.mProjection, DirectX::XMMatrixTranspose(DirectX::XMMatrixOrthographicLH(50, 50, np, fp)));

	scene_state.vFrustumParams = SimpleMath::Vector4(backBufferSurfaceDesc->Width, backBufferSurfaceDesc->Height, scene_state.mProjection._22, ratio);

	scene_state.vFrustumNearFar = SimpleMath::Vector4(np, fp, 0, 0);

	SCG = new SwapChainGraphicResources();

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = 512;// backBufferSurfaceDesc->Width;
	textureDesc.Height = 512;// backBufferSurfaceDesc->Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;////DXGI_FORMAT_R32G8X24_TYPELESS;// DXGI_FORMAT_R24G8_TYPELESS;// ;// ;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	dsv_desc.Flags = 0;
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT; //DXGI_FORMAT_D32_FLOAT_S8X24_UINT;// DXGI_FORMAT_D24_UNORM_S8_UINT;// ;//;
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC sr_desc;
	sr_desc.Format = DXGI_FORMAT_R32_FLOAT;////DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;// DXGI_FORMAT_R24_UNORM_X8_TYPELESS;// ;// ;
	sr_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sr_desc.Texture2D.MostDetailedMip = 0;
	sr_desc.Texture2D.MipLevels = 1; //-1

	hr = device->CreateTexture2D(&textureDesc, 0, SCG->depthStencilT.ReleaseAndGetAddressOf());

	hr = device->CreateDepthStencilView(SCG->depthStencilT.Get(), &dsv_desc, SCG->depthStencilV.ReleaseAndGetAddressOf());

	hr = device->CreateShaderResourceView(SCG->depthStencilT.Get(), &sr_desc, SCG->depthStencilSRV.ReleaseAndGetAddressOf());

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	hr = device->CreateTexture2D(&textureDesc, nullptr, SCG->faseNormalT.ReleaseAndGetAddressOf());

	hr = device->CreateUnorderedAccessView(SCG->faseNormalT.Get(), nullptr, SCG->faseNormalV.ReleaseAndGetAddressOf());

	hr = device->CreateShaderResourceView(SCG->faseNormalT.Get(), nullptr, SCG->faseNormalSRV.ReleaseAndGetAddressOf());


	hr = device->CreateTexture2D(&textureDesc, nullptr, SCG->vertexNormalT.ReleaseAndGetAddressOf());

	hr = device->CreateUnorderedAccessView(SCG->vertexNormalT.Get(), nullptr, SCG->vertexNormalV.ReleaseAndGetAddressOf());

	hr = device->CreateShaderResourceView(SCG->vertexNormalT.Get(), nullptr, SCG->vertexNormalSRV.ReleaseAndGetAddressOf());


	hr = device->CreateTexture2D(&textureDesc, nullptr, SCG->vertexTangentT.ReleaseAndGetAddressOf());

	hr = device->CreateUnorderedAccessView(SCG->vertexTangentT.Get(), nullptr, SCG->vertexTangentV.ReleaseAndGetAddressOf());

	hr = device->CreateShaderResourceView(SCG->vertexTangentT.Get(), nullptr, SCG->vertexTangentSRV.ReleaseAndGetAddressOf());


	hr = device->CreateTexture2D(&textureDesc, nullptr, SCG->vertexBinormalT.ReleaseAndGetAddressOf());

	hr = device->CreateUnorderedAccessView(SCG->vertexBinormalT.Get(), nullptr, SCG->vertexBinormalV.ReleaseAndGetAddressOf());

	hr = device->CreateShaderResourceView(SCG->vertexBinormalT.Get(), nullptr, SCG->vertexBinormalSRV.ReleaseAndGetAddressOf());

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	textureDesc.Width = backBufferSurfaceDesc->Width;
	textureDesc.Height = backBufferSurfaceDesc->Height;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

	hr = device->CreateTexture2D(&textureDesc, nullptr, SCG->refractionT.ReleaseAndGetAddressOf());

	hr = device->CreateRenderTargetView(SCG->refractionT.Get(), nullptr, SCG->refractionV.ReleaseAndGetAddressOf());

	hr = device->CreateShaderResourceView(SCG->refractionT.Get(), nullptr, SCG->refractionSRV.ReleaseAndGetAddressOf());


	hr = device->CreateTexture2D(&textureDesc, nullptr, SCG->reflectionT.ReleaseAndGetAddressOf());
	
	hr = device->CreateRenderTargetView(SCG->reflectionT.Get(), nullptr, SCG->reflectionV.ReleaseAndGetAddressOf());

	hr = device->CreateShaderResourceView(SCG->reflectionT.Get(), nullptr, SCG->reflectionSRV.ReleaseAndGetAddressOf());

	return S_OK;
}
//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	g_DialogResourceManager.OnD3D11ReleasingSwapChain();

	delete SCG;
}