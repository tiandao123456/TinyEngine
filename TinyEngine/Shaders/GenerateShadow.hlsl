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
	float4 pos   : SV_POSITION;
};


VertexOut VS(VertexIn vin)
{
	//计算深度图所用，只保留深度信息
	//lightVPMatrix:灯光的view*正交投影变换
	VertexOut vout;
	float4 posT = mul(float4(vin.pos, 1.0f), worldMatrix);
	vout.pos = mul(posT, lightVPMatrix);
	return vout;
}

void PS(VertexOut input)
{

}






