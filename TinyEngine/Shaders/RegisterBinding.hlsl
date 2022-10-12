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
//法线贴图和纹理贴图
Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D shadowMap : register(t2);

//mvp矩阵，实际上vp矩阵可以固定，在每个
//update中只传输model矩阵(将顶点从本地坐标系
//转换到世界坐标系中)
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

//采样器
SamplerState gsamLinear : register(s0);
SamplerComparisonState gsamShadow : register(s1);








