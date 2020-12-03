#include "pch.h"
#include "D3D11PShader.h"
#include "D3D11GraphicsEngineBase.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "D3D11ConstantBuffer.h"
#include <d3dcompiler.h>

using namespace DirectX;

D3D11PShader::D3D11PShader() {
	PixelShader = nullptr;

	// Insert into state-map
	ID = D3D11ObjectIdManager::AddPShader( this );
}

D3D11PShader::~D3D11PShader() {
	// Remove from state map
	D3D11ObjectIdManager::ErasePShader( this );

	if ( PixelShader )PixelShader->Release();

	for ( unsigned int i = 0; i < ConstantBuffers.size(); i++ ) {
		delete ConstantBuffers[i];
	}
}

//--------------------------------------------------------------------------------------
// Find and compile the specified shader
//--------------------------------------------------------------------------------------
HRESULT D3D11PShader::CompileShaderFromFile( const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, const std::vector<D3D_SHADER_MACRO>& makros ) {
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
XRESULT D3D11PShader::LoadShader( const char* pixelShader, const std::vector<D3D_SHADER_MACRO>& makros ) {
	HRESULT hr;
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

	ID3DBlob* psBlob;

	if ( Engine::GAPI->GetRendererState().RendererSettings.EnableDebugLog )
		LogInfo() << "Compilling pixel shader: " << pixelShader;
	File = pixelShader;

	// Compile shaders
	if ( FAILED( CompileShaderFromFile( pixelShader, "PSMain", "ps_5_0", &psBlob, makros ) ) ) {
		return XR_FAILED;
	}

	// Create the shader
	LE( engine->GetDevice()->CreatePixelShader( psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &PixelShader ) );

#ifndef PUBLIC_RELEASE
	PixelShader->SetPrivateData( WKPDID_D3DDebugObjectName, strlen( pixelShader ), pixelShader );
#endif

	psBlob->Release();

	return XR_SUCCESS;
}

/** Applys the shaders */
XRESULT D3D11PShader::Apply() {
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

	engine->GetContext()->PSSetShader( PixelShader, nullptr, 0 );

	return XR_SUCCESS;
}

/** Returns a reference to the constantBuffer vector*/
std::vector<D3D11ConstantBuffer*>& D3D11PShader::GetConstantBuffer() {
	return ConstantBuffers;
}