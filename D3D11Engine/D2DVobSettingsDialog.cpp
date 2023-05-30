#include "pch.h"
#include "D2DVobSettingsDialog.h"
#include "D2DView.h"
#include "SV_Label.h"
#include "SV_Checkbox.h"
#include "SV_Panel.h"
#include "SV_Slider.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "BaseGraphicsEngine.h"
#include "SV_GMeshInfoView.h"
#include "zCMaterial.h"
#include "SV_NamedSlider.h"
#include "zCVisual.h"

D2DVobSettingsDialog::D2DVobSettingsDialog( D2DView* view, D2DSubView* parent ) : D2DDialog( view, parent ) {
    SetPositionCentered( D2D1::Point2F( view->GetRenderTarget()->GetSize().width / 2, view->GetRenderTarget()->GetSize().height / 2 ), D2D1::SizeF( 700, 450 ) );
    Header->SetCaption( L"VOB Settings" );

    Vob = nullptr;

    InitControls();
}


D2DVobSettingsDialog::~D2DVobSettingsDialog() {}

/** Initializes the controls of this view */
XRESULT D2DVobSettingsDialog::InitControls() {
    D2DSubView::InitControls();

    AddButton( "Close", CloseButtonPressed, this );
    //AddButton("[*] Apply", ApplyButtonPressed, this);

    MeshView = new SV_GMeshInfoView( MainView, MainPanel );
    MeshView->SetPositionAndSize( D2D1::Point2F( 10, 10 ), D2D1::SizeF( GetSize().height - Header->GetSize().height, GetSize().height - Header->GetSize().height ) );
    MeshView->AlignUnder( Header, 1 );

    float textwidth = 80;
    RenderMode = new SV_NamedSlider( MainView, MainPanel );
    RenderMode->AlignRightTo( MeshView, 10 );
    RenderMode->GetLabel()->SetCaption( L"Draw mode:" );
    RenderMode->GetLabel()->SetSize( D2D1::SizeF( textwidth, RenderMode->GetLabel()->GetSize().height ) );
    RenderMode->GetSlider()->SetPositionAndSize( D2D1::Point2F( 0, 0 ), D2D1::SizeF( 150, 15 ) );
    RenderMode->UpdateDimensions();
    RenderMode->GetSlider()->SetSliderChangedCallback( SliderDragged, this );
    RenderMode->GetSlider()->SetMinMax( 0.0f, 3.0f );

    std::vector<std::string> modes;
    modes.emplace_back( "Wireframe" );
    modes.emplace_back( "SolidWireframe" );
    modes.emplace_back( "Textured" );
    modes.emplace_back( "Lit" );
    RenderMode->GetSlider()->SetDisplayValues( modes );
    RenderMode->GetSlider()->SetIsIntegralSlider( true );
    RenderMode->GetSlider()->SetValue( 3 );

    TesselationFactorSetting = new SV_NamedSlider( MainView, MainPanel );
    TesselationFactorSetting->AlignRightTo( MeshView, 10 );
    TesselationFactorSetting->GetLabel()->SetCaption( L"Tesselation:" );
    TesselationFactorSetting->GetLabel()->SetSize( D2D1::SizeF( textwidth, TesselationFactorSetting->GetLabel()->GetSize().height ) );
    TesselationFactorSetting->GetSlider()->SetPositionAndSize( D2D1::Point2F( 0, 0 ), D2D1::SizeF( 150, 15 ) );
    TesselationFactorSetting->UpdateDimensions();
    TesselationFactorSetting->GetSlider()->SetSliderChangedCallback( SliderDragged, this );
    TesselationFactorSetting->GetSlider()->SetValue( 0.0f );
    TesselationFactorSetting->AlignUnder( RenderMode, 5.0f );

    RoundnessSetting = new SV_NamedSlider( MainView, MainPanel );
    RoundnessSetting->AlignRightTo( MeshView );
    RoundnessSetting->GetLabel()->SetCaption( L"Roundness:" );
    RoundnessSetting->GetLabel()->SetSize( D2D1::SizeF( textwidth, RoundnessSetting->GetLabel()->GetSize().height ) );
    RoundnessSetting->GetSlider()->SetPositionAndSize( D2D1::Point2F( 0, 0 ), D2D1::SizeF( 150, 15 ) );
    RoundnessSetting->UpdateDimensions();
    RoundnessSetting->GetSlider()->SetSliderChangedCallback( SliderDragged, this );
    RoundnessSetting->AlignUnder( TesselationFactorSetting, 5.0f );
    RoundnessSetting->GetSlider()->SetValue( 1.0f );

    DisplacementStrengthSetting = new SV_NamedSlider( MainView, MainPanel );
    DisplacementStrengthSetting->AlignRightTo( MeshView );
    DisplacementStrengthSetting->GetLabel()->SetCaption( L"Displacement:" );
    DisplacementStrengthSetting->GetLabel()->SetSize( D2D1::SizeF( textwidth, DisplacementStrengthSetting->GetLabel()->GetSize().height ) );
    DisplacementStrengthSetting->GetSlider()->SetPositionAndSize( D2D1::Point2F( 0, 0 ), D2D1::SizeF( 150, 15 ) );
    DisplacementStrengthSetting->UpdateDimensions();
    DisplacementStrengthSetting->GetSlider()->SetSliderChangedCallback( SliderDragged, this );
    DisplacementStrengthSetting->AlignUnder( RoundnessSetting, 5.0f );
    DisplacementStrengthSetting->GetSlider()->SetMinMax( -2.0f, 2.0f );
    DisplacementStrengthSetting->GetSlider()->SetValue( 0.5f );

    return XR_SUCCESS;
}

/** Close button */
void D2DVobSettingsDialog::CloseButtonPressed( SV_Button* sender, void* userdata ) {
    D2DVobSettingsDialog* d = reinterpret_cast<D2DVobSettingsDialog*>(userdata);
    d->SetHidden( true );
}

void D2DVobSettingsDialog::SliderDragged( SV_Slider* sender, void* userdata ) {
    D2DVobSettingsDialog* d = reinterpret_cast<D2DVobSettingsDialog*>(userdata);

    if ( !d->Vob )
        return;

#if ENABLE_TESSELATION > 0
    VisualTesselationSettings* ts = nullptr;
    if ( d->Vob )
        ts = &d->Vob->VisualInfo->TesselationInfo;
#endif

    if ( sender == d->DisplacementStrengthSetting->GetSlider() ) {
#if ENABLE_TESSELATION > 0
        float oldValue = ts->buffer.VT_DisplacementStrength;
        ts->buffer.VT_DisplacementStrength = sender->GetValue();

        if ( ts->buffer.VT_DisplacementStrength == 0.0f ) {
            // If this is the case, we just set it to 0.0f from something higher. Unsmooth the normals!

            if ( d->Vob ) {
                d->Vob->VisualInfo->ClearPNAENInfo();
                d->Vob->VisualInfo->CreatePNAENInfo( false );
            }
        } else if ( oldValue == 0.0f ) {
            // Here we just set it to something higher than 0.0f, Smooth the normals!
            if ( d->Vob ) {
                d->Vob->VisualInfo->ClearPNAENInfo();
                d->Vob->VisualInfo->CreatePNAENInfo( true );
            }
        }
#endif
    } else if ( sender == d->RoundnessSetting->GetSlider() ) {
#if ENABLE_TESSELATION > 0
        ts->buffer.VT_Roundness = sender->GetValue();
#endif
    } else if ( sender == d->TesselationFactorSetting->GetSlider() ) {
#if ENABLE_TESSELATION > 0
        ts->buffer.VT_TesselationFactor = sender->GetValue();

        if ( ts->buffer.VT_TesselationFactor > 0.0f ) {
            if ( d->Vob )
                d->Vob->VisualInfo->CreatePNAENInfo( ts->buffer.VT_DisplacementStrength > 0.0f ); // This only creates missing infos
        }
#endif
    } else if ( sender == d->RenderMode->GetSlider() ) {
        d->MeshView->SetRenderMode( static_cast<SV_GMeshInfoView::ERenderMode>(sender->GetValue() + 0.5f) );
    }

#if ENABLE_TESSELATION > 0
    // Save changes
    d->Vob->VisualInfo->SaveMeshVisualInfo( d->Vob->VisualInfo->VisualName );

    // Update everything
    ts->UpdateConstantbuffer();
#endif
    d->MeshView->UpdateView();
}

/** Sets the Vob to do settings on */
void D2DVobSettingsDialog::SetVobInfo( BaseVobInfo* vob ) {
#ifdef BUILD_SPACER
    return;
#endif

    Vob = nullptr; // Reset the dialog first without messing with the vobs settings

    std::map<zCTexture*, MeshInfo*> meshes;

    // Filter meshes by texture
    for ( auto& [material, vmeshes] : vob->VisualInfo->Meshes ) {
        if ( !material )
            continue;

        if ( !vmeshes.empty() ) {
            meshes[material->GetTexture()] = vmeshes.front();
        }
    }

#if ENABLE_TESSELATION > 0
    RoundnessSetting->GetSlider()->SetValue( vob->VisualInfo->TesselationInfo.buffer.VT_Roundness );
    TesselationFactorSetting->GetSlider()->SetValue( vob->VisualInfo->TesselationInfo.buffer.VT_TesselationFactor );
    DisplacementStrengthSetting->GetSlider()->SetValue( vob->VisualInfo->TesselationInfo.buffer.VT_DisplacementStrength );
#else
    RoundnessSetting->GetSlider()->SetValue( 0.f );
    TesselationFactorSetting->GetSlider()->SetValue( 0.f );
    DisplacementStrengthSetting->GetSlider()->SetValue( 0.f );
#endif

    Vob = vob;

    // Set them
    MeshView->SetMeshes( meshes, vob->VisualInfo );
    MeshView->UpdateView();

    Header->SetCaption( Toolbox::ToWideChar( "Visual Settings (" + Vob->VisualInfo->VisualName + ")" ) );
}
