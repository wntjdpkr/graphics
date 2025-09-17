////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = 0;
	m_Camera = 0;
	
	m_Ground = 0;

	m_TextureShader = 0;
	m_Bitmap = 0;

	m_Text = 0;

	m_Raptor = nullptr;
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd, InputClass* input)
{
	bool result;
	m_Input = input;
	XMMATRIX baseViewMatrix;
	m_cameraYaw = 0.0f;
	m_cameraPitch = 0.0f;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	// Create the Direct3D object.
	m_D3D = new D3DClass;
	if(!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 0.0f, -6.0f);	// for cube
	m_Camera->SetRotation(0.0f, 0.0f, 0.0f);
	std::wstring modelPaths[12] = {
	L"./data/coel.obj",
	L"./data/gall.obj",
	L"./data/indomi.obj",
	L"./data/atroci.obj",
	L"./data/dilopho.obj",
	L"./data/Raptor.obj",
	L"./data/stygimo.obj",
	L"./data/pyro.obj",
	L"./data/stego.obj",
	L"./data/r2d2.obj",
	L"./data/tree.obj",
	L"./data/plane.obj"	// �� �� ��� �߰�
	};
	std::wstring texturePaths[12] = {
	L"./data/coel.dds",
	L"./data/gall.dds",
	L"./data/indomi.dds",
	L"./data/atroci.dds",
	L"./data/dilopho.dds",
	L"./data/Raptor.dds",
	L"./data/stygimo.dds",
	L"./data/pyro.dds",
	L"./data/stego.dds",
	L"./data/r2d2.dds",
	L"./data/tree.dds",
	L"./data/plane.dds"
	};

	for (int i = 0; i < 12; ++i) {
		ModelClass* model = new ModelClass;
		if (!model->Initialize(m_D3D->GetDevice(), modelPaths[i].c_str(), texturePaths[i].c_str())) {
			MessageBox(hwnd, L"Could not initialize model.", L"Error", MB_OK);
			return false;
		}
		m_Models.push_back(model);
		m_Rotations.push_back(0.0f);  // ȸ���� �ʱ�ȭ (�� �𵨺�)
	}
	m_WorldMatrices.resize(1 + m_Models.size(), XMMatrixIdentity());
	std::vector<XMFLOAT3> initialPositions = {    // 1) �𵨺� �ʱ� ��ġ�� XMFLOAT3 �迭�� ����
		{ -1.0f,  0.3f,  0.0f },  // model 0
		{  0.0f,  0.0f,  0.0f },  // model 1
		{  3.0f,  0.0f,  0.0f },  // model 2
		{  1.0f,  0.0f,  0.0f },  // model 3
		{  1.5f,  0.0f,  0.0f },  // model 4
		{ -1.6f,  0.0f,  0.0f },  // model 5
		{ -2.0f,  0.0f,  0.0f },  // model 6
		{ -3.0f,  0.0f,  0.0f },  // model 7
		{ -4.0f,  0.0f,  0.0f },  // model 8
		{ -5.5f,  0.0f,  0.0f },  // model 9
		{ -4.5f,  0.0f,  -1.0f },  // model 10
		{ 0.5f,  -0.1f,  0.0f }   // model 11
	};
	m_Positions = initialPositions; // 2) m_Models.size()�� ���ƾ� ��

	// 2) �𵨺� �ʱ� ������ ���� (initialPositions�� ���� ũ�⿩�� ��)
	std::vector<XMFLOAT3> initialScales = {
		{1.0f, 1.0f, 1.0f},  // model 0
		{0.1f, 0.1f, 0.1f},  // model 1
		{0.07f, 0.07f, 0.07f},  // model 2
		{0.1f, 0.1f, 0.1f},  // model 3
		{0.1f, 0.1f, 0.1f},  // model 4
		{1.0f, 1.0f, 1.0f},  // model5
		{0.07f, 0.07f, 0.07f},  // model6
		{0.1f, 0.1f, 0.1f},  // model7
		{0.07f, 0.07f, 0.07f},  // model8
		{0.1f, 0.1f, 0.1f},  // model 9
		{0.05f, 0.05f, 0.05f},// model 10
		{0.1f, 0.1f, 0.1f} // model 11 (Plane�� ���� ���)
	};
	m_Scales = initialScales;

	// �Ϲ� �𵨿� Shader
	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader)
	{
		return false;
	}
	result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd, false); // �ν��Ͻ� ��Ȱ��ȭ
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the general texture shader.", L"Error", MB_OK);
		return false;
	}

	// Raptor �ν��Ͻ̿� Shader
	m_InstancedShader = new TextureShaderClass;
	if (!m_InstancedShader)
	{
		return false;
	}
	result = m_InstancedShader->Initialize(m_D3D->GetDevice(), hwnd, true); // �ν��Ͻ� Ȱ��ȭ
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the instanced shader.", L"Error", MB_OK);
		return false;
	}


	// Create the bitmap object.
	m_Bitmap = new BitmapClass;
	if (!m_Bitmap)
	{
		return false;
	}

	// Initialize the bitmap object.
	result = m_Bitmap->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, L"./data/bluesky.dds", screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
		return false;
	}

	// Initialize a base view matrix with the camera for 2D user interface rendering.
//	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(baseViewMatrix);

	// Create the text object.
	m_Text = new TextClass;
	if (!m_Text)
	{
		return false;
	}

	// Initialize the text object.
	result = m_Text->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
		return false;
	}
	m_startBitmap = new BitmapClass;
	result = m_startBitmap->Initialize(
		m_D3D->GetDevice(),
		screenWidth, screenHeight,
		L"./data/start.dds",    // �� �� ��ΰ� ���� ����(��: Debug/) �������� �����ϴ��� Ȯ��
		screenWidth, screenHeight
	);
	if (!result) {
		MessageBox(hwnd, L"Could not load start.dds", L"Error", MB_OK);
		return false;
	}
	m_startText = new TextClass;
	m_startText->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
	int cx = screenWidth / 2, cy = screenHeight / 2;
	m_startText->SetMenuText(1, "lego jurrasic park", cx - 240, cy - 340, 1.0f, 1.0f, 0.1f, m_D3D->GetDeviceContext());
	m_startText->SetMenuText(2, "made by jusung park", cx - 240, cy - 300, 0.0f, 1.0f, 0.0f, m_D3D->GetDeviceContext());
	m_startText->SetMenuText(3, "how to play: wasd with mouse move", cx - 240, cy - 260, 1.0f, 1.0f, 0.1f, m_D3D->GetDeviceContext());
	m_startScreen = true;

	{
		// 1) ���� Ŀ�� ��ġ�� ������ Ŭ���̾�Ʈ ��ǥ�� ��ȯ
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);

		// 2) InputClass �� GraphicsClass �� ���� ���콺 ��ǥ�� ����ȭ
		m_Input->MouseMove(p.x, p.y);
		m_prevMouseX = p.x;
		m_prevMouseY = p.y;
	}
	// (2) Initialize()�� �� �ʱ�ȭ �� �ν��Ͻ� ���� ����
// --- Initialize() �� �κп� �߰� ---
	m_Raptor = new ModelClass;
	m_Raptor->Initialize(m_D3D->GetDevice(), L"./data/raptor.obj", L"./data/raptor.dds");

	// �� �ν��Ͻ��� ��ġ ���� (��: x ���� 2��ŭ)
	SetRaptorInstancePositions();  // ��ġ ����
	UpdateRaptorInstanceMatrices();
	// ȸ�� �𵨸� �պ� �̵� ����
	for (int i = 0; i < m_Models.size(); ++i) {
		if (i == 1 || i == 2 || i == 7) {
			m_MoveStartPositions.push_back(m_Positions[i]);
			m_MoveEndPositions.push_back(XMFLOAT3(m_Positions[i].x + 3.0f, m_Positions[i].y, m_Positions[i].z));
			m_MoveSpeeds.push_back(0.01f * (i + 1));
			m_MovingForward.push_back(true);
		}
		else {
			// ���� ������ ä��� �̵��� �� �ϵ���
			m_MoveStartPositions.push_back(m_Positions[i]);
			m_MoveEndPositions.push_back(m_Positions[i]);
			m_MoveSpeeds.push_back(0.0f);
			m_MovingForward.push_back(true);
		}
	}
	return true;

}
void GraphicsClass::SetRaptorInstancePositions()
{
	std::array<XMFLOAT3, 10> positions = {
		XMFLOAT3{ -1.6f,  0.0f,  1.0f },
		XMFLOAT3{ -2.4f,  0.0f,  0.0f },
		XMFLOAT3{ -3.4f,  0.0f,  0.0f },
		XMFLOAT3{  3.0f,  0.0f,  0.0f },
		XMFLOAT3{  4.0f,  0.0f,  0.8f },
		XMFLOAT3{  5.0f,  0.0f,  -0.8f },
		XMFLOAT3{  7.0f,  0.0f,  1.0f },
		XMFLOAT3{  7.0f,  0.0f,  1.8f },
		XMFLOAT3{  7.7f,  0.0f,  0.2f },
		XMFLOAT3{  7.7f,  0.0f,  1.0f }
	};

	m_raptorPositionList = positions;  // std::array �� �ܼ� ���� ����
}
void GraphicsClass::UpdateRaptorInstanceMatrices()
{
	for (int i = 0; i < 10; ++i) {
		m_raptorInstances[i] = XMMatrixTranslation(
			m_raptorPositionList[i].x,
			m_raptorPositionList[i].y,
			m_raptorPositionList[i].z
		);
	}
	m_Raptor->InitializeInstanceBuffer(m_D3D->GetDevice(), m_raptorInstances.data(), 10);
}
void GraphicsClass::Shutdown()
{
	//vector models
	for (auto model : m_Models) {
		if (model) {
			model->Shutdown();
			delete model;
		}
	}
	m_Models.clear();

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the D3D object.
	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}

	// Release the bitmap object.
	if (m_Bitmap)
	{
		m_Bitmap->Shutdown();
		delete m_Bitmap;
		m_Bitmap = 0;
	}

	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}
	if (m_Text) {
		m_Text->Shutdown();
		delete m_Text;
		m_Text = nullptr;
	}
	if (m_Raptor) {
		m_Raptor->ShutdownInstanceBuffer();
		m_Raptor->Shutdown();
		delete m_Raptor;
		m_Raptor = 0;
	}
	return;
}
bool GraphicsClass::RenderStartScreen()
{
	XMMATRIX worldMatrix, orthoMatrix;

	// 2D UI �������� ���� ���� ��ĵ��� �����մϴ�.
	XMMATRIX identityViewMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixIdentity();

	m_D3D->GetOrthoMatrix(orthoMatrix);

	// ���� �����մϴ�.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 2D �������� ���� Z ���۸� ���ϴ�.
	m_D3D->TurnZBufferOff();

	// 1. (���� ������ ���� ���¿���) �������� ��� ��Ʈ���� ���� �������մϴ�.
	m_startBitmap->Render(m_D3D->GetDeviceContext(), 0, 0);
	m_TextureShader->Render(
		m_D3D->GetDeviceContext(),
		m_startBitmap->GetIndexCount(),
		worldMatrix,
		identityViewMatrix,
		orthoMatrix,
		m_startBitmap->GetTexture()
	);

	// 2. �������� �ؽ�Ʈ�� �������ϱ� ���� ���� ������ �մϴ�.
	m_D3D->TurnOnAlphaBlending();

	m_startText->Render(
		m_D3D->GetDeviceContext(),
		worldMatrix,
		identityViewMatrix,
		orthoMatrix
	);

	// 3. ����ߴ� ������ ���µ��� ������� �����մϴ�.
	m_D3D->TurnOffAlphaBlending();
	m_D3D->TurnZBufferOn();

	// ���� �����մϴ�.
	m_D3D->EndScene();

	return true;
}

bool GraphicsClass::Frame(int fps, int cpu)
{
	bool result;

	// 1. ���� ȭ�� ó��
	// �� ����� ���� ȭ���� ��� ������ ����մϴ�.
	if (m_startScreen)
	{
		// ENTER Ű�� ������ ���� ȭ���� �����մϴ�.
		if (m_Input->IsKeyDown(VK_RETURN))
		{
			m_startScreen = false;
		}
		else
		{
			// ���� ȭ�鸸 �������ϰ� �Լ��� ��� �����մϴ�.
			return RenderStartScreen();
		}
	}

	//
	// ���������������������������������������������
	// ������ �����ߴ� �ߺ��� if (m_startScreen) ����� ������ �����߽��ϴ�.
	// ���������������������������������������������
	//

	// 2. �ΰ��� ������ ó��
	float moveSpeed = 0.2f;
	float rotateSpeed = 0.002f;

	// �� ȸ�� ó��
	for (int i = 0; i < m_Rotations.size(); ++i) {
		if (i == 10) {
			UpdateBillboardMatrix(i);
			continue;
		}
		if (i == 1 || i == 2 || i == 7) {
			m_Rotations[i] += XM_PI * 0.01f * (i + 1);
			if (m_Rotations[i] > XM_2PI)
				m_Rotations[i] -= XM_2PI;
		}

		XMFLOAT3 s = m_Scales[i];
		XMMATRIX scale = XMMatrixScaling(s.x, s.y, s.z);
		XMMATRIX rotate = XMMatrixRotationY(m_Rotations[i]);
		XMFLOAT3 pos = m_Positions[i];
		XMMATRIX trans = XMMatrixTranslation(pos.x, pos.y, pos.z);

		m_WorldMatrices[i + 1] = scale * rotate * trans;
	}

	// �� �պ� �̵� ó��
	for (int i = 0; i < m_Positions.size(); ++i) {
		if (!(i == 1 || i == 2 || i == 7)) continue;

		XMFLOAT3& pos = m_Positions[i];
		XMFLOAT3 start = m_MoveStartPositions[i];
		XMFLOAT3 end = m_MoveEndPositions[i];
		float speed = m_MoveSpeeds[i];

		if (m_MovingForward[i]) {
			pos.x += speed;
			if (pos.x >= end.x) {
				pos.x = end.x;
				m_MovingForward[i] = false;
			}
		}
		else {
			pos.x -= speed;
			if (pos.x <= start.x) {
				pos.x = start.x;
				m_MovingForward[i] = true;
			}
		}
	}

	// ī�޶� �̵� ó��
	if (m_Input->IsKeyDown('W')) m_Camera->Move(+moveSpeed, 0);
	if (m_Input->IsKeyDown('S')) m_Camera->Move(-moveSpeed, 0);
	if (m_Input->IsKeyDown('A')) m_Camera->Move(0, -moveSpeed);
	if (m_Input->IsKeyDown('D')) m_Camera->Move(0, +moveSpeed);

	// ���콺 ȸ�� ó��
	int currMouseX = m_Input->GetMouseX();
	int currMouseY = m_Input->GetMouseY();
	int dx = currMouseX - m_prevMouseX;
	int dy = currMouseY - m_prevMouseY;

	m_cameraYaw += dx * rotateSpeed;
	m_cameraPitch += dy * rotateSpeed;

	if (m_cameraPitch > XM_PIDIV2 - 0.1f)  m_cameraPitch = XM_PIDIV2 - 0.1f;
	if (m_cameraPitch < -XM_PIDIV2 + 0.1f) m_cameraPitch = -XM_PIDIV2 + 0.1f;

	m_prevMouseX = currMouseX;
	m_prevMouseY = currMouseY;

	// �ø� ��� ��ȯ
	bool cPressedNow = (GetAsyncKeyState('C') & 0x8000) != 0;
	if (cPressedNow && !m_cKeyPressedLastFrame) {
		m_cullingEnabled = !m_cullingEnabled;
		if (m_cullingEnabled)
			m_D3D->TurnCullingOn();
		else
			m_D3D->TurnCullingOff();
	}
	m_cKeyPressedLastFrame = cPressedNow;

	// ���� ��� ��ȯ
	if (GetAsyncKeyState('3') & 0x8000) {
		if (m_currentFilterMode != 3) {
			m_TextureShader->SetFilterMode(3);
			m_currentFilterMode = 3;
		}
	}
	if (GetAsyncKeyState('4') & 0x8000) {
		if (m_currentFilterMode != 4) {
			m_TextureShader->SetFilterMode(4);
			m_currentFilterMode = 4;
		}
	}

	// 3. �ؽ�Ʈ ���� ������Ʈ
	if (!m_Text->SetFPS(fps, m_D3D->GetDeviceContext())) return false;
	if (!m_Text->SetCPU(cpu, m_D3D->GetDeviceContext())) return false;

	int totalPolyCount = 0;
	for (auto model : m_Models) {
		if (model) {
			totalPolyCount += model->GetIndexCount() / 3;
		}
	}
	if (!m_Text->SetPolyCount(totalPolyCount, m_D3D->GetDeviceContext())) return false;
	if (!m_Text->SetResolution(m_screenWidth, m_screenHeight, m_D3D->GetDeviceContext())) return false;
	if (!m_Text->SetModelCount(static_cast<int>(m_Models.size()), m_D3D->GetDeviceContext())) return false;

	// 4. ���� ������
	result = Render();
	if (!result) return false;

	return true;
}

bool GraphicsClass::Render()
{
	XMMATRIX viewMatrix, projectionMatrix, orthoMatrix;
	bool result;

	// 1. ��� �� �ʱ�ȭ
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 2. ���� �� ���� ��Ʈ���� ȹ��
	m_D3D->GetProjectionMatrix(projectionMatrix);
	m_D3D->GetOrthoMatrix(orthoMatrix);

	// 3. ���� ī�޶� (ȸ�� ����)
	m_Camera->SetRotation(0.0f, 0.0f, 0.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);

	// 4. ���� ��Ʈ���� ���� (����)
	m_WorldMatrices[0] = XMMatrixIdentity();

	// 5. ��� ������ (Z ���� ����)
	m_D3D->TurnZBufferOff();

	result = m_Bitmap->Render(m_D3D->GetDeviceContext(), 0, 0);
	if (!result) return false;

	result = m_TextureShader->Render(
		m_D3D->GetDeviceContext(),
		m_Bitmap->GetIndexCount(),
		m_WorldMatrices[0],   // ���� ����
		viewMatrix,
		orthoMatrix,
		m_Bitmap->GetTexture()
	);
	if (!result) return false;

	// �ΰ��� UI �ؽ�Ʈ ������ (���� ����)
	m_D3D->TurnOnAlphaBlending();
	// ������ �κ�: TextClass�� ����� m_baseViewMatrix�� ����Ͽ� �ؽ�Ʈ ��ġ�� ������ŵ�ϴ�.
	result = m_Text->Render(m_D3D->GetDeviceContext(), m_WorldMatrices[0], m_Text->GetBaseViewMatrix(), orthoMatrix);
	if (!result) return false;
	m_D3D->TurnOffAlphaBlending();

	// 6. 3D �������� ���� Z ���� �ѱ�
	m_D3D->TurnZBufferOn();

	// �� ���ν��Ͻ� �� ��ο� �� slot 1 ����ε�
	{
		ID3D11Buffer* nullBuffers[1] = { nullptr };
		UINT          nullOffsets[1] = { 0 };
		m_D3D->GetDeviceContext()->IASetVertexBuffers(1, 1, nullBuffers, nullOffsets, nullOffsets);
	}

	// 7. ī�޶� ȸ�� ����
	m_Camera->SetRotation(m_cameraPitch, m_cameraYaw, 0.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);

	// 8. �Ϲ� ��(����) ������
	for (int i = 0; i < static_cast<int>(m_Models.size()); ++i)
	{
		if (i == 10) continue;  // Ʈ�� ���� ���߿� ���� ������

		m_Models[i]->Render(m_D3D->GetDeviceContext());

		result = m_TextureShader->Render(
			m_D3D->GetDeviceContext(),
			m_Models[i]->GetIndexCount(),
			m_WorldMatrices[i + 1],
			viewMatrix,
			projectionMatrix,
			m_Models[i]->GetTexture()
		);
		if (!result) return false;
	}

	// 9. raptor �� �ν��Ͻ� ������
	m_Raptor->RenderInstancedBuffers(m_D3D->GetDeviceContext(), 10);
	result = m_InstancedShader->RenderInstanced(
		m_D3D->GetDeviceContext(),
		m_Raptor->GetIndexCount(),
		10,
		XMMatrixIdentity(),
		viewMatrix,
		projectionMatrix,
		m_Raptor->GetTexture()
	);
	if (!result) return false;

	// 10. Ʈ�� �� (���� �ؽ�ó ����) ������
	m_D3D->TurnOnAlphaBlending();

	m_Models[10]->Render(m_D3D->GetDeviceContext());

	result = m_TextureShader->Render(
		m_D3D->GetDeviceContext(),
		m_Models[10]->GetIndexCount(),
		m_WorldMatrices[10 + 1],
		viewMatrix,
		projectionMatrix,
		m_Models[10]->GetTexture()
	);

	m_D3D->TurnOffAlphaBlending();

	if (!result) return false;

	// 11. ��� ����
	m_D3D->EndScene();

	return true;
}
void GraphicsClass::UpdateBillboardMatrix(int modelIndex)
{
	// Ʈ�� ��ġ �� ������
	XMFLOAT3 treePos = m_Positions[modelIndex];
	XMFLOAT3 treeScale = m_Scales[modelIndex];

	XMVECTOR posVec = XMLoadFloat3(&treePos);
	XMFLOAT3 camPos = m_Camera->GetPosition();
	XMVECTOR camVec = XMLoadFloat3(&camPos);

	// billboard�� Z���� ī�޶� ������ ������ ��
	XMVECTOR toCamera = XMVector3Normalize(camVec - posVec);

	// Y��(Up)�� ���� (Axis-Aligned Billboarding)
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, toCamera));
	XMVECTOR forward = XMVector3Normalize(XMVector3Cross(right, up));

	// ȸ�� ��� ����
	XMMATRIX rotation = XMMATRIX(right, up, forward, XMVectorSet(0, 0, 0, 1));

	XMMATRIX scale = XMMatrixScaling(treeScale.x, treeScale.y, treeScale.z);
	XMMATRIX translate = XMMatrixTranslation(treePos.x, treePos.y, treePos.z);

	// ���� ���� ���
	m_WorldMatrices[modelIndex + 1] = scale * rotation * translate;
}
