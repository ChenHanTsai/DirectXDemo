
/////////////
// GLOBALS //
/////////////
//Texture2D shaderTexture:register(t0);
//SamplerState SampleType:register(s0);

struct V_OUT
{

	float4 posH : SV_POSITION;
	float4 color : COLOR;
};

float4 main(V_OUT input) : SV_TARGET
{
	float4  textureColor;
	//textureColor = float4(0, 1, 0, 1);
	textureColor = input.color;
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	//textureColor = shaderTexture.Sample(SampleType, input.tex);

	return textureColor;
	//return input.color;
}
