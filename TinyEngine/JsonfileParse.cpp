#include "stdafx.h"
#include "JsonfileParse.h"

void JsonfileParse::GetStaticMeshActorData(std::string actorName)
{
	Json::Reader reader;
	Json::Value root;

	//���ļ��ж�ȡ����֤��ǰ�ļ���demo.json�ļ�  
	std::ifstream in("test.json");

	int t = 3;
	AllocConsole();

	if (!in)
	{
		_cprintf("i=%i ", t);
		exit(-1);
	}

	reader.parse(in, message);
	_cprintf("i=%s ", message["camp1"]);
}

void JsonfileParse::GetCameraActorData(std::string actorName)
{

}
