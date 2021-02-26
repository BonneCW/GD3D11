#pragma once

struct GothicDepthBufferStateInfo;

class BaseDepthBufferState {
public:
    BaseDepthBufferState( const GothicDepthBufferStateInfo& state ) {}
    virtual ~BaseDepthBufferState() {}
};

struct GothicBlendStateInfo;

class BaseBlendStateInfo {
public:
    BaseBlendStateInfo( const GothicBlendStateInfo& state ) {}
    virtual ~BaseBlendStateInfo() {}

};

struct GothicRasterizerStateInfo;

class BaseRasterizerStateInfo {
public:
    BaseRasterizerStateInfo( const GothicRasterizerStateInfo& state ) {}
    virtual ~BaseRasterizerStateInfo() {}
};
