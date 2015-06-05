#pragma once
#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")
#include <DirectXMath.h>
using namespace DirectX;
//#include "XTime.h"
class camera
{
private:
//	XTime m_Xtime;
	XMMATRIX m_translation = XMMatrixIdentity();

	XMMATRIX m_rotationX = XMMatrixIdentity();
	XMMATRIX m_rotationY = XMMatrixIdentity();

	XMMATRIX m_rotateMatrix;
	XMMATRIX m_MultiplyMatrix;
	XMMATRIX m_viewMatrix;


	XMMATRIX m_mirrorRightViewMatrix;
	XMMATRIX m_mirrorLeftViewMatrix;
	
	XMVECTOR m_lookVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	
	//rotate y 180
	XMMATRIX m_rotateYAxisBack = XMMatrixRotationY(180.0f / 180 * 3.14f);

	//rotate x 90
	XMMATRIX m_rotateXAxisGod = XMMatrixRotationX(45.0f / 180 * 3.14f);
	XMMATRIX m_liftYAxisGod = XMMatrixTranslation(0,30.0,0);
	XMMATRIX m_godCamera;

	int m_counter = 0;
	float m_deltaX = 0;
	float m_deltaY = 0;
	float m_deltaZ = 0;

	float m_rotateDeltaX = 0;
	float m_rotateDeltaY = 0;

	XMFLOAT3 originPos;

	XMFLOAT3 eyePos[2];
	XMFLOAT3 dir;
	//previous mouse position
	POINT                   m_PreCursorPos;     
	
public:
	void getGodView(XMMATRIX& m_XMMATRIX);

	void getMirrorRight(XMMATRIX& m_XMMATRIX);
	void getMirrorLeft(XMMATRIX& m_XMMATRIX);

	void getViewMatrixBack(XMMATRIX& m_XMMATRIX);

	void getMultiplyMatrix(XMMATRIX& m_XMMATRIX);

	void getEyePosLeft(XMFLOAT3& _left);


	 void getEyePosRight(XMFLOAT3& _right);

	void SetOriginPos(XMFLOAT3 _pos)
	{
		originPos = _pos;
	}
	void SetPreMousePos();
	void getViewMatrix(XMMATRIX& m_XMMATRIX);
	void Release();
	void input(float dt);
	void update();

	void getCameraPos(XMFLOAT3& _xmfloat3);
	XMFLOAT3 returnCameraPos()const;
	camera();
	~camera();
};

