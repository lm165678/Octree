struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	return IN.col;
}