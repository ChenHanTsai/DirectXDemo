#pragma pack_matrix(row_major)//it will use row major

struct DS_IN
{
	float3 posH : POSITION;
	float3 tex : TEXCOORD0;
	float3 nor : NORMAL;

	float3 tangent:TANGENT;
	float3 binormal:BINORMAL;
};

struct DS_OUTPUT
{
	float4 posH : SV_POSITION;
	float3 tex : TEXCOORD0;
	float3 nor : NORMAL;
	float3 viewDir:TEXCOORD1;
	float3 pointLightPosVOut:TEXCOORD2;
	float3 spotLightPosVOut:TEXCOORD3;

	float3 tangent:TANGENT;
	float3 binormal:BINORMAL;
	 
};
cbuffer TotlaMatrix : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
};

// Output patch constant data.
struct ConstantOutPut
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

cbuffer CameraLightBuffer
{
	float3 cameraPos;
	float pad;

	//point light
	float3 pointLightPos;
	float pointLightNumber;

	//spot light
	float3 spotLightPos;
	float pad2;
};



#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
	ConstantOutPut input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<DS_IN, NUM_CONTROL_POINTS> patch)
{
	//declard
	DS_IN calculateInput;
	DS_OUTPUT output;

	//calculate weight by const data output
	calculateInput.posH = float4(patch[0].posH*domain.x + patch[1].posH*domain.y + patch[2].posH*domain.z, 1);
	//tex
	calculateInput.tex = float3(patch[0].tex*domain.x + patch[1].tex*domain.y + patch[2].tex*domain.z);
	//normal
	calculateInput.nor = float3(patch[0].nor*domain.x + patch[1].nor*domain.y + patch[2].nor*domain.z);

	//tangent
	calculateInput.tangent = float3(patch[0].tangent*domain.x + patch[1].tangent*domain.y + patch[2].tangent*domain.z);

	//binormal
	calculateInput.binormal = float3(patch[0].binormal*domain.x + patch[1].binormal*domain.y + patch[2].binormal*domain.z);

	float3 inputWorldPos;
	float3 ptLightWorldPos;
	float3 spotLightWorldPos;
	// ensures translation is preserved during matrix multiply  
	float4 localH = float4(calculateInput.posH,1);

	// move local space vertex from vertex buffer into world space.
	localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projectionMatrix);
	output.posH = localH;

	//tex
	output.tex = calculateInput.tex;

	//normal
	//send to the world space
	output.nor = mul(float4(calculateInput.nor, 0), worldMatrix).xyz;
	//normalized
	output.nor = normalize(output.nor);

	
	//**********know the vertex position at world space**********
	inputWorldPos = mul(calculateInput.posH, worldMatrix);

	//**********calculate for specular view **********
	output.viewDir = cameraPos - inputWorldPos;
	//normalize
	output.viewDir = normalize(output.viewDir);

	//**********know the point light pos at world space**********
	ptLightWorldPos = mul(pointLightPos, worldMatrix);
	//calculate pt light vector
	output.pointLightPosVOut = normalize(ptLightWorldPos - inputWorldPos);

	//**********know the point spotLight pos at world space**********
	spotLightWorldPos = mul(spotLightPos, worldMatrix);
	//calculate the spot light
	output.spotLightPosVOut = normalize(spotLightWorldPos - inputWorldPos);


	//****************bump map info****************
	output.tangent = mul(calculateInput.tangent, (float3x3)worldMatrix);
	output.tangent = normalize(output.tangent);

	//****************binormal ****************
	output.binormal = mul(calculateInput.binormal, (float3x3)worldMatrix);
	output.binormal = normalize(output.binormal);

	return output;
}
