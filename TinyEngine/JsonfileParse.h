#pragma once
#pragma comment(lib, "json_vc71_libmtd.lib")
class JsonfileParse
{
private:
	//负责读取Json文件
	Json::Reader reader;
	Json::Value message;
	std::string filePath = ".\test.json";

public:
	//根据传入的名称进行匹配
	//静态网格体actor对应的顶点数据，索引数据，世界变换矩阵
	void GetStaticMeshActorData(std::string actorName);

	//相机的view矩阵
	void GetCameraActorData(std::string actorName);
};

