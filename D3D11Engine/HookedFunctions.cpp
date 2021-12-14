#include "pch.h"
#include "HookedFunctions.h"

#include "zCBspTree.h"
#include "zCWorld.h"
#include "oCGame.h"
#include "zCMaterial.h"
#include "zFILE.h"
#include "zCOption.h"
#include "zCRndD3D.h"
#include "zCParticleFX.h"
#include "zCView.h"
#include "CGameManager.h"
#include "zCVisual.h"
#include "zCTimer.h"
#include "zCModel.h"
#include "oCSpawnManager.h"
#include "zCVob.h"
#include "zCTexture.h"
#include "zCThread.h"
#include "zCResourceManager.h"
#include "zCQuadMark.h"
#include "oCNPC.h"
#include "zCSkyController_Outdoor.h"

#include "zQuat.h"
#include "zMat4.h"

#if _MSC_VER >= 1300
#include <Tlhelp32.h>
#endif

#include "StackWalker.h"

/** Init all hooks here */
void HookedFunctionInfo::InitHooks() {
    LogInfo() << "Initializing hooks";

    oCGame::Hook();
    zCBspTree::Hook();
    zCWorld::Hook();
    zCMaterial::Hook();
    zCBspNode::Hook();
    zFILE::Hook();
    zCOption::Hook();
    zCRndD3D::Hook();
    zCParticleFX::Hook();
    zCView::Hook();
    CGameManager::Hook();
    zCVisual::Hook();
    zCTimer::Hook();
    zCModel::Hook();
    zCModelPrototype::Hook();
    oCSpawnManager::Hook();
    zCVob::Hook();
    zCTexture::Hook();
    zCThread::Hook();
    //zCResourceManager::Hook();
    zCQuadMark::Hook();
    oCNPC::Hook();
    zCSkyController_Outdoor::Hook();

    //XHook(original_zCExceptionHandler_UnhandledExceptionFilter, GothicMemoryLocations::Functions::zCExceptionHandler_UnhandledExceptionFilter, HookedFunctionInfo::hooked_zCExceptionHandlerUnhandledExceptionFilter);
    //XHook(original_HandledWinMain, GothicMemoryLocations::Functions::HandledWinMain, HookedFunctionInfo::hooked_HandledWinMain);
    //XHook(original_ExitGameFunc, GothicMemoryLocations::Functions::ExitGameFunc, HookedFunctionInfo::hooked_ExitGameFunc);

    // Kill the check for doing freelook only in fullscreen, since we force the game to run windowed internally
    //int flSize = GothicMemoryLocations::GlobalObjects::NOP_FreelookWindowedCheckEnd - GothicMemoryLocations::GlobalObjects::NOP_FreelookWindowedCheckStart;
    //VirtualProtect((void *)GothicMemoryLocations::GlobalObjects::NOP_FreelookWindowedCheckStart, flSize, PAGE_EXECUTE_READWRITE, &dwProtect);
    //REPLACE_RANGE(GothicMemoryLocations::GlobalObjects::NOP_FreelookWindowedCheckStart, GothicMemoryLocations::GlobalObjects::NOP_FreelookWindowedCheckEnd-1, INST_NOP);

    // Hook the single bink-function
    XHook( GothicMemoryLocations::zCBinkPlayer::GetPixelFormat, HookedFunctionInfo::hooked_zBinkPlayerGetPixelFormat );

#if defined(BUILD_GOTHIC_2_6_fix) || defined(BUILD_GOTHIC_1_08k)
    XHook( original_zCBinkPlayerOpenVideo, GothicMemoryLocations::zCBinkPlayer::OpenVideo, HookedFunctionInfo::hooked_zBinkPlayerOpenVideo );
#endif
    original_Alg_Rotation3DNRad = (Alg_Rotation3DNRad)GothicMemoryLocations::Functions::Alg_Rotation3DNRad;

//G1 patches
#ifdef BUILD_GOTHIC_1_08k
#ifdef BUILD_1_12F
    LogInfo() << "Patching: Fix integer overflow crash";
    PatchAddr( 0x00506B31, "\xEB" );

    LogInfo() << "Patching: Marking texture as cached-in after cache-out - fix";
    PatchAddr( 0x005E90BE, "\x90\x90" );

    LogInfo() << "Patching: Disable dx7 window transitions";
    PatchAddr( 0x0075CA7B, "\x90\x90" );
    PatchAddr( 0x0074DAD0, "\x90\x90" );
#else
    LogInfo() << "Patching: BroadCast fix";
    {
        char* zSPYwnd[5];
        DWORD zSPY = reinterpret_cast<DWORD>(FindWindowA( nullptr, "[zSpy]" ));
        memcpy( zSPYwnd, &zSPY, 4 );
        PatchAddr( 0x00447F5A, zSPYwnd );
        PatchAddr( 0x00449280, zSPYwnd );
        PatchAddr( 0x004480AF, zSPYwnd );
    }

    LogInfo() << "Patching: LOW_FPS_NaN_check";
    PatchAddr( 0x007CF732, "\x81\x3B\x00\x00\xC0\xFF\x0F\x84\x3B\xFF\xD4\xFF\x81\x3B\x00\x00\xC0\x7F\x0F\x84\x2F\xFF\xD4\xFF\xD9\x03\x8D\x44\x8C\x1C\xE9\x33\xFF\xD4\xFF" );
    PatchAddr( 0x0051F682, "\xE9\xAB\x00\x2B\x00\x90" );
    PatchAddr( 0x007CF755, "\x81\x7C\xE4\x20\x00\x00\xC0\xFF\x0F\x84\x43\xF0\xD4\xFF\x81\x7C\xE4\x20\x00\x00\xC0\x7F\x0F\x84\x35\xF0\xD4\xFF\xE9\xDA\xEF\xD4\xFF" );
    PatchAddr( 0x005F0EAA, "\xE8\xA6\xE8\x1D\x00" );

    LogInfo() << "Patching: Fix integer overflow crash";
    PatchAddr( 0x004F4024, "\xEB" );
    PatchAddr( 0x004F43FC, "\xEB" );

    LogInfo() << "Patching: Marking texture as cached-in after cache-out - fix";
    PatchAddr( 0x005CA683, "\x90\x90" );

    LogInfo() << "Patching: Improve loading times by disabling some unnecessary features";
    PatchAddr( 0x005A4FE0, "\xC3\x90\x90" );
    PatchAddr( 0x0055848A, "\xE9\xE2\x01\x00\x00\x90" );
    PatchAddr( 0x005F7F7C, "\x1F" );
    PatchAddr( 0x005F8D40, "\x1F" );
    PatchAddr( 0x00525BC4, "\xEB" );
    PatchAddr( 0x0051E425, "\x90\x90" );
    PatchAddr( 0x0051E5B5, "\xEB\x22" );
    PatchAddr( 0x0051E62A, "\x8D\x24\x24\x8B\x4A\x30\x8B\x04\xA9\x8B\x48\x40\x83\xC0\x38\x85\xC9\x74\x28\x33\xF6\x85\xC9\x7E\x22\x8B\x18\x8B\xFB\x8D\x1B\x39\x17\x74\x0A\x46\x83\xC7\x04\x3B\xF1\x7C\xF4\xEB\x0E\x49\x3B\xF1\x89\x48\x08\x74\x06\x8B\x04\x8B\x89\x04\xB3\x8B\x42\x38\x45\x3B\xE8\x7C\xC0\xEB\x65\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90" );
    
    // Skip loading lightmaps and make general purpose lightmap to be able to detect indoor vobs
    {
        PatchAddr( 0x0055787E, "\xE9\x5E\x0B\x00\x00" );
        PatchAddr( 0x00557A11, "\xE9\xCB\x09\x00\x00" );
        PatchAddr( 0x005582C9, "\x8B\xCB\xE8\xA6\xEF\xFF\xFF\xEB\x2E\x90\x90" );
        PatchAddr( 0x00557276, "\xE9\x00\x00\x00\x00" );
        XHook( 0x00557276, HookedFunctionInfo::hooked_SetLightmap );
    }

    LogInfo() << "Patching: Fix using settings in freelook mode";
    PatchAddr( 0x00478FE2, "\x0F\x84\x9A\x00\x00\x00" );

    LogInfo() << "Patching: Disable dx7 window transitions";
    PatchAddr( 0x0072018B, "\x90\x90" );
    PatchAddr( 0x00711F70, "\x90\x90" );

    LogInfo() << "Patching: Show correct tris on toggle frame";
    {
        char* trisHndl[5];
        DWORD trisHandle = reinterpret_cast<DWORD>(&Engine::GAPI->GetRendererState().RendererInfo.FrameDrawnTriangles);
        memcpy( trisHndl, &trisHandle, 4 );
        PatchAddr( 0x0063DA2E, "\xA1\xD0\x5E\x8C\x00\x90\x90\x90\x90\x90\x90" );
        PatchAddr( 0x0063DA2F, trisHndl );
    }

    LogInfo() << "Patching: Decouple barrier from sky";
    PatchAddr( 0x00632146, "\x90\x90\x90\x90\x90" );

    // Show DirectX11 as currently used graphic device
    {
        PatchAddr( 0x0071F8DF, "\x55\x56\xBE\x00\x00\x00\x00\x90\x90\x90\x90" );
        PatchAddr( 0x0071F8EC, "\x83\xFE\x01" );
        PatchAddr( 0x0071F9EC, "\x81\xC6\x18\xE7\x8D\x00" );
        PatchAddr( 0x0071FA01, "\x90\x90" );

        PatchAddr( 0x0071F5D9, "\xB8\x01\x00\x00\x00\xC3\x90" );
        PatchAddr( 0x0071F5E9, "\xB8\x01\x00\x00\x00\xC3\x90" );

        PatchAddr( 0x0042BB0D, "\xE8\xC7\x3A\x2F\x00\x90" );
        PatchAddr( 0x0042BBE1, "\xE8\x03\x3A\x2F\x00\x90" );

        XHook( 0x0071F5D9, HookedFunctionInfo::hooked_GetNumDevices );
    }
#endif
#endif

//G2 patches
#ifdef BUILD_GOTHIC_2_6_fix
    zQuat::Hook();
    zMat4::Hook();

    LogInfo() << "Patching: BroadCast fix";
    {
        char* zSPYwnd[5];
        DWORD zSPY = reinterpret_cast<DWORD>(FindWindowA( nullptr, "[zSpy]" ));
        memcpy( zSPYwnd, &zSPY, 4 );
        PatchAddr( 0x0044C5DA, zSPYwnd );
        PatchAddr( 0x0044D9A0, zSPYwnd );
        PatchAddr( 0x0044C72F, zSPYwnd );
    }

    LogInfo() << "Patching: Interupt gamestart sound";
    PatchAddr( 0x004DB89F, "\x00" );

    LogInfo() << "Patching: Fix low framerate";
    PatchAddr( 0x004DDC6F, "\x08" );

    LogInfo() << "Patching: LOW_FPS_NaN_check";
    PatchAddr( 0x0066E59A, "\x81\x3A\x00\x00\xC0\xFF\x0F\x84\xF3\x3C\xEC\xFF\x81\x3A\x00\x00\xC0\x7F\x0F\x84\xE7\x3C\xEC\xFF\xD9\x45\x00\x8D\x44\x8C\x20\xE9\xEB\x3C\xEC\xFF" );
    PatchAddr( 0x005322A2, "\xE9\xF3\xC2\x13\x00\x90\x90" );
    PatchAddr( 0x0066E5BE, "\x81\x7C\xE4\x20\x00\x00\xC0\xFF\x0F\x84\x2A\x2B\xEC\xFF\x81\x7C\xE4\x20\x00\x00\xC0\x7F\x0F\x84\x1C\x2B\xEC\xFF\xE9\xC1\x2A\xEC\xFF" );
    PatchAddr( 0x0061E412, "\xE8\xA7\x01\x05\x00" );

    LogInfo() << "Patching: Fix integer overflow crash";
    PatchAddr( 0x00502F94, "\xEB" );
    PatchAddr( 0x00503343, "\xEB" );

    LogInfo() << "Patching: Texture size is lower than 32 - fix";
    PatchAddr( 0x005F4E20, "\xC7\x05\xBC\xB3\x99\x00\x00\x40\x00\x00\xEB\x4D\x90\x90" );

    LogInfo() << "Patching: Marking texture as cached-in after cache-out - fix";
    PatchAddr( 0x005F5573, "\x90\x90" );

    LogInfo() << "Patching: Fix dynamic lights huge impact on FPS in some locations";
    PatchAddr( 0x006092C4, "\xE9\x45\x02\x00\x00\x90" );
    PatchAddr( 0x00609544, "\xE9\x25\x02\x00\x00\x90" );

#ifndef BUILD_SPACER_NET
    LogInfo() << "Patching: Improve loading times by disabling some unnecessary features";
    PatchAddr( 0x005C6E30, "\xC3\x90\x90\x90\x90\x90" );
    PatchAddr( 0x00571256, "\xE9\xC6\x02\x00\x00\x90" );
    PatchAddr( 0x006C8748, "\x90\x90\x90\x90\x90\x90" );
    PatchAddr( 0x00530D75, "\x90\x90" );
    PatchAddr( 0x006265AE, "\x1F" );
    PatchAddr( 0x006274E6, "\x1F" );
    PatchAddr( 0x005396C9, "\xEB" );
    PatchAddr( 0x00530F05, "\xEB\x22" );
    PatchAddr( 0x00530F7A, "\x8D\xA4\x24\x00\x00\x00\x00\x8B\x4A\x30\x8B\x04\xA9\x8B\x48\x40\x83\xC0\x38\x85\xC0\x74\x28\x33\xF6\x85\xC9\x7E\x22\x8B\x18\x8B\xFB\x8D\x1B\x39\x17\x74\x0A\x46\x83\xC7\x04\x3B\xF1\x7C\xF4\xEB\x0E\x49\x3B\xF1\x89\x48\x08\x74\x06\x8B\x04\x8B\x89\x04\xB3\x8B\x42\x38\x45\x3B\xE8\x7C\xC0\xEB\x61\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90" );

    // Skip loading lightmaps and make general purpose lightmap to be able to detect indoor vobs
    {
        PatchAddr( 0x00570746, "\xE9\x62\x0A\x00\x00" );
        PatchAddr( 0x005708AC, "\xE9\xFC\x08\x00\x00" );
        PatchAddr( 0x00571098, "\x8B\xCB\xE8\x13\x58\xFF\xFF\xEB\x2E\x90\x90" );
        PatchAddr( 0x005668B2, "\xE9\x00\x00\x00\x00" );
        XHook( 0x005668B2, HookedFunctionInfo::hooked_SetLightmap );
    }

    LogInfo() << "Patching: Fix using settings in freelook mode";
    PatchAddr( 0x004806C2, "\x0F\x84\x9A\x00\x00\x00" );
#endif

    LogInfo() << "Patching: Disable dx7 window transitions";
    PatchAddr( 0x00658BCB, "\x90\x90" );
    PatchAddr( 0x006483A2, "\x90\x90" );

    LogInfo() << "Patching: Show correct tris on toggle frame";
    {
        char* trisHndl[5];
        DWORD trisHandle = reinterpret_cast<DWORD>(&Engine::GAPI->GetRendererState().RendererInfo.FrameDrawnTriangles);
        memcpy( trisHndl, &trisHandle, 4 );
        PatchAddr( 0x006C80F2, "\x36\x8B\x3D\x08\x2F\x98\x00" );
        PatchAddr( 0x006C80F5, trisHndl );
    }

    // Show DirectX11 as currently used graphic device
    {
        PatchAddr( 0x006581AD, "\x57\xBD\x00\x00\x00\x00\x90" );
        PatchAddr( 0x006581B8, "\x83\xFD\x01\x90\x90\x90\x90" );
        PatchAddr( 0x00658302, "\x81\xC5\x30\x4C\x9A\x00" );
        PatchAddr( 0x00658321, "\x8B\xFD" );
        PatchAddr( 0x00658329, "\x55" );

        PatchAddr( 0x00657EA9, "\xB8\x01\x00\x00\x00\xC3\x90" );
        PatchAddr( 0x00657EB9, "\xB8\x01\x00\x00\x00\xC3\x90" );

        PatchAddr( 0x0042DF1F, "\xE8\x85\x9F\x22\x00\x90" );
        PatchAddr( 0x0042E000, "\xE8\xB4\x9E\x22\x00\x90" );

        XHook( 0x00657EA9, HookedFunctionInfo::hooked_GetNumDevices );
    }

    // HACK Workaround to fix debuglines in godmode
    LogInfo() << "Patching: Godmode Debuglines";
    // oCMagFrontier::GetDistanceNewWorld
    PatchAddr( 0x00473f37, "\xBD\x00\x00\x00\x00" ); // replace MOV EBP, 0x1 with MOV EBP, 0x0
    // oCMagFrontier::GetDistanceDragonIsland
    PatchAddr( 0x004744c1, "\xBF\x00\x00\x00\x00" ); // replace MOV EDI, 0x1 with MOV EDI, 0x0
    // oCMagFrontier::GetDistanceAddonWorld
    PatchAddr( 0x00474681, "\xBF\x00\x00\x00\x00" ); // replace MOV EDI, 0x1 with MOV EDI, 0x0
#endif
}

/** Function hooks */
int __stdcall HookedFunctionInfo::hooked_HandledWinMain( HINSTANCE hInstance, HINSTANCE hPrev, LPSTR szCmdLine, int sw ) {
    int r = HookedFunctions::OriginalFunctions.original_HandledWinMain( hInstance, hPrev, szCmdLine, sw );

    return r;
}

void __fastcall HookedFunctionInfo::hooked_zCActiveSndAutoCalcObstruction( void* thisptr, void* unknwn, int i ) {
    // Just do nothing here. Something was inside zCBspTree::Render that managed this and thus voices get really quiet in indoor locations
    // This function is for calculating the automatic volume-changes when the camera goes in/out buildings
    // We keep everything on the same level by removing it
}

void __cdecl HookedFunctionInfo::hooked_ExitGameFunc() {
    Engine::OnShutDown();

    HookedFunctions::OriginalFunctions.hooked_ExitGameFunc();
}

long __stdcall HookedFunctionInfo::hooked_zCExceptionHandlerUnhandledExceptionFilter( EXCEPTION_POINTERS* exceptionPtrs ) {
    return HookedFunctions::OriginalFunctions.original_zCExceptionHandler_UnhandledExceptionFilter( exceptionPtrs );
}

/** Returns the pixelformat of a bink-surface */
long __fastcall HookedFunctionInfo::hooked_zBinkPlayerGetPixelFormat( void* thisptr, void* unknwn, zTRndSurfaceDesc& desc ) {
    // Return values seems to be:
    // 3 - bgra
    // 4 - rgba
    
    // Union/Systempack FixBink seems to work only with bgra format
    if ( Engine::GAPI->GetRendererState().RendererInfo.FixBink )
        return 3;

    // Use rgba format with disabled FixBink because for some reason there seem to be some ugly graphical glitches
    return 4;
}

int __fastcall HookedFunctionInfo::hooked_zBinkPlayerOpenVideo( void* thisptr, void* unknwn, zSTRING str ) {
    // Need to save window resolution before starting video
    // Union/Systempack reads these values from Gothic.ini when opening video
    Engine::GAPI->SaveWindowResolution();
    Engine::GAPI->GetRendererState().RendererInfo.FirstVideoFrame = 1;
    int r = HookedFunctions::OriginalFunctions.original_zCBinkPlayerOpenVideo( thisptr, str );

    struct BinkInfo {
        unsigned int ResX;
        unsigned int ResY;
        // ... unimportant
    };

    // Grab the resolution
    // This structure stores width and height as first two parameters, as ints.
    BinkInfo* res = *(BinkInfo**)(((char*)thisptr) + (GothicMemoryLocations::zCBinkPlayer::Offset_VideoHandle));
    if ( res ) {
        Engine::GAPI->GetRendererState().RendererInfo.PlayingMovieResolution = INT2( res->ResX, res->ResY );
    }

    return r;
}

int __cdecl HookedFunctionInfo::hooked_GetNumDevices() {
    Engine::GraphicsEngine->OnUIEvent( BaseGraphicsEngine::EUIEvent::UI_OpenSettings );
    return 1;
}

void __fastcall HookedFunctionInfo::hooked_SetLightmap( void* polygonPtr ) {
    static zCLightmap* lightmap = nullptr;
    if ( !lightmap ) {
        #ifdef BUILD_GOTHIC_1_08k
        DWORD alloc_lightmap = reinterpret_cast<DWORD( __cdecl* )( DWORD )>( 0x54EBE0 )( 0x4C ); // malloc memory
        reinterpret_cast<void( __cdecl* )( DWORD, DWORD )>( 0x75A250 )( alloc_lightmap, 0x8CF070 ); // inform zengine object created
        reinterpret_cast<void( __fastcall* )( DWORD )>( 0x5CDD70 )( alloc_lightmap ); // call the constructor
        #else
        DWORD alloc_lightmap = reinterpret_cast<DWORD( __cdecl* )( DWORD )>( 0x565F50 )( 0x4C ); // malloc memory
        reinterpret_cast<void( __cdecl* )( DWORD, DWORD )>( 0x5AAEB0 )( alloc_lightmap, 0x99B250 ); // inform zengine object created
        reinterpret_cast<void( __fastcall* )( DWORD )>( 0x5F8EA0 )( alloc_lightmap ); // call the constructor
        #endif

        lightmap = reinterpret_cast<zCLightmap*>(alloc_lightmap);
    }

    zCPolygon* polygon = reinterpret_cast<zCPolygon*>(polygonPtr);
    polygon->SetLightmap( lightmap );
    zCObject_AddRef( lightmap ); // Make sure it won't get deleted
}
