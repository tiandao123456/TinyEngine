//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);

SamplerState gsamLinear : register(s0);

cbuffer cbPerObject : register(b0)
{
	float4x4 worldViewProj;
};

struct VertexIn
{
	float3 pos   : POSITION;
	float3 normal: NORMAL;
	float2 uv    : TEXCOORD;
	float3 color : COLOR;
};

struct VertexOut
{
	float4 pos   : SV_POSITION;
	float3 normal: NORMAL;
	float2 uv    : TEXCOORD;
	float4 color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.pos = mul(float4(vin.pos, 1.0f), worldViewProj);
	vout.color = float4(vin.color, 1.0f);
	vout.normal = vin.normal;
	vout.uv = vin.uv;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float3 lightDirection = float3(1.0, 1.0, 1.0);
	float4 lightColor = float4(1.0, 1.0, 1.0, 1.0);
	float4 ambientColor = float4(0.1, 0.0, 0.0, 1.0);

	float4 diffuseAlbedo = diffuseMap.Sample(gsamLinear, pin.uv);
	float4 pixelNormal = normalMap.Sample(gsamLinear, pin.uv);

	lightDirection = normalize(lightDirection);
	float dotNormalLight = max(dot(float3(pixelNormal.x, pixelNormal.y, pixelNormal.z), lightDirection), 0.0f);
	float4 diffuseColor = dotNormalLight * lightColor * diffuseAlbedo;

	return diffuseColor + ambientColor;
}






