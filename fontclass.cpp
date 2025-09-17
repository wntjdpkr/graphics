///////////////////////////////////////////////////////////////////////////////
// Filename: fontclass.cpp
///////////////////////////////////////////////////////////////////////////////
#include "fontclass.h"
#include <string>      // std::string과 std::getline을 위해 추가
#include <sstream>     // std::stringstream을 위해 추가


FontClass::FontClass()
{
	m_Font = 0;
	m_Texture = 0;
}


FontClass::FontClass(const FontClass& other)
{
}


FontClass::~FontClass()
{
}


bool FontClass::Initialize(ID3D11Device* device, const WCHAR* fontFilename, const WCHAR* textureFilename)
{
	bool result;


	// Load in the text file containing the font data.
	result = LoadFontData(fontFilename);
	if(!result)
	{
		return false;
	}

	// Load the texture that has the font characters on it.
	result = LoadTexture(device, textureFilename);
	if(!result)
	{
		return false;
	}

	return true;
}


void FontClass::Shutdown()
{
	// Release the font texture.
	ReleaseTexture();

	// Release the font data.
	ReleaseFontData();

	return;
}


bool FontClass::LoadFontData(const WCHAR* filename)
{
	ifstream fin;
	int i;
	char temp;

	// Create the font spacing buffer.
	m_Font = new FontType[95];
	if (!m_Font)
	{
		return false;
	}

	// Read in the font size and spacing between chars.
	fin.open(filename);
	if (fin.fail())
	{
		return false;
	}

	std::string line; // 한 줄을 읽어올 string 변수

	// Read in the 95 used ascii characters for text.
	for (i = 0; i < 95; i++)
	{
		// 1. 파일에서 한 줄을 통째로 읽어옵니다.
		if (!std::getline(fin, line))
		{
			// 만약 95줄을 다 읽기 전에 파일이 끝나면 실패 처리
			fin.close();
			return false;
		}

		// 2. 읽어온 한 줄(line)을 stringstream에 넣고 파싱합니다.
		std::stringstream ss(line);
		std::string dummy; // 필요 없는 앞부분 데이터를 버리기 위한 변수

		// 3. 공백을 기준으로 단어를 읽어옵니다. 
		//    앞의 두 단어는 dummy 변수에 넣어 무시합니다.
		ss >> dummy >> dummy;

		// 4. 필요한 숫자 데이터 세 개를 읽어옵니다.
		//    이 방식은 공백이나 탭이 여러 개 있어도 안정적으로 동작합니다.
		ss >> m_Font[i].left;
		ss >> m_Font[i].right;
		ss >> m_Font[i].size;
	}

	// Close the file.
	fin.close();

	return true;
}


void FontClass::ReleaseFontData()
{
	// Release the font data array.
	if(m_Font)
	{
		delete [] m_Font;
		m_Font = 0;
	}

	return;
}


bool FontClass::LoadTexture(ID3D11Device* device, const WCHAR* filename)
{
	bool result;


	// Create the texture object.
	m_Texture = new TextureClass;
	if(!m_Texture)
	{
		return false;
	}

	// Initialize the texture object.
	result = m_Texture->Initialize(device, filename);
	if(!result)
	{
		return false;
	}

	return true;
}


void FontClass::ReleaseTexture()
{
	// Release the texture object.
	if(m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = 0;
	}

	return;
}


ID3D11ShaderResourceView* FontClass::GetTexture()
{
	return m_Texture->GetTexture();
}


void FontClass::BuildVertexArray(void* vertices, const char* sentence, float drawX, float drawY)
{
	VertexType* vertexPtr;
	int numLetters, index, i, letter;


	// Coerce the input vertices into a VertexType structure.
	vertexPtr = (VertexType*)vertices;

	// Get the number of letters in the sentence.
	numLetters = (int)strlen(sentence);

	// Initialize the index to the vertex array.
	index = 0;

	// Draw each letter onto a quad.
	for(i=0; i<numLetters; i++)
	{
		letter = ((int)sentence[i]) - 32;

		// If the letter is a space then just move over three pixels.
		if(letter == 0)
		{
			drawX = drawX + 3.0f;
		}
		else
		{
			// First triangle in quad.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + m_Font[letter].size), (drawY - 16), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 1.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX, (drawY - 16), 0.0f);  // Bottom left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 1.0f);
			index++;

			// Second triangle in quad.
			vertexPtr[index].position = XMFLOAT3(drawX, drawY, 0.0f);  // Top left.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].left, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3(drawX + m_Font[letter].size, drawY, 0.0f);  // Top right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 0.0f);
			index++;

			vertexPtr[index].position = XMFLOAT3((drawX + m_Font[letter].size), (drawY - 16), 0.0f);  // Bottom right.
			vertexPtr[index].texture = XMFLOAT2(m_Font[letter].right, 1.0f);
			index++;

			// Update the x location for drawing by the size of the letter and one pixel.
			drawX = drawX + m_Font[letter].size + 1.0f;
		}
	}

	return;
}