//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

cbuffer Matrices_PerFrame : register( b0 )
{
	matrix M_View;
	matrix M_Proj;
	matrix M_ViewProj;	
};

cbuffer Matrices_PerInstances : register( b1 )
{
	float M_TotalTime;
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTex1		: TEXCOORD0;
	float2 vTex2		: TEXCOORD1;
	float4 vDiffuse		: DIFFUSE;
};

struct VS_OUTPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float2 vTexcoord2		: TEXCOORD1;
	float4 vDiffuse			: TEXCOORD2;
	float3 vNormalWS		: TEXCOORD4;
	float3 vWorldPosition	: TEXCOORD5;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	//Input.vPosition = float3(-Input.vPosition.x, Input.vPosition.y, -Input.vPosition.z);
	
	float3 positionWorld = Input.vPosition;
	float2 texAniMap = Input.vTex2 * M_TotalTime;
	texAniMap -= floor( texAniMap );
	
	//Output.vPosition = float4(Input.vPosition, 1);
	Output.vPosition = mul( float4(positionWorld,1), M_ViewProj);
	Output.vTexcoord = Input.vTex1 + texAniMap;
	Output.vDiffuse  = Input.vDiffuse;
	Output.vNormalWS = Input.vNormal;
	Output.vWorldPosition = positionWorld;
	Output.vTexcoord2.x = mul(float4(positionWorld,1), M_View).z;
	Output.vTexcoord2.y = length(mul(float4(positionWorld,1), M_View));
	//Output.vWorldPosition = positionWorld;
	
	return Output;
}

