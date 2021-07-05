#pragma once
#include "zSTRING.h"
#include "zCTexture.h"

class zFont {
public:
    zSTRING name;
    int height;
    zCTexture* tex;
    uint8_t width[256];
    zVEC2 fontuv1[256];
    zVEC2 fontuv2[256];
};
