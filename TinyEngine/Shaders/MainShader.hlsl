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
	float4 shadowposH : POSITION;
	float3 normal     : NORMAL;
	float2 uv         : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	//���㾭��mvp�任�������
	vout.pos = mul(mul(float4(vin.pos, 1.0f), worldMatrix), cameraVPMatrix);
	//�ȱ任�����磬�ٳ���light��view*����ͶӰ*T(��x��y������任��[-1,1])
	vout.shadowposH = mul(mul(float4(vin.pos, 1.0f), worldMatrix), lightVPTexMatrix);
	vout.normal = vin.normal;
	vout.uv = vin.uv;

	return vout;
}

float ShadowCalculation(float4 ShadowPosH)
{
	//����͸�ӳ���
	//ShadowPosH.xyz /= ShadowPosH.w;
	float currentDepth = ShadowPosH.z;
	uint width, height, numMips;
	shadowMap.GetDimensions(0, width, height, numMips);
	float2 pixelPos = ShadowPosH.xy * width;
	float depthInMap = shadowMap.Load(int3(pixelPos, 0)).r;
	//��ǰ��ȸ����ʱ������ֵΪ0
	return currentDepth > depthInMap ? 0 : 1;
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

	return diffuseColor * (ShadowCalculation(pin.shadowposH) + 0.1) + ambientColor;
}




