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
	//设置为友元，使得实例化模板类能够通过GetInstance函数访问派生类中的默认构造函数
	friend class Singleton<AssetManage>;

private:
	std::unordered_map<std::string, StaticMesh> staticMeshAssets;

public:
	std::unordered_map<std::string, StaticMesh>& GetStaticMeshAssets();
};

