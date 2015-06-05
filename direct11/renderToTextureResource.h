#pragma once
#include <d3d11.h>
#define SAFE_RELEASE(p){if(p){p->Release();p=NULL;}}

#include <DirectXMath.h>
using namespace DirectX;
class renderToTextureResource
{
private:
	int counter = 0;
	ID3D11Texture2D* m_renderTargetTexture;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11ShaderResourceView* m_shaderResourceView;
public:
	renderToTextureResource();
	~renderToTextureResource();

	bool init(ID3D11Device*, int, int);
	void Release();

	void setRenderTarget(ID3D11DeviceContext* _ID3D11DeviceContext, ID3D11DepthStencilView*);
	void clearRenderTarget(ID3D11DeviceContext* _ID3D11DeviceContext, ID3D11DepthStencilView*,  XMFLOAT4* _xmfloat4);
	ID3D11ShaderResourceView* getShaderResourceView()const
	{
		return m_shaderResourceView;
	}
};

