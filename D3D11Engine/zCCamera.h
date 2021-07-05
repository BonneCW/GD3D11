#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "BaseGraphicsEngine.h"
#include "zTypes.h"


class zCCamera {
public:

    enum ETransformType {
        TT_WORLD,
        TT_VIEW,
        TT_WORLDVIEW,
        TT_WORLDVIEW_INV,
        TT_VIEW_INV
    };

    static bool IsFreeLookActive() {
#ifdef BUILD_GOTHIC_2_6_fix
        return (*(int*)(GothicMemoryLocations::zCCamera::Var_FreeLook)) != 0;
#else
        return false;
#endif
    }

    const DirectX::XMFLOAT4X4& GetTransformDX( const ETransformType type ) {
#ifdef BUILD_GOTHIC_2_6_fix
        switch ( type ) {
        case ETransformType::TT_WORLD:			return trafoWorld;		break;
        case ETransformType::TT_VIEW:			return trafoView;		break;
        case ETransformType::TT_WORLDVIEW:		return camMatrix;		break;
        case ETransformType::TT_WORLDVIEW_INV:	return camMatrixInv;	break;
        case ETransformType::TT_VIEW_INV:		return trafoViewInv;	break;
        default:						        return camMatrix;		break;
        };
#else
        // TODO: Check if zCCamera is same with G1
        XCALL( GothicMemoryLocations::zCCamera::GetTransform );
#endif
    }

    void SetTransform( const ETransformType type, const DirectX::XMFLOAT4X4& mat ) {
        XCALL( GothicMemoryLocations::zCCamera::SetTransform );
    }

    void SetTransformXM( const ETransformType type, const DirectX::XMMATRIX& mat ) {
        DirectX::XMFLOAT4X4 m; DirectX::XMStoreFloat4x4( &m, mat );
        SetTransform( type, m );
    }

    void Activate() {
        XCALL( GothicMemoryLocations::zCCamera::Activate );
    }

    void SetFOV( float azi, float elev ) {
        XCALL( GothicMemoryLocations::zCCamera::SetFOV );
    }

    void GetFOV( float& fovH, float& fovV ) {
        XCALL( GothicMemoryLocations::zCCamera::GetFOV_f2 );
    }

    void UpdateViewport() {
        XCALL( GothicMemoryLocations::zCCamera::UpdateViewport );
    }

    zTCam_ClipType BBox3DInFrustum( const zTBBox3D& box ) {
        //int flags = 15; // Full clip, no farplane
        int flags = 63;
        return BBox3DInFrustum( box, flags );
    }

    zTCam_ClipType BBox3DInFrustum( const zTBBox3D& box, int& clipFlags ) {
        XCALL( GothicMemoryLocations::zCCamera::BBox3DInFrustum );
    }

    float GetFarPlane() {
        return *(float*)((char*)this + GothicMemoryLocations::zCCamera::Offset_FarPlane);
    }

    float GetNearPlane() {
        return *(float*)((char*)this + GothicMemoryLocations::zCCamera::Offset_NearPlane);
    }

    void SetFarPlane( float value ) {
        XCALL( GothicMemoryLocations::zCCamera::SetFarPlane );
    }

    /*void GetCameraPosition(DirectX::XMFLOAT3 & v)
    {
        XCALL(GADDR::zCCamera_GetCameraPosition);
    }*/

    /*static void SetFreeLook(bool freeLook)
    {
        bool * f = (bool *)GothicMemoryLocations::zCCamera::Var_FreeLook;
        *f = freeLook;
    }*/

    /** Returns the frustumplanes */
    zTPlane* GetFrustumPlanes() {
        return FrustumPlanes;
    }

    /** Returns the signbits for the frustumplanes */
    byte* GetFrustumSignBits() {
        return SignBits;
    }

    static zCCamera* GetCamera() { return *(zCCamera**)GothicMemoryLocations::GlobalObjects::zCCamera; }

    /** Frustum Planes in world space */
    zTPlane FrustumPlanes[6];
    byte SignBits[6];

    zTViewportData vpdata;

    void* targetView;
    XMFLOAT4X4 camMatrix;
    XMFLOAT4X4 camMatrixInv;

    int tremorToggle;
    float tremorScale;
    float3 tremorAmplitude;
    float3 tremorOrigin;
    float tremorVelo;

    // Transformation matrices
    XMFLOAT4X4 trafoView;
    XMFLOAT4X4 trafoViewInv;
    XMFLOAT4X4 trafoWorld;
    zCMatrixStack<XMFLOAT4X4, 8> trafoViewStack;
    zCMatrixStack<XMFLOAT4X4, 8> trafoWorldStack;
    zCMatrixStack<XMFLOAT4X4, 8> trafoWorldViewStack;
    XMFLOAT4X4 trafoProjection;
private:
};
