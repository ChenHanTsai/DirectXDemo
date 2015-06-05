#pragma once
#include <vector>
#include <DirectXMath.h>
#include <d3d11.h>
#pragma comment (lib,"d3d11.lib")
using namespace DirectX;

#define SAFE_RELEASE(p){if(p){p->Release();p=NULL;}}
class ObjLoader
{
	struct InstanceType
	{
		XMFLOAT3 pos;

	};

	struct vertexStrcture
	{
		XMFLOAT3 pos;
		XMFLOAT3 uvw;
		XMFLOAT3 nrm;

		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
	};
	
	struct indexStrcture
	{
		unsigned int pos = 0;
		unsigned int uvw = 0;
		unsigned int nrm = 0;
	};

	struct faceStrcture
	{
		indexStrcture m_indexStrcture[3];
	};

	struct int3
	{
		unsigned int m_int[3];
			 
	};

	
	
private:
	std::vector<int3> m_threeIndexData;
	
	//1. read obj file data
	std::vector<XMFLOAT3> m_posData;
	std::vector<XMFLOAT3> m_textureData;
	std::vector<XMFLOAT3> m_normalData;
	std::vector<faceStrcture> m_faceData;

	//2.middle data
	std::vector<vertexStrcture> m_vertex_data;
	std::vector<unsigned int> m_indices_data;

	//3.continouse data
	vertexStrcture* m_vertex_Obj_Data;
	unsigned int* m_index_Obj_Data;
	


	ID3D11ShaderResourceView* m_SRV[3];// = nullptr;
	ID3D11Buffer *m_ID3D11Buffer_vertex = nullptr;
	ID3D11Buffer *m_ID3D11_index_Buffer = nullptr;
	ID3D11Buffer *m_instanceBuffer = nullptr;
	
	wchar_t* m_fileTextureName = nullptr;
	wchar_t* m_fileTextureName2 = nullptr;
	wchar_t* m_fileMapName = nullptr;
	ID3D11Device *m_d3d11Device;
	ID3D11DeviceContext *m_d3d11DeviceContext;
	
	int m_instanceNumber = 20;
public:

	void setTexture();
	ObjLoader(ID3D11DeviceContext *_m_d3d11DeviceContext);

	void setMeshNotInstance();
	void setMeshInstance();
	void setDeviceContext(ID3D11DeviceContext *_m_d3d11DeviceContext)
	{
		m_d3d11DeviceContext = _m_d3d11DeviceContext;
	}

	void setInstanceBuffer();
	 int get_InstanceNumber()const
	{
		return m_instanceNumber;
	}

	
	ID3D11ShaderResourceView** get_SRV()
	{
		return m_SRV;
	}

	ID3D11Buffer* get_Vertex_Buffer()
	{
		return m_ID3D11Buffer_vertex;
	}

	ID3D11Buffer* get_Index_Buffer()
	{
		return m_ID3D11_index_Buffer;
	}
	
	vertexStrcture* get_VertexObj()const
	{
		return m_vertex_Obj_Data;
	}


	unsigned int* get_IndexObj()const
	{
		return m_index_Obj_Data;
	}


	unsigned int getIndicesNumber()
	{
		return m_indices_data.size();
	}

	unsigned int getVertexNumber()
	{
		return m_vertex_data.size();
	}

	void LoadFromFile(char* fileName, wchar_t* _TexturefileName, wchar_t* _TexturefileName2, wchar_t* _fileMapName, ID3D11Device *_d3d11Device);
	void makeVertexData();
	bool checkThreeIndex(int, int, int,  int&);
	ObjLoader()
	{}
	~ObjLoader();

	void calculateTangentBinormal(vertexStrcture _v1, vertexStrcture _v2, vertexStrcture _v3, XMFLOAT3& _tangent, XMFLOAT3& _binormal);
	void calculateNormal(XMFLOAT3 _tangent, XMFLOAT3 _binormal, XMFLOAT3& _normal);
};

