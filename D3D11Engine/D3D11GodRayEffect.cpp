#include "pch.h"
#include "D3D11GodRayEffect.h"
#include "D3D11GraphicsEngineBase.h"
#include "Engine.h"
#include "D3D11ShaderManager.h"
#include "D3D11GraphicsEngineBase.h"

D3D11GodRayEffect::D3D11GodRayEffect() {
    QuadHDS = nullptr;
    QuadPS = nullptr;
    QuadVS = nullptr;
    //D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;
    //QuadVS = engine->GetShaderManager().GetVShader
}

D3D11GodRayEffect::~D3D11GodRayEffect() {}
