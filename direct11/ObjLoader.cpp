#include "ObjLoader.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include "DDSTextureLoader.h"

ObjLoader::ObjLoader(ID3D11DeviceContext *_m_d3d11DeviceContext)
{
	m_d3d11DeviceContext = _m_d3d11DeviceContext;
	m_SRV[0] = 0;
	m_SRV[1] = 0;
	m_SRV[2] = 0;
}


ObjLoader::~ObjLoader()
{
	SAFE_RELEASE(m_ID3D11Buffer_vertex);
	SAFE_RELEASE(m_ID3D11_index_Buffer);
	SAFE_RELEASE(m_SRV[0]);
	SAFE_RELEASE(m_SRV[1]);
	SAFE_RELEASE(m_SRV[2]);
	SAFE_RELEASE(m_instanceBuffer);

	delete[] m_vertex_Obj_Data;
	delete[]  m_index_Obj_Data;
}

void ObjLoader::LoadFromFile(char* fileName, wchar_t* _TexturefileName, wchar_t* _TexturefileName2, wchar_t* _fileMapName, ID3D11Device *_d3d11Device)
{
	
	m_d3d11Device = _d3d11Device;
	m_fileTextureName = _TexturefileName;
	m_fileTextureName2 = _TexturefileName2;
	m_fileMapName = _fileMapName;
	std::fstream fin;
	char fileData;
	char tempChar;
//	int number;
	XMFLOAT3 temp;
	faceStrcture tempFace;
	indexStrcture tempIndex;
	//float zero = 0;
	fin.open(fileName,std::ios_base::in);

	
	if (fin.is_open())
	{
	fin.get(fileData);

		//counter the counter
		while (!fin.eof())
		{
			fin.get(fileData);
			if (fileData == 'v')
			{
				//read next char
				fin.get(fileData);

				switch (fileData)
				{
				case ' ':
				
					fin >> temp.x >> temp.y >> temp.z;
					m_posData.push_back(temp);
				//	m_vertexCounter++;
					break;
				case 't':
					temp.z = 0;
					fin >> temp.x >> temp.y;// >> temp.z;
					m_textureData.push_back(temp);
					//m_textureCounter++;
					break;
				case 'n':
					fin >> temp.x  >> temp.y >> temp.z;
					m_normalData.push_back(temp);
				//	m_normalCounter++;
					break;
				default:
					break;
				};
				
			}

			if (fileData == 'f')
			{
				for (int i = 0; i < 3; i++)
				{
					fin >> tempFace.m_indexStrcture[i].pos >> tempChar 
						>> tempFace.m_indexStrcture[i].uvw >> tempChar >> tempFace.m_indexStrcture[i].nrm;			
				}
				m_faceData.push_back(tempFace);
			
			}

			//next
			//fin.get(fileData);
			fin.ignore(INT_MAX, '\n');

			// Read in the remainder of the line.
			//while (fileData != '\n')
			//{
			//	fin.get(fileData);
			//}

			//// Start reading the beginning of the next line.
			//fin.get(fileData);
		/*	fopenLab6.ignore(INT_MAX, '\n');
			fin.get(fileData);*/
		}
		m_faceData.pop_back();
		fin.close();
	}
	else
	{
		assert(0);
	}


	makeVertexData();

}

void ObjLoader::makeVertexData()
{
	
	vertexStrcture tempVertexStrcture;
	XMFLOAT3 tangent, binormal, normal;

	unsigned int posIndex = 0;
	unsigned int uvwIndex = 0;
	unsigned int nrmIndex = 0;

	for (int faceIndex = 0; faceIndex < m_faceData.size(); faceIndex++)
	{
		

		for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++)
		{
			posIndex = m_faceData[faceIndex].m_indexStrcture[vertexIndex].pos-1;
			uvwIndex = m_faceData[faceIndex].m_indexStrcture[vertexIndex].uvw-1;
			nrmIndex = m_faceData[faceIndex].m_indexStrcture[vertexIndex].nrm-1;

			//check three index here
			int tempIndex = 0;
			bool findRepeat = checkThreeIndex(posIndex, uvwIndex, nrmIndex, tempIndex);
			
			if (findRepeat == false)
			{
				tempVertexStrcture.pos.x = m_posData[posIndex].x;
				tempVertexStrcture.pos.y = m_posData[posIndex].y;
				tempVertexStrcture.pos.z = m_posData[posIndex].z;

				tempVertexStrcture.uvw.x = m_textureData[uvwIndex].x;
				tempVertexStrcture.uvw.y = 1.0f-m_textureData[uvwIndex].y;
				tempVertexStrcture.uvw.z = m_textureData[uvwIndex].z;

				tempVertexStrcture.nrm.x = m_normalData[nrmIndex].x;
				tempVertexStrcture.nrm.y = m_normalData[nrmIndex].y;
				tempVertexStrcture.nrm.z = m_normalData[nrmIndex].z;


				m_vertex_data.push_back(tempVertexStrcture);

				m_indices_data.push_back(tempIndex);

				//3 right now, calculate normal
				if (vertexIndex == 2 && m_fileMapName!=nullptr)
				{
					int i = m_vertex_data.size() ;
					calculateTangentBinormal(m_vertex_data[i-3], m_vertex_data[i -2 ], m_vertex_data[i - 1],
						tangent, binormal);

					calculateNormal(tangent, binormal, normal);

					m_vertex_data[i - 1].nrm = normal;
					m_vertex_data[i - 1].tangent = tangent;
					m_vertex_data[i - 1].binormal = binormal;

					m_vertex_data[i -2].nrm = normal;
					m_vertex_data[i -2].tangent = tangent;
					m_vertex_data[i -2].binormal = binormal;

					m_vertex_data[i-3].nrm = normal;
					m_vertex_data[i-3].tangent = tangent;
					m_vertex_data[i-3].binormal = binormal;
				}
			}
			else
			{
				m_indices_data.push_back(tempIndex);
			}		
		}
	
	
	}
	
	//make continouse data
	m_vertex_Obj_Data = new vertexStrcture[m_vertex_data.size()];
	for (int i = 0; i < m_vertex_data.size(); i++)
	{
		m_vertex_Obj_Data[i].pos = m_vertex_data[i].pos;
		m_vertex_Obj_Data[i].uvw = m_vertex_data[i].uvw;
		m_vertex_Obj_Data[i].nrm = m_vertex_data[i].nrm;

		if (m_fileMapName != nullptr)
		{
			m_vertex_Obj_Data[i].tangent = m_vertex_data[i].tangent;
			m_vertex_Obj_Data[i].binormal = m_vertex_data[i].binormal;
		}
	
	}

	//make continouse data
	m_index_Obj_Data = new unsigned int[m_indices_data.size()];
	for (int i = 0; i < m_indices_data.size(); i++)
	{
		m_index_Obj_Data[i] = m_indices_data[i];
	}

}

bool ObjLoader::checkThreeIndex(int _posIndex, int _uvwIndex, int _nrmIndex,int& indexNumber)
{
	int counterIndex = 0;
	for (int i = 0; i < m_threeIndexData.size(); i++)
	{
		if (m_threeIndexData[i].m_int[0] == _posIndex && m_threeIndexData[i].m_int[1] == _uvwIndex && m_threeIndexData[i].m_int[2] == _nrmIndex)
		{
			//find it
			indexNumber = counterIndex ;
			return true;
		}
		counterIndex++;
	}
	
	int3 tempInt3;
	tempInt3.m_int[0] = _posIndex;
	tempInt3.m_int[1] = _uvwIndex;
	tempInt3.m_int[2] = _nrmIndex;
	m_threeIndexData.push_back(tempInt3);
	//not find repeat
	indexNumber = m_threeIndexData.size() - 1;
	return false;
}

void ObjLoader::setMeshInstance()
{
	HRESULT hr;

	//*************************data************************
	D3D11_BUFFER_DESC m_D3D11_BUFFER_DESC_line;
	D3D11_SUBRESOURCE_DATA m_D3D11_SUBRESOURCE_DATA_line;
	ZeroMemory(&m_D3D11_SUBRESOURCE_DATA_line, sizeof(D3D11_SUBRESOURCE_DATA));
	ZeroMemory(&m_D3D11_BUFFER_DESC_line, sizeof(D3D11_BUFFER_DESC));

	m_D3D11_SUBRESOURCE_DATA_line.pSysMem = m_vertex_Obj_Data;
	m_D3D11_BUFFER_DESC_line.Usage = D3D11_USAGE_IMMUTABLE;                // write access access by CPU and GPU
	m_D3D11_BUFFER_DESC_line.ByteWidth = sizeof(vertexStrcture)* m_vertex_data.size();             // size is the VERTEX struct * 3
	m_D3D11_BUFFER_DESC_line.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	m_D3D11_BUFFER_DESC_line.CPUAccessFlags = NULL;
	m_d3d11Device->CreateBuffer(&m_D3D11_BUFFER_DESC_line, &m_D3D11_SUBRESOURCE_DATA_line, &m_ID3D11Buffer_vertex);       // create the buffer

	//************************* instance************************
	InstanceType* _instance = new InstanceType[m_instanceNumber];
	for (int i = 0; i < m_instanceNumber; i++)
	{
		_instance[i].pos.x = 0;
		_instance[i].pos.y = 0;
		_instance[i].pos.z = 0;

		_instance[i].pos.z += i*5.0f;
	}

	D3D11_BUFFER_DESC m_D3D11_i_BUFFER_DESC_instance;
	ZeroMemory(&m_D3D11_i_BUFFER_DESC_instance, sizeof(D3D11_BUFFER_DESC));
	D3D11_SUBRESOURCE_DATA m_D3D11_i_SUBRESOURCE_DATA_Instance;
	ZeroMemory(&m_D3D11_i_SUBRESOURCE_DATA_Instance, sizeof(D3D11_SUBRESOURCE_DATA));

	m_D3D11_i_SUBRESOURCE_DATA_Instance.pSysMem = _instance;
	m_D3D11_i_BUFFER_DESC_instance.Usage = D3D11_USAGE_IMMUTABLE;                // write access access by CPU and GPU
	m_D3D11_i_BUFFER_DESC_instance.ByteWidth = sizeof(InstanceType) *m_instanceNumber;
	m_D3D11_i_BUFFER_DESC_instance.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	m_D3D11_i_BUFFER_DESC_instance.CPUAccessFlags = NULL;
	m_d3d11Device->CreateBuffer(&m_D3D11_i_BUFFER_DESC_instance, &m_D3D11_i_SUBRESOURCE_DATA_Instance, &m_instanceBuffer);       // create the buffer

	//************************* m_indices************************
	D3D11_BUFFER_DESC m_D3D11_i_BUFFER_DESC_line;
	ZeroMemory(&m_D3D11_i_BUFFER_DESC_line, sizeof(D3D11_BUFFER_DESC));
	D3D11_SUBRESOURCE_DATA m_D3D11_i_SUBRESOURCE_DATA_line;
	ZeroMemory(&m_D3D11_i_SUBRESOURCE_DATA_line, sizeof(D3D11_SUBRESOURCE_DATA));

	m_D3D11_i_SUBRESOURCE_DATA_line.pSysMem = m_index_Obj_Data;
	m_D3D11_i_BUFFER_DESC_line.Usage = D3D11_USAGE_IMMUTABLE;                // write access access by CPU and GPU
	m_D3D11_i_BUFFER_DESC_line.ByteWidth = sizeof(unsigned int) * m_indices_data.size();
	m_D3D11_i_BUFFER_DESC_line.BindFlags = D3D11_BIND_INDEX_BUFFER;       // use as a vertex buffer
	m_D3D11_i_BUFFER_DESC_line.CPUAccessFlags = NULL;
	m_d3d11Device->CreateBuffer(&m_D3D11_i_BUFFER_DESC_line, &m_D3D11_i_SUBRESOURCE_DATA_line, &m_ID3D11_index_Buffer);       // create the buffer


	delete[] _instance;

	setTexture();

}
void ObjLoader::setTexture()
{
	HRESULT hr;
	if (m_fileTextureName != nullptr)
		hr = CreateDDSTextureFromFile(m_d3d11Device, m_fileTextureName, NULL, &m_SRV[0]);

	if (m_fileTextureName2 != nullptr)
		hr = CreateDDSTextureFromFile(m_d3d11Device, m_fileTextureName2, NULL, &m_SRV[1]);
	else
		hr = CreateDDSTextureFromFile(m_d3d11Device, m_fileTextureName, NULL, &m_SRV[1]);

	if (m_fileMapName != nullptr)
		hr = CreateDDSTextureFromFile(m_d3d11Device, m_fileMapName, NULL, &m_SRV[2]);
}
void ObjLoader::setInstanceBuffer()
{
	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPt[2];

	//set the buffer strides
	strides[0] = sizeof(vertexStrcture);
	strides[1] = sizeof(InstanceType);

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
void ObjLoader::setMeshNotInstance()
{
	HRESULT hr;
	//*************************data************************
	D3D11_BUFFER_DESC m_D3D11_BUFFER_DESC_line;
	D3D11_SUBRESOURCE_DATA m_D3D11_SUBRESOURCE_DATA_line;
	ZeroMemory(&m_D3D11_SUBRESOURCE_DATA_line, sizeof(D3D11_SUBRESOURCE_DATA));
	ZeroMemory(&m_D3D11_BUFFER_DESC_line, sizeof(D3D11_BUFFER_DESC));

	m_D3D11_SUBRESOURCE_DATA_line.pSysMem = m_vertex_Obj_Data;
	m_D3D11_BUFFER_DESC_line.Usage = D3D11_USAGE_IMMUTABLE;                // write access access by CPU and GPU
	m_D3D11_BUFFER_DESC_line.ByteWidth = sizeof(vertexStrcture)* m_vertex_data.size();             // size is the VERTEX struct * 3
	m_D3D11_BUFFER_DESC_line.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	m_D3D11_BUFFER_DESC_line.CPUAccessFlags = NULL;
	m_d3d11Device->CreateBuffer(&m_D3D11_BUFFER_DESC_line, &m_D3D11_SUBRESOURCE_DATA_line, &m_ID3D11Buffer_vertex);       // create the buffer
	//************************* m_indices************************

	D3D11_BUFFER_DESC m_D3D11_i_BUFFER_DESC_line;
	ZeroMemory(&m_D3D11_i_BUFFER_DESC_line, sizeof(D3D11_BUFFER_DESC));
	D3D11_SUBRESOURCE_DATA m_D3D11_i_SUBRESOURCE_DATA_line;
	ZeroMemory(&m_D3D11_i_SUBRESOURCE_DATA_line, sizeof(D3D11_SUBRESOURCE_DATA));

	m_D3D11_i_SUBRESOURCE_DATA_line.pSysMem = m_index_Obj_Data;
	m_D3D11_i_BUFFER_DESC_line.Usage = D3D11_USAGE_IMMUTABLE;                // write access access by CPU and GPU
	m_D3D11_i_BUFFER_DESC_line.ByteWidth = sizeof(unsigned int) * m_indices_data.size();
	m_D3D11_i_BUFFER_DESC_line.BindFlags = D3D11_BIND_INDEX_BUFFER;       // use as a vertex buffer
	m_D3D11_i_BUFFER_DESC_line.CPUAccessFlags = NULL;
	m_d3d11Device->CreateBuffer(&m_D3D11_i_BUFFER_DESC_line, &m_D3D11_i_SUBRESOURCE_DATA_line, &m_ID3D11_index_Buffer);       // create the buffer

	//*************************texture ************************
	setTexture();
}


void ObjLoader::calculateTangentBinormal(vertexStrcture _v1, vertexStrcture _v2, vertexStrcture _v3, XMFLOAT3& _tangent, XMFLOAT3& _binormal)
{
	XMFLOAT3 vector1, vector2;
	XMFLOAT2 tu, tv;
	float den;
	float length;

	//calculate the two vector
	vector1.x = _v2.pos.x - _v1.pos.x;
	vector1.y = _v2.pos.y - _v1.pos.y;
	vector1.z = _v2.pos.z - _v1.pos.z;

	vector2.x = _v3.pos.x - _v1.pos.x;
	vector2.y = _v3.pos.y - _v1.pos.y;
	vector2.z = _v3.pos.z - _v1.pos.z;
	//calculate the tu,tv
	tu.x = _v2.uvw.x - _v1.uvw.x;
	tv.x = _v2.uvw.y - _v1.uvw.y;
	
	tu.y = _v3.uvw.x - _v1.uvw.x;
	tv.y = _v3.uvw.y - _v1.uvw.y;

	//calculate the denominator of the tangent/binormal equation
	den = 1 / (tu.x*tv.y-tu.y*tv.x);

	_tangent.x = (tv.y*vector1.x - tv.x*vector2.x)*den;
	_tangent.y = (tv.y*vector1.y - tv.x*vector2.y)*den;
	_tangent.z = (tv.y*vector1.z - tv.x*vector2.z)*den;

	_binormal.x = (tu.x*vector2.x - tu.y*vector1.x)*den;
	_binormal.y = (tu.x*vector2.y - tu.y*vector1.y)*den;
	_binormal.z = (tu.x*vector2.z - tu.y*vector1.z)*den;

	//normalize the _tangent
	length = sqrtf(_tangent.x*_tangent.x + _tangent.y*_tangent.y + _tangent.z*_tangent.z);
	_tangent.x = _tangent.x / length;
	_tangent.y = _tangent.y / length;
	_tangent.z = _tangent.z / length;

	//normalize the _binormal
	length = sqrtf(_binormal.x*_binormal.x + _binormal.y*_binormal.y + _binormal.z*_binormal.z);
	_binormal.x = _binormal.x / length;
	_binormal.y = _binormal.y / length;
	_binormal.z = _binormal.z / length;
}
void ObjLoader::calculateNormal(XMFLOAT3 _tangent, XMFLOAT3 _binormal, XMFLOAT3& _normal)
{
	float length;

	XMVECTOR tangent = XMLoadFloat3(&_tangent);
	XMVECTOR binormal = XMLoadFloat3(&_binormal);
	//XMVECTOR result = XMVector3Cross(tangent, binormal);
	XMVECTOR result = XMVector3Cross(binormal, tangent);
	result = XMVector3Normalize(result);
	_normal = XMFLOAT3(result.m128_f32[0], result.m128_f32[1], result.m128_f32[2]);

	//_normal.x = _tangent.y*_binormal.z - _tangent.z*_binormal.y;

}