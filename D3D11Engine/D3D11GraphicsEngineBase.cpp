#include "D3D11GraphicsEngineBase.h"

#include "BaseAntTweakBar.h"
#include "D3D11LineRenderer.h"
#include "D3D11PipelineStates.h"
#include "D3D11PointLight.h"
#include "D3D11PShader.h"
#include "D3D11ShaderManager.h"
#include "D3D11VShader.h"
#include "RenderToTextureBuffer.h"
#include "zCView.h"

const int DRAWVERTEXARRAY_BUFFER_SIZE = 2048 * sizeof(ExVertexStruct);
const int NUM_MAX_BONES = 96;
const int INSTANCING_BUFFER_SIZE = sizeof(VobInstanceInfo) * 2048;

// If defined, creates a debug-version of the d3d11-device
//#define DEBUG_D3D11

D3D11GraphicsEngineBase::D3D11GraphicsEngineBase() {
	PresentPending = false;

	// Match the resolution with the current desktop resolution
	Resolution = Engine::GAPI->GetRendererState()->RendererSettings.LoadedResolution;
}

D3D11GraphicsEngineBase::~D3D11GraphicsEngineBase() {
	GothicDepthBufferStateInfo::DeleteCachedObjects();
	GothicBlendStateInfo::DeleteCachedObjects();
	GothicRasterizerStateInfo::DeleteCachedObjects();

	for (size_t i = 0; i < DeferredContextsAll.size(); i++) {
		SAFE_RELEASE(DeferredContextsAll[i]);
	}
}

/** Called after the fake-DDraw-Device got created */
XRESULT D3D11GraphicsEngineBase::Init() {
	HRESULT hr;

	LogInfo() << "Initializing Device...";

	// Create DXGI factory
	LE(CreateDXGIFactory(__uuidof(IDXGIFactory), &DXGIFactory));
	LE(DXGIFactory->EnumAdapters(0, &DXGIAdapter)); // Get first adapter

	// Find out what we are rendering on to write it into the logfile
	DXGI_ADAPTER_DESC adpDesc;
	DXGIAdapter->GetDesc(&adpDesc);

	std::wstring wDeviceDescription(adpDesc.Description);
	std::string deviceDescription(wDeviceDescription.begin(), wDeviceDescription.end());
	DeviceDescription = deviceDescription;
	LogInfo() << "Rendering on: " << deviceDescription.c_str();

	int flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	D3D_FEATURE_LEVEL featurelevel = D3D_FEATURE_LEVEL_11_0;

	// Create D3D11-Device
#ifndef DEBUG_D3D11
	LE(D3D11CreateDevice(DXGIAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags, &featurelevel, 1, D3D11_SDK_VERSION, &Device, nullptr, &Context));
#else
	LE(D3D11CreateDevice(DXGIAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, flags | D3D11_CREATE_DEVICE_DEBUG, &featurelevel, 1, D3D11_SDK_VERSION, &Device, nullptr, &Context));
#endif

	if (hr == DXGI_ERROR_UNSUPPORTED) {
		LogErrorBox() << "Your GPU (" << deviceDescription.c_str() << ") does not support Direct3D 11, so it can't run GD3D11!\n"
			"It has to be at least Featurelevel 11_0 compatible, which requires at least:"
			" *	Nvidia GeForce GTX4xx or newer"
			" *	AMD Radeon 5xxx or newer\n\n"
			"The game will now close.";
		exit(0);
	}

	LE(Device->CreateDeferredContext(0, &DeferredContext)); // Used for multithreaded texture loading

	LogInfo() << "Creating ShaderManager";

	ShaderManager = std::make_unique<D3D11ShaderManager>();
	ShaderManager->Init();
	ShaderManager->LoadShaders();

	PS_DiffuseNormalmapped = ShaderManager->GetPShader("PS_DiffuseNormalmapped");
	PS_Diffuse = ShaderManager->GetPShader("PS_Diffuse");
	PS_DiffuseNormalmappedAlphatest = ShaderManager->GetPShader("PS_DiffuseNormalmappedAlphaTest");
	PS_DiffuseAlphatest = ShaderManager->GetPShader("PS_DiffuseAlphaTest");
	PS_Simple = ShaderManager->GetPShader("PS_Simple");
	PS_SimpleAlphaTest = ShaderManager->GetPShader("PS_SimpleAlphaTest");
	VS_Ex = ShaderManager->GetVShader("VS_Ex");
	VS_ExInstancedObj = ShaderManager->GetVShader("VS_ExInstancedObj");
	VS_ExRemapInstancedObj = ShaderManager->GetVShader("VS_ExRemapInstancedObj");
	VS_ExSkeletal = ShaderManager->GetVShader("VS_ExSkeletal");

	// Create default sampler state
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = -3.402823466e+38F; // -FLT_MAX
	samplerDesc.MaxLOD = 3.402823466e+38F; // FLT_MAX

	LE(GetDevice()->CreateSamplerState(&samplerDesc, &DefaultSamplerState));
	GetContext()->PSSetSamplers(0, 1, DefaultSamplerState.GetAddressOf());
	GetContext()->VSSetSamplers(0, 1, DefaultSamplerState.GetAddressOf());
	GetContext()->DSSetSamplers(0, 1, DefaultSamplerState.GetAddressOf());
	GetContext()->HSSetSamplers(0, 1, DefaultSamplerState.GetAddressOf());

	TempVertexBuffer = std::make_unique<D3D11VertexBuffer>();
	TempVertexBuffer->Init(nullptr, DRAWVERTEXARRAY_BUFFER_SIZE, D3D11VertexBuffer::B_VERTEXBUFFER, D3D11VertexBuffer::U_DYNAMIC, D3D11VertexBuffer::CA_WRITE);

	TransformsCB = std::make_unique<D3D11ConstantBuffer>(sizeof(VS_ExConstantBuffer_PerFrame), nullptr);

	LineRenderer = std::make_unique<D3D11LineRenderer>();

	SetDefaultStates();
	UpdateRenderStates();

	return XR_SUCCESS;
}

/** Called when the game created its window */
XRESULT D3D11GraphicsEngineBase::SetWindow(HWND hWnd) {
	LogInfo() << "Creating swapchain";
	OutputWindow = hWnd;

	OnResize(Resolution);

	return XR_SUCCESS;
}

/** Called on window resize/resolution change */
XRESULT D3D11GraphicsEngineBase::OnResize(INT2 newSize)
{
	HRESULT hr;

	if (memcmp(&Resolution, &newSize, sizeof(newSize)) == 0 && SwapChain)
		return XR_SUCCESS; // Don't resize if we don't have to

	Resolution = newSize;
	INT2 bbres = GetBackbufferResolution();

	RECT desktopRect;
	GetClientRect(GetDesktopWindow(), &desktopRect);
	SetWindowPos(OutputWindow, nullptr, 0, 0, desktopRect.right, desktopRect.bottom, 0);

	Backbuffer.reset();

	if (!SwapChain)
	{
		LogInfo() << "Creating new swapchain! (Format: DXGI_FORMAT_R8G8B8A8_UNORM)";

		DXGI_SWAP_CHAIN_DESC scd;
		ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

		scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		scd.BufferCount = 1;
		scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		scd.OutputWindow = OutputWindow;
		scd.SampleDesc.Count = 1;
		scd.SampleDesc.Quality = 0;
		scd.BufferDesc.Height = bbres.y;
		scd.BufferDesc.Width = bbres.x;
		scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		// Check for windowed mode
		bool windowed = Engine::GAPI->HasCommandlineParameter("ZWINDOW") ||
			Engine::GAPI->GetIntParamFromConfig("zStartupWindowed");
		scd.Windowed = windowed;

#ifdef BUILD_GOTHIC_1_08k
#ifdef PUBLIC_RELEASE
		scd.Windowed = false;
#else
		scd.Windowed = true;
#endif
#endif


		LE(DXGIFactory->CreateSwapChain(GetDevice(), &scd, &SwapChain));

		if (!SwapChain)
		{
			LogError() << "Failed to create Swapchain! Program will now exit!";
			exit(0);
		}

		// Need to init AntTweakBar now that we have a working swapchain
		XLE(Engine::AntTweakBar->Init());
	}
	else
	{
		LogInfo() << "Resizing swapchain  (Format: DXGI_FORMAT_R8G8B8A8_UNORM)";

		if (FAILED(SwapChain->ResizeBuffers(0, bbres.x, bbres.y, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
		{
			LogError() << "Failed to resize swapchain!";
			return XR_FAILED;
		}
	}

	// Successfully resized swapchain, re-get buffers
	ID3D11Texture2D * backbuffer = nullptr;
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)& backbuffer);

	// Recreate RenderTargetView
	ID3D11RenderTargetView* backbufferRTV;
	ID3D11ShaderResourceView * backbufferSRV;
	LE(GetDevice()->CreateRenderTargetView(backbuffer, nullptr, &backbufferRTV));
	LE(GetDevice()->CreateShaderResourceView(backbuffer, nullptr, &backbufferSRV));

	Backbuffer = std::make_unique<RenderToTextureBuffer>(backbuffer, backbufferSRV, backbufferRTV, (UINT)Resolution.x, (UINT)Resolution.y);

	// Recreate DepthStencilBuffer
	DepthStencilBuffer = std::make_unique<RenderToDepthStencilBuffer>(GetDevice(), Resolution.x, Resolution.y, DXGI_FORMAT_R32_TYPELESS, nullptr, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);

	// Bind our newly created resources
	GetContext()->OMSetRenderTargets(1, Backbuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)bbres.x;
	viewport.Height = (float)bbres.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	GetContext()->RSSetViewports(1, &viewport);

	// Create other buffers
	HDRBackBuffer = std::make_unique<RenderToTextureBuffer>(GetDevice(), Resolution.x, Resolution.y, DXGI_FORMAT_R16G16B16A16_FLOAT);

	Engine::AntTweakBar->OnResize(newSize);

	GetContext()->OMSetRenderTargets(1, Backbuffer->GetRenderTargetViewPtr(), DepthStencilBuffer->GetDepthStencilView());

	return XR_SUCCESS;
}

/** Called when the game wants to render a new frame */
XRESULT D3D11GraphicsEngineBase::OnBeginFrame()
{
	// Enter the critical section for safety while executing the deferred command list
	Engine::GAPI->EnterResourceCriticalSection();
	ID3D11CommandList* dc_cl = nullptr;
	GetDeferredMediaContext()->FinishCommandList(true, &dc_cl);

	// Copy list of textures we are operating on
	Engine::GAPI->MoveLoadedTexturesToProcessedList();
	Engine::GAPI->ClearFrameLoadedTextures();

	Engine::GAPI->LeaveResourceCriticalSection();
	if (dc_cl)
	{
		//LogInfo() << "Executing command list";
		GetContext()->ExecuteCommandList(dc_cl, true);
		dc_cl->Release();
	}

	// Force the mode upon Gothic
	zCView::SetMode(static_cast<int>(Resolution.x / Engine::GAPI->GetRendererState()->RendererSettings.GothicUIScale), static_cast<int>(Resolution.y / Engine::GAPI->GetRendererState()->RendererSettings.GothicUIScale), 32);

	// Notify the shader manager
	ShaderManager->OnFrameStart();

	return XR_SUCCESS;

}

/** Called when the game ended it's frame */
XRESULT D3D11GraphicsEngineBase::OnEndFrame()
{
	Present();

	// At least Present should have flushed the pipeline, so these textures should be ready by now
	Engine::GAPI->SetFrameProcessedTexturesReady();

	return XR_SUCCESS;
}

/** Called to set the current viewport */
XRESULT D3D11GraphicsEngineBase::SetViewport(const ViewportInfo& viewportInfo)
{
	// Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = static_cast<float>(viewportInfo.TopLeftX);
	viewport.TopLeftY = static_cast<float>(viewportInfo.TopLeftY);
	viewport.Width    = static_cast<float>(viewportInfo.Width);
	viewport.Height   = static_cast<float>(viewportInfo.Height);
	viewport.MinDepth = viewportInfo.MinZ;
	viewport.MaxDepth = viewportInfo.MaxZ;

	GetContext()->RSSetViewports(1, &viewport);

	return XR_SUCCESS;
}


/** Returns the shadermanager */
D3D11ShaderManager* D3D11GraphicsEngineBase::GetShaderManager()
{
	return ShaderManager.get();
}

/** Called when the game wants to clear the bound rendertarget */
XRESULT D3D11GraphicsEngineBase::Clear(const float4 & color)
{
	GetContext()->ClearDepthStencilView(DepthStencilBuffer->GetDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	GetContext()->ClearRenderTargetView(HDRBackBuffer->GetRenderTargetView(), (float *)& color);
	GetContext()->ClearRenderTargetView(Backbuffer->GetRenderTargetView(), (float *)& color);

	return XR_SUCCESS;
}

/** Creates a vertexbuffer object (Not registered inside) */
XRESULT D3D11GraphicsEngineBase::CreateVertexBuffer(D3D11VertexBuffer** outBuffer)
{
	*outBuffer = new D3D11VertexBuffer;
	return XR_SUCCESS;
}

/** Creates a texture object (Not registered inside) */
XRESULT D3D11GraphicsEngineBase::CreateTexture(D3D11Texture ** outTexture)
{
	*outTexture = new D3D11Texture;

	return XR_SUCCESS;
}

/** Creates a constantbuffer object (Not registered inside) */
XRESULT D3D11GraphicsEngineBase::CreateConstantBuffer(D3D11ConstantBuffer** outCB, void * data, int size)
{
	*outCB = new D3D11ConstantBuffer(size, data);

	return XR_SUCCESS;
}

/** Returns a list of available display modes */
XRESULT D3D11GraphicsEngineBase::GetDisplayModeList(std::vector<DisplayModeInfo>* modeList, bool includeSuperSampling)
{
	HRESULT hr;
	UINT numModes = 0;
	std::unique_ptr<DXGI_MODE_DESC[]> displayModes = nullptr;
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	IDXGIOutput* output;

	// Get desktop rect
	RECT desktop;
	GetClientRect(GetDesktopWindow(), &desktop);

	if (!DXGIAdapter)
		return XR_FAILED;

	DXGIAdapter->EnumOutputs(0, &output);

	if (!output)
		return XR_FAILED;

	hr = output->GetDisplayModeList(format, 0, &numModes, nullptr);

	displayModes = std::make_unique<DXGI_MODE_DESC[]>(numModes);

	// Get the list
	hr = output->GetDisplayModeList(format, 0, &numModes, displayModes.get());

	for (unsigned int i = 0; i < numModes; i++)
	{
		if (displayModes[i].Format != format)
			continue;

		DisplayModeInfo info;
		info.Height = displayModes[i].Height;
		info.Width = displayModes[i].Width;
		info.Bpp = 32;

		if (info.Width > (unsigned long)desktop.right || info.Height > (unsigned long)desktop.bottom)
			continue; // Skip bigger sizes than the desktop rect, because DXGI doesn't like them apparently TODO: Fix this if possible!

		if (!modeList->empty() && memcmp(&modeList->back(), &info, sizeof(info)) == 0)
			continue; // Already have that in list

		modeList->push_back(info);
	}

	if (includeSuperSampling)
	{
		// Put supersampling resolutions in, up to just below 8k
		int i = 2;
		DisplayModeInfo ssBase = modeList->back();
		while (ssBase.Width * i < 8192 && ssBase.Height * i < 8192)
		{
			DisplayModeInfo info;
			info.Height = ssBase.Height * i;
			info.Width = ssBase.Width * i;
			info.Bpp = 32;

			modeList->push_back(info);
			i++;
		}
	}
	displayModes.reset();
	output->Release();
	return XR_SUCCESS;
}

/** Presents the current frame to the screen */
XRESULT D3D11GraphicsEngineBase::Present()
{
	// Set default viewport
	SetViewport(ViewportInfo(0, 0, Resolution.x, Resolution.y));

	// Reset State
	SetDefaultStates();

	// Draw debug-lines
	LineRenderer->Flush();

	// Draw ant tweak bar
	Engine::AntTweakBar->Draw();

	bool vsync = Engine::GAPI->GetRendererState()->RendererSettings.EnableVSync;
	if (SwapChain->Present(vsync ? 1 : 0, 0) == DXGI_ERROR_DEVICE_REMOVED)
	{
		switch (GetDevice()->GetDeviceRemovedReason())
		{
			case DXGI_ERROR_DEVICE_HUNG:
				LogErrorBox() << "Device Removed! (DXGI_ERROR_DEVICE_HUNG)";
				exit(0);
				break;

			case DXGI_ERROR_DEVICE_REMOVED:
				LogErrorBox() << "Device Removed! (DXGI_ERROR_DEVICE_REMOVED)";
				exit(0);
				break;

			case DXGI_ERROR_DEVICE_RESET:
				LogErrorBox() << "Device Removed! (DXGI_ERROR_DEVICE_RESET)";
				exit(0);
				break;

			case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
				LogErrorBox() << "Device Removed! (DXGI_ERROR_DRIVER_INTERNAL_ERROR)";
				exit(0);
				break;

			case DXGI_ERROR_INVALID_CALL:
				LogErrorBox() << "Device Removed! (DXGI_ERROR_INVALID_CALL)";
				exit(0);
				break;

			case S_OK:
				LogInfo() << "Device removed, but we're fine!";
				break;

			default:
				LogWarnBox() << "Device Removed! (Unknown reason)";
		}
	}

	// We did our present, set the next frame ready
	PresentPending = false;

	return XR_SUCCESS;
}

/** Called when we started to render the world */
XRESULT D3D11GraphicsEngineBase::OnStartWorldRendering()
{
	if (PresentPending)
		return XR_FAILED;

	PresentPending = true;

	// Update transforms
	UpdateTransformsCB();

	// Force farplane
	Engine::GAPI->SetFarPlane(Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius * WORLD_SECTION_SIZE);

	return XR_SUCCESS;
}

/** Returns the line renderer object */
BaseLineRenderer* D3D11GraphicsEngineBase::GetLineRenderer()
{
	return LineRenderer.get();
}

/** Sets up the default rendering state */
void D3D11GraphicsEngineBase::SetDefaultStates()
{
	Engine::GAPI->GetRendererState()->RasterizerState.SetDefault();
	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	Engine::GAPI->GetRendererState()->DepthState.SetDefault();
	Engine::GAPI->GetRendererState()->SamplerState.SetDefault();

	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	Engine::GAPI->GetRendererState()->DepthState.SetDirty();
	Engine::GAPI->GetRendererState()->SamplerState.SetDirty();

	GetContext()->PSSetSamplers(0, 1, DefaultSamplerState.GetAddressOf());

	UpdateRenderStates();
}

/** Draws a vertexarray, used for rendering gothics UI */
XRESULT D3D11GraphicsEngineBase::DrawVertexArray(ExVertexStruct* vertices, unsigned int numVertices, unsigned int startVertex, unsigned int stride)
{
	UpdateRenderStates();
	D3D11VShader* vShader = ShaderManager->GetVShader("VS_TransformedEx");
	D3D11PShader * pShader = ShaderManager->GetPShader("PS_FixedFunctionPipe");

	// Bind the FF-Info to the first PS slot
	pShader->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	pShader->GetConstantBuffer()[0]->BindToPixelShader(0);

	vShader->Apply();
	pShader->Apply();

	// Set vertex type
	GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind the viewport information to the shader
	D3D11_VIEWPORT vp;
	UINT num = 1;
	GetContext()->RSGetViewports(&num, &vp);

	// Update viewport information
	const float scale = Engine::GAPI->GetRendererState()->RendererSettings.GothicUIScale;
	float2 temp2Float2[2];
	temp2Float2[0].x = vp.TopLeftX / scale;
	temp2Float2[0].y = vp.TopLeftY / scale;
	temp2Float2[1].x = vp.Width / scale;
	temp2Float2[1].y = vp.Height / scale;

	vShader->GetConstantBuffer()[0]->UpdateBuffer(temp2Float2);
	vShader->GetConstantBuffer()[0]->BindToVertexShader(0);

	D3D11_BUFFER_DESC desc;
	TempVertexBuffer->GetVertexBuffer()->GetDesc(&desc);

	// Check if we need a bigger vertexbuffer
	if (desc.ByteWidth < stride * numVertices)
	{
		LogInfo() << "TempVertexBuffer too small (" << desc.ByteWidth << "), need " << stride * numVertices << " bytes. Recreating buffer.";

		// Buffer too small, recreate it
		TempVertexBuffer = std::make_unique<D3D11VertexBuffer>();

		TempVertexBuffer->Init(nullptr, stride * numVertices, D3D11VertexBuffer::B_VERTEXBUFFER, D3D11VertexBuffer::U_DYNAMIC, D3D11VertexBuffer::CA_WRITE);
	}

	// Send vertexdata to the GPU
	TempVertexBuffer->UpdateBuffer(vertices, stride * numVertices);

	UINT offset = 0;
	UINT uStride = stride;
	ID3D11Buffer* buffer = TempVertexBuffer->GetVertexBuffer();
	GetContext()->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

	//Draw the mesh
	GetContext()->Draw(numVertices, startVertex);

	return XR_SUCCESS;
}

/** Recreates the renderstates */
XRESULT D3D11GraphicsEngineBase::UpdateRenderStates()
{
	if (Engine::GAPI->GetRendererState()->BlendState.StateDirty)
	{
		D3D11BlendStateInfo* state = (D3D11BlendStateInfo *)GothicStateCache::s_BlendStateMap[Engine::GAPI->GetRendererState()->BlendState];

		if (!state)
		{
			// Create new state
			state = new D3D11BlendStateInfo(Engine::GAPI->GetRendererState()->BlendState);

			GothicStateCache::s_BlendStateMap[Engine::GAPI->GetRendererState()->BlendState] = state;
		}

		FFBlendState = state->State;

		Engine::GAPI->GetRendererState()->BlendState.StateDirty = false;
		GetContext()->OMSetBlendState(FFBlendState.Get(), (float *)&DirectX::SimpleMath::Vector4::Zero, 0xFFFFFFFF);
	}

	if (Engine::GAPI->GetRendererState()->RasterizerState.StateDirty)
	{
		D3D11RasterizerStateInfo* state = (D3D11RasterizerStateInfo *)GothicStateCache::s_RasterizerStateMap[Engine::GAPI->GetRendererState()->RasterizerState];

		if (!state)
		{
			// Create new state
			state = new D3D11RasterizerStateInfo(Engine::GAPI->GetRendererState()->RasterizerState);

			GothicStateCache::s_RasterizerStateMap[Engine::GAPI->GetRendererState()->RasterizerState] = state;
		}

		FFRasterizerState = state->State;

		Engine::GAPI->GetRendererState()->RasterizerState.StateDirty = false;
		GetContext()->RSSetState(FFRasterizerState.Get());
	}

	if (Engine::GAPI->GetRendererState()->DepthState.StateDirty)
	{
		D3D11DepthBufferState* state = (D3D11DepthBufferState *)GothicStateCache::s_DepthBufferMap[Engine::GAPI->GetRendererState()->DepthState];

		if (!state)
		{
			// Create new state
			state = new D3D11DepthBufferState(Engine::GAPI->GetRendererState()->DepthState);

			GothicStateCache::s_DepthBufferMap[Engine::GAPI->GetRendererState()->DepthState] = state;
		}

		FFDepthStencilState = state->State;

		Engine::GAPI->GetRendererState()->DepthState.StateDirty = false;
		GetContext()->OMSetDepthStencilState(FFDepthStencilState.Get(), 0);
	}
	
	return XR_SUCCESS;
}


/** Constructs the makro list for shader compilation */
void D3D11GraphicsEngineBase::ConstructShaderMakroList(std::vector<D3D10_SHADER_MACRO> & list)
{
	const GothicRendererSettings& s = Engine::GAPI->GetRendererState()->RendererSettings;
	D3D10_SHADER_MACRO m;

	m.Name = "SHD_ENABLE";
	m.Definition = s.EnableShadows ? "1" : "0";
	list.push_back(m);

	m.Name = "SHD_FILTER_16TAP_PCF";
	m.Definition = s.EnableSoftShadows ? "1" : "0";
	list.push_back(m);

	m.Name = nullptr;
	m.Definition = nullptr;
	list.push_back(m);
}

void D3D11GraphicsEngineBase::SetupVS_ExMeshDrawCall()
{
	UpdateRenderStates();

	if (ActiveVS)ActiveVS->Apply();
	if (ActivePS)ActivePS->Apply();

	GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void D3D11GraphicsEngineBase::SetupVS_ExConstantBuffer()
{
	const DirectX::SimpleMath::Matrix& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	const DirectX::SimpleMath::Matrix& view  = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	const DirectX::SimpleMath::Matrix& proj  = Engine::GAPI->GetProjectionMatrix();

	VS_ExConstantBuffer_PerFrame cb;
	cb.View = view;
	cb.Projection = proj;
	cb.ViewProj = proj * view;

	ActiveVS->GetConstantBuffer()[0]->UpdateBuffer(&cb);
	ActiveVS->GetConstantBuffer()[0]->BindToVertexShader(0);
	ActiveVS->GetConstantBuffer()[0]->BindToDomainShader(0);
	ActiveVS->GetConstantBuffer()[0]->BindToHullShader(0);
}

void D3D11GraphicsEngineBase::SetupVS_ExPerInstanceConstantBuffer()
{
	DirectX::SimpleMath::Matrix& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;

	VS_ExConstantBuffer_PerInstance cb;
	cb.World = world;

	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&cb);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(1);
}

/** Sets the active pixel shader object */
XRESULT D3D11GraphicsEngineBase::SetActivePixelShader(const std::string & shader)
{
	ActivePS = ShaderManager->GetPShader(shader);

	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngineBase::SetActiveVertexShader(const std::string & shader)
{
	ActiveVS = ShaderManager->GetVShader(shader);

	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngineBase::SetActiveHDShader(const std::string & shader)
{
	ActiveHDS = ShaderManager->GetHDShader(shader);

	return XR_SUCCESS;
}

XRESULT D3D11GraphicsEngineBase::SetActiveGShader(const std::string & shader)
{
	ActiveGS = ShaderManager->GetGShader(shader);

	return XR_SUCCESS;
}

/** Puts the current world matrix into a CB and binds it to the given slot */
void D3D11GraphicsEngineBase::SetupPerInstanceConstantBuffer(int slot)
{
	const DirectX::SimpleMath::Matrix& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;

	VS_ExConstantBuffer_PerInstance cb;
	cb.World = world;

	ActiveVS->GetConstantBuffer()[1]->UpdateBuffer(&cb);
	ActiveVS->GetConstantBuffer()[1]->BindToVertexShader(slot);
}

/** Updates the transformsCB with new values from the GAPI */
void D3D11GraphicsEngineBase::UpdateTransformsCB()
{
	const DirectX::SimpleMath::Matrix& world = Engine::GAPI->GetRendererState()->TransformState.TransformWorld;
	const DirectX::SimpleMath::Matrix& view  = Engine::GAPI->GetRendererState()->TransformState.TransformView;
	const DirectX::SimpleMath::Matrix& proj  = Engine::GAPI->GetProjectionMatrix();

	VS_ExConstantBuffer_PerFrame cb;
	cb.View = view;
	cb.Projection = proj;
	cb.ViewProj = proj * view;

	TransformsCB->UpdateBuffer(&cb);
}

/** Creates a bufferobject for a shadowed point light */
XRESULT D3D11GraphicsEngineBase::CreateShadowedPointLight(BaseShadowedPointLight** outPL, VobLightInfo* lightInfo, bool dynamic)
{
	if (Engine::GAPI->GetRendererState()->RendererSettings.EnablePointlightShadows > 0)
		*outPL = new D3D11PointLight(lightInfo, dynamic);
	else
		*outPL = nullptr;

	return XR_SUCCESS;
}

/** Draws a vertexbuffer, non-indexed, binding the FF-Pipe values */
XRESULT D3D11GraphicsEngineBase::DrawVertexBufferFF(D3D11VertexBuffer* vb, unsigned int numVertices, unsigned int startVertex, unsigned int stride)
{
	SetupVS_ExMeshDrawCall();

	// Bind the FF-Info to the first PS slot
	ActivePS->GetConstantBuffer()[0]->UpdateBuffer(&Engine::GAPI->GetRendererState()->GraphicsState);
	ActivePS->GetConstantBuffer()[0]->BindToPixelShader(0);

	UINT offset = 0;
	UINT uStride = stride;
	ID3D11Buffer* buffer = vb->GetVertexBuffer();
	GetContext()->IASetVertexBuffers(0, 1, &buffer, &uStride, &offset);

	//Draw the mesh
	GetContext()->Draw(numVertices, startVertex);

	Engine::GAPI->GetRendererState()->RendererInfo.FrameDrawnTriangles += numVertices;

	return XR_SUCCESS;
}


/** Binds viewport information to the given constantbuffer slot */
XRESULT D3D11GraphicsEngineBase::BindViewportInformation(const std::string & shader, int slot)
{
	D3D11_VIEWPORT vp;
	UINT num = 1;
	GetContext()->RSGetViewports(&num, &vp);

	// Update viewport information
	float scale = Engine::GAPI->GetRendererState()->RendererSettings.GothicUIScale;
	float2 f2[2];
	f2[0].x = vp.TopLeftX / scale;
	f2[0].y = vp.TopLeftY / scale;
	f2[1].x = vp.Width / scale;
	f2[1].y = vp.Height / scale;

	D3D11PShader * ps = ShaderManager->GetPShader(shader);
	D3D11VShader* vs = ShaderManager->GetVShader(shader);

	if (vs)
	{
		vs->GetConstantBuffer()[slot]->UpdateBuffer(f2);
		vs->GetConstantBuffer()[slot]->BindToVertexShader(slot);
	}

	if (ps)
	{
		ps->GetConstantBuffer()[slot]->UpdateBuffer(f2);
		ps->GetConstantBuffer()[slot]->BindToVertexShader(slot);
	}

	return XR_SUCCESS;
}

/** Returns the graphics-device this is running on */
std::string D3D11GraphicsEngineBase::GetGraphicsDeviceName()
{
	return DeviceDescription;
}