
/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture[3]:register(t0);
SamplerState SampleType:register(s0);

//for light color
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

	float3 tangent:TANGENT;
	float3 binormal:BINORMAL;
};


float4 main(V_OUT input) : SV_TARGET
{
	//for output texture color
	float4 textureColor1 = float4(0, 0, 0, 0);
	float4 textureColor2 = float4(0, 0, 0, 0);
	float4 textureColor = float4(0, 0, 0, 0);

	float4 bumpMap = float4(0, 0, 0, 0);
	float3 bumpNormal = float3(0, 0, 0);

	//ambient
	float4 ambientColor = float4(0, 0, 0, 0);

	//diffuse
	float4 diffuseColor = float4(0, 0, 0, 0);
	float3 lightDirCal = float3(0, 0, 0);
	float lightRatio = 0;

	//specular
	float3 reflectDir = float3(0, 0, 0);
	float3 reflectDir2 = float3(0, 0, 0);

	float specular = 0;
	float specular2 = 0;

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

	//normalize first
	input.viewDir = normalize(input.viewDir);
	input.pointLightPosVOut = normalize(input.pointLightPosVOut);
	input.spotLightPosVOut = normalize(input.spotLightPosVOut);
	input.tangent = normalize(input.tangent);
	input.binormal = normalize(input.binormal);

	//sample for texture data
	textureColor1 = shaderTexture[0].Sample(SampleType, input.tex);
	textureColor = textureColor1;
	textureColor2 = shaderTexture[1].Sample(SampleType, input.tex);
	textureColor = textureColor1*textureColor2*2.0f;
	textureColor = saturate(textureColor);

	//textureColor = shaderTexture[1].Sample(SampleType, input.tex);//debug
	
	//****************calculate the normal from the bump data****************
	//sample for bump data
	bumpMap = shaderTexture[2].Sample(SampleType, input.tex);
	//bumpMap = shaderTexture[0].Sample(SampleType, input.tex);//debug

	//range from (0,1)->(-1,1)
	bumpMap = bumpMap * 2 - 1.0f;
	
	//calculate the normal from normal map
	bumpNormal = (bumpMap.x*input.tangent+ bumpMap.y*input.binormal+ bumpMap.z*input.nor);
	
	//normalize
	bumpNormal = normalize(bumpNormal);

	//****************calculate the light****************
	//set the ambient light 
	ambientColor = ambientLight;

	//directional light invert
	lightDirCal = -diffuseSource;

	//clamp
//	lightRatio = saturate(dot(lightDirCal, input.nor));
	lightRatio = saturate(dot(lightDirCal, bumpNormal));
	//specular light & directional light
	if (lightRatio > 0)
	{
		//diffuse Color cal
		diffuseColor = saturate(lightRatio* diffuseLight);

		combineDisuseAndAmbientColor = diffuseColor + ambientColor;

		//only at light can be seen to calculate specular
		//calculate the relection direction
		//reflectDir = normalize(2 * lightRatio*input.nor - lightDirCal);
		reflectDir = normalize(2 * lightRatio*bumpNormal - lightDirCal);

		//calculate the refection amount based ont the degree
		float temp = saturate(dot(reflectDir, input.viewDir));

		//based on the power adjust
		specular = pow(temp, specualrPower/2);
		//	specularLight*specular;
	}
	else
	{
		combineDisuseAndAmbientColor = ambientColor;
	}

	//it maybe over 1
	combineDisuseAndAmbientColor = saturate(combineDisuseAndAmbientColor);

	//point light
//	pointLightRatio = saturate(dot(input.nor, input.pointLightPosVOut));
	pointLightRatio = saturate(dot(bumpNormal, input.pointLightPosVOut));
	pointLightColor = pointLightRatio*pointLight;

	//calculate spot light
	spotLightDirNor = normalize(spotLightDir);
	spotLightRatio = saturate(dot(-input.spotLightPosVOut, spotLightDirNor));
	if (spotLightRatio > spotDegree)
	{
	//	spotLightRatio = saturate(dot(input.spotLightPosVOut, input.nor));
		spotLightRatio = saturate(dot(input.spotLightPosVOut, bumpNormal));

		reflectDir2 = normalize(2 * spotLightRatio*bumpNormal - spotLightDirNor);

		//calculate the refection amount based ont the degree
		float temp = saturate(dot(reflectDir2, input.viewDir));

		//based on the power adjust
	//	specular2 = pow(temp, specualrPower);

		//spotLightRatio = 0.9f;
	}
	else
	{
		spotLightRatio = 0.0f;
	//	spotLightRatio = saturate(dot(input.spotLightPosVOut, bumpNormal));
	}

	spotLightColor = saturate(spotLightRatio*spotLight);

	//cal final color
	finalColor = (combineDisuseAndAmbientColor + pointLightColor + spotLightColor)*textureColor + specular*specularLight+ specular2*specularLight;;

	//adjust by specular light at the end
	finalColor = saturate(finalColor);

	//	finalColor = pointLightColorCal*textureColor;
	return finalColor;
}
