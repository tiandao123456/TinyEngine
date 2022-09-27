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

//法线贴图和纹理贴图
Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);

//mvp矩阵，实际上vp矩阵可以固定，在每个
//update中只传输model矩阵(将顶点从本地坐标系
//转换到世界坐标系中)
cbuffer cbWorld : register(b0)
{
	float4x4 world;
};

//材质结构体
cbuffer cbMaterial: register(b2)
{
	// 散射光反射率
	float4 diffuseAlbedo;
	// 菲涅耳R0参数
	float3 fresnelR0;
	// 粗糙度
	float roughness;
};

cbuffer cbViewProj: register(b3)
{
	float4x4 viewProj;
}

//采样器
SamplerState gsamLinear : register(s0);

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

	vout.pos = mul(mul(float4(vin.pos, 1.0f), world), viewProj);
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
	float4 pixelNorma = normalMap.Sample(gsamLinear, pin.uv);

	lightDirection = normalize(lightDirection);
	float dotNormalLight = max(dot(float3(pixelNorma.x, pixelNorma.y, pixelNorma.z), lightDirection), 0.0f);
	float4 diffuseColor = dotNormalLight * lightColor * diffuseAlbedo;

	return diffuseColor + ambientColor;
}






