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
	
	if( vertexID >= 9 )
	{
		uint vID = 11 - vertexID;
		Output.vTexcoord = float2(min((vID << 1) & 6, 2), 2 - (vID & 2));
		Output.vPosition = float4(Output.vTexcoord * float2(1.0f, -0.12f) + float2(-1.0f, 1.0f), 0.0, 1);
	}
	else if( vertexID >= 6 )
	{
		uint vID = 8 - vertexID;
		Output.vTexcoord = float2(min((vID << 1) & 6, 2), 2 - (vID & 2));
		Output.vPosition = float4(float2(0.0f, -0.76f) + (Output.vTexcoord * float2(1.0f, -0.12f) + float2(-1.0f, 0.0f)), 0.0, 1);
	}
	else if( vertexID >= 3 )
	{
		uint vID = vertexID - 3;
		Output.vTexcoord = float2((vID << 1) & 2, vID & 2);
		Output.vPosition = float4(float2(0.0f, -0.76f) + (Output.vTexcoord * float2(1.0f, -0.12f) + float2(-1.0f, 0.0f)), 0.0, 1);
	}
	else
	{
		Output.vTexcoord = float2((vertexID << 1) & 2, vertexID & 2);
		Output.vPosition = float4(Output.vTexcoord * float2(1.0f, -0.12f) + float2(-1.0f, 1.0f), 0.0, 1);
	}
	
	Output.vEyeRay = 0;
	
	return Output;
}
