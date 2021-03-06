#include <AdaptiveTesselation.h>


static const float mTessellationFactor = 170.0f;

cbuffer Matrices_PerFrame : register( b0 )
{
	matrix M_View;
	matrix M_Proj;
	matrix M_ViewProj;	
};

cbuffer DefaultHullShaderConstantBuffer : register( b1 )
{
	float H_EdgesPerScreenHeight;
	float H_Proj11;
	float H_GlobalTessFactor;
	float H_FarPlane;
	float2 H_ScreenResolution;
	float2 h_pad2;
};




struct VS_OUTPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vNormalWS		: TEXCOORD3;
	float3 vNormalVS		: TEXCOORD4;
	float3 vWorldPosition	: TEXCOORD5;
};

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside: SV_InsideTessFactor;
};

float GetPostProjectionSphereExtent(float3 Origin, float Diameter)
{
    float4 ClipPos = mul(float4( Origin, 1.0 ), M_ViewProj);
    return abs(Diameter * M_Proj[1][1] / ClipPos.w);
}

float CalculateTessellationFactor(float3 Control0, float3 Control1)
{
    float e0 = distance(Control0,Control1);
    float3 m0 = (Control0 + Control1)/2;
    return max(1,H_GlobalTessFactor * GetPostProjectionSphereExtent(m0,e0));
}

//--------------------------------------------------------------------------------------
// Patch Constant Function
//--------------------------------------------------------------------------------------
ConstantOutputType TessPatchConstantFunction(InputPatch<VS_OUTPUT, 3> inputPatch, uint patchId : SV_PrimitiveID)
{   
    ConstantOutputType output;
  
	//float factor = max(1, (1.0f-(inputPatch[0].vViewPosition.z / 4000.0f)) * mTessellationFactor);
	//factor = min(factor, 100);
	float factor = 40.0f;
  
	float3 viewPosition[3];
	viewPosition[0] = mul(float4(inputPatch[0].vWorldPosition, 1), M_View);
	viewPosition[1] = mul(float4(inputPatch[1].vWorldPosition, 1), M_View);
	viewPosition[2] = mul(float4(inputPatch[2].vWorldPosition, 1), M_View);
	
	float far = 12000.0f;
	float tessPow = 1 / 6.0f;
	float dist[3];
	dist[0] = pow(saturate(length(viewPosition[0]) / far), tessPow);
	dist[1] = pow(saturate(length(viewPosition[1]) / far), tessPow);
	dist[2] = pow(saturate(length(viewPosition[2]) / far), tessPow);
	
	dist[0] = max(1, 1 + H_GlobalTessFactor * (1.0f - dist[0]));
	dist[1] = max(1, 1 + H_GlobalTessFactor * (1.0f - dist[1]));
	dist[2] = max(1, 1 + H_GlobalTessFactor * (1.0f - dist[2]));
	
	
	
	// Assign tessellation levels
    output.edges[0] = 0.5f * (dist[1] + dist[2]);
    output.edges[1] = 0.5f * (dist[2] + dist[0]);
    output.edges[2] = 0.5f * (dist[0] + dist[1]);
    output.inside   = (output.edges[0] + output.edges[1] + output.edges[2]) / 3.0f;
   
	output.edges[0] = 1.0f;
    output.edges[1] = 1.0f;
    output.edges[2] = 1.0f;
    output.inside   = 1.0f;
	
    return output;
}
//--------------------------------------------------------------------------------------
// Hull Shader
//--------------------------------------------------------------------------------------
[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("TessPatchConstantFunction")]
//[maxtessfactor(64.0)]
VS_OUTPUT HSMain(InputPatch<VS_OUTPUT, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    VS_OUTPUT output;

	output.vTexcoord	 	= patch[pointId].vTexcoord; 	
	output.vTexcoord2		= patch[pointId].vTexcoord2;		
	output.vDiffuse			= patch[pointId].vDiffuse;			
	output.vWorldPosition 	= patch[pointId].vWorldPosition; 	
	output.vNormalVS		= patch[pointId].vNormalVS;	
	output.vNormalWS		= patch[pointId].vNormalWS;		
	//output.vViewPosition	= patch[pointId].vViewPosition;	
	//output.vPosition		= patch[pointId].vPosition;		
	
    return output;
}