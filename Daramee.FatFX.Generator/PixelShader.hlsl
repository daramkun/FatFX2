struct PIXEL_IN
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD;
};
		
Texture2D texture0 : register(t0);
SamplerState samplerState0 : register(s0);

cbuffer OVERLAY : register(b0)
{
	float4 overlayColor;
};

float4 main(PIXEL_IN pin) : SV_Target
{
	return texture0.Sample(samplerState0, pin.texcoord) * overlayColor;
}