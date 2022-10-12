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
Texture2D shadowMap : register(t2);

//mvp����ʵ����vp������Թ̶�����ÿ��
//update��ֻ����model����(������ӱ�������ϵ
//ת������������ϵ��)
cbuffer cbWorld : register(b0)
{
	float4x4 worldMatrix;
};

cbuffer cbViewProj: register(b1)
{
    float4x4 cameraVPMatrix;
	float4x4 lightVPMatrix;
	float4x4 lightVPTexMatrix;
	float3 cameraLoc;
};

//������
SamplerState gsamLinear : register(s0);
SamplerComparisonState gsamShadow : register(s1);








