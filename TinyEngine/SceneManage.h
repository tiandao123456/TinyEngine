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
	//����Ϊ��Ԫ��ʹ��ʵ����ģ�����ܹ�ͨ��GetInstance���������������е�Ĭ�Ϲ��캯��
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

