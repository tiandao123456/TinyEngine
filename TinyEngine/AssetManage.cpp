#include "stdafx.h"
#include "AssetManage.h"

std::unordered_map<std::string, StaticMesh>& AssetManage::GetStaticMeshAssets()
{
	return staticMeshAssets;
}
