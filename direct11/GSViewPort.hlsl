#pragma pack_matrix(row_major)//it will use row major
// format of output verticies
// these could be going to the
// rasterizer the (SO)stage or both.

struct GSOutput
{
//	float4 color : COLOR;
	float4 posH : SV_POSITION;

	//use for geo view port
	//unit viewport : SV_ViewportArrayIndex;
};

cbuffer TotlaMatrix : register(b0)
{
	float4x4 worldMatrix1;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
};
[instance(2)]
// convert each incoming world-space line segment into a projected triangle. 
[maxvertexcount(3)] // max vertex data to be output (limit 1024 total scalars)
//void main(line float4 input[2] : SV_POSITION, inout TriangleStream< GSOutput > output)
void main(point float4 input[1] : POSITION, inout TriangleStream< GSOutput > output, uint InstanceID : SV_GSInstanceID)
{
	// red green and blue vertex
	GSOutput verts[3] =
	{
		float4(1, 0, 0, 1),// float4(0, 0, 0, 1),
		float4(0, 1, 0, 1), //float4(0, 0, 0, 1),
		float4(0, 0, 1, 1)//, float4(0, 0, 0, 1)
	};
	// bottom left
	//verts[0].posH.xyz = input[0].xyz;
	//verts[0].posH.x -= 0.5f;
	//// bottom right
	//verts[2].posH = verts[0].posH;
	//verts[2].posH.x += 10.0f;
	//// top center
	//verts[1].posH.xyz = input[1].xyz;
	// prep triangle for rasterization

	//copy first
	verts[0].posH.xyz = input[0].xyz;
	verts[1].posH.xyz = input[0].xyz;
	verts[2].posH.xyz = input[0].xyz;
	verts[0].posH.x += 0.5f;
	verts[1].posH.x -= 0.5f;
	verts[2].posH.y += 0.5f;
	if (InstanceID == 0)
	{
		
	}
	else
	{
		verts[0].posH.z -= 0.5f;
		verts[1].posH.z -= 0.5f;
		verts[2].posH.z -= 0.5f;
	}
	
	float4x4 mVP = mul(viewMatrix, projectionMatrix);
		float4x4 identity = float4x4(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
		for (uint i = 0; i < 3; ++i) {
			verts[i].posH = mul(verts[i].posH, mVP);
			/*verts[i].posH = mul(verts[i].posH, identity);
			verts[i].posH = mul(verts[i].posH, viewMatrix);
			verts[i].posH = mul(verts[i].posH, projectionMatrix);*/
		}
	// send verts to the rasterizer
	output.Append(verts[0]);
	output.Append(verts[1]);
	output.Append(verts[2]);
	// do not connect to other triangles
	output.RestartStrip();
}
