#pragma once

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")
#include <DirectXMath.h>
using namespace DirectX;

#include "DDSTextureLoader.h"k
#define SAFE_RELEASE(p){if(p){p->Release();p=NULL;}}
class foliage
{
	//*********Vertex buffer data
	struct VertexType
	{		
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};
	struct instanceType
	{
		XMMATRIX matrix;
		XMFLOAT3 color;
	};

	struct MatrixType
	{
		XMMATRIX viewMatrix;
		XMMATRIX projectMatrix;
	};

	MatrixType m_MatrixType;
	struct  foliageType
	{
		//pos
		float x, z;
		
		//color
		float r, g, b;
	};

	
public:

	bool init(ID3D11Device* _ID3D11Device, ID3D11DeviceContext* _ID3D11DeviceContext,WCHAR* _fileName, int _count);
	void Release();
	void Render(ID3D11VertexShader * _vs, ID3D11PixelShader *_ps, ID3D11InputLayout * _input, ID3D11SamplerState* m_sampleState);
	void Frame(XMFLOAT3 _cameraPos);

	void getVertexNumber();
	void getIndexNumber();

	void setVSConstantBuffer();

	void updateMatrix(const XMMATRIX& _viewMatrix,const XMMATRIX& _projectMatrix);

	ID3D11ShaderResourceView* get_SRV()
	{
		return m_SRV;
	}

	foliage();
	~foliage();
	bool initializeBuffer(ID3D11Device* _device);
	bool generatePosition();
	
private:

	instanceType _instanceType;
	ID3D11DeviceContext* m_d3d11DeviceContext;
	ID3D11Device* m_d3d11Device;

	int m_verticeCount;
	int m_foliageCount;
	
	
	foliageType* m_foliageArray;
	instanceType* m_instanceType;

	float m_windRotation;
	int m_windDirection;

	ID3D11Buffer *m_matrixBuffer = nullptr;
	ID3D11Buffer *m_ID3D11Buffer_vertex = nullptr;
	ID3D11Buffer *m_instanceBuffer = nullptr;
	ID3D11ShaderResourceView* m_SRV = nullptr;

	void setVertexBuffer(const void* _vertex_data, const int& _vertexNumber);
	void setInstanceBuffer(const void* m_instanceType, const int& _instanceCount);

	void renderBuffer();

	void bindInstanceBuffer();

	void 	bindVSConstantBuffer();
};

