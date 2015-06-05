#pragma pack_matrix(row_major)//it will use row major
struct V_IN
{
	float3 posL : POSITION;
	float3 tex : TEXCOORD;
	float3 nor : NORMAL;

	float3 tangent:TANGENT;
	float3 binormal:BINORMAL;
};

struct V_OUT
{
	float3 posH : POSITION;
	float3 tex : TEXCOORD0;
	float3 nor : NORMAL;

	float3 tangent:TANGENT;
	float3 binormal:BINORMAL;
};


V_OUT main(V_IN input)
{
	V_OUT output = (V_OUT)0;

	//only send data to Hull Shader

	//pos
	output.posH = input.posL;

	//tex
	output.tex = input.tex;
	
	//normal
	output.nor = input.nor;

	//tangent
	output.tangent = input.tangent;

	//binormal
	output.binormal = input.binormal;

	return output; 
}
