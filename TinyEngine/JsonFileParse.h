#pragma once
#pragma comment(lib, "json_vc71_libmtd.lib")
#include "AssetManage.h"
#include "SceneManage.h"

class JsonFileParse
{
private:
	//�����ȡJson�ļ�
	Json::Reader reader;
	Json::Value value;
	std::string filePath = "SceneMessage.json";

private:
	void FillStaticMeshDatas(const Json::Value& arrayElement);
	std::vector<float> FillWorldMatrixDatas(const Json::Value& arrayElement);

public:
	//���Json�ļ����н�������Ҫ����������
	void ParseStaticMeshDatas();
	void ParseLightDatas();
	void ParseCameraDatas();

	//����̨����������ݣ���֤���ݵ���ȷ��
	void ShowAllData();
};

