#pragma pack_matrix(row_major)//it will use row major
struct V_IN
{
	float3 posL : POSITION;
	//float3 colorIn : COLOR;
	float3 tex : TEXCOORD;
	float3 nor : NORMAL;
	float3 instancePos:TEXCOORD1;
};

struct V_OUT
{
	//float4 colorOut : COLOR;
	float4 posH : SV_POSITION;
	float3 tex : TEXCOORD;
	float3 nor : NORMAL;
};

cbuffer TotlaMatrix : register(b0)
{
	float4x4 worldMatrix1;
	//	float4x4 worldMatrix2;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
};

V_OUT main(V_IN input)
{
	V_OUT output = (V_OUT)0;
	
	//instance work here
	input.posL.x += input.instancePos.x;
	input.posL.y += input.instancePos.y;
	input.posL.z += input.instancePos.z;

	float4 localH = float4(input.posL, 1);
	localH = mul(localH, worldMatrix1);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projectionMatrix);
	//// TODO: Move into view space, then projection space
	output.posH = localH;
	//	output.colorOut = input.colorIn;
	output.tex = input.tex;
	return output; // send projected vertex to the rasterizer stage
}