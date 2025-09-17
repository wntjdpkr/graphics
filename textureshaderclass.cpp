////////////////////////////////////////////////////////////////////////////////
// Filename: textureshaderclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "textureshaderclass.h"


TextureShaderClass::TextureShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_sampleState = 0;

}


TextureShaderClass::TextureShaderClass(const TextureShaderClass& other)
{
}


TextureShaderClass::~TextureShaderClass()
{
}


bool TextureShaderClass::Initialize(ID3D11Device* device, HWND hwnd, bool useInstancing)
{
    return InitializeShader(device, hwnd, L"./data/textureShader.hlsl", useInstancing);
}


void TextureShaderClass::Shutdown()
{
	// Shutdown the vertex and pixel shaders as well as the related objects.
	ShutdownShader();

	return;
}


bool TextureShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, 
								XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
	if(!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}


bool TextureShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, const WCHAR* fileName, bool useInstancing)
{
    HRESULT result;
    ID3D10Blob* errorMessage = nullptr;
    ID3D10Blob* vertexShaderBuffer = nullptr;
    ID3D10Blob* pixelShaderBuffer = nullptr;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[6];
    unsigned int numElements;
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
	m_device = device;
    // 1. 인스턴싱 매크로 설정 여부
    D3D_SHADER_MACRO* shaderMacros = nullptr;
    D3D_SHADER_MACRO macros[] = {
        { "INSTANCE_RENDER", "1" },
        { nullptr, nullptr }
    };
    if (useInstancing) shaderMacros = macros;

    // 2. Vertex Shader 컴파일 (매크로 적용)
    result = D3DCompileFromFile(fileName, shaderMacros, nullptr, "TextureVertexShader", "vs_5_0",
        D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
    if (FAILED(result)) {
        if (errorMessage) OutputShaderErrorMessage(errorMessage, hwnd, fileName);
        else MessageBox(hwnd, fileName, L"Missing Shader File", MB_OK);
        return false;
    }

    // 3. Pixel Shader는 매크로 없이 컴파일
    result = D3DCompileFromFile(fileName, nullptr, nullptr, "TexturePixelShader", "ps_5_0",
        D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
    if (FAILED(result)) {
        if (errorMessage) OutputShaderErrorMessage(errorMessage, hwnd, fileName);
        else MessageBox(hwnd, fileName, L"Missing Shader File", MB_OK);
        return false;
    }

    // 4. Shader 객체 생성
    device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
    device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);

    // 5. 입력 레이아웃 설정
    if (useInstancing)
    {
        // 일반 vertex data
        polygonLayout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
            D3D11_INPUT_PER_VERTEX_DATA, 0 };
        polygonLayout[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA, 0 };

        // 인스턴싱용 world matrix 행 4개
        for (int i = 0; i < 4; ++i) {
            polygonLayout[2 + i].SemanticName = "TEXCOORD";
            polygonLayout[2 + i].SemanticIndex = 1 + i;
            polygonLayout[2 + i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            polygonLayout[2 + i].InputSlot = 1;
            polygonLayout[2 + i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
            polygonLayout[2 + i].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
            polygonLayout[2 + i].InstanceDataStepRate = 1;
        }

        numElements = 6;
    }
    else
    {
        polygonLayout[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
            D3D11_INPUT_PER_VERTEX_DATA, 0 };
        polygonLayout[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA, 0 };

        numElements = 2;
    }

    result = device->CreateInputLayout(polygonLayout, numElements,
        vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result)) return false;

    vertexShaderBuffer->Release();
    pixelShaderBuffer->Release();

    // 6. 상수 버퍼 생성
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&matrixBufferDesc, nullptr, &m_matrixBuffer);
    if (FAILED(result)) return false;

    // 7. 샘플러 생성
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    memset(samplerDesc.BorderColor, 0, sizeof(float) * 4);
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
    if (FAILED(result)) return false;

    return true;
}



void TextureShaderClass::ShutdownShader()
{
	// Release the sampler state.
	if(m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}

	// Release the matrix constant buffer.
	if(m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	// Release the layout.
	if(m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	// Release the pixel shader.
	if(m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	// Release the vertex shader.
	if(m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}

// 인스턴싱 전용 렌더 함수
bool TextureShaderClass::RenderInstanced(ID3D11DeviceContext* deviceContext,
    int indexCount,
    int instanceCount,
    XMMATRIX worldMatrix,
    XMMATRIX viewMatrix,
    XMMATRIX projectionMatrix,
    ID3D11ShaderResourceView* texture)
{
    // 1) 셰이더 파라미터 설정
    if (!SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture))
        return false;
    // 2) 파이프라인에 셰이더/레이아웃 바인딩
    deviceContext->IASetInputLayout(m_layout);
    deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
    deviceContext->PSSetShader(m_pixelShader, nullptr, 0);
    // 3) 텍스처 & 샘플러
    deviceContext->PSSetShaderResources(0, 1, &texture);
    deviceContext->PSSetSamplers(0, 1, &m_sampleState);
    // 4) 인스턴스 드로우
    deviceContext->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
    return true;
}
void TextureShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, 
	const WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(i=0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}


bool TextureShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, 
											 XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;


	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Now set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

	return true;
}


void TextureShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}
void TextureShaderClass::SetFilterMode(int mode)
{
	if (!m_sampleState || !m_device) return;

	m_sampleState->Release();
	m_sampleState = nullptr;

	D3D11_SAMPLER_DESC desc = {};
	if (mode == 3)
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	else if (mode == 4)
		desc.Filter = D3D11_FILTER_ANISOTROPIC;

	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 8;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	m_device->CreateSamplerState(&desc, &m_sampleState);
}