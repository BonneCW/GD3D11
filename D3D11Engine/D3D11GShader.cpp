#include "pch.h"
#include "D3D11GShader.h"
#include "D3D11GraphicsEngineBase.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "D3D11ConstantBuffer.h"
#include <d3dcompiler.h>
#include "D3D11_Helpers.h"

using namespace DirectX;

D3D11GShader::D3D11GShader() {
	GeometryShader = nullptr;

	// Insert into state-map
	ID = D3D11ObjectIdManager::AddGShader( this );
}

D3D11GShader::~D3D11GShader() {
	// Remove from state map
	D3D11ObjectIdManager::EraseGShader( this );

	SAFE_RELEASE( GeometryShader );

	for ( unsigned int i = 0; i < ConstantBuffers.size(); i++ ) {
		delete ConstantBuffers[i];
	}
}

//--------------------------------------------------------------------------------------
// Find and compile the specified shader
//--------------------------------------------------------------------------------------
HRESULT D3D11GShader::CompileShaderFromFile( const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, const std::vector<D3D_SHADER_MACRO>& makros ) {
	HRESULT hr = S_OK;

	char dir[260];
	GetCurrentDirectoryA( 260, dir );
	SetCurrentDirectoryA( Engine::GAPI->GetStartDirectory().c_str() );

	DWORD dwShaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	//dwShaderFlags |= D3DCOMPILE_DEBUG;
#else
	dwShaderFlags |= D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	// Construct makros
	std::vector<D3D_SHADER_MACRO> m;
	D3D11GraphicsEngineBase::ConstructShaderMakroList( m );

	// Push these to the front
	m.insert( m.begin(), makros.begin(), makros.end() );

	Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;
	hr = D3DCompileFromFile( Toolbox::ToWideChar( szFileName ).c_str(), &m[0], D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
	if ( FAILED( hr ) ) {
		LogInfo() << "Shader compilation failed!";
		if ( pErrorBlob.Get() ) {
			LogErrorBox() << (char*)pErrorBlob->GetBufferPointer() << "\n\n (You can ignore the next error from Gothic about too small video memory!)";
		}

		SetCurrentDirectoryA( dir );
		return hr;
	}

	SetCurrentDirectoryA( dir );
	return S_OK;
}

/** Loads both shaders at the same time */
XRESULT D3D11GShader::LoadShader( const char* geometryShader, const std::vector<D3D_SHADER_MACRO>& makros, bool createStreamOutFromVS, int soLayout ) {
	HRESULT hr;
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

	Microsoft::WRL::ComPtr<ID3DBlob> gsBlob;

	LogInfo() << "Compiling geometry shader: " << geometryShader;
	File = geometryShader;

	if ( !createStreamOutFromVS ) {
		// Compile shaders
		if ( FAILED( CompileShaderFromFile( geometryShader, "GSMain", "gs_5_0", gsBlob.GetAddressOf(), makros ) ) ) {
			return XR_FAILED;
		}

		// Create the shader
		LE( engine->GetDevice()->CreateGeometryShader( gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), nullptr, &GeometryShader ) );
	} else {
		// Compile vertexshader
		if ( FAILED( CompileShaderFromFile( geometryShader, "VSMain", "vs_5_0", gsBlob.GetAddressOf(), makros ) ) ) {
			return XR_FAILED;
		}

		D3D11_SO_DECLARATION_ENTRY* soDec = nullptr;
		int numSoDecElements = 0;
		UINT stride = 0;

		struct output11 {
			float4 vDiffuse;
			float3 vPosition;
			float2 vSize;
			float3 vVelocity;
			int type;
		};

		D3D11_SO_DECLARATION_ENTRY layout11[] =
		{
			{ 0, "POSITION", 0, 0, 3, 0},
			{ 0, "DIFFUSE", 0, 0, 4, 0},
			{ 0, "SIZE", 0, 0, 2, 0},
			{ 0, "TYPE", 0, 0, 1, 0},
			{ 0, "VELOCITY", 0, 0, 3, 0},

		};

		switch ( soLayout ) {
		case 11:
		default:
			soDec = layout11;
			numSoDecElements = sizeof( layout11 ) / sizeof( layout11[0] );
			stride = sizeof( output11 );
			break;
		}

		// Create the shader from a vertexshader
		engine->GetDevice()->CreateGeometryShaderWithStreamOutput( gsBlob->GetBufferPointer(), gsBlob->GetBufferSize(), soDec, numSoDecElements, &stride, 1, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &GeometryShader );
	}

	SetDebugName( GeometryShader, geometryShader );

	return XR_SUCCESS;
}

/** Applys the shaders */
XRESULT D3D11GShader::Apply() {
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

	engine->GetContext()->GSSetShader( GeometryShader, nullptr, 0 );

	return XR_SUCCESS;
}

/** Returns a reference to the constantBuffer vector*/
std::vector<D3D11ConstantBuffer*>& D3D11GShader::GetConstantBuffer() {
	return ConstantBuffers;
}
