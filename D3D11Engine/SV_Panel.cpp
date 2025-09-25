#include "SV_Panel.h"

#include "pch.h"
#include "D2DView.h"
#include "D3D11GraphicsEngineBase.h"
#include "Engine.h"
using namespace Microsoft::WRL;

SV_Panel::SV_Panel( D2DView* view, D2DSubView* parent ) : D2DSubView( view, parent ) {
    PanelColor = D2D1::ColorF( 1, 1, 1, 1 );
    HasShadow = true;
    RenderMode = PR_Background;
    ShadowRange = 20.0f;
    Image = nullptr;
    HasDarkOverlay = false;
    HasGlossyOutline = false;
}

SV_Panel::~SV_Panel() {
    SAFE_RELEASE( Image );
}

/** Draws this sub-view */
void SV_Panel::Draw( const D2D1_RECT_F& clientRectAbs, float deltaTime ) {
    //Set up the layer for this control
    MainView->GetRenderTarget()->SetTransform( D2D1::Matrix3x2F::Translation( clientRectAbs.left, clientRectAbs.top ) );

    if ( HasShadow )
        MainView->DrawSmoothShadow( &ViewRect, ShadowRange, 0.8f, false, 7 );

    if ( RenderMode == PR_SolidColor ) {
        MainView->GetBrush()->SetColor( PanelColor );
        MainView->GetRenderTarget()->FillRectangle( ViewRect, MainView->GetBrush() );
    } else if ( RenderMode == PR_Background ) {
        float darkness = 1.0f - powf( 0.7f, static_cast<float>(Level) );

        MainView->GetBackgroundBrush()->SetStartPoint( D2D1::Point2F( -clientRectAbs.left, -clientRectAbs.top ) );
        MainView->GetBackgroundBrush()->SetEndPoint( D2D1::Point2F( clientRectAbs.right - clientRectAbs.left, clientRectAbs.bottom - clientRectAbs.top ) );

        MainView->GetRenderTarget()->FillRectangle( D2D1::RectF( ViewRect.left, ViewRect.top, ViewRect.right, ViewRect.bottom ), MainView->GetBackgroundBrush() );

        MainView->GetBrush()->SetColor( D2D1::ColorF( 0, 0, 0, darkness ) );
        MainView->GetRenderTarget()->FillRectangle( ViewRect, MainView->GetBrush() );
    } else if ( RenderMode == PR_Image ) {
        if ( Image ) {
            MainView->GetRenderTarget()->DrawBitmap( Image, ViewRect );
        } else {
            MainView->GetBrush()->SetColor( D2D1::ColorF( 0, 0, 0, 1 ) );
            MainView->GetRenderTarget()->FillRectangle( ViewRect, MainView->GetBrush() );
        }
    }

    if ( HasDarkOverlay ) {
        D2D1_RECT_F rr = ViewRect;
        D2DView::ShrinkRect( &rr, 3 );

        MainView->GetBrush()->SetColor( D2D1::ColorF( 0, 0, 0, 0.4f ) );
        MainView->GetRenderTarget()->FillRoundedRectangle( D2D1::RoundedRect( rr, 3, 3 ), MainView->GetBrush() );
    }

    if ( HasGlossyOutline ) {
        // Draw outer metallic deco-line
        MainView->GetLinearReflectBrush()->SetStartPoint( D2D1::Point2F( ViewRect.left, ViewRect.top ) );
        MainView->GetLinearReflectBrush()->SetEndPoint( D2D1::Point2F( ViewRect.right, ViewRect.bottom ) );
        MainView->GetRenderTarget()->DrawRectangle( ViewRect, MainView->GetLinearReflectBrush() );

        MainView->GetBrush()->SetColor( SV_DEF_INNER_LINE_COLOR );

        // Draw inner deco-line
        D2D1_RECT_F r = ViewRect;
        D2DView::ShrinkRect( &r, 1 );
        MainView->GetRenderTarget()->DrawRectangle( r, MainView->GetBrush() );
    }

    D2DSubView::Draw( clientRectAbs, deltaTime );
}

/** Sets the border color */
void SV_Panel::SetPanelColor( const D2D1_COLOR_F& color ) {
    PanelColor = color;
}

/** Set if this panel should have a shadow */
void SV_Panel::SetPanelShadow( bool shadow, float range ) {
    HasShadow = shadow;
    ShadowRange = range;
}

/** Set if this panel should render as a solid color */
void SV_Panel::SetRenderMode( EPanelRenderMode mode ) {
    RenderMode = mode;
}

/** Returns the panels solid color */
D2D1_COLOR_F SV_Panel::GetPanelColor() const {
    return PanelColor;
}

/** Sets the image of this panel from d3d11 */
HRESULT SV_Panel::SetD3D11TextureAsImage( ID3D11Texture2D* texture, INT2 size ) {
    SAFE_RELEASE( Image );

    D3D11GraphicsEngineBase* engine = reinterpret_cast<D3D11GraphicsEngineBase*>(Engine::GraphicsEngine);
    HRESULT hr;
    // Since D2D can't load DXTn-Textures on Windows 7, we copy it over to a smaller texture here in d3d11

    // Create texture
    CD3D11_TEXTURE2D_DESC textureDesc( DXGI_FORMAT_B8G8R8A8_UNORM, size.x, size.y, 1, 1, 0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ, 1, 0, 0 );

    ComPtr<ID3D11Texture2D> staging;
    LE( engine->GetDevice()->CreateTexture2D( &textureDesc, nullptr, staging.ReleaseAndGetAddressOf() ) );

    engine->GetContext()->CopyResource( staging.Get(), texture );

    D2D1_BITMAP_PROPERTIES properties;
    properties = D2D1::BitmapProperties( D2D1::PixelFormat( DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE ), 0, 0 );

    // Get data out
    D3D11_MAPPED_SUBRESOURCE mapped;
    engine->GetContext()->Map( staging.Get(), 0, D3D11_MAP_READ, 0, &mapped );
    MainView->GetRenderTarget()->CreateBitmap( D2D1::SizeU( size.x, size.y ), mapped.pData, mapped.RowPitch, properties, &Image );
    engine->GetContext()->Unmap( staging.Get(), 0 );

    return S_OK;
}

/** Set if this should have a dark overlay on top */
void SV_Panel::SetDarkOverlay( bool value ) {
    HasDarkOverlay = value;
}

/** Set if this should have a glossy outline */
void SV_Panel::SetGlossyOutline( bool value ) {
    HasGlossyOutline = value;
}
