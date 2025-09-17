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
	// roll(z)�� �� ������ �ϴ� ����
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

//����ī�޶�
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
	// ī�޶� ��ġ�� XMVector�� �ε�
	XMVECTOR position = XMLoadFloat3(&m_position);

	// pitch: ���Ʒ� ȸ�� (X��), yaw: �¿� ȸ�� (Y��), roll�� 0
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f);

	// �⺻ ���� ���� (���� ����)
	XMVECTOR defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR defaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	// ȸ�� ����� �����ؼ� ī�޶��� forward/right ���� ����
	XMVECTOR forward = XMVector3TransformCoord(defaultForward, rotationMatrix);
	XMVECTOR right = XMVector3TransformCoord(defaultRight, rotationMatrix);

	// ī�޶��� ���� ���ʹ� forward, right�� ����
	XMVECTOR up = XMVector3Cross(forward, right);

	// ī�޶� �ٶ󺸴� ���� = ��ġ + forward
	XMVECTOR lookAt = position + forward;

	// �� ��� ����
	m_viewMatrix = XMMatrixLookAtLH(position, lookAt, up);

	// ���� ���� ���� (�̵� � Ȱ��)
	XMStoreFloat3(&m_forward, forward);
	XMStoreFloat3(&m_right, right);
	XMStoreFloat3(&m_up, up);
}


void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}