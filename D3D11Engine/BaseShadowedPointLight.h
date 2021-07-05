#pragma once

struct BaseVobInfo;
class BaseShadowedPointLight {
public:
    BaseShadowedPointLight();
    virtual ~BaseShadowedPointLight();

    /** Called when a vob got removed from the world */
    virtual void OnVobRemovedFromWorld( BaseVobInfo* vob ) {};
};

