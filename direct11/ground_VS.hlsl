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


V_OUT main(V_IN input)
{
	V_OUT output = (V_OUT)0;

	float3 inputWorldPos;
	float3 ptLightWorldPos;
	float3 spotLightWorldPos;
	// ensures translation is preserved during matrix multiply  
	float4 localH = float4(input.posL, 1);

		// move local space vertex from vertex buffer into world space.
		localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projectionMatrix);
	output.posH = localH;

	//tex
	output.tex = input.tex;

	//normal
	//send to the world space
	output.nor = mul(float4(input.nor, 0), worldMatrix).xyz;

	//normalized
	output.nor = normalize(output.nor);

	//calculate specular
	//know the vertex position at world space
	inputWorldPos = mul(input.posL, worldMatrix);

	output.viewDir = cameraPos - inputWorldPos;
	//normalize
	output.viewDir = normalize(output.viewDir);

	//know the point light pos at world space
	ptLightWorldPos = mul(pointLightPos, worldMatrix);
	spotLightWorldPos = mul(spotLightPos, worldMatrix);

	//calculate pt light vector
	output.pointLightPosVOut = normalize(ptLightWorldPos - inputWorldPos);

	//calculate the spot light
	output.spotLightPosVOut = normalize(spotLightWorldPos - inputWorldPos);

	output.tangent = input.tangent;
	output.binormal = input.binormal;

	return output; // send projected vertex to the rasterizer stage
}
