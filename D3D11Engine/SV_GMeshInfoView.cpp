#include "SV_GMeshInfoView.h"

#include "D2DView.h"
#include "D3D11GraphicsEngine.h"
#include "RenderToTextureBuffer.h"
#include "SV_Panel.h"
#include "zCTexture.h"

const float MESH_ROT_SPEED = 0.01f;
const float ZOOM_SPEED = 0.01f;
const float DEFAULT_FOV = 90.0f;

SV_GMeshInfoView::SV_GMeshInfoView( D2DView* view, D2DSubView* parent ) : D2DSubView( view, parent ) {
	Panel = new SV_Panel( view, this );
	//Panel->SetRect(D2D1::RectF(0, SV_TABCONTROL_HEADER_SIZE_Y, GetSize().width, GetSize().height));
	Panel->SetRenderMode( SV_Panel::PR_Image );
	Panel->SetGlossyOutline( true );

	IsDraggingView = false;

	ObjectPosition = XMVectorZero();
	SetObjectOrientation( 0, 0, 10.0f );
	RT = nullptr;
	DS = nullptr;
	VisualInfo = nullptr;
	RenderMode = RM_Lit;

	FOV = DEFAULT_FOV;
}

SV_GMeshInfoView::~SV_GMeshInfoView() {
	delete RT;
	delete DS;
}

/** Sets the current render mode */
void SV_GMeshInfoView::SetRenderMode( ERenderMode mode ) {
	RenderMode = mode;
}

/** Sets the mesh infos for this view */
void SV_GMeshInfoView::SetMeshes( const std::map<zCTexture*, MeshInfo*>& meshes, BaseVisualInfo* visInfo ) {
	Meshes = meshes;
	VisualInfo = visInfo;
	FOV = DEFAULT_FOV;

	// Find boundingbox of skeletal
	XMFLOAT3 bbmin = XMFLOAT3( FLT_MAX, FLT_MAX, FLT_MAX );
	XMFLOAT3 bbmax = XMFLOAT3( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	for ( auto it = Meshes.cbegin(); it != Meshes.cend(); ++it ) {
		std::vector<ExVertexStruct>& vertices = it->second->Vertices;
		for ( unsigned int i = 0; i < vertices.size(); i++ ) {
			// Check bounding box
			bbmin.x = bbmin.x > vertices[i].Position.x ? vertices[i].Position.x : bbmin.x;
			bbmin.y = bbmin.y > vertices[i].Position.y ? vertices[i].Position.y : bbmin.y;
			bbmin.z = bbmin.z > vertices[i].Position.z ? vertices[i].Position.z : bbmin.z;

			bbmax.x = bbmax.x < vertices[i].Position.x ? vertices[i].Position.x : bbmax.x;
			bbmax.y = bbmax.y < vertices[i].Position.y ? vertices[i].Position.y : bbmax.y;
			bbmax.z = bbmax.z < vertices[i].Position.z ? vertices[i].Position.z : bbmax.z;
		}
	}

	FXMVECTOR XMV_bbmin = XMLoadFloat3( &bbmin );
	FXMVECTOR XMV_bbmax = XMLoadFloat3( &bbmax );

	ObjectPosition = (XMV_bbmin + XMV_bbmax) * -0.5f;
	float distance;
	XMStoreFloat( &distance, XMVector3Length( XMV_bbmin - XMV_bbmin ) * 0.5f );
	SetObjectOrientation( 0, 0, distance );
}

/** Sets the name of this view */
void SV_GMeshInfoView::SetName( const std::string& name ) {}

/** Sets the rotation of this object in the view */
void SV_GMeshInfoView::SetObjectOrientation( float yaw, float pitch, float distance ) {

	ObjectWorldMatrix = XMMatrixTranslation(XMVectorGetX(ObjectPosition), XMVectorGetY(ObjectPosition), XMVectorGetZ(ObjectPosition));

	XMMATRIX rotY = XMMatrixRotationY( yaw );
	XMMATRIX rotZ = XMMatrixRotationZ( pitch );

	ObjectWorldMatrix = XMMatrixTranspose(ObjectWorldMatrix * rotY * rotZ);

	constexpr XMVECTORF32 c_XM_0100 = { 0, 1, 0, 0 };
	ObjectViewMatrix = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(-distance, 0, 0, 0), g_XMZero, c_XM_0100) );
}

/** Updates the view */
void SV_GMeshInfoView::UpdateView() {
	if ( IsHidden() )
		return;

	D3D11GraphicsEngine* g = reinterpret_cast<D3D11GraphicsEngine*>(Engine::GraphicsEngine);

	ObjectProjMatrix = XMMatrixTranspose( XMMatrixPerspectiveFovLH(XMConvertToRadians(FOV), GetSize().height / GetSize().width, 0.01f, 10000.0f) );

	g->SetDefaultStates();
	Engine::GAPI->GetRendererState().RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState().RasterizerState.SetDirty();

	XMMATRIX oldProj = XMLoadFloat4x4(&Engine::GAPI->GetProjTransform());

	// Set transforms
	Engine::GAPI->SetWorldTransformXM(ObjectWorldMatrix);
	Engine::GAPI->SetViewTransformXM(ObjectViewMatrix);
	Engine::GAPI->SetProjTransformXM(ObjectProjMatrix);

	// Set Viewport
	D3D11_VIEWPORT oldVP;
	UINT numVP = 1;
	g->GetContext()->RSGetViewports( &numVP, &oldVP );

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = GetSize().width;
	vp.Height = GetSize().height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	g->GetContext()->RSSetViewports( 1, &vp );

	// Clear
	g->GetContext()->ClearRenderTargetView( RT->GetRenderTargetView().Get(), reinterpret_cast<float*>(&float4( 0, 0, 0, 0 )) );
	g->GetContext()->ClearDepthStencilView( DS->GetDepthStencilView().Get(), D3D11_CLEAR_DEPTH, 1.0f, 0 );

	// Bind RTV
	g->GetContext()->OMSetRenderTargets( 1, RT->GetRenderTargetView().GetAddressOf(), DS->GetDepthStencilView().Get() );

	// Setup shaders
	g->SetActiveVertexShader( "VS_Ex" );
	g->SetActivePixelShader( "PS_DiffuseAlphaTest" );

	switch ( RenderMode ) {
	case RM_Lit:
		g->SetActivePixelShader( "PS_Preview_TexturedLit" );
		DrawMeshes();
		break;

	case RM_Textured:
		g->SetActivePixelShader( "PS_Preview_Textured" );
		DrawMeshes();
		break;

	case RM_TexturedWireFrame:
		g->SetActivePixelShader( "PS_Preview_Textured" );
		DrawMeshes();
		// No break here, render wireframe right after

	case RM_Wireframe:
		g->SetActivePixelShader( "PS_Preview_White" );
		Engine::GAPI->GetRendererState().RasterizerState.Wireframe = true;
		Engine::GAPI->GetRendererState().RasterizerState.SetDirty();
		DrawMeshes();
		Engine::GAPI->GetRendererState().RasterizerState.Wireframe = false;
		Engine::GAPI->GetRendererState().RasterizerState.SetDirty();
		break;
	}
	// Reset viewport
	g->GetContext()->RSSetViewports( 1, &oldVP );
	Engine::GAPI->SetProjTransformXM(oldProj);

	// Update panel
	Panel->SetD3D11TextureAsImage( RT->GetTexture().Get(), INT2( RT->GetSizeX(), RT->GetSizeY() ) );
}

/** Draws the meshes to the buffer */
void SV_GMeshInfoView::DrawMeshes() {
	D3D11GraphicsEngine* g = reinterpret_cast<D3D11GraphicsEngine*>(Engine::GraphicsEngine);

	g->SetupVS_ExMeshDrawCall();
	g->SetupVS_ExConstantBuffer();
	g->SetupVS_ExPerInstanceConstantBuffer();

#if ENABLE_TESSELATION > 0
	VisualTesselationSettings* ts = nullptr;
	if ( VisualInfo )
		ts = &VisualInfo->TesselationInfo;
#endif

	// Draw each texture
	for ( auto it = Meshes.cbegin(); it != Meshes.cend(); ++it ) {
#if ENABLE_TESSELATION > 0
		// Set up tesselation if wanted
		if ( ts && !it->second->IndicesPNAEN.empty() && ts->buffer.VT_TesselationFactor > 0.0f ) {
			g->Setup_PNAEN( D3D11GraphicsEngine::PNAEN_Default );
			ts->Constantbuffer->BindToDomainShader( 1 );
			ts->Constantbuffer->BindToHullShader( 1 );

			if ( it->first->CacheIn( -1 ) == zRES_CACHED_IN ) {
				// Draw
				it->first->Bind( 0 );
				g->DrawVertexBufferIndexed( it->second->MeshVertexBuffer, it->second->MeshIndexBufferPNAEN, it->second->IndicesPNAEN.size() );
			}
		} else if ( VisualInfo )
#endif
        {
			g->GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
			g->GetContext()->DSSetShader( nullptr, nullptr, 0 );
			g->GetContext()->HSSetShader( nullptr, nullptr, 0 );
			g->SetActiveHDShader( "" );
			g->SetActiveVertexShader( "VS_Ex" );

			if ( it->first && it->first->CacheIn( -1 ) == zRES_CACHED_IN ) {
				// Draw
				it->first->Bind( 0 );
				g->DrawVertexBufferIndexed( it->second->MeshVertexBuffer, it->second->MeshIndexBuffer, it->second->Indices.size() );
			}
		}
	}
}

/** Sets the position and size of this sub-view */
void SV_GMeshInfoView::SetRect( const D2D1_RECT_F& rect ) {
	D3D11GraphicsEngine* g = reinterpret_cast<D3D11GraphicsEngine*>(Engine::GraphicsEngine);

	D2DSubView::SetRect( rect );

	Panel->SetRect( D2D1::RectF( 0, 0, GetSize().width, GetSize().height ) );

	// Create new RT
	delete RT;
	RT = new RenderToTextureBuffer( g->GetDevice().Get(), std::max<UINT>( 8, GetSize().width ), std::max<UINT>( 8, GetSize().height ), DXGI_FORMAT_R8G8B8A8_UNORM );

	delete DS;
	DS = new RenderToDepthStencilBuffer( g->GetDevice().Get(), std::max<UINT>( 8, GetSize().width ), std::max<UINT>( 8, GetSize().height ), DXGI_FORMAT_R32_TYPELESS, nullptr, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT );
}

/** Processes a window-message. Return false to stop the message from going to children */
bool SV_GMeshInfoView::OnWindowMessage( HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs ) {
	switch ( msg ) {
	case WM_MOUSEMOVE:
		if ( IsDraggingView ) {
			POINT p = D2DView::GetCursorPosition();

			float yaw, pitch, distance;
			GetObjectOrientation( yaw, pitch, distance );

			yaw += (LastDragPosition.x - p.x) * MESH_ROT_SPEED;
			pitch -= (LastDragPosition.y - p.y) * MESH_ROT_SPEED;

			SetObjectOrientation( yaw, pitch, distance );
			UpdateView();

			LastDragPosition = p;
			return false;
		}
		break;

	case WM_LBUTTONDOWN:
	{
		POINT p = D2DView::GetCursorPosition();

		D2D1_RECT_F panelrect = D2D1::RectF( Panel->GetPosition().x + clientRectAbs.left, Panel->GetPosition().y + clientRectAbs.top, Panel->GetSize().width + clientRectAbs.left, Panel->GetSize().height + clientRectAbs.top );
		if ( PointInsideRect( D2D1::Point2F( static_cast<float>(p.x), static_cast<float>(p.y) ), panelrect ) ) {
			IsDraggingView = true;
			LastDragPosition = p;
			return false;
		}
		break;
	}
	case WM_LBUTTONUP:
		if ( IsDraggingView ) {
			IsDraggingView = false;
			return false;
		}
		break;

	case WM_MOUSEWHEEL:
	{
		POINT p = D2DView::GetCursorPosition();

		D2D1_RECT_F panelrect = D2D1::RectF( Panel->GetPosition().x + clientRectAbs.left, Panel->GetPosition().y + clientRectAbs.top, Panel->GetSize().width + clientRectAbs.left, Panel->GetSize().height + clientRectAbs.top );
		if ( PointInsideRect( D2D1::Point2F( static_cast<float>(p.x), static_cast<float>(p.y) ), panelrect ) ) {
			FOV += GET_WHEEL_DELTA_WPARAM( wParam ) * ZOOM_SPEED;
			UpdateView();
		}
		break;
	}
	}
	return D2DSubView::OnWindowMessage( hWnd, msg, wParam, lParam, clientRectAbs );
}

/** Returns the view-properties */
void SV_GMeshInfoView::GetObjectOrientation( float& yaw, float& pitch, float& distance ) {
	yaw = ObjectYaw;
	pitch = ObjectPitch;
	distance = ObjectDistance;
}
