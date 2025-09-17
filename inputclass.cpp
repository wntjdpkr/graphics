////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "inputclass.h"


InputClass::InputClass()
{
}


InputClass::InputClass(const InputClass& other)
{
}


InputClass::~InputClass()
{
}


void InputClass::Initialize()
{
	for (int i = 0; i < 256; i++) m_keys[i] = false;
	m_mouseX = 0;
	m_mouseY = 0;
}
void InputClass::MouseMove(int x, int y) {
	m_mouseX = x;
	m_mouseY = y;
}
int InputClass::GetMouseX() { return m_mouseX; }
int InputClass::GetMouseY() { return m_mouseY; }

void InputClass::KeyDown(unsigned int input)
{
	// If a key is pressed then save that state in the key array.
	m_keys[input] = true;
	return;
}


void InputClass::KeyUp(unsigned int input)
{
	// If a key is released then clear that state in the key array.
	m_keys[input] = false;
	return;
}


bool InputClass::IsKeyDown(unsigned int key)
{
	// Return what state the key is in (pressed/not pressed).
	return m_keys[key];
}
