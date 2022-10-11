#include "stdafx.h"
#include "SceneManage.h"

//场景中的StaticMeshActor,由一个StaticMeshName对应多个worldMatrix
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
