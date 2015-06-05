#include "mirror.h"
#include <assert.h>

mirror::mirror()
{
	m_index_buffer = nullptr;
	m_vertex_buffer = nullptr;
	m_SRV = nullptr;
	m_counter++;
}

void mirror::setSRV(wchar_t* _TexturefileName)
{
	HRESULT hr;
	hr = CreateDDSTextureFromFile(m_d3d11Device, _TexturefileName, NULL, &m_SRV);
	if (FAILED(hr))
	{
		assert(0);
	}
}
mirror::~mirror()
{
}

void mirror::Release()
{
	SAFE_RELEASE(m_index_buffer);
	SAFE_RELEASE(m_vertex_buffer);
	SAFE_RELEASE(m_SRV);
	m_counter--;
	
}
void mirror::setPos(ID3D11Device *_m_d3d11Device, const XMFLOAT2& topleft, const XMFLOAT2& topright, const XMFLOAT2& botright, const XMFLOAT2& botleft)
{
	m_d3d11Device = _m_d3d11Device;
	float cameraZ = 0;

	//********************m_backMirror_VERTEX	
	//topleft
	m_backMirror_VERTEX[0].xyz.x = topleft.x;
	m_backMirror_VERTEX[0].xyz.y = topleft.y;
	m_backMirror_VERTEX[0].xyz.z = cameraZ;
	m_backMirror_VERTEX[0].uvw.x = 0;
	m_backMirror_VERTEX[0].uvw.y = 0;
	m_backMirror_VERTEX[0].uvw.z = 0;

	//top right
	m_backMirror_VERTEX[1].xyz.x = topright.x;
	m_backMirror_VERTEX[1].xyz.y = topright.y;
	m_backMirror_VERTEX[1].xyz.z = cameraZ;
	m_backMirror_VERTEX[1].uvw.x = 1;
	m_backMirror_VERTEX[1].uvw.y = 0;
	m_backMirror_VERTEX[1].uvw.z = 0;

	//bot right
	m_backMirror_VERTEX[2].xyz.x = botright.x;
	m_backMirror_VERTEX[2].xyz.y = botright.y;
	m_backMirror_VERTEX[2].xyz.z = cameraZ;
	m_backMirror_VERTEX[2].uvw.x = 1;
	m_backMirror_VERTEX[2].uvw.y = 1;
	m_backMirror_VERTEX[2].uvw.z = 0;

	//bot left
	m_backMirror_VERTEX[3].xyz.x = botleft.x;
	m_backMirror_VERTEX[3].xyz.y = botleft.y;
	m_backMirror_VERTEX[3].xyz.z = cameraZ;
	m_backMirror_VERTEX[3].uvw.x = 0;
	m_backMirror_VERTEX[3].uvw.y = 1;
	m_backMirror_VERTEX[3].uvw.z = 0;

	for (int i = 0; i < 4; i++)
	{
		m_backMirror_VERTEX[i].nrm.x = 0;
		m_backMirror_VERTEX[i].nrm.y = 0;
		m_backMirror_VERTEX[i].nrm.z = -1;
	}
	unsigned int mirror_indicies[6] = { 0, 2, 3, 0, 1, 2 };

	setVertexBuffer(m_backMirror_VERTEX, rectNumber);
	setIndexBuffer(mirror_indicies, rectIndexNumber);
}
void mirror::setData(ID3D11Device *_m_d3d11Device,float _width, float _height, float _left, float _top)
{
	m_d3d11Device = _m_d3d11Device;
	left = _left;
	top = _top;
	width = _width;
	height = _height;

	float cameraZ = 0;


	//********************m_backMirror_VERTEX	
	m_backMirror_VERTEX[0].xyz.x = left;
	m_backMirror_VERTEX[0].xyz.y = top;
	m_backMirror_VERTEX[0].xyz.z = cameraZ;
	m_backMirror_VERTEX[0].uvw.x = 0;
	m_backMirror_VERTEX[0].uvw.y = 0;
	m_backMirror_VERTEX[0].uvw.z = 0;

	m_backMirror_VERTEX[1].xyz.x = m_backMirror_VERTEX[0].xyz.x + width;
	m_backMirror_VERTEX[1].xyz.y = m_backMirror_VERTEX[0].xyz.y;
	m_backMirror_VERTEX[1].xyz.z = cameraZ;
	m_backMirror_VERTEX[1].uvw.x = 1;
	m_backMirror_VERTEX[1].uvw.y = 0;
	m_backMirror_VERTEX[1].uvw.z = 0;

	m_backMirror_VERTEX[2].xyz.x = m_backMirror_VERTEX[0].xyz.x + width;
	m_backMirror_VERTEX[2].xyz.y = m_backMirror_VERTEX[0].xyz.y - height;
	m_backMirror_VERTEX[2].xyz.z = cameraZ;
	m_backMirror_VERTEX[2].uvw.x = 1;
	m_backMirror_VERTEX[2].uvw.y = 1;
	m_backMirror_VERTEX[2].uvw.z = 0;

	m_backMirror_VERTEX[3].xyz.x = m_backMirror_VERTEX[0].xyz.x;
	m_backMirror_VERTEX[3].xyz.y = m_backMirror_VERTEX[0].xyz.y - height;
	m_backMirror_VERTEX[3].xyz.z = cameraZ;
	m_backMirror_VERTEX[3].uvw.x = 0;
	m_backMirror_VERTEX[3].uvw.y = 1;
	m_backMirror_VERTEX[3].uvw.z = 0;

	for (int i = 0; i < 4; i++)
	{
		m_backMirror_VERTEX[i].nrm.x = 0;
		m_backMirror_VERTEX[i].nrm.y = 0;
		m_backMirror_VERTEX[i].nrm.z = -1;
	}
	unsigned int mirror_indicies[6] = { 0, 2, 3, 0, 1, 2 };

	setVertexBuffer(m_backMirror_VERTEX, rectNumber);
	setIndexBuffer(mirror_indicies, rectIndexNumber);
}

void mirror::render()
{}
void mirror::setVertexBuffer(const void* _vertex_data, const int& _vertexNumber)
{
	D3D11_BUFFER_DESC m_D3D11_BUFFER_DESC;
	D3D11_SUBRESOURCE_DATA m_D3D11_SUBRESOURCE_DATA;
	ZeroMemory(&m_D3D11_SUBRESOURCE_DATA, sizeof(D3D11_SUBRESOURCE_DATA));
	ZeroMemory(&m_D3D11_BUFFER_DESC, sizeof(D3D11_BUFFER_DESC));
	m_D3D11_SUBRESOURCE_DATA.pSysMem = _vertex_data;
	m_D3D11_BUFFER_DESC.Usage = D3D11_USAGE_IMMUTABLE;                // write access access by CPU and GPU
	m_D3D11_BUFFER_DESC.ByteWidth = sizeof(SIMPLE_VERTEX)* _vertexNumber;             // size is the VERTEX struct * 3
	m_D3D11_BUFFER_DESC.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	m_D3D11_BUFFER_DESC.CPUAccessFlags = NULL;
	m_d3d11Device->CreateBuffer(&m_D3D11_BUFFER_DESC, &m_D3D11_SUBRESOURCE_DATA, &m_vertex_buffer);       // create the buffer
}
void mirror::setIndexBuffer(const void* _index_data, const int& _indexNumber)
{
	D3D11_BUFFER_DESC m_D3D11_i_BUFFER_DESC;
	ZeroMemory(&m_D3D11_i_BUFFER_DESC, sizeof(D3D11_BUFFER_DESC));
	D3D11_SUBRESOURCE_DATA m_D3D11_i_SUBRESOURCE_DATA;
	ZeroMemory(&m_D3D11_i_SUBRESOURCE_DATA, sizeof(D3D11_SUBRESOURCE_DATA));

	m_D3D11_i_SUBRESOURCE_DATA.pSysMem = _index_data;
	m_D3D11_i_BUFFER_DESC.Usage = D3D11_USAGE_IMMUTABLE;                // write access access by CPU and GPU
	m_D3D11_i_BUFFER_DESC.ByteWidth = sizeof(unsigned int) * _indexNumber;
	m_D3D11_i_BUFFER_DESC.BindFlags = D3D11_BIND_INDEX_BUFFER;       // use as a vertex buffer
	m_D3D11_i_BUFFER_DESC.CPUAccessFlags = NULL;
	m_d3d11Device->CreateBuffer(&m_D3D11_i_BUFFER_DESC, &m_D3D11_i_SUBRESOURCE_DATA, &m_index_buffer);       // create the buffer
}

