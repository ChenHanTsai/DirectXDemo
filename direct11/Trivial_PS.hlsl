
/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture:register(t0);
SamplerState SampleType:register(s0);

//for diffuse color
cbuffer LightData
{
	float4 ambientLight;
	float4 diffuseLight;
	float3 diffuseSource;

	//specular
	float specualrPower;
	float4 specularLight;

	//point light
	float4 pointLight;

	//spot light
	float4 spotLight;
	float3 spotLightDir;
	float spotDegree;
	
};

struct V_OUT
{
	float4 posH : SV_POSITION;
	float3 tex : TEXCOORD0;
	float3 nor : NORMAL;
	float3 viewDir:TEXCOORD1;
	float3 pointLightPosVOut:TEXCOORD2;
	float3 spotLightPosVOut:TEXCOORD3;
};


float4 main(V_OUT input) : SV_TARGET
{
	//for output texture color
	float4 textureColor = float4(0, 0, 0, 0);
	
	//ambient
	float4 ambientColor = float4(0, 0, 0, 0);
	
	//diffuse
	float4 diffuseColor = float4(0,0,0,0);
	float3 lightDirCal = float3(0, 0, 0);
	float lightRatio = 0;
	
	//specular
	float3 reflectDir = float3(0, 0, 0);
	float specular = 0;
	

	//point light
	float pointLightRatio = 0;
	float4 pointLightColor = float4(0, 0, 0, 0);

	//calculate
	float4 combineDisuseAndAmbientColor = float4(0, 0, 0, 0);

	//spot light ratio
	float spotLightRatio;
	float3 spotLightDirNor;
	float4 spotLightColor = float4(0, 0, 0, 0);
	//final
	float4 finalColor = float4(0, 0, 0, 0);

	input.viewDir = normalize(input.viewDir);
	input.pointLightPosVOut = normalize(input.pointLightPosVOut);
	input.spotLightPosVOut = normalize(input.spotLightPosVOut);
	
	//sample for texture data
	textureColor = shaderTexture.Sample(SampleType, input.tex);
	
	//set the ambient light 
	ambientColor = ambientLight;

	//directional light invert
	lightDirCal = -diffuseSource;

	//clamp
	lightRatio = saturate(dot(lightDirCal, input.nor));
	//specular light & directional light
	if (lightRatio > 0)
	{
		//diffuse Color cal
		diffuseColor = saturate(lightRatio* diffuseLight);
		combineDisuseAndAmbientColor = diffuseColor + ambientColor;

		//only at light can be seen to calculate specular
		//calculate the relection direction
		reflectDir = normalize(2 * lightRatio*input.nor - lightDirCal);

		//calculate the refection amount based ont the degree
		float temp = saturate(dot(reflectDir, input.viewDir));

		//based on the power adjust
		specular = pow(temp, specualrPower);
	//	specularLight*specular;
	}
	else
	{
		combineDisuseAndAmbientColor = ambientColor;
	}

	//it maybe over 1
	combineDisuseAndAmbientColor = saturate(combineDisuseAndAmbientColor);
	
	//point light
	pointLightRatio = saturate(dot(input.nor, input.pointLightPosVOut));
	pointLightColor = pointLightRatio*pointLight;

	//calculate spot light
	spotLightDirNor = normalize(spotLightDir);
	spotLightRatio = saturate(dot(-input.spotLightPosVOut, spotLightDirNor));
	if (spotLightRatio > spotDegree)
	{
		spotLightRatio = saturate(dot(input.spotLightPosVOut, input.nor));
	}
	else
	{
		spotLightRatio = 0.0f;
	}
	spotLightColor = spotLightRatio*spotLight;

	//cal final color
	finalColor = (combineDisuseAndAmbientColor + pointLightColor + spotLightColor)*textureColor + specular*specularLight;

	//adjust by specular light at the end
	finalColor = saturate(finalColor );

//	finalColor = pointLightColorCal*textureColor;
	return finalColor;
}
