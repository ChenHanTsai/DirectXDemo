
/////////////
// GLOBALS //
/////////////
TextureCube shaderTexture:register(t0);
SamplerState SampleType:register(s0);

struct V_OUT
{
	//float4 colorOut : COLOR;
	float4 posH : SV_POSITION;
	float3 tex : TEXCOORD;
	float3 nor : NORMAL;
};

float4 main(V_OUT input) : SV_TARGET
{
	float4 tempTextureColor, textureColor;

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	return textureColor;
}
