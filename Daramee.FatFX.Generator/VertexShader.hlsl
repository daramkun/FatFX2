struct VERTEX_IN
{
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct PIXEL_IN
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD;
};

cbuffer WORLD : register(b0)
{
	float4x4 worldMatrix;
};

cbuffer PROJECTION : register(b1)
{
	float4x4 projectionMatrix;
};

PIXEL_IN main(VERTEX_IN vin)
{
	PIXEL_IN vout;
	vout.position = mul(mul(float4(vin.position, 1), worldMatrix), projectionMatrix);
	vout.texcoord = vin.texcoord;
	return vout;
}