#pragma once
#include "Common/Singleton.h"
#include <unordered_map>
class SceneManage:public Singleton<SceneManage>
{
public:
	SceneManage(const SceneManage&) = delete;
	SceneManage& operator=(const SceneManage&) = delete;
protected:
	SceneManage() = default;
	//设置为友元，使得实例化模板类能够通过GetInstance函数访问派生类中的默认构造函数
	friend class Singleton<SceneManage>;
private:
	std::unordered_map<std::string, StaticMeshActor> sceneStaticMeshActor;
	std::unordered_map<std::string, Camera> CameraActor;
	Light light;
public:
	std::unordered_map<std::string, StaticMeshActor>& GetStaticMeshActor();
	std::unordered_map<std::string, Camera>& GetCameraActor();
	Light& GetLight();
};

