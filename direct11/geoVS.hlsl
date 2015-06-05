#pragma pack_matrix(row_major)//it will use row major
struct V_IN
{
	float3 posL : POSITION;
	float3 tex : TEXCOORD;
	float3 nor : NORMAL;
};

struct V_OUT
{
	//float4 colorOut : COLOR;
	float4 posH : POSITION;
	//float4 posH : SV_POSITION;
	//float3 tex : TEXCOORD;
//	float3 nor : NORMAL;
};

cbuffer TotlaMatrix : register(b0)
{
	float4x4 worldMatrix1;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
};

V_OUT main(V_IN input)
{
	V_OUT output = (V_OUT)0;

	float4 localH = float4(input.posL, 1);
	//localH = mul(localH, worldMatrix1);
//	localH = mul(localH, viewMatrix);
//	localH = mul(localH, projectionMatrix);
	//// TODO: Move into view space, then projection space
	output.posH = localH;
	//	output.colorOut = input.colorIn;
//	output.tex = input.tex;
	return output; // send projected vertex to the rasterizer stage
}