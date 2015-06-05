#include "foliage.h"
#include <time.h>

foliage::foliage()
{
	m_ID3D11Buffer_vertex = nullptr;
}


foliage::~foliage()
{
}

bool foliage::init(ID3D11Device* _ID3D11Device, ID3D11DeviceContext* _d3d11DeviceContext, WCHAR* _fileName, int _count)
{
	m_d3d11Device = _ID3D11Device;
	m_d3d11DeviceContext = _d3d11DeviceContext;
	HRESULT hr;
	m_foliageCount = _count;

	bool result = generatePosition();
	if (!result)
		return false;

	result = initializeBuffer(_ID3D11Device);
	if (!result)
		return false;

	hr = CreateDDSTextureFromFile(_ID3D11Device, _fileName, NULL, &m_SRV);

	 m_windRotation = 0;
	 m_windDirection = 1;

}

//set up the constant vertex buffer and instant buffer for vertex shader 
bool foliage::initializeBuffer(ID3D11Device* _device)
{
	VertexType* _vertexType;

	//number vertices
	m_verticeCount = 6;

	_vertexType = new VertexType[6];
	if (!_vertexType)
		return false;

	_vertexType[0].position = XMFLOAT3(0, 0, 0);
	_vertexType[0].texture = XMFLOAT2(0, 1);

	_vertexType[1].position = XMFLOAT3(0, 1, 0);
	_vertexType[1].texture = XMFLOAT2(0, 0);

	_vertexType[2].position = XMFLOAT3(1, 0, 0);
	_vertexType[2].texture = XMFLOAT2(1, 1);

	_vertexType[3].position = XMFLOAT3(1, 0, 0);
	_vertexType[3].texture = XMFLOAT2(1, 1);

	_vertexType[4].position = XMFLOAT3(0, 1, 0);
	_vertexType[4].texture = XMFLOAT2(0, 0);

	_vertexType[5].position = XMFLOAT3(1, 1, 0);
	_vertexType[5].texture = XMFLOAT2(1, 0);

	setVertexBuffer(_vertexType, m_verticeCount);

	delete[] _vertexType;

	m_instanceType = new instanceType[m_foliageCount];


	for (int i = 0; i < m_foliageCount; i++)
	{
		m_instanceType[i].matrix = XMMatrixIdentity();
		m_instanceType[i].color = XMFLOAT3(m_foliageArray[i].r, m_foliageArray[i].g, m_foliageArray[i].b);
	}

	setInstanceBuffer(m_instanceType, m_foliageCount);
	
	setVSConstantBuffer();

	return true;
}

void foliage::updateMatrix(const XMMATRIX& _viewMatrix, const XMMATRIX& _projectMatrix)
{
	m_MatrixType.viewMatrix = _viewMatrix;
	m_MatrixType.projectMatrix = _projectMatrix;
}
void foliage::setVSConstantBuffer()
{
	HRESULT hr;
	//*********foliage data***********************
	D3D11_BUFFER_DESC m_D3D11_BUFFER_DESC_foliage;
	D3D11_SUBRESOURCE_DATA m_D3D11_SUBRESOURCE_DATA_foliage;
	ZeroMemory(&m_D3D11_BUFFER_DESC_foliage, sizeof(D3D11_BUFFER_DESC));
	ZeroMemory(&m_D3D11_SUBRESOURCE_DATA_foliage, sizeof(D3D11_SUBRESOURCE_DATA));

	m_D3D11_SUBRESOURCE_DATA_foliage.pSysMem = &m_MatrixType;
	m_D3D11_BUFFER_DESC_foliage.Usage = D3D11_USAGE_DYNAMIC;
	m_D3D11_BUFFER_DESC_foliage.ByteWidth = sizeof(m_MatrixType);             // size is the toShader
	m_D3D11_BUFFER_DESC_foliage.BindFlags = D3D11_BIND_CONSTANT_BUFFER;       // use as a vertex buffer
	m_D3D11_BUFFER_DESC_foliage.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_d3d11Device->CreateBuffer(&m_D3D11_BUFFER_DESC_foliage, &m_D3D11_SUBRESOURCE_DATA_foliage, &m_matrixBuffer);
}
void foliage::Frame(XMFLOAT3 _cameraPos)
{
	XMFLOAT3 modelPos;
	float radien;
	XMMATRIX billBoardRotateY, windRotateX,xmtranslation;
	//update the wind 
	//direction 1
	if (m_windDirection == 1)
	{
		m_windRotation += 0.1;
		if (m_windRotation > 10.0f)
			m_windDirection = 2;
	}
	else
	{//direction2
		m_windRotation -= 0.1;
		if (m_windRotation < -10.0f)
			m_windDirection = 1;
	}

	//load the instance buffer with the update locations
	for (int i = 0; i < m_foliageCount; i++)
	{
		//get the position of this foliage
		modelPos.x = m_foliageArray[i].x;
		modelPos.y = -0.1f;
		modelPos.z = m_foliageArray[i].z;

		//calculate the rotation to like billboard
		radien = atan2(modelPos.x - _cameraPos.x, modelPos.z - _cameraPos.z);

		//set up the Y axis rotation
		billBoardRotateY = XMMatrixRotationY(radien);

		//set up the wind X rotation
		windRotateX = XMMatrixRotationX(m_windRotation/180.0f*3.14f);

		xmtranslation = XMMatrixTranslation(modelPos.x, modelPos.y, modelPos.z);

		//calculate the matrix
		m_instanceType[i].matrix = (billBoardRotateY*windRotateX)*xmtranslation;
	}
}

bool foliage::generatePosition()
{
	int i = 0;

	float red, green;

	m_foliageArray = new foliageType[m_foliageCount];

	if (!m_foliageArray)
		return false;

	srand((int)time(NULL));

	for (int i = 0; i < m_foliageCount; i++)
	{
		//pos
		m_foliageArray[i].x = ((float)rand() / (float)(RAND_MAX))*9.0f - 4.5f;
		m_foliageArray[i].z = ((float)rand() / (float)(RAND_MAX))*9.0f - 4.5f;
		//color
		red = ((float)rand() / (float)(RAND_MAX))*1.0f;
		green = ((float)rand() / (float)(RAND_MAX))*1.0f;
		m_foliageArray[i].r = red+1.0f;
		m_foliageArray[i].g = green + 0.5f;
		m_foliageArray[i].b = 0;
	}
	return true;
}


void foliage::setVertexBuffer(const void* _vertex_data, const int& _vertexNumber)
{
	D3D11_BUFFER_DESC m_D3D11_BUFFER_DESC;
	D3D11_SUBRESOURCE_DATA m_D3D11_SUBRESOURCE_DATA;
	ZeroMemory(&m_D3D11_SUBRESOURCE_DATA, sizeof(D3D11_SUBRESOURCE_DATA));
	ZeroMemory(&m_D3D11_BUFFER_DESC, sizeof(D3D11_BUFFER_DESC));
	m_D3D11_SUBRESOURCE_DATA.pSysMem = _vertex_data;
	m_D3D11_BUFFER_DESC.Usage = D3D11_USAGE_IMMUTABLE;                // write access access by CPU and GPU
	m_D3D11_BUFFER_DESC.ByteWidth = sizeof(VertexType)* _vertexNumber;             // size is the VERTEX struct * 3
	m_D3D11_BUFFER_DESC.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	m_D3D11_BUFFER_DESC.CPUAccessFlags = NULL;
	m_d3d11Device->CreateBuffer(&m_D3D11_BUFFER_DESC, &m_D3D11_SUBRESOURCE_DATA, &m_ID3D11Buffer_vertex);       // create the buffer
}
void foliage::setInstanceBuffer(const void* m_instanceType, const int& _instanceCount)
{
	D3D11_BUFFER_DESC m_D3D11_i_BUFFER_DESC;
	ZeroMemory(&m_D3D11_i_BUFFER_DESC, sizeof(D3D11_BUFFER_DESC));
	D3D11_SUBRESOURCE_DATA m_D3D11_i_SUBRESOURCE_DATA;
	ZeroMemory(&m_D3D11_i_SUBRESOURCE_DATA, sizeof(D3D11_SUBRESOURCE_DATA));

	m_D3D11_i_SUBRESOURCE_DATA.pSysMem = m_instanceType;
	m_D3D11_i_BUFFER_DESC.Usage = D3D11_USAGE_DYNAMIC;               
	m_D3D11_i_BUFFER_DESC.ByteWidth = sizeof(unsigned int) * _instanceCount;
	m_D3D11_i_BUFFER_DESC.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_D3D11_i_BUFFER_DESC.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_d3d11Device->CreateBuffer(&m_D3D11_i_BUFFER_DESC, &m_D3D11_i_SUBRESOURCE_DATA, &m_instanceBuffer);       // create the buffer
}
void foliage::Release()
{
	
	SAFE_RELEASE(m_ID3D11Buffer_vertex );
	SAFE_RELEASE(m_instanceBuffer );
	SAFE_RELEASE(m_SRV);
	SAFE_RELEASE(m_matrixBuffer);
	delete []m_instanceType;

	delete[]m_foliageArray;
}

void foliage::renderBuffer()
{
	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPt[2];

	//set the buffer strides
	strides[0] = sizeof(VertexType);
	strides[1] = sizeof(instanceType);

	//set the offset to both buffer
	offsets[0] = 0;
	offsets[1] = 0;

	//set the array buffer to both buffer
	bufferPt[0] = m_ID3D11Buffer_vertex;
	bufferPt[1] = m_instanceBuffer;

	//use device context to call
	m_d3d11DeviceContext->IASetVertexBuffers(0, 2, bufferPt, strides, offsets);

	//set the type
	m_d3d11DeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void foliage::bindVSConstantBuffer()
{
	
	D3D11_MAPPED_SUBRESOURCE ms2;
	ZeroMemory(&ms2, sizeof(D3D11_MAPPED_SUBRESOURCE));
	m_d3d11DeviceContext->Map(m_matrixBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
	memcpy(ms2.pData, &m_MatrixType, sizeof(m_MatrixType));                // copy the data
	m_d3d11DeviceContext->Unmap(m_matrixBuffer, NULL);
}

void foliage::bindInstanceBuffer()
{
	D3D11_MAPPED_SUBRESOURCE ms2;
	ZeroMemory(&ms2, sizeof(D3D11_MAPPED_SUBRESOURCE));
	m_d3d11DeviceContext->Map(m_instanceBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms2);   // map the buffer
	memcpy(ms2.pData, m_instanceType, sizeof(instanceType)*m_foliageCount);                // copy the data
	m_d3d11DeviceContext->Unmap(m_instanceBuffer, NULL);                                     // unmap the buffer
}

void foliage::Render(ID3D11VertexShader * _vs, ID3D11PixelShader *_ps, ID3D11InputLayout * _input, ID3D11SamplerState* m_sampleState)
{
	bindInstanceBuffer();

	bindVSConstantBuffer();

	//the vertex and constant buffer should be ready by this function
	renderBuffer();

	unsigned int bufferNumber = 0;

	m_d3d11DeviceContext->VSSetShader(_vs, 0, 0);
	m_d3d11DeviceContext->HSSetShader(NULL, 0, 0);
	m_d3d11DeviceContext->DSSetShader(NULL, 0, 0);
	m_d3d11DeviceContext->GSSetShader(NULL, 0, 0);
	m_d3d11DeviceContext->PSSetShader(_ps, 0, 0);
	m_d3d11DeviceContext->IASetInputLayout(_input);
	m_d3d11DeviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);
	m_d3d11DeviceContext->PSSetShaderResources(0, 1, &m_SRV);
	m_d3d11DeviceContext->PSSetSamplers(0, 1, &m_sampleState);
	m_d3d11DeviceContext->DrawInstanced(m_verticeCount, m_foliageCount, 0, 0);
}