#pragma once

#include "basegraphicsengine.h"
#include <dxgi1_5.h>

class D3D11DepthBufferState;
class D3D11BlendStateInfo;
class D3D11RasterizerStateInfo;
class D3D11PShader;
class D3D11VShader;
class D3D11HDShader;
class D3D11Texture;
class D3D11GShader;

namespace D3D11ObjectIDs {

	/** Map to get a texture by ID */
	__declspec(selectany) std::map<UINT16, D3D11Texture*> TextureByID;


	__declspec(selectany) std::map<UINT8, D3D11PShader*> PShadersByID;

	__declspec(selectany) std::map<UINT8, D3D11VShader*> VShadersByID;

	__declspec(selectany) std::map<UINT8, D3D11HDShader*> HDShadersByID;

	__declspec(selectany) std::map<UINT8, D3D11GShader*> GShadersByID;

	__declspec(selectany) std::map<UINT8, D3D11DepthBufferState*> DepthStateByID;

	__declspec(selectany) std::map<UINT8, D3D11BlendStateInfo*> BlendStateByID;

	__declspec(selectany) std::map<UINT8, D3D11RasterizerStateInfo*> RasterizerStateByID;

	__declspec(selectany) struct CounterStruct {
		CounterStruct() {
			memset( this, 0, sizeof( CounterStruct ) );
		}

		int PShadersCounter;
		int TextureCounter;
		int VShadersCounter;
		int HDShadersCounter;
		int GShadersCounter;
		int DepthStateCounter;
		int BlendStateCounter;
		int RasterizerCounter;
	} Counters;
}

class D3D11ObjectIdManager {
public:
	static UINT8 AddVShader( D3D11VShader* s ) {
		std::unique_lock<std::mutex> lock( VShadersByIDMutex );
		UINT8 id = 0;
		if ( !D3D11ObjectIDs::VShadersByID.empty() ) {
			id = D3D11ObjectIDs::Counters.VShadersCounter++;
		}
		D3D11ObjectIDs::VShadersByID[id] = s;
		return id;
	}
	static UINT8 AddPShader( D3D11PShader* s ) {
		std::unique_lock<std::mutex> lock( PShadersByIDMutex );
		UINT8 id = 0;
		if ( !D3D11ObjectIDs::PShadersByID.empty() ) {
			id = D3D11ObjectIDs::Counters.PShadersCounter++;
		}
		D3D11ObjectIDs::PShadersByID[id] = s;
		return id;
	}
	static UINT8 AddHDShader( D3D11HDShader* s ) {
		std::unique_lock<std::mutex> lock( HDShadersByIDMutex );
		UINT8 id = 0;
		if ( !D3D11ObjectIDs::HDShadersByID.empty() ) {
			id = D3D11ObjectIDs::Counters.HDShadersCounter++;
		}
		D3D11ObjectIDs::HDShadersByID[id] = s;
		return id;
	}
	static UINT8 AddGShader( D3D11GShader* s ) {
		std::unique_lock<std::mutex> lock( GShadersByIDMutex );
		UINT8 id = 0;
		if ( !D3D11ObjectIDs::GShadersByID.empty() ) {
			id = D3D11ObjectIDs::Counters.GShadersCounter++;
		}
		D3D11ObjectIDs::GShadersByID[id] = s;
		return id;
	}

	static void EraseVShader( D3D11VShader* s ) {
		std::unique_lock<std::mutex> lock( VShadersByIDMutex );
		for ( auto it = D3D11ObjectIDs::VShadersByID.begin(); it != D3D11ObjectIDs::VShadersByID.end();) {
			if ( it->second == s ) { it = D3D11ObjectIDs::VShadersByID.erase( it ); } else { ++it; }
		}
	}
	static void ErasePShader( D3D11PShader* s ) {
		std::unique_lock<std::mutex> lock( PShadersByIDMutex );
		for ( auto it = D3D11ObjectIDs::PShadersByID.begin(); it != D3D11ObjectIDs::PShadersByID.end();) {
			if ( it->second == s ) { it = D3D11ObjectIDs::PShadersByID.erase( it ); } else { ++it; }
		}
	}
	static void EraseHDShader( D3D11HDShader* s ) {
		std::unique_lock<std::mutex> lock( HDShadersByIDMutex );
		for ( auto it = D3D11ObjectIDs::HDShadersByID.begin(); it != D3D11ObjectIDs::HDShadersByID.end();) {
			if ( it->second == s ) { it = D3D11ObjectIDs::HDShadersByID.erase( it ); } else { ++it; }
		}
	}
	static void EraseGShader( D3D11GShader* s ) {
		std::unique_lock<std::mutex> lock( GShadersByIDMutex );
		for ( auto it = D3D11ObjectIDs::GShadersByID.begin(); it != D3D11ObjectIDs::GShadersByID.end();) {
			if ( it->second == s ) { it = D3D11ObjectIDs::GShadersByID.erase( it ); } else { ++it; }
		}
	}
private:
	inline static std::mutex VShadersByIDMutex;
	inline static std::mutex PShadersByIDMutex;
	inline static std::mutex HDShadersByIDMutex;
	inline static std::mutex GShadersByIDMutex;
};

struct RenderToTextureBuffer;
struct RenderToDepthStencilBuffer;
class D3D11ShaderManager;

class D3D11VertexBuffer;
class D3D11LineRenderer;
class D3D11ConstantBuffer;

class D3D11GraphicsEngineBase : public BaseGraphicsEngine {
public:
	D3D11GraphicsEngineBase();
	~D3D11GraphicsEngineBase();

	/** Called after the fake-DDraw-Device got created */
	virtual XRESULT Init();

	/** Called when the game created its window */
	virtual XRESULT SetWindow( HWND hWnd );

	/** Called on window resize/resolution change */
	virtual XRESULT OnResize( INT2 newSize );

	/** Called when the game wants to render a new frame */
	virtual XRESULT OnBeginFrame();

	/** Called when the game ended it's frame */
	virtual XRESULT OnEndFrame();

	/** Called to set the current viewport */
	virtual XRESULT SetViewport( const ViewportInfo& viewportInfo );

	/** Called when the game wants to clear the bound rendertarget */
	virtual XRESULT Clear( const float4& color );

	/** Creates a vertexbuffer object (Not registered inside) */
	virtual XRESULT CreateVertexBuffer( D3D11VertexBuffer** outBuffer );

	/** Creates a texture object (Not registered inside) */
	virtual XRESULT CreateTexture( D3D11Texture** outTexture );

	/** Creates a constantbuffer object (Not registered inside) */
	virtual XRESULT CreateConstantBuffer( D3D11ConstantBuffer** outCB, void* data, int size );

	/** Creates a bufferobject for a shadowed point light */
	virtual XRESULT CreateShadowedPointLight( BaseShadowedPointLight** outPL, VobLightInfo* lightInfo, bool dynamic = false );

	/** Returns a list of available display modes */
	virtual XRESULT GetDisplayModeList( std::vector<DisplayModeInfo>* modeList, bool includeSuperSampling = false );

	/** Presents the current frame to the screen */
	virtual XRESULT Present();

	/** Called when we started to render the world */
	virtual XRESULT OnStartWorldRendering();

	/** Returns the line renderer object */
	virtual BaseLineRenderer* GetLineRenderer();

	/** Returns the graphics-device this is running on */
	virtual std::string GetGraphicsDeviceName();

	/** Saves a screenshot */
	virtual void SaveScreenshot() {}

	/** Returns the shadermanager */
	D3D11ShaderManager& GetShaderManager();

	/** Draws a vertexarray, used for rendering gothics UI */
	virtual XRESULT DrawVertexArray( ExVertexStruct* vertices, unsigned int numVertices, unsigned int startVertex = 0, unsigned int stride = sizeof( ExVertexStruct ) );

	/** Draws a vertexbuffer, non-indexed, binding the FF-Pipe values */
	virtual XRESULT DrawVertexBufferFF( D3D11VertexBuffer* vb, unsigned int numVertices, unsigned int startVertex, unsigned int stride = sizeof( ExVertexStruct ) );

	/** Binds viewport information to the given constantbuffer slot */
	XRESULT D3D11GraphicsEngineBase::BindViewportInformation( const std::string& shader, int slot );

	/** Returns the Device/Context */
	const Microsoft::WRL::ComPtr<ID3D11Device>& GetDevice() { return Device; }
	const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& GetContext() { return Context; }
	const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& GetDeferredMediaContext() { return DeferredContext; }

	/** Returns the current resolution */
	virtual INT2 GetResolution() { return Resolution; }

	/** Recreates the renderstates */
	XRESULT UpdateRenderStates();

	/** Constructs the makro list for shader compilation */
	static void ConstructShaderMakroList( std::vector<D3D_SHADER_MACRO>& list );

	/** Sets up the default rendering state */
	void SetDefaultStates();

	/** Sets up a draw call for a VS_Ex-Mesh */
	void SetupVS_ExMeshDrawCall();
	void SetupVS_ExConstantBuffer();
	void SetupVS_ExPerInstanceConstantBuffer();

	/** Puts the current world matrix into a CB and binds it to the given slot */
	void SetupPerInstanceConstantBuffer( int slot = 1 );

	/** Sets the active pixel shader object */
	virtual XRESULT SetActivePixelShader( const std::string& shader );
	virtual XRESULT SetActiveVertexShader( const std::string& shader );
	virtual XRESULT SetActiveHDShader( const std::string& shader );
	virtual XRESULT SetActiveGShader( const std::string& shader );
	//virtual int MeasureString(std::string str, zFont* zFont);

protected:
	/** Updates the transformsCB with new values from the GAPI */
	void UpdateTransformsCB();

	/** Device-objects */
	Microsoft::WRL::ComPtr<IDXGIFactory2> DXGIFactory2;
	Microsoft::WRL::ComPtr<IDXGIAdapter> DXGIAdapter;
	std::string DeviceDescription;

	Microsoft::WRL::ComPtr<ID3D11Device> Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeferredContext;

	/** Deferred contexts for threadpool */
	std::map<int, ID3D11DeviceContext*> DeferredContextsByThread;
	std::vector<ID3D11DeviceContext*> DeferredContextCache;
	std::vector<ID3D11DeviceContext*> DeferredContextsAll;
	std::mutex DeferredContextsByThreadMutex;

	/** Swapchain and resources */
	Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain;
	std::unique_ptr<RenderToTextureBuffer> Backbuffer;
	std::unique_ptr<RenderToDepthStencilBuffer> DepthStencilBuffer;
	std::unique_ptr<RenderToTextureBuffer> HDRBackBuffer;

	/** States */
	Microsoft::WRL::ComPtr<ID3D11SamplerState> DefaultSamplerState;

	/** Output-window (Gothics main window)*/
	HWND OutputWindow;

	/** Total resolution we are rendering at */
	INT2 Resolution;

	/** Shader manager */
	std::unique_ptr<D3D11ShaderManager> ShaderManager;

	/** Dynamic buffer for vertex array rendering */
	std::unique_ptr<D3D11VertexBuffer> TempVertexBuffer;

	/** Constantbuffers */
	std::unique_ptr<D3D11ConstantBuffer> TransformsCB; // Holds View/Proj-Transforms

	/** Shaders */
	std::shared_ptr<D3D11PShader> PS_DiffuseNormalmapped;
	std::shared_ptr<D3D11PShader> PS_DiffuseNormalmappedFxMap;
	std::shared_ptr<D3D11PShader> PS_Diffuse;
	std::shared_ptr<D3D11PShader> PS_DiffuseNormalmappedAlphatest;
	std::shared_ptr<D3D11PShader> PS_DiffuseNormalmappedAlphatestFxMap;
	std::shared_ptr<D3D11PShader> PS_DiffuseAlphatest;
	std::shared_ptr<D3D11PShader> PS_Simple;
	std::shared_ptr<D3D11PShader> PS_SimpleAlphaTest;
	std::shared_ptr<D3D11PShader> PS_LinDepth;
	std::shared_ptr<D3D11VShader> VS_Ex;
	std::shared_ptr<D3D11VShader> VS_ExInstancedObj;
	std::shared_ptr<D3D11VShader> VS_ExRemapInstancedObj;
	std::shared_ptr<D3D11VShader> VS_ExSkeletal;
	std::shared_ptr<D3D11GShader> GS_Billboard;

	std::shared_ptr<D3D11VShader> ActiveVS;
	std::shared_ptr<D3D11PShader> ActivePS;
	std::shared_ptr<D3D11HDShader> ActiveHDS;
	std::shared_ptr<D3D11GShader> ActiveGS;

	/** FixedFunction-State render states */
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> FFRasterizerState;
	size_t FFRasterizerStateHash;
	Microsoft::WRL::ComPtr<ID3D11BlendState> FFBlendState;
	size_t FFBlendStateHash;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> FFDepthStencilState;
	size_t FFDepthStencilStateHash;

	/** Debug line-renderer */
	std::unique_ptr<D3D11LineRenderer> LineRenderer;

	/** If true, we are still waiting for a present to happen. Don't draw everything twice! */
	bool PresentPending;
};
