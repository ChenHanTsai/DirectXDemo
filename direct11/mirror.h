#pragma once

#include <DirectXMath.h>
using namespace DirectX;

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")

#define SAFE_RELEASE(p){if(p){p->Release();p=NULL;}}

#define rectNumber 4
#define rectIndexNumber 6
#include "DDSTextureLoader.h"

class mirror
{
	struct SIMPLE_VERTEX
	{
		XMFLOAT3 xyz;
		XMFLOAT3 uvw;
		XMFLOAT3 nrm;

		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
		SIMPLE_VERTEX()
		{

		}

	};

private:
	ID3D11Buffer *m_index_buffer;
	ID3D11Buffer *m_vertex_buffer;
	ID3D11Device *m_d3d11Device;
	float left;
	float top;
	float width;
	float height;
	int m_counter = 0;

	ID3D11ShaderResourceView* m_SRV;
public:

	void setSRV(wchar_t* _TexturefileName);

	ID3D11ShaderResourceView* getSRV()const
	{
		return m_SRV;
	}
	SIMPLE_VERTEX m_backMirror_VERTEX[rectNumber];
	mirror();
	~mirror();

	ID3D11Buffer *getIndexBuffer()const
	{
		return m_index_buffer;
	}

	ID3D11Buffer *getVertexBuffer()const
	{
		return m_vertex_buffer;
	}

	void setPos(ID3D11Device *_m_d3d11Device, const XMFLOAT2& topleft, const XMFLOAT2& topright, const XMFLOAT2& botright, const XMFLOAT2& botleft);
	void setData(ID3D11Device *_m_d3d11Device,float _width, float _height, float _left, float _top);
	void render();
	void Release();
	void setVertexBuffer(const void* _vertex_data, const int& _vertexNumber);
	void setIndexBuffer(const void* _index_data, const int& _indexNumber);
};

