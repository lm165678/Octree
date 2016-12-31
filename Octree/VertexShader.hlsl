cbuffer cbProj : register (b0)
{
	matrix matProj;
}

cbuffer cbView : register (b1)
{
	matrix matView;
}

cbuffer cbModel : register (b2)
{
	matrix matModel;
}

struct VS_INPUT
{
	float4 pos : POSITION;
	float4 col : COLOR;
};

struct VS_OUTPUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT;
	OUT.pos = mul(mul(mul(IN.pos, matModel), matView), matProj);
	OUT.col = IN.col;
	OUT.col.w = 1.0f;
	return OUT;
}