#pragma once
#pragma comment(lib, "json_vc71_libmtd.lib")
#include "AssetManage.h"
#include "SceneManage.h"

class JsonFileParse
{
private:
	//负责读取Json文件
	Json::Reader reader;
	Json::Value value;
	std::string filePath = "SceneMessage.json";

private:
	void FillStaticMeshDatas(const Json::Value& arrayElement);
	std::vector<float> FillWorldMatrixDatas(const Json::Value& arrayElement);

public:
	//针对Json文件进行解析，主要有下述数据
	void ParseStaticMeshDatas();
	void ParseLightDatas();
	void ParseCameraDatas();

	//控制台窗口输出数据，验证数据的正确性
	void ShowAllData();
};

