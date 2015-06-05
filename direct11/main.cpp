//////////////
// INCLUDES //
//////////////
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>

#include <iostream>
#include <ctime>
#include "XTime.h"
#include <cmath>
using namespace std;

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")

#include "../direct11/Trivial_PS.csh"
#include "../direct11/Trivial_VS.csh"

#include "../direct11/MyVertexShader.csh"
#include "../direct11/MyPixelShader.csh"

#include "../direct11/InstanceVS.csh"
#include "../direct11/InstancePS.csh"

#include "../direct11/geoShader.csh"
#include "../direct11/geoPS.csh"
#include "../direct11/geoVS.csh"

//tessellation
#include "../direct11/Tes_VS.csh"
#include "../direct11/Tes_HS.csh"
#include "../direct11/Tes_DS.csh"
#include "../direct11/Tes_PS.csh"

//ground
#include "../direct11/ground_PS.csh"
#include "../direct11/ground_VS.csh"
//multiple output
#include "../direct11/GSViewPort.csh"

//foliage
#include "../direct11/foliage_PS.csh"
#include "../direct11/foliage_VS.csh"
#include "../direct11/foliage.h"

#define BACKBUFFER_WIDTH	1200
#define BACKBUFFER_HEIGHT	800
#define SAFE_RELEASE(p){if(p){p->Release();p=NULL;}}
#define circleNumber 360
#define starNumber 22
#define rainNumber 3000
#define cubeNumber 776
#define skyboxNumber 8
#define gridNumber 4
#define gridIndex 6
#define skyboxIndexNumber 36

#define lotusNumber 6706
#define lotusIndexNumber 15354
#define spotLightDegree 10.0f
#define rectNumber 4
#define rasterizeNumber 8
#define mirrorNumber 3
//***model
#include "Cube.h"

#include <windows.h>
#include "checkerboard.h"
#include "numbers_test.h"

#include "DDSTextureLoader.h"
#include <DirectXMath.h>
#include "lotus.h"
using namespace DirectX;

#include "camera.h"
#include "ObjLoader.h"
#include <vector>
#include "renderToTextureResource.h"
#include "mirror.h"
struct color
{
	float a, r, g, b;
	color(){}
	color(float _a, float _r, float _g, float _b)
	{
		a = _a;r = _r;g = _g;b = _b;
	}
};
struct SIMPLE_VERTEX
{
	XMFLOAT3 xyz;
	XMFLOAT3 uvw;
	XMFLOAT3 nrm;


	XMFLOAT3 tangent;
	XMFLOAT3 binormal;
	SIMPLE_VERTEX(){}
};



struct LightPosData
{
	XMFLOAT3 cameraPos;
	float pad;

	XMFLOAT3 pointLightPos;
	float pointLightNumber;

	XMFLOAT3 spotLightPos;
	float pad2;
};
struct TessellationConstantBuffer
{
	float tessellactionAmout;
	XMFLOAT3 padding;
};
struct MatrixCalculate
{
	XMMATRIX m_worldMatrix1;
	XMMATRIX m_viewMatrix;
	XMMATRIX m_projectionMatrix;

	void setProject(float _near, float _far, float FOV, float aspectRatio)
	{
		float scaleY = 1 / tanf(FOV / 2);
		float scaleX = scaleY*aspectRatio;
		float zNear = 0.1f;
		if (_near != 0)
			zNear = _near;

		float zFar;
		if (_far > _near)
			zFar = _far;
		else
		{
			zFar = _near;
		}

		m_projectionMatrix.r->m128_f32[0] = scaleX;
		m_projectionMatrix.r->m128_f32[1] = 0;
		m_projectionMatrix.r->m128_f32[2] = 0;
		m_projectionMatrix.r->m128_f32[3] = 0;
		m_projectionMatrix.r->m128_f32[4] = 0;
		m_projectionMatrix.r->m128_f32[5] = scaleY;
		m_projectionMatrix.r->m128_f32[6] = 0;
		m_projectionMatrix.r->m128_f32[7] = 0;
		m_projectionMatrix.r->m128_f32[8] = 0;
		m_projectionMatrix.r->m128_f32[9] = 0;
		m_projectionMatrix.r->m128_f32[10] = zFar / (zFar - _near);
		m_projectionMatrix.r->m128_f32[11] = 1;
		m_projectionMatrix.r->m128_f32[12] = 0;
		m_projectionMatrix.r->m128_f32[13] = 0;
		m_projectionMatrix.r->m128_f32[14] = -(zFar *zNear) / (zFar - zNear);
		m_projectionMatrix.r->m128_f32[15] = 0;
	}
};

//for diffuse color
struct LightColorData
{
	//ambient
	XMFLOAT4 ambientColor;

	//directional 
	XMFLOAT4 diffuseColor;
	XMFLOAT3 lightDirectionSource;

	//specular
	float specualrPower;
	XMFLOAT4 specularColor;

	//point light
	XMFLOAT4 pointLightColor;

	//spot light
	XMFLOAT4 spotLightColor;
	XMFLOAT3 spotLightDir;
	float spotDegree;
};

struct foliageInstanceType
{
	XMMATRIX matrix;
	XMFLOAT3 color;
};

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;

	camera* m_camera;

	//obj loader
	std::vector< ObjLoader*> m_objLoader;

	//obj use geo
	std::vector< ObjLoader*> m_objLoaderGeo;
	mirror* m_mirror;

	//foliage
	foliage* m_foliage;
	//enum for rasterizs state
	enum rasterizeState{mass_on_fill,mass_off_fill,mass_on_wire,mass_off_wire};

	rasterizeState m_rasterizeState = mass_on_fill;

	enum tessellState{ tessell_on, tessell_off };
	tessellState m_tessell_state = tessell_on;
public:
	
	renderToTextureResource* m_renderToTexture;

	camera* getCamera()
	{
		return m_camera;
	}
	ID3D11DeviceContext * m_ID3D11DeviceContext;// the pointer to our Direct3D device context
	ID3D11Device *m_d3d11Device;      // the pointer to our Direct3D device interface
	IDXGISwapChain* m_IDXGISwapChain; // the pointer to the swap chain interface
	ID3D11RenderTargetView *m_RenderTargetView[2];

	//************lotus ************
	ID3D11Buffer *m_ID3D11Buffer_lotus;
	ID3D11Buffer *m_ID3D11_index_Buffer_lotus;
	SIMPLE_VERTEX m_lotus_VERTEX[lotusNumber];
	ID3D11ShaderResourceView* m_lotusSRV;

	//************skybox ************
	ID3D11Buffer *m_ID3D11Buffer_skybox;
	SIMPLE_VERTEX m_skybox_VERTEX[8];
	ID3D11Buffer *m_ID3D11_index_Buffer_skybox;
	ID3D11ShaderResourceView* m_skybox;
	//************Light Pos buffer************
	ID3D11Buffer*m_Buffer_LightPos;
	LightPosData m_LightPosData;

	//************Light Color*****************
	ID3D11Buffer*m_Buffer_LightColor;
	LightColorData m_lightColorData;

	ID3D11Buffer *m_ID3D11Buffer;
	ID3D11Buffer *m_ID3D11Buffer_cube;
	ID3D11Buffer *m_ID3D11Buffer_line;

	//***********rain***************
	ID3D11Buffer *m_ID3D11Buffer_rain = nullptr;
	SIMPLE_VERTEX m_rain_VERTEX[rainNumber];

	//*************************
	SIMPLE_VERTEX m_CIRCLE_VERTEX[360];
	SIMPLE_VERTEX m_STAR_VERTEX[starNumber];
	SIMPLE_VERTEX m_CUBE_VERTEX[cubeNumber];
	SIMPLE_VERTEX m_Grid_VERTEX[gridNumber];
	SIMPLE_VERTEX m_backMirror_VERTEX[rectNumber];


	unsigned short m_indices_cube[cubeNumber];

	ID3D11Buffer *m_ID3D11_index_Buffer;
	ID3D11Buffer *m_ID3D11_index_Buffer_cube;
	ID3D11Buffer *m_ID3D11_index_Buffer_line;
	ID3D11InputLayout *pLayout;
	ID3D11InputLayout *pLayout5;


	ID3D11InputLayout *pLayout_skybox;
	ID3D11InputLayout *pLayout_Instnace;
	//depth buffer
	ID3D11Texture2D* m_pDepthStencil;
	ID3D11DepthStencilView* m_ID3D11DepthStencilView;

	// the vertex shader
	// the pixel shader
	ID3D11VertexShader *pVS;
	ID3D11PixelShader *pPS;

	//ground
	ID3D11PixelShader *groundPS;
	ID3D11VertexShader *groundVS;

	ID3D11VertexShader *pVS_Instance;
	ID3D11PixelShader *pPS_Instance;

	ID3D11VertexShader *pVS_skybox;
	ID3D11PixelShader *pPS_skybox;

	//********geometry shader
	ID3D11GeometryShader* pGeoShader ;
	ID3D11Buffer *m_ID3D11_gs_Buffer ;
	ID3D11VertexShader *pVS_Geo;
	ID3D11PixelShader *pPS_Geo;
	ID3D11InputLayout *pLayout_geo;

	//********foliage shader
	ID3D11VertexShader *pfoliage_VS;
	ID3D11PixelShader *pfoliage_PS;
	ID3D11InputLayout *pLayoutFoliage;
	//*******tessellaction state**********
	ID3D11VertexShader *pTes_VS;
	ID3D11HullShader *pTes_HS;
	ID3D11DomainShader *pTes_DS;
	ID3D11PixelShader *pTes_PS;
	ID3D11Buffer *m_ID3D11_Tes_Buffer;
	TessellationConstantBuffer m_TessellationConstantBuffer;

	//*******tessellaction state**********
	ID3D11ComputeShader* p_ComputeS;

	//******Matrix**********
	MatrixCalculate m_MatrixCalculate;
	MatrixCalculate m_MatrixSkyBox;
	MatrixCalculate m_MatrixRain;
	MatrixCalculate m_MatrixMirror;
	
	ID3D11Buffer *m_ID3D11MatrixBuffer_OBJECT;
	ID3D11Buffer *m_ID3D11MatrixBuffer_skyBox;
	ID3D11Buffer *m_ID3D11MatrixBuffer_SCENE;
	ID3D11Buffer *m_ID3D11MatrixBuffer_rain;
	ID3D11Buffer *m_ID3D11MatrixBuffer_mirror;

	XTime m_Xtime[2];

	/////////////////////////texture/////////////////////////
	ID3D11SamplerState* m_sampleState;
	ID3D11ShaderResourceView* m_ID3D11ShaderResourceView;

	ID3D11ShaderResourceView* m_groundSRV;
	ID3D11Resource* m_ID3D11Resource;
	ID3D11Texture2D* m_ID3D11Texture2D_Texture;


	//three state
	ID3D11RasterizerState* m_ID3D11_RS[rasterizeNumber];
	

	//blend state
	ID3D11BlendState * m_ID3D11BlendState_on;
	ID3D11BlendState * m_ID3D11BlendState_off;
	ID3D11BlendState * m_ID3D11BlendState_alphaCoverOn;
	ID3D11BlendState * m_ID3D11BlendState_alphaCoverOff;

	//view port
	D3D11_VIEWPORT m_D3D11_VIEWPORT[3];

	//********foliage
	foliageInstanceType m_foliageInstanceType;

public:
	void setSecondTexture(const wchar_t* _fileName, ID3D11ShaderResourceView*& _shaderRSV);

	void setMatrix(ID3D11Buffer *&m_ID3D11MatrixBuffer_OBJECT, MatrixCalculate &m_MatrixCalculate);


	int	numberCounter = 0;

	void setMeshData(const void* _vertex_data, const int& _vertexNumber, const void* _indices_data, const int& _indexNumber,
		const wchar_t* _fileName, ID3D11ShaderResourceView*& _shaderRSV, ID3D11Buffer*& _dataBuffer, ID3D11Buffer*& _indexBuffer);

	void renderDataInstance(ID3D11VertexShader* _vsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
		ID3D11ShaderResourceView*const _shaderRSV, UINT _vertexNumber, UINT _instanceNumber);

	void renderDataInstanceIndex(ID3D11VertexShader* _vsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input, ID3D11Buffer *const _indexBuffer,
		ID3D11ShaderResourceView**const _shaderRSV, UINT _vertexNumber, UINT _instanceNumber);

	void renderData(ID3D11VertexShader* _vsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
		ID3D11Buffer * const _dataBuffer, ID3D11Buffer *const _indexBuffer, ID3D11ShaderResourceView**const _shaderRSV, UINT _indexNumber);

	void renderDataSingleTexture(ID3D11VertexShader* _vsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
		ID3D11Buffer * const _dataBuffer, ID3D11Buffer *const _indexBuffer, ID3D11ShaderResourceView*const _shaderRSV, UINT _indexNumber);

	void  renderDataMultiTexture(ID3D11VertexShader* _vsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
		ID3D11Buffer * const _dataBuffer, ID3D11Buffer *const _indexBuffer, ID3D11ShaderResourceView**const _shaderRSV, UINT _indexNumber, UINT _textureNumber);

	void renderDataGeoShader(ID3D11VertexShader* _vsShader, ID3D11GeometryShader* _gsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
		ID3D11Buffer * const _dataBuffer, ID3D11ShaderResourceView*const _shaderRSV, UINT vertexNumber);

	void renderDataTessShader(ID3D11VertexShader* _vsShader, ID3D11HullShader* _hsShader, ID3D11DomainShader* _dsShader,
		ID3D11GeometryShader* _gsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
		ID3D11Buffer * const _dataBuffer, ID3D11Buffer *const _indexBuffer, ID3D11ShaderResourceView**const _shaderRSV, UINT _indexNumber);

	void setVertexInputBuffer(const void* _vertex_data, const int& _vertexNumber, ID3D11Buffer*& _dataBuffer);

	DEMO_APP(HINSTANCE hinst, WNDPROC proc);

	void godCamera();
	bool Run();
	bool ShutDown();
	void SetBlendState();

	void turnOnAlphaValue();

	void turnOffAlphaValue();
private:
	void setRasterizationState();
	//shader buffer
	void setShaderBuffer();

	void setConstatBuffer();

	//use render to texture
	void renderToTexture(renderToTextureResource* m_renderToTexture);

	//set to normal setrendertarget
	void setbackBufferRenderTarget();

	//set init, clear function
	void beginScene(ID3D11RenderTargetView* _ID3D11RenderTargetView);

	//render model data
	void renderAll();

	//render mirror
	void renderMirror();

	void normalCamera();

	void normalCameraYAxixReverse(char _char);

	
};


void DEMO_APP::turnOffAlphaValue()
{
	float blendFactor[4];

	// Setup the blend factor.
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;

	// Turn off the alpha blending.
	m_ID3D11DeviceContext->OMSetBlendState(m_ID3D11BlendState_alphaCoverOff, blendFactor, 0xffffffff);

	return;
}
void DEMO_APP::turnOnAlphaValue()
{
	float blendFactor[4];

	// Setup the blend factor.
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;

	// Turn on the alpha blending.
	m_ID3D11DeviceContext->OMSetBlendState(m_ID3D11BlendState_alphaCoverOn, blendFactor, 0xffffffff);

	return;
}
//****************RasterizerState	init ****************
void DEMO_APP::setRasterizationState()
{
	//[0] cull front  +  wire frame + msaa enable
	//[1] cull front  +  wire frame + msaa disable 
	//[2] cull front  +  fill		+ msaa enable
	//[3] cull front  +  fill		+ msaa disable 
	//[4] cull back   +  wire frame + msaa enable
	//[5] cull back   +  wire frame + msaa disable 
	//[6] cull back   +  fill		+ msaa enable
	//[7] cull back   +  fill		+ msaa disable 
//	m_ID3D11_RS = new ID3D11RasterizerState[6];

	for (int i = 0; i < rasterizeNumber; i++)
	{

		D3D11_RASTERIZER_DESC m_D3D11_RASTERIZER_DESC;
		ZeroMemory(&m_D3D11_RASTERIZER_DESC, sizeof(D3D11_RASTERIZER_DESC));

		if (i == 0 || i == 1 || i == 2 || i == 3)
			m_D3D11_RASTERIZER_DESC.CullMode = D3D11_CULL_FRONT;
		else
			m_D3D11_RASTERIZER_DESC.CullMode = D3D11_CULL_BACK;

		if (i == 0 || i == 1 || i == 4 || i == 5)
			m_D3D11_RASTERIZER_DESC.FillMode = D3D11_FILL_WIREFRAME;
		else
			m_D3D11_RASTERIZER_DESC.FillMode = D3D11_FILL_SOLID;
		
		if (i == 0 || i == 2 || i == 4 || i == 6)
			m_D3D11_RASTERIZER_DESC.AntialiasedLineEnable = true;
		else
			m_D3D11_RASTERIZER_DESC.AntialiasedLineEnable = false;

		m_D3D11_RASTERIZER_DESC.DepthBias = 0;
		m_D3D11_RASTERIZER_DESC.FrontCounterClockwise = false;
		m_D3D11_RASTERIZER_DESC.DepthClipEnable = true;
		m_D3D11_RASTERIZER_DESC.ScissorEnable = false;
		m_D3D11_RASTERIZER_DESC.MultisampleEnable = false;

		m_d3d11Device->CreateRasterizerState(&m_D3D11_RASTERIZER_DESC, &m_ID3D11_RS[i]);
		
	}
	m_ID3D11DeviceContext->RSSetState(m_ID3D11_RS[6]);
}
void DEMO_APP::beginScene(ID3D11RenderTargetView* _ID3D11RenderTargetView)
{
	float m_float[4];
	//rgba
	m_float[0] = 0.0f;
	m_float[1] = 0.0f;
	m_float[2] = 0.0f;
	m_float[3] = 1.0f;

	//render clear first
	m_ID3D11DeviceContext->ClearDepthStencilView(m_ID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1, 0);

	m_ID3D11DeviceContext->ClearRenderTargetView(_ID3D11RenderTargetView, m_float);
}
void DEMO_APP::renderToTexture(renderToTextureResource* m_renderToTexture)
{
	//use m_renderToTexture class rendertarget to be the render to texture
	m_renderToTexture->setRenderTarget(m_ID3D11DeviceContext, m_ID3D11DepthStencilView);

	XMFLOAT4 _background = XMFLOAT4(0, 0, 0, 1);

	//clear the render to texture background
	m_renderToTexture->clearRenderTarget(m_ID3D11DeviceContext, m_ID3D11DepthStencilView, &_background);
}

void DEMO_APP::setbackBufferRenderTarget()
{
	// set the render target as the m_RenderTargetView
	m_ID3D11DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView[0], m_ID3D11DepthStencilView);

}
//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************
void DEMO_APP::setVertexInputBuffer(const void* _vertex_data, const int& _vertexNumber, ID3D11Buffer*& _dataBuffer)
{
	//D3D11_SHADER_RESOURCE_VIEW_DESC x;

	//*************************data************************
	D3D11_BUFFER_DESC m_D3D11_BUFFER_DESC_line;
	D3D11_SUBRESOURCE_DATA m_D3D11_SUBRESOURCE_DATA_line;
	ZeroMemory(&m_D3D11_SUBRESOURCE_DATA_line, sizeof(D3D11_SUBRESOURCE_DATA));
	ZeroMemory(&m_D3D11_BUFFER_DESC_line, sizeof(D3D11_BUFFER_DESC));

	m_D3D11_SUBRESOURCE_DATA_line.pSysMem = _vertex_data;
	m_D3D11_BUFFER_DESC_line.Usage = D3D11_USAGE_IMMUTABLE;                // write access access by CPU and GPU
	m_D3D11_BUFFER_DESC_line.ByteWidth = sizeof(SIMPLE_VERTEX)* _vertexNumber;             // size is the VERTEX struct * 3
	m_D3D11_BUFFER_DESC_line.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	m_D3D11_BUFFER_DESC_line.CPUAccessFlags = NULL;
	m_d3d11Device->CreateBuffer(&m_D3D11_BUFFER_DESC_line, &m_D3D11_SUBRESOURCE_DATA_line, &_dataBuffer);       // create the buffer
}

void DEMO_APP::setMeshData(const void* _vertex_data, const int& _vertexNumber, const void* _indices_data, const int& _indexNumber,
	const wchar_t* _fileName, ID3D11ShaderResourceView*& _shaderRSV, ID3D11Buffer*& _dataBuffer, ID3D11Buffer*& _indexBuffer)
{
	//D3D11_SHADER_RESOURCE_VIEW_DESC x;

	//*************************data************************
	if (_vertexNumber != 0)
	{
		D3D11_BUFFER_DESC m_D3D11_BUFFER_DESC_line;
		D3D11_SUBRESOURCE_DATA m_D3D11_SUBRESOURCE_DATA_line;
		ZeroMemory(&m_D3D11_SUBRESOURCE_DATA_line, sizeof(D3D11_SUBRESOURCE_DATA));
		ZeroMemory(&m_D3D11_BUFFER_DESC_line, sizeof(D3D11_BUFFER_DESC));
		m_D3D11_SUBRESOURCE_DATA_line.pSysMem = _vertex_data;
		m_D3D11_BUFFER_DESC_line.Usage = D3D11_USAGE_IMMUTABLE;                // write access access by CPU and GPU
		m_D3D11_BUFFER_DESC_line.ByteWidth = sizeof(SIMPLE_VERTEX)* _vertexNumber;             // size is the VERTEX struct * 3
		m_D3D11_BUFFER_DESC_line.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
		m_D3D11_BUFFER_DESC_line.CPUAccessFlags = NULL;
		m_d3d11Device->CreateBuffer(&m_D3D11_BUFFER_DESC_line, &m_D3D11_SUBRESOURCE_DATA_line, &_dataBuffer);       // create the buffer
	}
	//************************* m_indices************************
	if (_indexNumber != 0)
	{
		D3D11_BUFFER_DESC m_D3D11_i_BUFFER_DESC_line;
		ZeroMemory(&m_D3D11_i_BUFFER_DESC_line, sizeof(D3D11_BUFFER_DESC));
		D3D11_SUBRESOURCE_DATA m_D3D11_i_SUBRESOURCE_DATA_line;
		ZeroMemory(&m_D3D11_i_SUBRESOURCE_DATA_line, sizeof(D3D11_SUBRESOURCE_DATA));

		m_D3D11_i_SUBRESOURCE_DATA_line.pSysMem = _indices_data;
		m_D3D11_i_BUFFER_DESC_line.Usage = D3D11_USAGE_IMMUTABLE;                // write access access by CPU and GPU
		m_D3D11_i_BUFFER_DESC_line.ByteWidth = sizeof(unsigned int) * _indexNumber;
		m_D3D11_i_BUFFER_DESC_line.BindFlags = D3D11_BIND_INDEX_BUFFER;       // use as a vertex buffer
		m_D3D11_i_BUFFER_DESC_line.CPUAccessFlags = NULL;
		m_d3d11Device->CreateBuffer(&m_D3D11_i_BUFFER_DESC_line, &m_D3D11_i_SUBRESOURCE_DATA_line, &_indexBuffer);       // create the buffer
	}

	//*************************texture ************************
	CreateDDSTextureFromFile(m_d3d11Device, _fileName, NULL, &_shaderRSV);
}
void DEMO_APP::setSecondTexture(const wchar_t* _fileName, ID3D11ShaderResourceView*& _shaderRSV)
{
	CreateDDSTextureFromFile(m_d3d11Device, _fileName, NULL, &_shaderRSV);
}
void DEMO_APP::setShaderBuffer()
{
	//element
	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_INPUT_ELEMENT_DESC vLayout2[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	D3D11_INPUT_ELEMENT_DESC vLayout5[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_INPUT_ELEMENT_DESC vLayout7[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 2, DXGI_FORMAT_R32G32B32xA32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	HRESULT hr;
	//*************************skybox************************
	hr = m_d3d11Device->CreateInputLayout(vLayout, 3, MyVertexShader, sizeof(MyVertexShader), &pLayout_skybox);
	hr = m_d3d11Device->CreateVertexShader(MyVertexShader, sizeof(MyVertexShader), NULL, &pVS_skybox);
	hr = m_d3d11Device->CreatePixelShader(MyPixelShader, sizeof(MyPixelShader), NULL, &pPS_skybox);

	//***************** general*****************
	hr = m_d3d11Device->CreateInputLayout(vLayout, 3, Trivial_VS, sizeof(Trivial_VS), &pLayout);
	hr = m_d3d11Device->CreateVertexShader(Trivial_VS, sizeof(Trivial_VS), NULL, &pVS);
	hr = m_d3d11Device->CreatePixelShader(Trivial_PS, sizeof(Trivial_PS), NULL, &pPS);

	//***************** instance*****************
	hr = m_d3d11Device->CreateInputLayout(vLayout2, 4, InstanceVS, sizeof(InstanceVS), &pLayout_Instnace);
	hr = m_d3d11Device->CreateVertexShader(InstanceVS, sizeof(InstanceVS), NULL, &pVS_Instance);
	hr = m_d3d11Device->CreatePixelShader(InstancePS, sizeof(InstancePS), NULL, &pPS_Instance);

	//****************geo shader****************************************************************
	hr = m_d3d11Device->CreateInputLayout(vLayout, 3, geoVS, sizeof(geoVS), &pLayout_geo);
	hr = m_d3d11Device->CreateVertexShader(geoVS, sizeof(geoVS), NULL, &pVS_Geo);
	hr = m_d3d11Device->CreateGeometryShader(geoShader, sizeof(geoShader), NULL, &pGeoShader);
	hr = m_d3d11Device->CreatePixelShader(geoPS, sizeof(geoPS), NULL, &pPS_Geo);

	//****************Tessellaction state****************************************************************
	//hr = m_d3d11Device->CreateInputLayout(vLayout5, 5, Tes_VS, sizeof(Tes_VS), &pLayout5);
	hr = m_d3d11Device->CreateVertexShader(Tes_VS, sizeof(Tes_VS), NULL, &pTes_VS);
	hr = m_d3d11Device->CreateHullShader(Tes_HS, sizeof(Tes_HS), NULL, &pTes_HS);
	hr = m_d3d11Device->CreateDomainShader(Tes_DS, sizeof(Tes_DS), NULL, &pTes_DS);
	hr = m_d3d11Device->CreatePixelShader(Tes_PS, sizeof(Tes_PS), NULL, &pTes_PS);

	//****************ground ****************************************************************
	hr = m_d3d11Device->CreateInputLayout(vLayout5, 5, ground_VS, sizeof(ground_VS), &pLayout5);
	hr = m_d3d11Device->CreatePixelShader(ground_PS, sizeof(ground_PS), NULL, &groundPS);
	hr = m_d3d11Device->CreateVertexShader(ground_VS, sizeof(ground_VS), NULL, &groundVS);

	//****************foliage ****************************************************************
	hr = m_d3d11Device->CreateInputLayout(vLayout7, 7, foliage_VS, sizeof(foliage_VS), &pLayoutFoliage);
	hr = m_d3d11Device->CreatePixelShader(foliage_PS, sizeof(foliage_PS), NULL, &pfoliage_PS);
	hr = m_d3d11Device->CreateVertexShader(foliage_VS, sizeof(foliage_VS), NULL, &pfoliage_VS);
}

void DEMO_APP::setConstatBuffer()
{
	HRESULT hr;
	//**********************************************************************************************
	//***********camera light position structure*************
	D3D11_BUFFER_DESC m_D3D11_BUFFER_DESC_LightPos;
	D3D11_SUBRESOURCE_DATA m_D3D11_SUBRESOURCE_DATA_LightPos;
	ZeroMemory(&m_D3D11_BUFFER_DESC_LightPos, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&m_D3D11_SUBRESOURCE_DATA_LightPos, sizeof(D3D11_SUBRESOURCE_DATA));


	m_D3D11_SUBRESOURCE_DATA_LightPos.pSysMem = &m_LightPosData;
	m_D3D11_BUFFER_DESC_LightPos.Usage = D3D11_USAGE_DYNAMIC;
	m_D3D11_BUFFER_DESC_LightPos.ByteWidth = sizeof(m_LightPosData);             // size is the toShader
	m_D3D11_BUFFER_DESC_LightPos.BindFlags = D3D11_BIND_CONSTANT_BUFFER;       // use as a vertex buffer
	m_D3D11_BUFFER_DESC_LightPos.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// create the light buffer
	hr = m_d3d11Device->CreateBuffer(&m_D3D11_BUFFER_DESC_LightPos, &m_D3D11_SUBRESOURCE_DATA_LightPos, &m_Buffer_LightPos);

	m_LightPosData.cameraPos = { 0.0f, 0.0f, 1.0f };
	m_LightPosData.pad = 0;
	m_LightPosData.pointLightPos = { 3.9f, 13.6f, 22.6f };
	m_LightPosData.pointLightNumber = 1;

	m_LightPosData.spotLightPos = { 0.0f, 1.0f, 0.0f };
	m_LightPosData.pad2 = 0;

	//*********light color structure************************
	D3D11_BUFFER_DESC m_D3D11_BUFFER_DESC_Light;
	D3D11_SUBRESOURCE_DATA m_D3D11_SUBRESOURCE_DATA_Light;
	ZeroMemory(&m_D3D11_BUFFER_DESC_Light, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&m_D3D11_SUBRESOURCE_DATA_Light, sizeof(D3D11_SUBRESOURCE_DATA));

	m_D3D11_SUBRESOURCE_DATA_Light.pSysMem = &m_lightColorData;
	m_D3D11_BUFFER_DESC_Light.Usage = D3D11_USAGE_DYNAMIC;
	m_D3D11_BUFFER_DESC_Light.ByteWidth = sizeof(m_lightColorData);             // size is the toShader
	m_D3D11_BUFFER_DESC_Light.BindFlags = D3D11_BIND_CONSTANT_BUFFER;       // use as a vertex buffer
	m_D3D11_BUFFER_DESC_Light.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// create the light buffer
	hr = m_d3d11Device->CreateBuffer(&m_D3D11_BUFFER_DESC_Light, &m_D3D11_SUBRESOURCE_DATA_Light, &m_Buffer_LightColor);
	//	m_lightColorData.ambientColor = { 0.3f, 0.3f, 0.3f, 1.0f };
	m_lightColorData.ambientColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_lightColorData.diffuseColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	m_lightColorData.lightDirectionSource = { 0.0f, -1.0f, -1.0f };
	m_lightColorData.specualrPower = 32.0f;
	m_lightColorData.specularColor = { 1.0f, 1.0f, 1.0f, 1.0f };
//	m_lightColorData.pointLightColor = { 0.0f, 1.0f, 0.0f, 1.0f };
	m_lightColorData.pointLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_lightColorData.spotLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_lightColorData.spotLightDir = { 0.0f, 0.0f, 1.0f };

	m_lightColorData.spotDegree = cosf(spotLightDegree / 180.0f*3.14f / 2);

	setMatrix(m_ID3D11MatrixBuffer_OBJECT, m_MatrixCalculate);
	setMatrix(m_ID3D11MatrixBuffer_skyBox, m_MatrixSkyBox);
	setMatrix(m_ID3D11MatrixBuffer_rain, m_MatrixRain);
	setMatrix(m_ID3D11MatrixBuffer_mirror, m_MatrixMirror);

	m_MatrixCalculate.setProject(0.1f, 100, 95, 0.5f);
	m_MatrixSkyBox.setProject(0.1f, 100, 95, 0.5f);
	m_MatrixRain.setProject(0.1f, 100, 95, 0.5f);

	//*********tessellaction data***********************
	D3D11_BUFFER_DESC m_D3D11_BUFFER_DESC_tessellaction;
	D3D11_SUBRESOURCE_DATA m_D3D11_SUBRESOURCE_DATA_tess;
	ZeroMemory(&m_D3D11_BUFFER_DESC_tessellaction, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&m_D3D11_SUBRESOURCE_DATA_tess, sizeof(D3D11_SUBRESOURCE_DATA));

	m_D3D11_SUBRESOURCE_DATA_tess.pSysMem = &m_TessellationConstantBuffer;
	m_D3D11_BUFFER_DESC_tessellaction.Usage = D3D11_USAGE_DYNAMIC;
	m_D3D11_BUFFER_DESC_tessellaction.ByteWidth = sizeof(m_lightColorData);             // size is the toShader
	m_D3D11_BUFFER_DESC_tessellaction.BindFlags = D3D11_BIND_CONSTANT_BUFFER;       // use as a vertex buffer
	m_D3D11_BUFFER_DESC_tessellaction.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_d3d11Device->CreateBuffer(&m_D3D11_BUFFER_DESC_tessellaction, &m_D3D11_SUBRESOURCE_DATA_tess, &m_ID3D11_Tes_Buffer);

	m_TessellationConstantBuffer.tessellactionAmout = 4.0f;
	m_TessellationConstantBuffer.padding = XMFLOAT3(0, 0, 0);

	
}

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{

	// ****************** BEGIN WARNING *****************3******// 
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY! 
	application = hinst;
	appWndProc = proc;

	WNDCLASSEX  wndClass;

	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	/*window = CreateWindow(	L"DirectXApplication", L"Lab 1a Line Land",	WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME|WS_MAXIMIZEBOX),
	CW_USEDEFAULT, CW_USEDEFAULT, window_size.right-window_size.left, window_size.bottom-window_size.top,
	NULL, NULL,	application, this );	*/
	window = CreateWindow(L"DirectXApplication", L"Graphic Lab2 ", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
	//********************* END WARNING ************************//

	//full screen
	DXGI_SWAP_CHAIN_DESC m_SCD;
	ZeroMemory(&m_SCD, sizeof(DXGI_SWAP_CHAIN_DESC));
	// fill the swap chain description struct
	m_SCD.BufferCount = 1;                                    // only need 1 back buffer
	m_SCD.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bits color
	m_SCD.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	m_SCD.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	m_SCD.OutputWindow = window;                                // the window to be used
	m_SCD.SampleDesc.Count = 1;                               // how many multisamples
	m_SCD.Windowed = TRUE;                                    // windowed mode
	// create a device, device context and swap chain using the information in the scd struct

	HRESULT hr;
#if(_DEBUG)
	UINT flagDebug = D3D11_CREATE_DEVICE_DEBUG;
#else if
	UINT flagDebug = 0;
#endif
	hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		flagDebug,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&m_SCD,
		&m_IDXGISwapChain,
		&m_d3d11Device,
		NULL,
		&m_ID3D11DeviceContext);

	//swap chain with DXGI_SWAP_CHAIN_DESC
	m_IDXGISwapChain->GetDesc(&m_SCD);

	// get the address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	m_IDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	// use them_RenderTargetView address to create the render target
	hr = m_d3d11Device->CreateRenderTargetView(pBackBuffer, NULL, &m_RenderTargetView[0]);
	hr = m_d3d11Device->CreateRenderTargetView(pBackBuffer, NULL, &m_RenderTargetView[1]);

	pBackBuffer->Release();

	// Set the viewport
	//half up
	ZeroMemory(&m_D3D11_VIEWPORT[0], sizeof(D3D11_VIEWPORT));
	m_D3D11_VIEWPORT[0].TopLeftX = 0;
	m_D3D11_VIEWPORT[0].TopLeftY = 0;
	m_D3D11_VIEWPORT[0].MinDepth = 0;
	m_D3D11_VIEWPORT[0].MaxDepth = 1;
	//m_IDXGISwapChain->GetDesc(&m_SCD);
	m_D3D11_VIEWPORT[0].Height = m_SCD.BufferDesc.Height/2;
	m_D3D11_VIEWPORT[0].Width = m_SCD.BufferDesc.Width;
//	m_ID3D11DeviceContext->RSSetViewports(1, &m_D3D11_VIEWPORT);

	//half down
	ZeroMemory(&m_D3D11_VIEWPORT[1], sizeof(D3D11_VIEWPORT));
	m_D3D11_VIEWPORT[1].TopLeftX = 0;
	m_D3D11_VIEWPORT[1].TopLeftY = m_SCD.BufferDesc.Height / 2;
	m_D3D11_VIEWPORT[1].MinDepth = 0;
	m_D3D11_VIEWPORT[1].MaxDepth = 1;
	m_D3D11_VIEWPORT[1].Height = m_SCD.BufferDesc.Height / 2;
	m_D3D11_VIEWPORT[1].Width = m_SCD.BufferDesc.Width;

	//all
	ZeroMemory(&m_D3D11_VIEWPORT[2], sizeof(D3D11_VIEWPORT));
	m_D3D11_VIEWPORT[2].TopLeftX = 0;
	m_D3D11_VIEWPORT[2].TopLeftY = 0;
	m_D3D11_VIEWPORT[2].MinDepth = 0;
	m_D3D11_VIEWPORT[2].MaxDepth = 1;
	m_D3D11_VIEWPORT[2].Height = m_SCD.BufferDesc.Height;
	m_D3D11_VIEWPORT[2].Width = m_SCD.BufferDesc.Width;



	
	//*************************ground************************
	float spacef = 100.0;
	float standY = -0.00f;
	float uv = 50.0f;
	for (int i = 0; i < 4; i++)
	{
		m_Grid_VERTEX[i].nrm.x = 0.0f;
		m_Grid_VERTEX[i].nrm.y = 1.0f;
		m_Grid_VERTEX[i].nrm.z = 0.0f;
	}
	//top left
	m_Grid_VERTEX[0].xyz.x = -spacef;
	m_Grid_VERTEX[0].xyz.y = standY;
	m_Grid_VERTEX[0].xyz.z = spacef;
	m_Grid_VERTEX[0].uvw.x = 0.0f;
	m_Grid_VERTEX[0].uvw.y = 0.0f;
	m_Grid_VERTEX[0].uvw.z = 0.0f;

	//top right
	m_Grid_VERTEX[1].xyz.x = spacef;
	m_Grid_VERTEX[1].xyz.y = standY;
	m_Grid_VERTEX[1].xyz.z = spacef;
	m_Grid_VERTEX[1].uvw.x = uv;
	m_Grid_VERTEX[1].uvw.y = 0.0f;
	m_Grid_VERTEX[1].uvw.z = 0.0f;

	//bot left
	m_Grid_VERTEX[2].xyz.x = -spacef;
	m_Grid_VERTEX[2].xyz.y = standY;
	m_Grid_VERTEX[2].xyz.z = -spacef;
	m_Grid_VERTEX[2].uvw.x = 0.0f;
	m_Grid_VERTEX[2].uvw.y = uv;
	m_Grid_VERTEX[2].uvw.z = 0.0f;

	//bot right
	m_Grid_VERTEX[3].xyz.x = spacef;
	m_Grid_VERTEX[3].xyz.y = standY;
	m_Grid_VERTEX[3].xyz.z = -spacef;
	m_Grid_VERTEX[3].uvw.x = uv;
	m_Grid_VERTEX[3].uvw.y = uv;
	m_Grid_VERTEX[3].uvw.z = 0.0f;

	unsigned int Line_indicies[6] = { 0, 3, 2, 0, 1, 3 };
//	setMeshData(m_Grid_VERTEX, gridNumber, Line_indicies, 6, L"../Asset/DDS/bleachedWood_seamless.dds",
//		m_groundSRV, m_ID3D11Buffer_line, m_ID3D11_index_Buffer_line);
//	setMeshData(m_Grid_VERTEX, gridNumber, Line_indicies, 6, L"../Asset/DDS/stone.dds",
	//	m_groundSRV[0], m_ID3D11Buffer_line, m_ID3D11_index_Buffer_line);
	
	//setSecondTexture(L"../Asset/DDS/dirt.dds", m_groundSRV[1]);
	//******************rain data
	int base = 1000;
	float divide = 50.0f;
	float shift = 10.0f;
	for (int i = 0; i < rainNumber; i++)
	{
		m_rain_VERTEX[i].xyz.x = rand() %base /divide-shift;
		m_rain_VERTEX[i].xyz.y = rand() %base /divide-shift;
		m_rain_VERTEX[i].xyz.z = rand() %base /divide-shift;
		m_rain_VERTEX[i].uvw.x = rand() %base /divide-shift;
		m_rain_VERTEX[i].uvw.y = rand() %base /divide-shift;
		m_rain_VERTEX[i].uvw.z = rand() %base /divide-shift;
		m_rain_VERTEX[i].nrm.x = rand() % base / divide-shift;
		m_rain_VERTEX[i].nrm.y = rand() % base / divide-shift;
		m_rain_VERTEX[i].nrm.z = rand() % base / divide-shift;
	}

	setVertexInputBuffer(m_rain_VERTEX, rainNumber, m_ID3D11Buffer_rain);
	//*************************lotus people************************
	//obj[0]
	//obj load hear
	ObjLoader* tempObjLoader = new ObjLoader(m_ID3D11DeviceContext);
	m_objLoader.push_back(tempObjLoader);
	tempObjLoader->LoadFromFile("../Asset/lotus/lotus.obj", L"../Asset/DDS/lotus.dds", NULL, NULL, m_d3d11Device);
	tempObjLoader->setMeshNotInstance();
	//*************************car************************
	//obj[1]
	tempObjLoader = new ObjLoader(m_ID3D11DeviceContext);
	m_objLoader.push_back(tempObjLoader);
	tempObjLoader->LoadFromFile("../Asset/car/car.obj", L"../Asset/DDS/car.dds", NULL, NULL, m_d3d11Device);
	tempObjLoader->setMeshInstance();
	//*************************house************************
	//obj[2]
	tempObjLoader = new ObjLoader(m_ID3D11DeviceContext);
	m_objLoader.push_back(tempObjLoader);
	tempObjLoader->LoadFromFile("../Asset/house/house.obj", L"../Asset/DDS/house.dds", NULL, NULL, m_d3d11Device);
	tempObjLoader->setMeshInstance();

	//*************************helicopter************************
	//obj[2]
	tempObjLoader = new ObjLoader(m_ID3D11DeviceContext);
	m_objLoader.push_back(tempObjLoader);
	tempObjLoader->LoadFromFile("../Asset/helicopter/helicopter.obj", L"../Asset/DDS/helicopter.dds", NULL, NULL, m_d3d11Device);
	tempObjLoader->setMeshInstance();

	//*************************ambulance************************
	//obj[3]
	tempObjLoader = new ObjLoader(m_ID3D11DeviceContext);
	m_objLoader.push_back(tempObjLoader);
	tempObjLoader->LoadFromFile("../Asset/ambulance/ambulance.obj", L"../Asset/DDS/ambulance.dds", NULL, L"../Asset/DDS/ambulance_Normal.dds", m_d3d11Device);
	tempObjLoader->setMeshInstance();
	//*************************streetlight************************
	//obj[４]
	tempObjLoader = new ObjLoader(m_ID3D11DeviceContext);
	m_objLoader.push_back(tempObjLoader);
	tempObjLoader->LoadFromFile("../Asset/streetlight/streetlight.obj", L"../Asset/DDS/streetlight.dds", NULL, NULL, m_d3d11Device);
	tempObjLoader->setMeshInstance();

	//obj[5]
	tempObjLoader = new ObjLoader(m_ID3D11DeviceContext);
	m_objLoader.push_back(tempObjLoader);
	tempObjLoader->LoadFromFile("../Asset/ground/ground.obj", L"../Asset/DDS/stone2.dds", L"../Asset/DDS/grass.dds", L"../Asset/DDS/stone2_Normal.dds", m_d3d11Device);
	tempObjLoader->setMeshInstance();


	//*************************ground debug************************
	//obj[5]
	tempObjLoader = new ObjLoader(m_ID3D11DeviceContext);
	m_objLoader.push_back(tempObjLoader);
	tempObjLoader->LoadFromFile("../Asset/ground/ground2.obj", L"../Asset/DDS/stone2.dds", L"../Asset/DDS/grass.dds", L"../Asset/DDS/stone2_Normal.dds", m_d3d11Device);
	tempObjLoader->setMeshInstance();

	//*************************ground************************
	

	//*************************USE GEO SHADER　ＢＥＬＯＷ************************
	//*************************USE GEO SHADER　ＢＥＬＯＷ************************
	//*************************USE GEO SHADER　ＢＥＬＯＷ************************

	//*************************ball ( triangle pillow later)************************
	//objGeo[0]
	tempObjLoader = new ObjLoader(m_ID3D11DeviceContext);
	m_objLoaderGeo.push_back(tempObjLoader);
	tempObjLoader->LoadFromFile("../Asset/ball/ball.obj", L"../Asset/DDS/numbers_test.dds", NULL, NULL, m_d3d11Device);
	tempObjLoader->setMeshInstance();

	//*************************tree geo shader use************************
	//objGeo[1]
	tempObjLoader = new ObjLoader(m_ID3D11DeviceContext);
	m_objLoaderGeo.push_back(tempObjLoader);
	tempObjLoader->LoadFromFile("../Asset/palmTree/palmtree.obj", L"../Asset/DDS/palmtree.dds", NULL, NULL, m_d3d11Device);
	tempObjLoader->setMeshInstance();
	//*************************tree2************************
	//objGeo[2]
	tempObjLoader = new ObjLoader(m_ID3D11DeviceContext);
	m_objLoaderGeo.push_back(tempObjLoader);
	tempObjLoader->LoadFromFile("../Asset/palmTree/palmtree2.obj", L"../Asset/DDS/palmtree.dds", NULL, NULL, m_d3d11Device);
	tempObjLoader->setMeshInstance();

	


	//*************************skybox************************
	//skybox
	float skyEdge = 50.0f;
	m_skybox_VERTEX[0].xyz.x = -skyEdge;
	m_skybox_VERTEX[0].xyz.y = -skyEdge;
	m_skybox_VERTEX[0].xyz.z = skyEdge;

	m_skybox_VERTEX[1].xyz.x = -skyEdge;
	m_skybox_VERTEX[1].xyz.y = skyEdge;
	m_skybox_VERTEX[1].xyz.z = skyEdge;

	m_skybox_VERTEX[2].xyz.x = skyEdge;
	m_skybox_VERTEX[2].xyz.y = skyEdge;
	m_skybox_VERTEX[2].xyz.z = skyEdge;

	m_skybox_VERTEX[3].xyz.x = skyEdge;
	m_skybox_VERTEX[3].xyz.y = -skyEdge;
	m_skybox_VERTEX[3].xyz.z = skyEdge;

	m_skybox_VERTEX[4].xyz.x = -skyEdge;
	m_skybox_VERTEX[4].xyz.y = -skyEdge;
	m_skybox_VERTEX[4].xyz.z = -skyEdge;

	m_skybox_VERTEX[5].xyz.x = -skyEdge;
	m_skybox_VERTEX[5].xyz.y = skyEdge;
	m_skybox_VERTEX[5].xyz.z = -skyEdge;

	m_skybox_VERTEX[6].xyz.x = skyEdge;
	m_skybox_VERTEX[6].xyz.y = skyEdge;
	m_skybox_VERTEX[6].xyz.z = -skyEdge;

	m_skybox_VERTEX[7].xyz.x = skyEdge;
	m_skybox_VERTEX[7].xyz.y = -skyEdge;
	m_skybox_VERTEX[7].xyz.z = -skyEdge;

	for (int i = 0; i < 8; i++)
	{
		m_skybox_VERTEX[i].uvw.x = m_skybox_VERTEX[i].xyz.x;
		m_skybox_VERTEX[i].uvw.y = m_skybox_VERTEX[i].xyz.y;
		m_skybox_VERTEX[i].uvw.z = m_skybox_VERTEX[i].xyz.z;
	}

	unsigned int skybox_indicies[skyboxIndexNumber] =
	{ 0, 1, 3, 1, 2, 3 //far
	, 6, 1, 5, 2, 1, 6, //top
	7, 5, 4, 6, 5, 7, //near
	4, 0, 7, 7, 0, 3, //bot
	6, 7, 3, 2, 6, 3, //right
	0, 4, 5, 0, 5, 1 };//left

	setMeshData(m_skybox_VERTEX, skyboxNumber, skybox_indicies, skyboxIndexNumber, L"../direct11/OutputCube.dds",
		m_skybox, m_ID3D11Buffer_skybox, m_ID3D11_index_Buffer_skybox);
	
	//*************************particle************************
	//particle

	//setInputBuffer()
	//************************SAMPLER************************
	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	/*samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;*/
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = FLT_MAX;

	//**************** Create the texture sampler state.****************
	hr = m_d3d11Device->CreateSamplerState(&samplerDesc, &m_sampleState);
	m_ID3D11DeviceContext->PSSetSamplers(0, 1, &m_sampleState);

	//*************************depth************************
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(D3D11_TEXTURE2D_DESC));
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;

	descDepth.Width = m_SCD.BufferDesc.Width;
	descDepth.Height = m_SCD.BufferDesc.Height;
	descDepth.Width = m_SCD.BufferDesc.Width;
	descDepth.Height = m_SCD.BufferDesc.Height;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = m_d3d11Device->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);

	//*************************bind ************************
	D3D11_DEPTH_STENCIL_VIEW_DESC tempDescDSV;
	ZeroMemory(&tempDescDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	tempDescDSV.Format = DXGI_FORMAT_D32_FLOAT;
	tempDescDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	tempDescDSV.Texture2D.MipSlice = 0;

	// Create the depth stencil view
	hr = m_d3d11Device->CreateDepthStencilView(m_pDepthStencil, // Depth stencil texture
		&tempDescDSV, // Depth stencil desc
		&m_ID3D11DepthStencilView);  // [out] Depth stencil view


	//all the shader buffer here
	setShaderBuffer();

	//rasterization state init
	setRasterizationState();
	
	//set blendstate
	SetBlendState();
	//*********************************************IBlendState
	
	m_ID3D11DeviceContext->OMSetBlendState(m_ID3D11BlendState_off, 0, 0xffffffff);

	setConstatBuffer();
	//m_MatrixMirror.setProject(0.1f, 100, 95, 0.5f);

	setbackBufferRenderTarget();
	
	//set view port
	m_ID3D11DeviceContext->RSSetViewports(2, m_D3D11_VIEWPORT);

	//camera
	m_camera = new camera();

	//render to texture
	m_renderToTexture = new renderToTextureResource[2];

	for (int i = 0; i < 2; i++)
		m_renderToTexture[i].init(m_d3d11Device, m_SCD.BufferDesc.Width, m_SCD.BufferDesc.Height);

	m_mirror = new mirror[mirrorNumber];
	//										top left					top right			botright        botleft
	m_mirror[0].setPos(m_d3d11Device, XMFLOAT2(-1.2, -0.2), XMFLOAT2(-0.5, 0.2), XMFLOAT2(-0.4, -0.3), XMFLOAT2(-0.9, -0.95));
	m_mirror[1].setPos(m_d3d11Device, XMFLOAT2(0.5, 0.2), XMFLOAT2(1.2, -0.2), XMFLOAT2(0.9, -0.95), XMFLOAT2(0.4, -0.3));
	m_mirror[2].setData(m_d3d11Device, 2.0, 2.0f, -1.0, 1);
	m_mirror[2].setSRV(L"../Asset/DDS/pilot.dds");

	m_foliage = new foliage();
	m_foliage->init(m_d3d11Device,m_ID3D11DeviceContext, L"../Asset/DDS/foliage.dds", 1);

}

void DEMO_APP::SetBlendState()
{
	HRESULT hr;
	D3D11_BLEND_DESC m_D3D11_BLEND_DESC_on;
	ZeroMemory(&m_D3D11_BLEND_DESC_on, sizeof(D3D11_BLEND_DESC));
	m_D3D11_BLEND_DESC_on.IndependentBlendEnable = false;
	m_D3D11_BLEND_DESC_on.AlphaToCoverageEnable = false;
	m_D3D11_BLEND_DESC_on.RenderTarget[0].BlendEnable = true;
	m_D3D11_BLEND_DESC_on.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	m_D3D11_BLEND_DESC_on.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	m_D3D11_BLEND_DESC_on.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_D3D11_BLEND_DESC_on.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	m_D3D11_BLEND_DESC_on.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	m_D3D11_BLEND_DESC_on.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	m_D3D11_BLEND_DESC_on.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_d3d11Device->CreateBlendState(&m_D3D11_BLEND_DESC_on, &m_ID3D11BlendState_on);

	D3D11_BLEND_DESC m_D3D11_BLEND_DESC_off;
	ZeroMemory(&m_D3D11_BLEND_DESC_off, sizeof(D3D11_BLEND_DESC));
	m_D3D11_BLEND_DESC_off.IndependentBlendEnable = false;
	m_D3D11_BLEND_DESC_off.AlphaToCoverageEnable = false;
	m_D3D11_BLEND_DESC_off.RenderTarget[0].BlendEnable = false;
	m_D3D11_BLEND_DESC_off.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	m_D3D11_BLEND_DESC_off.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	m_D3D11_BLEND_DESC_off.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	m_D3D11_BLEND_DESC_off.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	m_D3D11_BLEND_DESC_off.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	m_D3D11_BLEND_DESC_off.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	m_D3D11_BLEND_DESC_off.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_d3d11Device->CreateBlendState(&m_D3D11_BLEND_DESC_off, &m_ID3D11BlendState_off);

	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
	blendStateDescription.AlphaToCoverageEnable = FALSE;
	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	hr = m_d3d11Device->CreateBlendState(&blendStateDescription, &m_ID3D11BlendState_alphaCoverOff);
	
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));
	blendStateDescription.AlphaToCoverageEnable = TRUE;
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	hr = m_d3d11Device->CreateBlendState(&blendStateDescription, &m_ID3D11BlendState_alphaCoverOn);

}
void  DEMO_APP::setMatrix(ID3D11Buffer *&m_ID3D11MatrixBuffer_OBJECT, MatrixCalculate &m_MatrixCalculate)
{
	D3D11_BUFFER_DESC m_D3D11_BUFFER_DESC_Matrix;
	D3D11_SUBRESOURCE_DATA m_D3D11_SUBRESOURCE_DATA_Matrix;
	ZeroMemory(&m_D3D11_SUBRESOURCE_DATA_Matrix, sizeof(D3D11_SUBRESOURCE_DATA));
	ZeroMemory(&m_D3D11_BUFFER_DESC_Matrix, sizeof(D3D11_BUFFER_DESC));

	m_D3D11_SUBRESOURCE_DATA_Matrix.pSysMem = &m_MatrixCalculate;
	m_D3D11_BUFFER_DESC_Matrix.Usage = D3D11_USAGE_DYNAMIC;
	m_D3D11_BUFFER_DESC_Matrix.ByteWidth = sizeof(m_MatrixCalculate);             // size is the toShader
	m_D3D11_BUFFER_DESC_Matrix.BindFlags = D3D11_BIND_CONSTANT_BUFFER;       // use as a vertex buffer
	m_D3D11_BUFFER_DESC_Matrix.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// create the matrix buffer
	HRESULT hr = m_d3d11Device->CreateBuffer(&m_D3D11_BUFFER_DESC_Matrix, &m_D3D11_SUBRESOURCE_DATA_Matrix, &m_ID3D11MatrixBuffer_OBJECT);

	m_MatrixCalculate.m_worldMatrix1 = XMMatrixIdentity();

	m_MatrixCalculate.m_viewMatrix = XMMatrixIdentity();
	m_MatrixCalculate.m_projectionMatrix = XMMatrixIdentity();


}

void DEMO_APP::renderDataTessShader(ID3D11VertexShader* _vsShader, ID3D11HullShader* _hsShader, ID3D11DomainShader* _dsShader,
	ID3D11GeometryShader* _gsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
	ID3D11Buffer * const _dataBuffer, ID3D11Buffer *const _indexBuffer, ID3D11ShaderResourceView**const _shaderRSV, UINT _indexNumber)
{
	UINT stride = sizeof(SIMPLE_VERTEX);
	UINT offset = 0;

	m_ID3D11DeviceContext->VSSetShader(_vsShader, 0, 0);
	m_ID3D11DeviceContext->HSSetShader(_hsShader, 0, 0);
	m_ID3D11DeviceContext->DSSetShader(_dsShader, 0, 0);
	m_ID3D11DeviceContext->GSSetShader(_gsShader, 0, 0);
	m_ID3D11DeviceContext->PSSetShader(_psShader, 0, 0);

	m_ID3D11DeviceContext->IASetInputLayout(_input);
	m_ID3D11DeviceContext->IASetVertexBuffers(0, 1, &_dataBuffer, &stride, &offset);
	m_ID3D11DeviceContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_ID3D11DeviceContext->PSSetShaderResources(0, 3, _shaderRSV);
//	m_ID3D11DeviceContext->PSSetShaderResources(0, 1, &_shaderRSV);

	m_ID3D11DeviceContext->PSSetSamplers(0, 1, &m_sampleState);
	m_ID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	m_ID3D11DeviceContext->DrawIndexed(_indexNumber, 0, 0);
}



void  DEMO_APP::renderDataMultiTexture(ID3D11VertexShader* _vsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
	ID3D11Buffer * const _dataBuffer, ID3D11Buffer *const _indexBuffer, ID3D11ShaderResourceView**const _shaderRSV, UINT _indexNumber, UINT _textureNumber)
{
	UINT stride = sizeof(SIMPLE_VERTEX);
	UINT offset = 0;

	m_ID3D11DeviceContext->VSSetShader(_vsShader, 0, 0);
	m_ID3D11DeviceContext->HSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->DSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->GSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->PSSetShader(_psShader, 0, 0);
	m_ID3D11DeviceContext->IASetInputLayout(_input);
	m_ID3D11DeviceContext->IASetVertexBuffers(0, 1, &_dataBuffer, &stride, &offset);
	m_ID3D11DeviceContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_ID3D11DeviceContext->PSSetShaderResources(0, _textureNumber, _shaderRSV);
	m_ID3D11DeviceContext->PSSetSamplers(0, 1, &m_sampleState);
	m_ID3D11DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ID3D11DeviceContext->DrawIndexed(_indexNumber, 0, 0);
}

void  DEMO_APP::renderDataSingleTexture(ID3D11VertexShader* _vsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
ID3D11Buffer * const _dataBuffer, ID3D11Buffer *const _indexBuffer, ID3D11ShaderResourceView*const _shaderRSV, UINT _indexNumber)
{
	UINT stride = sizeof(SIMPLE_VERTEX);
	UINT offset = 0;

	m_ID3D11DeviceContext->VSSetShader(_vsShader, 0, 0);
	m_ID3D11DeviceContext->HSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->DSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->GSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->PSSetShader(_psShader, 0, 0);
	m_ID3D11DeviceContext->IASetInputLayout(_input);
	m_ID3D11DeviceContext->IASetVertexBuffers(0, 1, &_dataBuffer, &stride, &offset);
	m_ID3D11DeviceContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_ID3D11DeviceContext->PSSetShaderResources(0, 1, &_shaderRSV);
	m_ID3D11DeviceContext->PSSetSamplers(0, 1, &m_sampleState);
	m_ID3D11DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ID3D11DeviceContext->DrawIndexed(_indexNumber, 0, 0);
}
void  DEMO_APP::renderData(ID3D11VertexShader* _vsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
	ID3D11Buffer * const _dataBuffer, ID3D11Buffer *const _indexBuffer, ID3D11ShaderResourceView**const _shaderRSV, UINT _indexNumber)
{
	UINT stride = sizeof(SIMPLE_VERTEX);
	UINT offset = 0;

	m_ID3D11DeviceContext->VSSetShader(_vsShader, 0, 0);
	m_ID3D11DeviceContext->HSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->DSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->GSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->PSSetShader(_psShader, 0, 0);
	m_ID3D11DeviceContext->IASetInputLayout(_input);
	m_ID3D11DeviceContext->IASetVertexBuffers(0, 1, &_dataBuffer, &stride, &offset);
	m_ID3D11DeviceContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_ID3D11DeviceContext->PSSetShaderResources(0, 3, _shaderRSV);
//	m_ID3D11DeviceContext->PSSetShaderResources(0, 1, &_shaderRSV);
	m_ID3D11DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ID3D11DeviceContext->DrawIndexed(_indexNumber, 0, 0);
}

void  DEMO_APP::renderDataGeoShader(ID3D11VertexShader* _vsShader, ID3D11GeometryShader* _gsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
	ID3D11Buffer * const _dataBuffer, ID3D11ShaderResourceView*const _shaderRSV, UINT vertexNumber)
{
	UINT stride = sizeof(SIMPLE_VERTEX);
	UINT offset = 0;

	m_ID3D11DeviceContext->VSSetShader(_vsShader, 0, 0);
	m_ID3D11DeviceContext->GSSetShader(_gsShader, 0, 0);
	m_ID3D11DeviceContext->PSSetShader(_psShader, 0, 0);

	m_ID3D11DeviceContext->IASetInputLayout(_input);
	m_ID3D11DeviceContext->IASetVertexBuffers(0, 1, &_dataBuffer, &stride, &offset);
//	m_ID3D11DeviceContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_ID3D11DeviceContext->PSSetShaderResources(0, 1, &_shaderRSV);
//	m_ID3D11DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ID3D11DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_ID3D11DeviceContext->Draw(vertexNumber, 0);
//	m_ID3D11DeviceContext->DrawIndexed(_indexNumber, 0, 0);
}
void  DEMO_APP::renderDataInstance(ID3D11VertexShader* _vsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
	ID3D11ShaderResourceView*const _shaderRSV, UINT _vertexNumber, UINT _instanceNumber)
{
	//set shadewr resource view
	m_ID3D11DeviceContext->PSSetShaderResources(0, 1, &_shaderRSV);

	//set input layout for instance
	m_ID3D11DeviceContext->IASetInputLayout(_input);

	//set vertex and pixel shader
	m_ID3D11DeviceContext->VSSetShader(_vsShader, 0, 0);
	m_ID3D11DeviceContext->HSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->DSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->GSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->PSSetShader(_psShader, 0, 0);

	//m_ID3D11DeviceContext->IASetVertexBuffers(0, 1, &_dataBuffer, &stride, &offset);
	//set the sampler state in the pixel shader
	m_ID3D11DeviceContext->PSSetSamplers(0, 1, &m_sampleState);
	m_ID3D11DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ID3D11DeviceContext->DrawInstanced(_vertexNumber, _instanceNumber,0, 0);
}


void  DEMO_APP::renderDataInstanceIndex(ID3D11VertexShader* _vsShader, ID3D11PixelShader* _psShader, ID3D11InputLayout* _input,
	ID3D11Buffer *const _indexBuffer, ID3D11ShaderResourceView**const _shaderRSV, UINT _indexNumber, UINT _instanceNumber)
{
	//set shadewr resource view
//	m_ID3D11DeviceContext->PSSetShaderResources(0, 1, &_shaderRSV);
	m_ID3D11DeviceContext->PSSetShaderResources(0, 1, _shaderRSV);
	//set input layout for instance
	m_ID3D11DeviceContext->IASetInputLayout(_input);

	//set vertex and pixel shader
	m_ID3D11DeviceContext->VSSetShader(_vsShader, 0, 0);
	m_ID3D11DeviceContext->HSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->DSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->GSSetShader(NULL, 0, 0);
	m_ID3D11DeviceContext->PSSetShader(_psShader, 0, 0);
	
	m_ID3D11DeviceContext->IASetIndexBuffer(_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//set the sampler state in the pixel shader
	m_ID3D11DeviceContext->PSSetSamplers(0, 1, &m_sampleState);
	m_ID3D11DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ID3D11DeviceContext->DrawIndexedInstanced(_indexNumber, _instanceNumber, 0, 0, 0);
	//m_ID3D11DeviceContext->DrawInstanced(_vertexNumber, _instanceNumber, 0, 0);
}
//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{


	m_Xtime[0].Signal();
	m_Xtime[1].Signal();

	if (0)
//	if (m_Xtime[1].TotalTimeExact() >= 1.0f)
	{
		//green red light
		m_LightPosData.pointLightPos.x += 1.0f;
		switch ((int)m_LightPosData.pointLightPos.x%4)
		{
		case 0:
			m_lightColorData.pointLightColor.x = 3.0f;
			m_lightColorData.pointLightColor = { 1.0f, 0.0f, 0.0f, 1.0f };
			break;
		case 1:
			m_lightColorData.pointLightColor = { 0.0f, 1.0f, 0.0f, 1.0f };
			break;
		case 2:
			m_lightColorData.pointLightColor = { 1.0f, 1.0f, 0.0f, 1.0f };
			break;
		case 3:
			m_lightColorData.pointLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			break;
		default:
			break;
		}
	
		XMMATRIX temp;
		m_camera->getMultiplyMatrix(temp);
		
		m_lightColorData.spotLightDir = { temp.r->m128_f32[8], temp.r->m128_f32[9], temp.r->m128_f32[10] };

		if (rand() % 2 == 0)
			m_camera->getEyePosLeft(m_LightPosData.spotLightPos);
		else
			m_camera->getEyePosRight(m_LightPosData.spotLightPos);

		m_lightColorData.lightDirectionSource = { rand() % 10 / 10.0f, rand() % 10 / 10.0f, rand() % 10 / 10.0f };
		m_Xtime[1].Restart();
	}
	

	if (m_Xtime[0].TotalTimeExact() >= 1.0f / 30.0f) //&& GetCapture() == window
	{
		m_MatrixRain.m_worldMatrix1.r->m128_f32[13] -= 0.1f;

		if (m_MatrixRain.m_worldMatrix1.r->m128_f32[13] <= -5.0f)
			m_MatrixRain.m_worldMatrix1.r->m128_f32[13] = 5.0f;

		m_camera->input(1);

		//camera update
		m_camera->update();

		m_camera->getCameraPos(m_LightPosData.cameraPos);

	
		m_Xtime[0].Restart();
	}

	XMMATRIX temp;
	m_camera->getMultiplyMatrix(temp);
	m_camera->getCameraPos(m_LightPosData.spotLightPos);
	m_lightColorData.spotLightDir = { temp.r->m128_f32[8], temp.r->m128_f32[9], temp.r->m128_f32[10] };

	////update
	//if (m_Xtime[0].TotalTimeExact() >= 1.0f / 30.0f)
	//{
	//	//	m_radian += 1.8f;
	//	//m_Xtime[0].Restart();
	//	m_MatrixCalculate.m_worldMatrix1.r->m128_f32[0] = cosf(-m_radian / 180 * 3.14f);
	//	m_MatrixCalculate.m_worldMatrix1.r->m128_f32[8] = sinf(-m_radian / 180 * 3.14f);
	//	m_MatrixCalculate.m_worldMatrix1.r->m128_f32[2] = -sinf(-m_radian / 180 * 3.14f);
	//	m_MatrixCalculate.m_worldMatrix1.r->m128_f32[10] = cosf(-m_radian / 180 * 3.14f);
	//	
	//}
	

	if (GetAsyncKeyState('U') & 1)
	{
		m_LightPosData.spotLightPos.y += 0.1f;
	}
	if (GetAsyncKeyState('I') & 1)
	{
		m_LightPosData.spotLightPos.y -= 0.1f;
	}

	if (GetAsyncKeyState('T') & 1)
	{
		m_LightPosData.spotLightPos.z += 0.1f;
	}
	if (GetAsyncKeyState('Y') & 1)
	{
		m_LightPosData.spotLightPos.z -= 0.1f;
	}

	if (GetAsyncKeyState('O') & 1)
	{
		m_LightPosData.pointLightPos.x += 0.1f;// = { 0.5f + , -0.4f, 0.0f };
	}
	if (GetAsyncKeyState('P') & 1)
	{
		m_LightPosData.pointLightPos.x -= 0.1f;
	}

	
	if (GetAsyncKeyState('3') & 1)
		m_rasterizeState = mass_on_fill;

	if (GetAsyncKeyState('4') & 1)
		m_rasterizeState = mass_off_fill;

	if (GetAsyncKeyState('5') & 1)
		m_rasterizeState = mass_on_wire;

	if (GetAsyncKeyState('6') & 1)
		m_rasterizeState = mass_off_wire;

	if (GetAsyncKeyState('7') & 1)
		m_tessell_state = tessell_on;

	if (GetAsyncKeyState('8') & 1)
		m_tessell_state = tessell_off;

	m_camera->getViewMatrix(m_MatrixCalculate.m_viewMatrix);
	m_camera->getViewMatrix(m_MatrixSkyBox.m_viewMatrix);
	m_camera->getViewMatrix(m_MatrixRain.m_viewMatrix);
	m_MatrixSkyBox.m_worldMatrix1 = XMMatrixTranslation(m_LightPosData.cameraPos.x, m_LightPosData.cameraPos.y, m_LightPosData.cameraPos.z);

	//all screen render to texture
	//m_RenderTargetView[1] is driver
	m_ID3D11DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView[0], 0);
	m_ID3D11DeviceContext->RSSetViewports(1, &m_D3D11_VIEWPORT[2]);


	//get the left camera shift
	normalCameraYAxixReverse('l');

	//render to texture instead of back buffer
	renderToTexture(&m_renderToTexture[0]);

	//render all to texture
	renderAll();

	//get the right camera shift
	normalCameraYAxixReverse('r');

	//render to texture instead of back buffer
	renderToTexture(&m_renderToTexture[1]);

	//render all to texture
	renderAll();

	// ****************front camera render ****************

	//use normal rendertarget to be the render to texture
	setbackBufferRenderTarget();

	m_ID3D11DeviceContext->RSSetViewports(1, &m_D3D11_VIEWPORT[2]);

	// ****************main pilot screen render ****************

	//clean
	beginScene(m_RenderTargetView[1]);

	//normal camera
	normalCamera();

	//render all
	renderAll();

	// ****************foliage render ****************

	//turn on alpha to cover
	turnOnAlphaValue();

	normalCamera();

	//set the instance buffer data here
	m_foliage->Frame(m_camera->returnCameraPos());
	m_foliage->updateMatrix(m_MatrixCalculate.m_viewMatrix, m_MatrixCalculate.m_projectionMatrix);

	//get the foliage matrix
	m_foliage->Render(pfoliage_VS, pfoliage_PS, pLayoutFoliage, m_sampleState);
//	turnOffAlphaValue();

	// ****************god camera render ****************
	//godCamera();

	////RT2 and half up
	//m_ID3D11DeviceContext->RSSetViewports(1, &m_D3D11_VIEWPORT[3]);
	//
	////render all
	//renderAll();

	// ****************mirror render ****************
	//RT2 and half down
	m_ID3D11DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView[1], 0);
	m_ID3D11DeviceContext->RSSetViewports(1, &m_D3D11_VIEWPORT[2]);


	//render mirror
	renderMirror();

	m_IDXGISwapChain->Present(0, 0);

	return true;
}

void  DEMO_APP::renderMirror()
{
	//must render use fill
	m_ID3D11DeviceContext->RSSetState(m_ID3D11_RS[6]);
	
	m_MatrixMirror.m_worldMatrix1 = XMMatrixIdentity();
	D3D11_MAPPED_SUBRESOURCE ms2;
	ZeroMemory(&ms2, sizeof(D3D11_MAPPED_SUBRESOURCE));
	m_ID3D11DeviceContext->Map(m_ID3D11MatrixBuffer_mirror, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
	memcpy(ms2.pData, &m_MatrixMirror, sizeof(m_MatrixMirror));                // copy the data
	m_ID3D11DeviceContext->Unmap(m_ID3D11MatrixBuffer_mirror, NULL);                                     // unmap the buffer
	m_ID3D11DeviceContext->VSSetConstantBuffers(0, 1, &m_ID3D11MatrixBuffer_mirror);
	
	renderDataSingleTexture(pVS, pPS, pLayout, m_mirror[0].getVertexBuffer(), m_mirror[0].getIndexBuffer(), m_renderToTexture[0].getShaderResourceView(), 6);
	renderDataSingleTexture(pVS, pPS, pLayout, m_mirror[1].getVertexBuffer(), m_mirror[1].getIndexBuffer(), m_renderToTexture[1].getShaderResourceView(), 6);

	m_ID3D11DeviceContext->OMSetBlendState(m_ID3D11BlendState_on, 0, 0xffffffff);
	renderDataSingleTexture(pVS, pPS, pLayout, m_mirror[2].getVertexBuffer(), m_mirror[2].getIndexBuffer(), m_mirror[2].getSRV(), 6);

}

//get the camera shift
void DEMO_APP::normalCameraYAxixReverse(char _char)
{
	//temporairly save camera first
	XMMATRIX tempMatrix = m_MatrixCalculate.m_viewMatrix;

	//local y 180 degree
	if (_char == 'l')
	m_camera->getMirrorLeft(m_MatrixCalculate.m_viewMatrix);
	else
	//local y 180 degree
	m_camera->getMirrorRight(m_MatrixCalculate.m_viewMatrix);

	D3D11_MAPPED_SUBRESOURCE ms2;
	ZeroMemory(&ms2, sizeof(D3D11_MAPPED_SUBRESOURCE));
	
	//camera matrix
	m_ID3D11DeviceContext->Map(m_ID3D11MatrixBuffer_OBJECT, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
	memcpy(ms2.pData, &m_MatrixCalculate, sizeof(m_MatrixCalculate));                // copy the data
	m_ID3D11DeviceContext->Unmap(m_ID3D11MatrixBuffer_OBJECT, NULL);                                     // unmap the buffer
	m_ID3D11DeviceContext->VSSetConstantBuffers(0, 1, &m_ID3D11MatrixBuffer_OBJECT);
	
	//mirror matrix set to Domain shader
	if (m_tessell_state == tessell_on)
	{
		m_ID3D11DeviceContext->Map(m_ID3D11MatrixBuffer_OBJECT, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
		memcpy(ms2.pData, &m_MatrixCalculate, sizeof(m_MatrixCalculate));                // copy the data
		m_ID3D11DeviceContext->Unmap(m_ID3D11MatrixBuffer_OBJECT, NULL);                                     // unmap the buffer
		m_ID3D11DeviceContext->DSSetConstantBuffers(0, 1, &m_ID3D11MatrixBuffer_OBJECT);
	}

	//save it back
	m_MatrixCalculate.m_viewMatrix = tempMatrix;
}

void DEMO_APP::godCamera()
{
	//temporairly save camera first
	XMMATRIX tempMatrix = m_MatrixCalculate.m_viewMatrix;

	m_camera->getGodView(m_MatrixCalculate.m_viewMatrix);

	D3D11_MAPPED_SUBRESOURCE ms2;
	ZeroMemory(&ms2, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//camera matrix
	m_ID3D11DeviceContext->Map(m_ID3D11MatrixBuffer_OBJECT, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
	memcpy(ms2.pData, &m_MatrixCalculate, sizeof(m_MatrixCalculate));                // copy the data
	m_ID3D11DeviceContext->Unmap(m_ID3D11MatrixBuffer_OBJECT, NULL);                                     // unmap the buffer
	m_ID3D11DeviceContext->VSSetConstantBuffers(0, 1, &m_ID3D11MatrixBuffer_OBJECT);

	//save it back
	m_MatrixCalculate.m_viewMatrix = tempMatrix;

}
void DEMO_APP::normalCamera()
{
	D3D11_MAPPED_SUBRESOURCE ms2;
	ZeroMemory(&ms2, sizeof(D3D11_MAPPED_SUBRESOURCE));
	//camera matrix
	m_ID3D11DeviceContext->Map(m_ID3D11MatrixBuffer_OBJECT, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
	memcpy(ms2.pData, &m_MatrixCalculate, sizeof(m_MatrixCalculate));                // copy the data
	m_ID3D11DeviceContext->Unmap(m_ID3D11MatrixBuffer_OBJECT, NULL);                                     // unmap the buffer
	m_ID3D11DeviceContext->VSSetConstantBuffers(0, 1, &m_ID3D11MatrixBuffer_OBJECT);
}
void DEMO_APP::renderAll()
{
	D3D11_MAPPED_SUBRESOURCE ms2;
	ZeroMemory(&ms2, sizeof(D3D11_MAPPED_SUBRESOURCE));

	//turn on blendstate
	m_ID3D11DeviceContext->OMSetBlendState(m_ID3D11BlendState_on, 0, 0xffffffff);

	//*************light data *****************
	//light pos buffer
	m_ID3D11DeviceContext->Map(m_Buffer_LightPos, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
	memcpy(ms2.pData, &m_LightPosData, sizeof(LightPosData));                // copy the data
	m_ID3D11DeviceContext->Unmap(m_Buffer_LightPos, NULL);                                     // unmap the buffer
	m_ID3D11DeviceContext->VSSetConstantBuffers(1, 1, &m_Buffer_LightPos);

	//light color buffer
	m_ID3D11DeviceContext->Map(m_Buffer_LightColor, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
	memcpy(ms2.pData, &m_lightColorData, sizeof(LightColorData));                // copy the data
	m_ID3D11DeviceContext->Unmap(m_Buffer_LightColor, NULL);                                     // unmap the buffer
	m_ID3D11DeviceContext->PSSetConstantBuffers(0, 1, &m_Buffer_LightColor);

	//************* draw D3D11_CULL_FRONT *****************
	if (m_rasterizeState == mass_on_fill)
		m_ID3D11DeviceContext->RSSetState(m_ID3D11_RS[2]);

	if (m_rasterizeState == mass_off_fill)
		m_ID3D11DeviceContext->RSSetState(m_ID3D11_RS[3]);

	if (m_rasterizeState == mass_on_wire)
		m_ID3D11DeviceContext->RSSetState(m_ID3D11_RS[0]);

	if (m_rasterizeState == mass_off_wire)
		m_ID3D11DeviceContext->RSSetState(m_ID3D11_RS[1]);

	//************* render CULL_FRONT *****************
	
	//************* draw CULL BACK *****************
	if (m_rasterizeState == mass_on_fill)
		m_ID3D11DeviceContext->RSSetState(m_ID3D11_RS[6]);

	if (m_rasterizeState == mass_off_fill)
		m_ID3D11DeviceContext->RSSetState(m_ID3D11_RS[7]);

	if (m_rasterizeState == mass_on_wire)
		m_ID3D11DeviceContext->RSSetState(m_ID3D11_RS[4]);

	if (m_rasterizeState == mass_off_wire)
		m_ID3D11DeviceContext->RSSetState(m_ID3D11_RS[5]);

	//************* render CULL_BACK *****************
	if (m_tessell_state == tessell_on)
	{
		m_ID3D11DeviceContext->Map(m_ID3D11_Tes_Buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
		memcpy(ms2.pData, &m_TessellationConstantBuffer, sizeof(TessellationConstantBuffer));                // copy the data
		m_ID3D11DeviceContext->Unmap(m_ID3D11_Tes_Buffer, NULL);                                     // unmap the buffer
		m_ID3D11DeviceContext->HSSetConstantBuffers(0, 1, &m_ID3D11_Tes_Buffer);

	
		m_ID3D11DeviceContext->Map(m_Buffer_LightPos, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
		memcpy(ms2.pData, &m_LightPosData, sizeof(LightPosData));                // copy the data
		m_ID3D11DeviceContext->Unmap(m_Buffer_LightPos, NULL);                                     // unmap the buffer
		m_ID3D11DeviceContext->DSSetConstantBuffers(1, 1, &m_Buffer_LightPos);
	}
	else
	{
	}
	
		//turn off blendstate
	m_ID3D11DeviceContext->OMSetBlendState(m_ID3D11BlendState_off, 0, 0xffffffff);

	for (int i = 0; i < m_objLoader.size(); i++)
	{
		switch (m_tessell_state)
		{
		case tessell_on:
	
			renderDataTessShader(pTes_VS, pTes_HS, pTes_DS, NULL, pTes_PS, pLayout5, m_objLoader[i]->get_Vertex_Buffer(), m_objLoader[i]->get_Index_Buffer(),
				m_objLoader[i]->get_SRV(), m_objLoader[i]->getIndicesNumber());
			break;
		case tessell_off:
			renderData(pVS, pPS, pLayout, m_objLoader[i]->get_Vertex_Buffer(), m_objLoader[i]->get_Index_Buffer(),
				m_objLoader[i]->get_SRV(), m_objLoader[i]->getIndicesNumber());
			break;
		default:
			break;
		}
	}

	for (int i = 0; i < m_objLoaderGeo.size(); i++)
	{
		m_objLoaderGeo[i]->setInstanceBuffer();
		renderDataInstanceIndex(pVS_Instance, pPS_Instance, pLayout_Instnace, m_objLoaderGeo[i]->get_Index_Buffer(),
			m_objLoaderGeo[i]->get_SRV(), m_objLoaderGeo[i]->getIndicesNumber(), m_objLoaderGeo[i]->get_InstanceNumber());
	}
	
	m_ID3D11DeviceContext->Map(m_ID3D11MatrixBuffer_skyBox, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
	memcpy(ms2.pData, &m_MatrixSkyBox, sizeof(m_MatrixSkyBox));                // copy the data
	m_ID3D11DeviceContext->Unmap(m_ID3D11MatrixBuffer_skyBox, NULL);                                     // unmap the buffer
	m_ID3D11DeviceContext->VSSetConstantBuffers(0, 1, &m_ID3D11MatrixBuffer_skyBox);

	//render skybox
	renderDataSingleTexture(pVS_skybox, pPS_skybox, pLayout_skybox, m_ID3D11Buffer_skybox, m_ID3D11_index_Buffer_skybox, m_skybox, skyboxIndexNumber);

	//m_ID3D11DeviceContext->RSSetViewports(1, &m_D3D11_VIEWPORT2);

	//rain matrix
	m_ID3D11DeviceContext->Map(m_ID3D11MatrixBuffer_rain, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
	memcpy(ms2.pData, &m_MatrixRain, sizeof(m_MatrixRain));                // copy the data
	m_ID3D11DeviceContext->Unmap(m_ID3D11MatrixBuffer_rain, NULL);                                     // unmap the buffer
	m_ID3D11DeviceContext->GSSetConstantBuffers(0, 1, &m_ID3D11MatrixBuffer_rain);

//	m_ID3D11DeviceContext->OMSetRenderTargets(1, m_RenderTargetView, 0);
	//render geo shader, rain
	renderDataGeoShader(pVS_Geo, pGeoShader, pPS_Geo, pLayout_geo, m_ID3D11Buffer_rain, NULL, rainNumber);

	
}



//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	m_IDXGISwapChain->SetFullscreenState(false, nullptr);
	this;
	// TODO: PART 1 STEP 6
	// close and release all existing COM objects
	SAFE_RELEASE(m_IDXGISwapChain);
	SAFE_RELEASE(m_d3d11Device);
	SAFE_RELEASE(m_ID3D11DeviceContext);
	SAFE_RELEASE(m_RenderTargetView[0]);
	SAFE_RELEASE(m_RenderTargetView[1]);

	SAFE_RELEASE(pLayout);
	SAFE_RELEASE(pLayout5);

	SAFE_RELEASE(m_pDepthStencil);
	SAFE_RELEASE(m_ID3D11DepthStencilView);
	SAFE_RELEASE(m_ID3D11MatrixBuffer_OBJECT);

	SAFE_RELEASE(pVS);
	SAFE_RELEASE(pPS);
	SAFE_RELEASE(m_sampleState);
	SAFE_RELEASE(m_ID3D11BlendState_alphaCoverOn);
	SAFE_RELEASE(m_ID3D11BlendState_alphaCoverOff);

	//skybox
	SAFE_RELEASE(m_skybox);
	SAFE_RELEASE(m_ID3D11Buffer_skybox);
	SAFE_RELEASE(m_ID3D11_index_Buffer_skybox);


	//light
	SAFE_RELEASE(m_Buffer_LightPos);
	SAFE_RELEASE(m_Buffer_LightColor);
	

	//ground PS
	SAFE_RELEASE(groundPS);
	SAFE_RELEASE(groundVS);

	//rain
	SAFE_RELEASE(m_ID3D11Buffer_rain);
	SAFE_RELEASE(m_ID3D11MatrixBuffer_rain);


	SAFE_RELEASE(pLayout_skybox);
	SAFE_RELEASE(pVS_skybox);
	SAFE_RELEASE(pPS_skybox);

	SAFE_RELEASE(pVS_Instance);
	SAFE_RELEASE(pPS_Instance);
	SAFE_RELEASE(pLayout_Instnace);
	
	for (int i = 0; i < rasterizeNumber; i++)
		SAFE_RELEASE(m_ID3D11_RS[i]);

	SAFE_RELEASE(m_ID3D11BlendState_on);
	SAFE_RELEASE(m_ID3D11BlendState_off);
	SAFE_RELEASE(m_camera);
	SAFE_RELEASE(m_ID3D11MatrixBuffer_skyBox);

	SAFE_RELEASE(pGeoShader);
	SAFE_RELEASE(pVS_Geo);
	SAFE_RELEASE(pPS_Geo);
	SAFE_RELEASE(pLayout_geo);
	
	//********foilage
	SAFE_RELEASE(pfoliage_PS);
	SAFE_RELEASE(pfoliage_VS);
	SAFE_RELEASE(pLayoutFoliage);

	SAFE_RELEASE(m_foliage);

	SAFE_RELEASE(pTes_VS);
	SAFE_RELEASE(pTes_HS);
	SAFE_RELEASE(pTes_DS);
	SAFE_RELEASE(pTes_PS);
	SAFE_RELEASE(m_ID3D11_Tes_Buffer);


	for (int i = 0; i < 2; i++)
		m_renderToTexture[i].Release();

	delete[]m_renderToTexture;

	SAFE_RELEASE(m_ID3D11MatrixBuffer_mirror);
	for (int i = 0; i < m_objLoader.size(); i++)
	{
		delete m_objLoader[i];
	}
	for (int i = 0; i < m_objLoaderGeo.size(); i++)
	{
		delete m_objLoaderGeo[i];
	}
	for (int i = 0; i < mirrorNumber; i++)
	{
		m_mirror[i].Release();
		
	}
	delete[] m_mirror;

	//this;
	UnregisterClass(L"DirectXApplication", application);
	return true;
}

// ****************** BEGIN WARNING ******T ALREADY!
DEMO_APP* m_global;//*****************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW I
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));

	DEMO_APP myApp(hInstance, (WNDPROC)WndProc);
	m_global = &myApp;
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp.Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp.ShutDown();
	return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_global == nullptr)
		return 1;

	DEMO_APP *m_DEMO_APP = m_global;
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case (WM_DESTROY) : { PostQuitMessage(0); }
						break;
	case (WM_LBUTTONDOWN) :
		m_DEMO_APP->getCamera()->SetPreMousePos();
		break;
	case (WM_LBUTTONUP) :
		ReleaseCapture();
		break;
	case (WM_SIZE) :
	{
		if (m_DEMO_APP->m_IDXGISwapChain)// (g_pSwapChain)
		{
			m_DEMO_APP->m_ID3D11DeviceContext->OMSetRenderTargets(0, 0, 0);

			//release rendertargerview
			m_DEMO_APP->m_RenderTargetView[0]->Release();

			m_DEMO_APP->m_ID3D11DeviceContext->ClearState();

			HRESULT hr;

		//	hr = m_DEMO_APP->m_IDXGISwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
			hr = m_DEMO_APP->m_IDXGISwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
			ID3D11Texture2D* pBuffer;
			hr = m_DEMO_APP->m_IDXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBuffer);
		
			//create render target
			hr = m_DEMO_APP->m_d3d11Device->CreateRenderTargetView(pBuffer, NULL,
				&m_DEMO_APP->m_RenderTargetView[0]);
		
			//no use anymore
			pBuffer->Release();

			//release depthstencilview	
			m_DEMO_APP->m_pDepthStencil->Release();

			DXGI_SWAP_CHAIN_DESC m_SCD;
			m_DEMO_APP->m_IDXGISwapChain->GetDesc(&m_SCD);
			//*************************depth************************
			D3D11_TEXTURE2D_DESC descDepth;
			ZeroMemory(&descDepth, sizeof(D3D11_TEXTURE2D_DESC));
			descDepth.Format = DXGI_FORMAT_D32_FLOAT;
			descDepth.SampleDesc.Count = 1;
			descDepth.SampleDesc.Quality = 0;
			descDepth.MipLevels = 1;
			descDepth.ArraySize = 1;
			
			descDepth.Width = m_SCD.BufferDesc.Width;// 1920;		
			descDepth.Height = m_SCD.BufferDesc.Height;//1080;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			descDepth.CPUAccessFlags = 0;
			descDepth.MiscFlags = 0;
			hr = m_DEMO_APP->m_d3d11Device->CreateTexture2D(&descDepth, NULL, &m_DEMO_APP->m_pDepthStencil);

			//release depthstencilview
			m_DEMO_APP->m_ID3D11DepthStencilView->Release();
			//*************************bind ************************
			D3D11_DEPTH_STENCIL_VIEW_DESC tempDescDSV;
			ZeroMemory(&tempDescDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
			tempDescDSV.Format = DXGI_FORMAT_D32_FLOAT;
			tempDescDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			tempDescDSV.Texture2D.MipSlice = 0;

		
			// Create the depth stencil view
			hr = m_DEMO_APP->m_d3d11Device->CreateDepthStencilView(m_DEMO_APP->m_pDepthStencil, // Depth stencil texture
				&tempDescDSV, // Depth stencil desc
				&m_DEMO_APP->m_ID3D11DepthStencilView); 

			float ratio = descDepth.Height / descDepth.Width;
		
			//set depthstencilview
			m_DEMO_APP->m_ID3D11DeviceContext->OMSetRenderTargets(1, &m_DEMO_APP->m_RenderTargetView[0], m_DEMO_APP->m_ID3D11DepthStencilView);
		
			m_DEMO_APP->m_D3D11_VIEWPORT[0].Height = m_SCD.BufferDesc.Height / 2;
			m_DEMO_APP->m_D3D11_VIEWPORT[0].Width = m_SCD.BufferDesc.Width;

			m_DEMO_APP->m_D3D11_VIEWPORT[1].TopLeftY = m_SCD.BufferDesc.Height / 2;
			m_DEMO_APP->m_D3D11_VIEWPORT[1].Height = m_SCD.BufferDesc.Height / 2;
			m_DEMO_APP->m_D3D11_VIEWPORT[1].Width = m_SCD.BufferDesc.Width;

			m_DEMO_APP->m_D3D11_VIEWPORT[2].Height = m_SCD.BufferDesc.Height ;
			m_DEMO_APP->m_D3D11_VIEWPORT[2].Width = m_SCD.BufferDesc.Width;
	

			m_DEMO_APP->m_ID3D11DeviceContext->RSSetViewports(2, m_DEMO_APP->m_D3D11_VIEWPORT);
			
			m_DEMO_APP->m_renderToTexture->Release();
			m_DEMO_APP->m_renderToTexture->init(m_DEMO_APP->m_d3d11Device, m_SCD.BufferDesc.Width, m_SCD.BufferDesc.Height);
		}
		return 1;
	}

	break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

