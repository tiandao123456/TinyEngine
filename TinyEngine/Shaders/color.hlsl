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
//
//������ͼ��������ͼ
Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D shadowMap : register(t4);

//mvp����ʵ����vp������Թ̶�����ÿ��
//update��ֻ����model����(������ӱ�������ϵ
//ת������������ϵ��)
cbuffer cbWorld : register(b0)
{
	float4x4 world;
};

//���ʽṹ��
cbuffer cbMaterial: register(b2)
{
	// ɢ��ⷴ����
	float4 diffuseAlbedo;
	// ������R0����
	float3 fresnelR0;
	// �ֲڶ�
	float roughness;
};

cbuffer cbViewProj: register(b3)
{
    float4x4 viewProj;
	float4x4 shadowTransform;
	float4x4 shadowMatrix;
};

//������
SamplerState gsamLinear : register(s0);








