#pragma pack_matrix(row_major)//it will use row major
struct V_IN
{
	float3 posL : POSITION;
	float2 tex : TEXCOORD;
	matrix instanceMatrix:WORLD;
	float3 instancecolor : TEXCOORD1;
};

struct V_OUT
{
	float4 posH : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 foliageColor : TEXCOORD1;
};

cbuffer TotlaMatrix : register(b0)
{
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
};


V_OUT main(V_IN input)
{
	V_OUT output = (V_OUT)0;

	
	float4 localH = float4(input.posL, 1);
	//float4 localH = float4((float3)input.posL, 1);
	// move local space vertex from vertex buffer into world space.
	localH = mul(localH, input.instanceMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projectionMatrix);
	output.posH = localH;

	//tex
	output.tex = input.tex;

	//color 
	output.foliageColor = input.instancecolor;
	
	return output; // send projected vertex to the rasterizer stage
}
