#include "Engine.h"
#include "GothicAPI.h"
#include "SteamOverlay.h"

// We can detect whether steam overlay is visible by checking if it request using mouse or keyboard
// easiest way to do it would be by using steamapi callbacks however because the game can be launched
// without steam it doesn't feel right doing it that way
namespace SteamOverlay
{
    static bool IsSteamOverlayEnabled = false;
    
    typedef bool( WINAPI* PFN_IsOverlayEnabled )();
    typedef bool( WINAPI* PFN_SteamOverlayIsUsingMouse )();
    typedef bool( WINAPI* PFN_SteamOverlayIsUsingKeyboard )();
    static PFN_IsOverlayEnabled IsOverlayEnabled = nullptr;
    static PFN_SteamOverlayIsUsingMouse SteamOverlayIsUsingMouse = nullptr;
    static PFN_SteamOverlayIsUsingKeyboard SteamOverlayIsUsingKeyboard = nullptr;

	void Init()
	{
        HMODULE soModule = GetModuleHandleA( "GameOverlayRenderer.dll" );
        if ( soModule ) {
            IsOverlayEnabled = reinterpret_cast<PFN_IsOverlayEnabled>(GetProcAddress( soModule, "IsOverlayEnabled" ));
            SteamOverlayIsUsingMouse = reinterpret_cast<PFN_SteamOverlayIsUsingMouse>(GetProcAddress( soModule, "SteamOverlayIsUsingMouse" ));
            SteamOverlayIsUsingKeyboard = reinterpret_cast<PFN_SteamOverlayIsUsingKeyboard>(GetProcAddress( soModule, "SteamOverlayIsUsingKeyboard" ));
        }
	}

	void Update()
	{
        if ( IsOverlayEnabled && IsOverlayEnabled() && (SteamOverlayIsUsingMouse || SteamOverlayIsUsingKeyboard) ) {
            bool isSOEnabled = (SteamOverlayIsUsingMouse && SteamOverlayIsUsingMouse()) || (SteamOverlayIsUsingKeyboard && SteamOverlayIsUsingKeyboard());
            if ( IsSteamOverlayEnabled != isSOEnabled ) {
                Engine::GAPI->SetEnableGothicInput( IsSteamOverlayEnabled );
                IsSteamOverlayEnabled = isSOEnabled;
            }
        }
	}
}
