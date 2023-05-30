//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

cbuffer Viewport : register( b0 )
{
	float2 V_ViewportPos;
	float2 V_ViewportSize;
};

//--------------------------------------------------------------------------------------
// Input / Output structures
//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 vPosition	: POSITION;
	float4 vDiffuse		: DIFFUSE;
};

struct VS_OUTPUT
{
	float4 vDiffuse			: TEXCOORD0;
	float4 vPosition		: SV_POSITION;
};

/** Transforms a pre-transformed xyzrhw-coordinate into d3d11-space */
float4 TransformXYZRHW(float4 xyzrhw)
{
	// MAGIC (:
	
	// Convert from viewport-coordinates to normalized device coordinates
	float3 ndc;
	ndc.x = ((2 * (xyzrhw.x - V_ViewportPos.x)) / V_ViewportSize.x) - 1;
	ndc.y = 1 - ((2 * (xyzrhw.y - V_ViewportPos.y)) / V_ViewportSize.y);
	ndc.z = xyzrhw.z;
	
	// Convert to clip-space. rhw is actually 1/w ("reciprocal"). So to undo the devide by w, devide by the given 1/w.
	float actualW = 1.0f / xyzrhw.w;
	float3 clipSpace = ndc.xyz * actualW;
	
	return float4(clipSpace, actualW);
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VSMain( VS_INPUT Input )
{
	VS_OUTPUT Output;
	
	Output.vPosition = TransformXYZRHW(Input.vPosition);
	Output.vDiffuse  = Input.vDiffuse;
	return Output;
}
