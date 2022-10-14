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

#include "RegisterBinding.hlsl"

struct VertexIn
{
	float3 pos   : POSITION;
	float3 normal: NORMAL;
	float2 uv    : TEXCOORD;
};

struct VertexOut
{
	float4 pos        : SV_POSITION;
	float4 shadowposH : POSITION0;
	float4 posW       : POSITION1;
	float3 normal     : NORMAL0;
	float2 uv         : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.posW = float4(vin.pos, 1.0f);
	//计算经过mvp变换后的坐标
	vout.pos = mul(mul(float4(vin.pos, 1.0f), worldMatrix), cameraVPMatrix);
	//先变换到世界，再乘以light的view*正交投影*T(将x，y的坐标变换到[-1,1])
	vout.shadowposH = mul(mul(float4(vin.pos, 1.0f), worldMatrix), lightVPTexMatrix);
	vout.normal = vin.normal;
	vout.uv = vin.uv;

	return vout;
}

float ShadowCalculation(float4 ShadowPosH)
{
	//进行透视除法
	ShadowPosH.xyz /= ShadowPosH.w;
	float currentDepth = ShadowPosH.z;
	uint width, height, numMips;
	shadowMap.GetDimensions(0, width, height, numMips);

	//使用PCF
	float dx = 1.0f / (float)width;
	float percentLit = 0.0f;
	//纹理上的偏移
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};
	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += shadowMap.SampleCmpLevelZero(gsamShadow,
			ShadowPosH.xy + offsets[i], currentDepth).r;
	}
	return percentLit / 9.0f;

	//float2 pixelPos = ShadowPosH.xy * width;
	//float depthInMap = shadowMap.Load(int3(pixelPos, 0)).r;
	////当前深度更深的时候设置值为0
	//return currentDepth > depthInMap ? 0 : 1;
}

float4 PS(VertexOut pin) : SV_Target
{
	float3 lightDirection = float3(1.0, 1.0, 1.0);
	float4 lightColor = float4(1.0, 1.0, 1.0, 1.0);
	float4 ambientColor = float4(0.1, 0.0, 0.0, 1.0);
	float4 specularColor = float4(1.0, 1.0, 1.0, 1.0);
	float shiness = 20.0;

	float4 diffuseAlbedo = diffuseMap.Sample(gsamPointWrap, pin.uv);
	float4 pixelNormal = normalMap.Sample(gsamPointWrap, pin.uv);
	pixelNormal = normalize(pixelNormal);

	lightDirection = normalize(lightDirection);
	float dotNormalLight = max(dot(float3(pixelNormal.x, pixelNormal.y, pixelNormal.z), lightDirection), 0.0f);
	float4 diffuseColor = dotNormalLight * lightColor * diffuseAlbedo;

	float viewDir = float3(cameraLoc.x - pin.pos.x, cameraLoc.y - pin.pos.y, cameraLoc.z - pin.pos.z);
	float halfDir = - normalize(viewDir + lightDirection);
	float specularDot = max(dot(halfDir, float3(pixelNormal.x, pixelNormal.y, pixelNormal.z)), 0.0f);
	specularColor = dotNormalLight * pow(specularDot,shiness) * specularColor * lightColor;

	return (diffuseColor + specularColor) * (ShadowCalculation(pin.shadowposH) + 0.1) + ambientColor;
}





