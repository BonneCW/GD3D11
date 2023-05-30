#pragma once
#include "BaseGraphicsEngine.h"
#include "d2ddialog.h"
#include "GothicGraphicsState.h"

class SV_Slider;
class SV_Label;
class SV_Checkbox;
class D2DSettingsDialog;

enum Languages {
    LANGUAGE_POLISH = 0,
    LANGUAGE_ENGLISH
};

struct FovOverrideCheckedChangedState {
    D2DSettingsDialog* SettingsDialog;
    SV_Label* horizFOVLabel;
    SV_Slider* horizFOVSlider;
    SV_Label* vertFOVLabel;
    SV_Slider* vertFOVSlider;
};

class D2DSettingsDialog : public D2DDialog {
public:
    D2DSettingsDialog( D2DView* view, D2DSubView* parent );
    ~D2DSettingsDialog();

    /** Initializes the controls of this view */
    virtual XRESULT InitControls();

    /** Checks if a change needs to reload the shaders */
    bool NeedsApply();

    /** Sets if this control is hidden */
    virtual void SetHidden( bool hidden );

    /** Called when the settings got re-opened */
    void OnOpenedSettings();

protected:
    /** Tab in main tab-control was switched */
    static void TextureQualitySliderChanged( SV_Slider* sender, void* userdata );
    static void ShadowQualitySliderChanged( SV_Slider* sender, void* userdata );
    static void ResolutionSliderChanged( SV_Slider* sender, void* userdata );
    static void ModeSliderChanged( SV_Slider* sender, void* userdata );
    static void FpsLimitSliderChanged( SV_Slider* sender, void* userdata );
    /** Close button */
    static void CloseButtonPressed( SV_Button* sender, void* userdata );

    /** Apply button */
    static void ApplyButtonPressed( SV_Button* sender, void* userdata );
    static void FovOverrideCheckedChanged( SV_Checkbox* sender, void* userdata );
    static void MTResourceManagerCheckedChanged( SV_Checkbox* sender, void* userdata );
    static void CompressBackBufferCheckedChanged( SV_Checkbox* sender, void* userdata );
    /** Initial renderer settings, used to determine a change */
    GothicRendererSettings InitialSettings;

    /** Current resolution setting */
    int TextureQuality;
    int ResolutionSetting;
    int CurrentWindowMode;
    int ActiveWindowMode;
    std::vector<DisplayModeInfo> Resolutions;
    FovOverrideCheckedChangedState* CheckedChangedState;
    SV_Slider* modeSlider;
};
