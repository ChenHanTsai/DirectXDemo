#include "camera.h"

#define stepSpeed 0.1f

camera::camera()
{
	originPos.x = 0;
	originPos.y = 3;
	originPos.z = -10;
	m_counter++;

	
}


camera::~camera()
{
}
void camera::Release()
{
	m_counter--;
	if (m_counter == 0)
		delete this;
}
void camera::input(float dt)
{
	POINT  mousePos;
	/*float rotateDeltaX;
	float rotateDeltaY;*/
	//dt = 1;
	m_deltaX = 0;
	m_deltaY = 0;
	m_deltaZ = 0;
	if (GetAsyncKeyState('W') & 1)
	{
		m_deltaZ += dt*stepSpeed;
	//	m_deltaZ += dt*stepSpeed;		
		//m_deltaZ += cosf(m_rotateDeltaX)*stepSpeed;
		//m_deltaY += sinf(m_rotateDeltaX)*stepSpeed;
	}
	if (GetAsyncKeyState('S') & 1)
	{
		m_deltaZ -= dt*stepSpeed;
		//m_deltaZ -= dt*stepSpeed;
	}
	if (GetAsyncKeyState('A') & 1)
	{
		m_deltaX -= dt*stepSpeed;
		//m_deltaX -= dt*stepSpeed;
	}
	if (GetAsyncKeyState('D') & 1)
	{
		m_deltaX += dt*stepSpeed;
		//m_deltaX += dt*stepSpeed;
	}
	if (GetAsyncKeyState('R') & 1)
	{
		m_deltaY += dt*stepSpeed;
	}
	if (GetAsyncKeyState('F') & 1)
	{
		m_deltaY -= dt*stepSpeed;
	}

	if (GetAsyncKeyState(VK_LBUTTON) )
	{
		// Hide the mouse pointer
		SetCursor(NULL);

		//get the cursor position
		GetCursorPos(&mousePos);

		//calculate the rotate distance
		float pi = 3.14f;
		m_rotateDeltaX += (float)(mousePos.x - m_PreCursorPos.x) / 300.0f;
		if (m_rotateDeltaX >= pi/2)
			m_rotateDeltaX = pi / 2;
		m_rotateDeltaY += (float)(mousePos.y - m_PreCursorPos.y) / 300.0f;
		if (m_rotateDeltaY >= pi / 2)
			m_rotateDeltaY = pi / 2;
		//reset our mouse back to the initial pos
		SetCursorPos(m_PreCursorPos.x, m_PreCursorPos.y);


	}
}

void camera::update()
{
	m_rotationX = XMMatrixRotationX(m_rotateDeltaY);
	m_rotationY = XMMatrixRotationY(m_rotateDeltaX);
	m_rotateMatrix = m_rotationX*m_rotationY;
	dir = XMFLOAT3(m_rotateMatrix.r->m128_f32[8], m_rotateMatrix.r->m128_f32[9], m_rotateMatrix.r->m128_f32[10]);

	originPos.x += dir.x*m_deltaZ;
	originPos.y += dir.y*m_deltaZ;
	originPos.z += dir.z*m_deltaZ;

	dir = XMFLOAT3(m_rotateMatrix.r->m128_f32[4], m_rotateMatrix.r->m128_f32[5], m_rotateMatrix.r->m128_f32[6]);
	originPos.x += dir.x*m_deltaY;
	originPos.y += dir.y*m_deltaY;
	originPos.z += dir.z*m_deltaY;

	dir = XMFLOAT3(m_rotateMatrix.r->m128_f32[0], m_rotateMatrix.r->m128_f32[1], m_rotateMatrix.r->m128_f32[2]);
	originPos.x += dir.x*m_deltaX;
	originPos.y += dir.y*m_deltaX;
	originPos.z += dir.z*m_deltaX;

	/*eyePos[0].x += dir.x*2;
	eyePos[1].x -= dir.x*2;*/
	m_translation = XMMatrixTranslation(originPos.x, originPos.y, originPos.z);
	m_MultiplyMatrix = m_rotateMatrix*m_translation;
	
	//calculate mirror right
	float Xadjust = 5.0f;
	float Zadjust = 5.0f;
	m_translation = XMMatrixTranslation(originPos.x - Xadjust, originPos.y + 0.8f, originPos.z - Zadjust);
	m_mirrorRightViewMatrix = (m_rotateYAxisBack*m_rotateMatrix)*m_translation;
	m_mirrorRightViewMatrix = XMMatrixInverse(NULL, m_mirrorRightViewMatrix);

	//calculate mirror left
	m_translation = XMMatrixTranslation(originPos.x + Xadjust, originPos.y + 0.8f, originPos.z - Zadjust);
	m_mirrorLeftViewMatrix = (m_rotateYAxisBack*m_rotateMatrix)*m_translation;
	m_mirrorLeftViewMatrix = XMMatrixInverse(NULL, m_mirrorLeftViewMatrix);

	//calculate god camera pos
	m_liftYAxisGod = XMMatrixTranslation(originPos.x, 10.0, originPos.z);
	m_godCamera = m_rotateXAxisGod*m_liftYAxisGod;
	//m_godCamera.r->m128_f32[12] = originPos.x;
	//m_godCamera.r->m128_f32[13] = 0;
	//m_godCamera.r->m128_f32[14] = originPos.z;
	m_godCamera = XMMatrixInverse(NULL, m_godCamera);



	m_viewMatrix = XMMatrixInverse(NULL,m_MultiplyMatrix);
}

void camera::SetPreMousePos()
{
	GetCursorPos(&m_PreCursorPos);
}
void camera::getViewMatrix(XMMATRIX& m_XMMATRIX)
{
	
	m_XMMATRIX = m_viewMatrix;
}

void camera::getViewMatrixBack(XMMATRIX& m_XMMATRIX)
{
	//XMMATRIX m_rotateYAxisBack = XMMatrixRotationY(90.0f / 180 * 3.14f);
//	m_XMMATRIX = m_viewMatrix*m_rotateYAxisBack;
}

void camera::getMultiplyMatrix(XMMATRIX& m_XMMATRIX)
{
	m_XMMATRIX = m_MultiplyMatrix;
}

void camera::getCameraPos(XMFLOAT3& _xmfloat3)
{
	_xmfloat3.x = originPos.x;
	_xmfloat3.y = originPos.y;
	_xmfloat3.z = originPos.z;
}

XMFLOAT3 camera::returnCameraPos()const
{
	return originPos;
}

void camera::getEyePosLeft(XMFLOAT3& _xmfloat3)
{
	_xmfloat3.x = originPos.x - m_viewMatrix.r->m128_f32[0]*2;
	_xmfloat3.y = originPos.y - m_viewMatrix.r->m128_f32[1] * 2;
	_xmfloat3.z = originPos.z - m_viewMatrix.r->m128_f32[2] * 2;
}

void camera::getEyePosRight(XMFLOAT3& _xmfloat3)
{
	_xmfloat3.x = originPos.x + m_viewMatrix.r->m128_f32[0] * 2;
	_xmfloat3.y = originPos.y + m_viewMatrix.r->m128_f32[1] * 2;
	_xmfloat3.z = originPos.z + m_viewMatrix.r->m128_f32[2] * 2;
}

void camera::getMirrorLeft(XMMATRIX& m_XMMATRIX)
{
	m_XMMATRIX = m_mirrorLeftViewMatrix;
}


void camera::getMirrorRight(XMMATRIX& m_XMMATRIX)
{
	m_XMMATRIX = m_mirrorRightViewMatrix;
}


void camera::getGodView(XMMATRIX& m_XMMATRIX)
{
	m_XMMATRIX = m_godCamera;
}
