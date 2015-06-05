#include "renderToTextureResource.h"


renderToTextureResource::renderToTextureResource()
{
	m_renderTargetTexture = nullptr;
	m_renderTargetView = nullptr;
	m_shaderResourceView = nullptr;
	counter++;
}


renderToTextureResource::~renderToTextureResource()
{
	
	
}

bool renderToTextureResource::init(ID3D11Device* m_d3d11Device, int _textureW, int _textureH)
{
	HRESULT hr;
	
	//create render target texture
	D3D11_TEXTURE2D_DESC renderTargetTextureDesc;
	ZeroMemory(&renderTargetTextureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	renderTargetTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetTextureDesc.SampleDesc.Count = 1;
	//renderTargetTextureDesc.SampleDesc.Quality = 0;
	renderTargetTextureDesc.MipLevels = 1;
	renderTargetTextureDesc.ArraySize = 1;
	renderTargetTextureDesc.Width = _textureW;
	renderTargetTextureDesc.Height = _textureH;
	renderTargetTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTargetTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
	renderTargetTextureDesc.CPUAccessFlags = 0;
	renderTargetTextureDesc.MiscFlags = 0;
	hr = m_d3d11Device->CreateTexture2D(&renderTargetTextureDesc, NULL, &m_renderTargetTexture);
	
	//create render target view
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = renderTargetTextureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	hr = m_d3d11Device->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &m_renderTargetView);

	//create shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC d3d11ShaderResourceViewDesc;
	d3d11ShaderResourceViewDesc.Format = renderTargetViewDesc.Format;
	d3d11ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	d3d11ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	d3d11ShaderResourceViewDesc.Texture2D.MipLevels = 1;
	hr = m_d3d11Device->CreateShaderResourceView(m_renderTargetTexture, &d3d11ShaderResourceViewDesc, &m_shaderResourceView);

	return true;
}
void renderToTextureResource::Release()
{
	SAFE_RELEASE(m_renderTargetTexture);
	SAFE_RELEASE(m_renderTargetView);
	SAFE_RELEASE(m_shaderResourceView);
	/*counter--;
	if (counter == 0)
		delete this;*/
}

void renderToTextureResource::setRenderTarget(ID3D11DeviceContext* _ID3D11DeviceContext, ID3D11DepthStencilView* _ID3D11DepthStencilView)
{
	//bind the render target view and depth stencil view from main to the output pipeline
	_ID3D11DeviceContext->OMSetRenderTargets(1, &m_renderTargetView, _ID3D11DepthStencilView);
}

//call every frame
void renderToTextureResource::clearRenderTarget(ID3D11DeviceContext* _ID3D11DeviceContext, ID3D11DepthStencilView* _ID3D11DepthStencilView, XMFLOAT4* _xmfloat4)
{
	//use this class to clear render target
	
	//clear back buffer
	_ID3D11DeviceContext->ClearRenderTargetView(m_renderTargetView,(float*) _xmfloat4);

	//clear depth buffer
	_ID3D11DeviceContext->ClearDepthStencilView(_ID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1, 0);
}