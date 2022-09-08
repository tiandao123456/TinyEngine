#pragma once
#pragma comment(lib, "json_vc71_libmtd.lib")
class JsonfileParse
{
private:
	//�����ȡJson�ļ�
	Json::Reader reader;
	Json::Value message;
	std::string filePath = ".\test.json";

public:
	//���ݴ�������ƽ���ƥ��
	//��̬������actor��Ӧ�Ķ������ݣ��������ݣ�����任����
	void GetStaticMeshActorData(std::string actorName);

	//�����view����
	void GetCameraActorData(std::string actorName);
};

