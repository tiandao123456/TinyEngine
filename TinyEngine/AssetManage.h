#pragma once
#include <unordered_map>
#include "Common/Singleton.h"
#include "CommonStruct.h"

class AssetManage:public Singleton<AssetManage>
{
public:
	AssetManage(const AssetManage&) = delete;
	AssetManage& operator=(const AssetManage&) = delete;
protected:
	AssetManage() = default;
	//����Ϊ��Ԫ��ʹ��ʵ����ģ�����ܹ�ͨ��GetInstance���������������е�Ĭ�Ϲ��캯��
	friend class Singleton<AssetManage>;

private:
	std::unordered_map<std::string, StaticMesh> staticMeshAssets;

public:
	std::unordered_map<std::string, StaticMesh>& GetStaticMeshAssets();
};

