
/////////////
// GLOBALS //
/////////////
Texture2D shaderTexture:register(t0);
SamplerState SampleType:register(s0);


struct V_OUT
{
	float4 posH : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 foliageColor : TEXCOORD1;

};


float4 main(V_OUT input) : SV_TARGET
{ 

	float4 finalColor,textureColor;

	//get the texture color
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	finalColor = textureColor*float4(input.foliageColor, 1);

	//saturate
	finalColor = saturate(finalColor);
	finalColor = float4(1, 0, 0, 1);
	finalColor = textureColor;
	return finalColor;
}
