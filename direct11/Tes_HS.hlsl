#pragma pack_matrix(row_major)//it will use row major

cbuffer TessellationBuffer
{
	float tessellationAmount;
	float3 padding;
};


struct HS_IN
{
	float3 posH : POSITION;
	float3 tex : TEXCOORD0;
	float3 nor : NORMAL;

	float3 tangent:TANGENT;
	float3 binormal:BINORMAL;
};

//output from the patch constant
struct constantPatchOut
{
	float edges[3]:SV_TessFactor;
	float inside : SV_InsideTessFactor;
};

struct HS_OUT
{
	float3 posH : POSITION;
	float3 tex : TEXCOORD0;
	float3 nor : NORMAL;

	float3 tangent:TANGENT;
	float3 binormal:BINORMAL;
};


//*******path constant function
constantPatchOut HSPathConstantFunc(InputPatch<HS_IN, 3>_inputPatch, uint patchId:SV_PrimitiveID)
{
	constantPatchOut output;

	//set the tessellation factors for the three edges of the triangle
	output.edges[0] = tessellationAmount;
	output.edges[1] = tessellationAmount;
	output.edges[2] = tessellationAmount;

	output.inside = tessellationAmount;

	return output;
}


[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPathConstantFunc")]
HS_OUT main(
	InputPatch<HS_IN, 3> _inputPatch,
	uint pointID : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
	HS_OUT Output;

	// Insert code to compute Output here
	Output.posH = _inputPatch[pointID].posH;
	Output.tex = _inputPatch[pointID].tex;
	Output.nor = _inputPatch[pointID].nor;

	Output.tangent = _inputPatch[pointID].tangent;
	Output.binormal = _inputPatch[pointID].binormal;

	return Output;
}
