////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"

#include "textureshaderclass.h"
#include "bitmapclass.h"
#include "textclass.h"
#include "inputclass.h"
#include <vector> 
#include <array>
/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND, InputClass* input);
	void Shutdown();
	bool Frame(int, int);

private:
	bool Render();
	void SetRaptorInstancePositions();
	void UpdateRaptorInstanceMatrices();
	bool RenderStartScreen();
	void UpdateBillboardMatrix(int modelIndex);

private:
	D3DClass* m_D3D;
	CameraClass* m_Camera;
	InputClass* m_Input;  // 키보드/마우스 입력 저장용 포인터
	std::vector<ModelClass*> m_Models; // 기존 m_Model1~3 대신
	std::vector<float> m_Rotations;

	ModelClass* m_Ground;
	TextureShaderClass* m_TextureShader;
	BitmapClass* m_Bitmap;
	int m_screenWidth, m_screenHeight;

	TextClass* m_Text;
	float m_cameraYaw;    // 회전 누적용
	float m_cameraPitch;
	int m_prevMouseX, m_prevMouseY;
	std::vector<XMMATRIX> m_WorldMatrices;   // 각 오브젝트별 월드매트릭스 (배경+모델)
	std::vector<XMFLOAT3> m_Positions; //모델 위치
	std::vector<XMFLOAT3> m_Scales;          // ← 모델별 스케일을 저장할 벡터

	ModelClass* m_Raptor;
	std::array<XMFLOAT3, 10> m_raptorPositionList;
	std::array<XMMATRIX, 10> m_raptorInstances;
	TextureShaderClass* m_InstancedShader; // 인스턴싱 전용 셰이더
	
	bool m_startScreen;
	BitmapClass* m_startBitmap;
	TextClass* m_startText;

	std::vector<XMFLOAT3> m_MoveStartPositions; // 왕복 시작점
	std::vector<XMFLOAT3> m_MoveEndPositions;   // 왕복 끝점
	std::vector<float> m_MoveSpeeds;            // 이동 속도
	std::vector<bool> m_MovingForward;          // 방향 플래그

	bool m_cullingEnabled = true;
	bool m_cKeyPressedLastFrame = false;
	int m_currentFilterMode = 3;
};

#endif