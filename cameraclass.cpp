////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "cameraclass.h"


CameraClass::CameraClass()
{
	m_position.x = 0.0f;
	m_position.y = 0.0f;
	m_position.z = 0.0f;

	m_rotation.x = 0.0f;
	m_rotation.y = 0.0f;
	m_rotation.z = 0.0f;

	m_yaw = 0.0f;
	m_pitch = 0.0f;
	m_forward = XMFLOAT3(0, 0, 1);
	m_right = XMFLOAT3(1, 0, 0);
	m_up = XMFLOAT3(0, 1, 0);
}


CameraClass::CameraClass(const CameraClass& other)
{
}


CameraClass::~CameraClass()
{
}


void CameraClass::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}


void CameraClass::SetRotation(float x, float y, float z)
{
	m_pitch = x;
	m_yaw = y;
	// roll(z)는 안 쓰지만 일단 저장
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
}


XMFLOAT3 CameraClass::GetPosition()
{
	return m_position;
}


XMFLOAT3 CameraClass::GetRotation()
{
	return m_rotation;
}

//프리카메라
void CameraClass::AdjustYaw(float delta) {
	m_yaw += delta;
}
void CameraClass::AdjustPitch(float delta) {
	m_pitch += delta;
	if (m_pitch > XM_PIDIV2 - 0.1f) m_pitch = XM_PIDIV2 - 0.1f;
	if (m_pitch < -XM_PIDIV2 + 0.1f) m_pitch = -XM_PIDIV2 + 0.1f;
}
void CameraClass::Move(float forwardDist, float rightDist) {
	XMVECTOR pos = XMLoadFloat3(&m_position);
	XMVECTOR fwd = XMLoadFloat3(&m_forward);
	XMVECTOR rgt = XMLoadFloat3(&m_right);
	pos += fwd * forwardDist;
	pos += rgt * rightDist;
	XMStoreFloat3(&m_position, pos);
}

// This uses the position and rotation of the camera to build and to update the view matrix.
void CameraClass::Render()
{
	// 카메라 위치를 XMVector로 로드
	XMVECTOR position = XMLoadFloat3(&m_position);

	// pitch: 위아래 회전 (X축), yaw: 좌우 회전 (Y축), roll은 0
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);

	// 기본 방향 벡터 (월드 기준)
	XMVECTOR defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR defaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	// 회전 행렬을 적용해서 카메라의 forward/right 벡터 생성
	XMVECTOR forward = XMVector3TransformCoord(defaultForward, rotationMatrix);
	XMVECTOR right = XMVector3TransformCoord(defaultRight, rotationMatrix);

	// 카메라의 위쪽 벡터는 forward, right의 외적
	XMVECTOR up = XMVector3Cross(forward, right);

	// 카메라가 바라보는 지점 = 위치 + forward
	XMVECTOR lookAt = position + forward;

	// 뷰 행렬 생성
	m_viewMatrix = XMMatrixLookAtLH(position, lookAt, up);

	// 방향 벡터 저장 (이동 등에 활용)
	XMStoreFloat3(&m_forward, forward);
	XMStoreFloat3(&m_right, right);
	XMStoreFloat3(&m_up, up);
}


void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}