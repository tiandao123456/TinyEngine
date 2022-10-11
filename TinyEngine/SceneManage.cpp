#include "stdafx.h"
#include "SceneManage.h"

//�����е�StaticMeshActor,��һ��StaticMeshName��Ӧ���worldMatrix
std::unordered_map<std::string, StaticMeshActor>& SceneManage::GetStaticMeshActor()
{
	return sceneStaticMeshActor;
}

std::unordered_map<std::string, Camera>& SceneManage::GetCameraActor()
{
	return CameraActor;
}
Light& SceneManage::GetLight()
{
	return light;
}
