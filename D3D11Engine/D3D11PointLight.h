#pragma once
#include "BaseShadowedPointLight.h"
#include "WorldConverter.h"
#include <thread>
#include <condition_variable>
#include <atomic>

class D3D11PointLight;

struct VobLightInfo;
struct RenderToDepthStencilBuffer;
struct RenderToTextureBuffer;
struct VobInfo;
struct SkeletalVobInfo;
class D3D11ConstantBuffer;
class D3D11PointLight : public BaseShadowedPointLight {
public:
    D3D11PointLight( VobLightInfo* info, bool dynamicLight = false );
    ~D3D11PointLight();

    /** Initializes the resources of this light */
    void InitResources();

    /** Draws the surrounding scene into the cubemap */
    void RenderCubemap( bool forceUpdate = false );

    /** Binds the shadowmap to the pixelshader */
    void OnRenderLight();

    /** Returns if this light is inited already */
    bool IsInited();

    /** Returns if this light needs an update */
    bool NeedsUpdate();

    /** Returns true if the light could need an update, but it's not very important */
    bool WantsUpdate();

    /** Returns true if this is the first time that light is being rendered */
    bool NotYetDrawn();

    /** Called when a vob got removed from the world */
    virtual void OnVobRemovedFromWorld( BaseVobInfo* vob );

protected:
    /** Renders the scene with the given view-proj-matrices */
    void RenderCubemapFace( const XMFLOAT4X4& view, const XMFLOAT4X4& proj, UINT faceIdx );

    /** Renders all cubemap faces at once, using the geometry shader */
    void RenderFullCubemap();

    std::list<VobInfo*> VobCache;
    std::list<SkeletalVobInfo*> SkeletalVobCache;
    std::map<MeshKey, WorldMeshInfo*, cmpMeshKey> WorldMeshCache;
    bool WorldCacheInvalid;

    VobLightInfo* LightInfo;
    std::unique_ptr<RenderToDepthStencilBuffer> DepthCubemap;
    XMFLOAT4X4 CubeMapViewMatrices[6];
    XMFLOAT3 LastUpdatePosition;
    DWORD LastUpdateColor;
    std::unique_ptr<D3D11ConstantBuffer> ViewMatricesCB;
    bool DynamicLight;
    std::atomic<bool> InitDone;
    bool DrawnOnce;
};

