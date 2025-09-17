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
	InputClass* m_Input;  // Ű����/���콺 �Է� ����� ������
	std::vector<ModelClass*> m_Models; // ���� m_Model1~3 ���
	std::vector<float> m_Rotations;

	ModelClass* m_Ground;
	TextureShaderClass* m_TextureShader;
	BitmapClass* m_Bitmap;
	int m_screenWidth, m_screenHeight;

	TextClass* m_Text;
	float m_cameraYaw;    // ȸ�� ������
	float m_cameraPitch;
	int m_prevMouseX, m_prevMouseY;
	std::vector<XMMATRIX> m_WorldMatrices;   // �� ������Ʈ�� �����Ʈ���� (���+��)
	std::vector<XMFLOAT3> m_Positions; //�� ��ġ
	std::vector<XMFLOAT3> m_Scales;          // �� �𵨺� �������� ������ ����

	ModelClass* m_Raptor;
	std::array<XMFLOAT3, 10> m_raptorPositionList;
	std::array<XMMATRIX, 10> m_raptorInstances;
	TextureShaderClass* m_InstancedShader; // �ν��Ͻ� ���� ���̴�
	
	bool m_startScreen;
	BitmapClass* m_startBitmap;
	TextClass* m_startText;

	std::vector<XMFLOAT3> m_MoveStartPositions; // �պ� ������
	std::vector<XMFLOAT3> m_MoveEndPositions;   // �պ� ����
	std::vector<float> m_MoveSpeeds;            // �̵� �ӵ�
	std::vector<bool> m_MovingForward;          // ���� �÷���

	bool m_cullingEnabled = true;
	bool m_cKeyPressedLastFrame = false;
	int m_currentFilterMode = 3;
};

#endif