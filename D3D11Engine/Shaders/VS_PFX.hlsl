//--------------------------------------------------------------------------------------
// Simple vertex shader
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Output structure
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float2 vTexcoord		: TEXCOORD0;
	float3 vEyeRay			: TEXCOORD1;
	float4 vPosition		: SV_POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//------------------------------------------------------------------ --------------------
VS_OUTPUT VSMain( uint vertexID : SV_VertexID )
{
	VS_OUTPUT Output;
	
	Output.vTexcoord = float2((vertexID << 1) & 2, vertexID & 2);
	Output.vPosition = float4(Output.vTexcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0, 1);
	
	Output.vEyeRay = 0;
	
	return Output;
}
