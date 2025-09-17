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
	L"./data/plane.obj"	// 새 모델 경로 추가
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
		m_Rotations.push_back(0.0f);  // 회전값 초기화 (각 모델별)
	}
	m_WorldMatrices.resize(1 + m_Models.size(), XMMatrixIdentity());
	std::vector<XMFLOAT3> initialPositions = {    // 1) 모델별 초기 위치를 XMFLOAT3 배열로 정의
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
	m_Positions = initialPositions; // 2) m_Models.size()와 같아야 함

	// 2) 모델별 초기 스케일 정의 (initialPositions와 같은 크기여야 함)
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
		{0.1f, 0.1f, 0.1f} // model 11 (Plane은 넓은 평면)
	};
	m_Scales = initialScales;

	// 일반 모델용 Shader
	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader)
	{
		return false;
	}
	result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd, false); // 인스턴싱 비활성화
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the general texture shader.", L"Error", MB_OK);
		return false;
	}

	// Raptor 인스턴싱용 Shader
	m_InstancedShader = new TextureShaderClass;
	if (!m_InstancedShader)
	{
		return false;
	}
	result = m_InstancedShader->Initialize(m_D3D->GetDevice(), hwnd, true); // 인스턴싱 활성화
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
		L"./data/start.dds",    // ← 이 경로가 실행 폴더(예: Debug/) 기준으로 존재하는지 확인
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
		// 1) 현재 커서 위치를 윈도우 클라이언트 좌표로 변환
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);

		// 2) InputClass 와 GraphicsClass 의 이전 마우스 좌표를 동기화
		m_Input->MouseMove(p.x, p.y);
		m_prevMouseX = p.x;
		m_prevMouseY = p.y;
	}
	// (2) Initialize()에 모델 초기화 후 인스턴스 버퍼 생성
// --- Initialize() 끝 부분에 추가 ---
	m_Raptor = new ModelClass;
	m_Raptor->Initialize(m_D3D->GetDevice(), L"./data/raptor.obj", L"./data/raptor.dds");

	// 각 인스턴스별 위치 설정 (예: x 간격 2만큼)
	SetRaptorInstancePositions();  // 위치 설정
	UpdateRaptorInstanceMatrices();
	// 회전 모델만 왕복 이동 설정
	for (int i = 0; i < m_Models.size(); ++i) {
		if (i == 1 || i == 2 || i == 7) {
			m_MoveStartPositions.push_back(m_Positions[i]);
			m_MoveEndPositions.push_back(XMFLOAT3(m_Positions[i].x + 3.0f, m_Positions[i].y, m_Positions[i].z));
			m_MoveSpeeds.push_back(0.01f * (i + 1));
			m_MovingForward.push_back(true);
		}
		else {
			// 더미 값으로 채우되 이동은 안 하도록
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

	m_raptorPositionList = positions;  // std::array 간 단순 복사 가능
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

	// 2D UI 렌더링을 위한 단위 행렬들을 생성합니다.
	XMMATRIX identityViewMatrix = XMMatrixIdentity();
	worldMatrix = XMMatrixIdentity();

	m_D3D->GetOrthoMatrix(orthoMatrix);

	// 씬을 시작합니다.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 2D 렌더링을 위해 Z 버퍼를 끕니다.
	m_D3D->TurnZBufferOff();

	// 1. (알파 블렌딩이 꺼진 상태에서) 불투명한 배경 비트맵을 먼저 렌더링합니다.
	m_startBitmap->Render(m_D3D->GetDeviceContext(), 0, 0);
	m_TextureShader->Render(
		m_D3D->GetDeviceContext(),
		m_startBitmap->GetIndexCount(),
		worldMatrix,
		identityViewMatrix,
		orthoMatrix,
		m_startBitmap->GetTexture()
	);

	// 2. 반투명한 텍스트를 렌더링하기 위해 알파 블렌딩을 켭니다.
	m_D3D->TurnOnAlphaBlending();

	m_startText->Render(
		m_D3D->GetDeviceContext(),
		worldMatrix,
		identityViewMatrix,
		orthoMatrix
	);

	// 3. 사용했던 렌더링 상태들을 원래대로 복구합니다.
	m_D3D->TurnOffAlphaBlending();
	m_D3D->TurnZBufferOn();

	// 씬을 종료합니다.
	m_D3D->EndScene();

	return true;
}

bool GraphicsClass::Frame(int fps, int cpu)
{
	bool result;

	// 1. 시작 화면 처리
	// 이 블록이 시작 화면의 모든 로직을 담당합니다.
	if (m_startScreen)
	{
		// ENTER 키를 누르면 시작 화면을 종료합니다.
		if (m_Input->IsKeyDown(VK_RETURN))
		{
			m_startScreen = false;
		}
		else
		{
			// 시작 화면만 렌더링하고 함수를 즉시 종료합니다.
			return RenderStartScreen();
		}
	}

	//
	// ▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼
	// 오류를 유발했던 중복된 if (m_startScreen) 블록을 완전히 삭제했습니다.
	// ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲
	//

	// 2. 인게임 프레임 처리
	float moveSpeed = 0.2f;
	float rotateSpeed = 0.002f;

	// 모델 회전 처리
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

	// 모델 왕복 이동 처리
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

	// 카메라 이동 처리
	if (m_Input->IsKeyDown('W')) m_Camera->Move(+moveSpeed, 0);
	if (m_Input->IsKeyDown('S')) m_Camera->Move(-moveSpeed, 0);
	if (m_Input->IsKeyDown('A')) m_Camera->Move(0, -moveSpeed);
	if (m_Input->IsKeyDown('D')) m_Camera->Move(0, +moveSpeed);

	// 마우스 회전 처리
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

	// 컬링 모드 전환
	bool cPressedNow = (GetAsyncKeyState('C') & 0x8000) != 0;
	if (cPressedNow && !m_cKeyPressedLastFrame) {
		m_cullingEnabled = !m_cullingEnabled;
		if (m_cullingEnabled)
			m_D3D->TurnCullingOn();
		else
			m_D3D->TurnCullingOff();
	}
	m_cKeyPressedLastFrame = cPressedNow;

	// 필터 모드 전환
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

	// 3. 텍스트 정보 업데이트
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

	// 4. 최종 렌더링
	result = Render();
	if (!result) return false;

	return true;
}

bool GraphicsClass::Render()
{
	XMMATRIX viewMatrix, projectionMatrix, orthoMatrix;
	bool result;

	// 1. 배경 색 초기화
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 2. 투영 및 직교 매트릭스 획득
	m_D3D->GetProjectionMatrix(projectionMatrix);
	m_D3D->GetOrthoMatrix(orthoMatrix);

	// 3. 배경용 카메라 (회전 없음)
	m_Camera->SetRotation(0.0f, 0.0f, 0.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);

	// 4. 월드 매트릭스 설정 (배경용)
	m_WorldMatrices[0] = XMMatrixIdentity();

	// 5. 배경 렌더링 (Z 버퍼 끄기)
	m_D3D->TurnZBufferOff();

	result = m_Bitmap->Render(m_D3D->GetDeviceContext(), 0, 0);
	if (!result) return false;

	result = m_TextureShader->Render(
		m_D3D->GetDeviceContext(),
		m_Bitmap->GetIndexCount(),
		m_WorldMatrices[0],   // 배경용 월드
		viewMatrix,
		orthoMatrix,
		m_Bitmap->GetTexture()
	);
	if (!result) return false;

	// 인게임 UI 텍스트 렌더링 (알파 블렌딩)
	m_D3D->TurnOnAlphaBlending();
	// 수정된 부분: TextClass에 저장된 m_baseViewMatrix를 사용하여 텍스트 위치를 고정시킵니다.
	result = m_Text->Render(m_D3D->GetDeviceContext(), m_WorldMatrices[0], m_Text->GetBaseViewMatrix(), orthoMatrix);
	if (!result) return false;
	m_D3D->TurnOffAlphaBlending();

	// 6. 3D 렌더링을 위해 Z 버퍼 켜기
	m_D3D->TurnZBufferOn();

	// ▶ 비인스턴스 모델 드로우 전 slot 1 언바인딩
	{
		ID3D11Buffer* nullBuffers[1] = { nullptr };
		UINT          nullOffsets[1] = { 0 };
		m_D3D->GetDeviceContext()->IASetVertexBuffers(1, 1, nullBuffers, nullOffsets, nullOffsets);
	}

	// 7. 카메라 회전 적용
	m_Camera->SetRotation(m_cameraPitch, m_cameraYaw, 0.0f);
	m_Camera->Render();
	m_Camera->GetViewMatrix(viewMatrix);

	// 8. 일반 모델(벡터) 렌더링
	for (int i = 0; i < static_cast<int>(m_Models.size()); ++i)
	{
		if (i == 10) continue;  // 트리 모델은 나중에 따로 렌더링

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

	// 9. raptor 모델 인스턴싱 렌더링
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

	// 10. 트리 모델 (투명 텍스처 적용) 렌더링
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

	// 11. 장면 종료
	m_D3D->EndScene();

	return true;
}
void GraphicsClass::UpdateBillboardMatrix(int modelIndex)
{
	// 트리 위치 및 스케일
	XMFLOAT3 treePos = m_Positions[modelIndex];
	XMFLOAT3 treeScale = m_Scales[modelIndex];

	XMVECTOR posVec = XMLoadFloat3(&treePos);
	XMFLOAT3 camPos = m_Camera->GetPosition();
	XMVECTOR camVec = XMLoadFloat3(&camPos);

	// billboard는 Z축이 카메라 방향을 보도록 함
	XMVECTOR toCamera = XMVector3Normalize(camVec - posVec);

	// Y축(Up)은 고정 (Axis-Aligned Billboarding)
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, toCamera));
	XMVECTOR forward = XMVector3Normalize(XMVector3Cross(right, up));

	// 회전 행렬 구성
	XMMATRIX rotation = XMMATRIX(right, up, forward, XMVectorSet(0, 0, 0, 1));

	XMMATRIX scale = XMMatrixScaling(treeScale.x, treeScale.y, treeScale.z);
	XMMATRIX translate = XMMatrixTranslation(treePos.x, treePos.y, treePos.z);

	// 최종 월드 행렬
	m_WorldMatrices[modelIndex + 1] = scale * rotation * translate;
}
