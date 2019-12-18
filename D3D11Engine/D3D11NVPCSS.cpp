#include "pch.h"
#include "D3D11NVPCSS.h"
#include "Engine.h"
#include "D3D11GraphicsEngine.h"
#include "GFSDK_ShadowLib_Common.h"
#include "GFSDK_ShadowLib.h"
#include "RenderToTextureBuffer.h"
#include "GothicAPI.h"

#pragma comment(lib, "GFSDK_ShadowLib_DX11.win32.lib")

D3D11NVPCSS::D3D11NVPCSS()
{
}


D3D11NVPCSS::~D3D11NVPCSS()
{
}

/** Initializes the library */
XRESULT D3D11NVPCSS::Init()
{
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;
	GFSDK_ShadowLib_Version version;
	version.uMajor = 3;
	version.uMinor = 0;
	GFSDK_ShadowLib_Status status;
	status =  GFSDK_ShadowLib_Create(&m_ShadowLibVersion, &m_pShadowLibCtx,);

	GFSDK_SSAO_CustomHeap CustomHeap;
	CustomHeap.new_ = ::operator new;
	CustomHeap.delete_ = ::operator delete;

	status = GFSDK_SSAO_CreateContext_D3D11(engine->GetDevice(), &AOContext, &CustomHeap);
	if (status != GFSDK_SSAO_OK)
	{
		LogError() << "Failed to initialize Nvidia HBAO+!";
		return XR_FAILED;
	}


	return XR_SUCCESS;
}