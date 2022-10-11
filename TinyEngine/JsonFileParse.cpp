#include "stdafx.h"
#include "JsonFileParse.h"

void JsonFileParse::FillStaticMeshDatas(const Json::Value& arrayElement)
{
	std::string arrayElementStaticMeshName = arrayElement["staticMeshName"].asString();

	//注意此处是引用，如果不加上引用的话就是局部变量
	auto &StaticMeshAssetsNameKeyToValue = AssetManage::GetInstance().GetStaticMeshAssets()[arrayElementStaticMeshName];

	//填充顶点数据
	auto arrayElementVertices = arrayElement["vertices"];
	for (auto i = 0; i < arrayElementVertices.size();)
	{
		float positionX = arrayElementVertices[i++].asDouble();
		float positionY = arrayElementVertices[i++].asDouble();
		float positionZ = arrayElementVertices[i++].asDouble();

		float normalX = arrayElementVertices[i++].asDouble();
		float normalY = arrayElementVertices[i++].asDouble();
		float normalZ = arrayElementVertices[i++].asDouble();

		float textureU = arrayElementVertices[i++].asDouble();
		float textureV = arrayElementVertices[i++].asDouble();

		StaticMeshAssetsNameKeyToValue.vertices.push_back(
			{ DirectX::XMFLOAT3(positionX, positionY, positionZ),
			  DirectX::XMFLOAT3(normalX,normalY, normalZ),
			  DirectX::XMFLOAT2(textureU,textureV) });
	}

	//填充索引数据
	auto arrayElementindices = arrayElement["indices"];
	for (auto i = 0; i < arrayElementindices.size(); i++)
	{
		std::uint16_t indiceValue = arrayElementindices[i].asUInt();
		StaticMeshAssetsNameKeyToValue.indices.push_back(indiceValue);
	}
}

//计算world矩阵
std::vector<float> JsonFileParse::FillWorldMatrixDatas(const Json::Value& arrayElement)
{
	std::vector<float> localWorldMatrix;
	auto &worldMatrixDatas = arrayElement["modelMatrix"];
	for (auto i = 0; i < worldMatrixDatas.size(); i++)
	{
		localWorldMatrix.push_back(worldMatrixDatas[i].asDouble());
	}
	return localWorldMatrix;
}

void JsonFileParse::ParseStaticMeshDatas()
{
	std::ifstream strJsonContent(filePath, std::ios::binary);
	//负责读取Json文件
	reader.parse(strJsonContent, value);
	Json::Value staticMeshDataArray = value["staticMeshData"];
	//针对重名情况设置数组
	std::vector<std::string> staticMeshNameArray;
	for (auto i = 0; i < staticMeshDataArray.size(); i++)
	{
		//名称转换到string并push到vector中
		Json::Value arrayElement = staticMeshDataArray[i];
		std::string arrayElementStaticMeshName = arrayElement["staticMeshName"].asString();

		//判断staticMeshName重复情况
		if (staticMeshNameArray.size() == 0)
		{
			staticMeshNameArray.push_back(arrayElementStaticMeshName);
			//填充StaticMesh数据
			FillStaticMeshDatas(arrayElement);

			auto &staticMeshNameKeyToValue = SceneManage::GetInstance().GetStaticMeshActor()[arrayElementStaticMeshName];
			
			//填充staticMeshActor的StaticMesh结构体
			staticMeshNameKeyToValue.staticMesh
				= AssetManage::GetInstance().GetStaticMeshAssets()[arrayElementStaticMeshName];

			staticMeshNameKeyToValue.indiceNum
				= AssetManage::GetInstance().GetStaticMeshAssets()[arrayElementStaticMeshName].indices.size();

			//world矩阵计算填充
			staticMeshNameKeyToValue.worldMatrix.push_back(
				FillWorldMatrixDatas(arrayElement));
		}
		else
		{
			for (auto j = 0; j < staticMeshNameArray.size(); j++)
			{
				//如果能够在数组中找到该名称
				if (arrayElementStaticMeshName == staticMeshNameArray[j])
				{
					//直接根据数组下标计算world矩阵赋值
					SceneManage::GetInstance().GetStaticMeshActor()[arrayElementStaticMeshName].worldMatrix.push_back(
						FillWorldMatrixDatas(arrayElement));
					//数据存放完成后直接break不再进行后面的循环
					break;
				}
				//在该数组中找不到该名称
				else
				{
					staticMeshNameArray.push_back(arrayElementStaticMeshName);
					//填充StaticMesh数据
					FillStaticMeshDatas(arrayElement);

					auto &staticMeshNameKeyToValue = SceneManage::GetInstance().GetStaticMeshActor()[arrayElementStaticMeshName];

					//填充staticMeshActor的StaticMesh结构体
					staticMeshNameKeyToValue.staticMesh
						= AssetManage::GetInstance().GetStaticMeshAssets()[arrayElementStaticMeshName];

					staticMeshNameKeyToValue.indiceNum
						= AssetManage::GetInstance().GetStaticMeshAssets()[arrayElementStaticMeshName].indices.size();
					
					//world矩阵计算填充
					staticMeshNameKeyToValue.worldMatrix.push_back(
						FillWorldMatrixDatas(arrayElement));
					break;
				}
			}
		}
	}
}
void JsonFileParse::ParseLightDatas()
{
	std::ifstream strJsonContent(filePath, std::ios::binary);
	reader.parse(strJsonContent, value);
	Json::Value light = value["lightInfo"];

	//获取光线的位置、方向、强度
	auto &sceneLight = SceneManage::GetInstance().GetLight();
	sceneLight.location.push_back(light["location"]["x"].asDouble());
	sceneLight.location.push_back(light["location"]["y"].asDouble());
	sceneLight.location.push_back(light["location"]["z"].asDouble());

	sceneLight.direction.push_back(light["direction"]["x"].asDouble());
	sceneLight.direction.push_back(light["direction"]["y"].asDouble());
	sceneLight.direction.push_back(light["direction"]["z"].asDouble());

	sceneLight.intensity = light["intensity"].asDouble();
}
void JsonFileParse::ParseCameraDatas()
{
	std::ifstream strJsonContent(filePath, std::ios::binary);
	reader.parse(strJsonContent, value);
	Json::Value cameraArray = value["cameraInfo"];

	for (auto i = 0; i < cameraArray.size(); i++)
	{
		auto cameraElement = cameraArray[i];
		std::string cameraName = cameraElement["cameraName"].asString();

		//注意此处为引用，不然所获得的值只会存储到cameraNameKey而不会存储键值对中
		auto &cameraNameKey = SceneManage::GetInstance().GetCameraActor()[cameraName];
		cameraNameKey.aspect = cameraElement["aspect"].asDouble();
		
		//获得相机location以及target
		cameraNameKey.location.push_back(cameraElement["location"]["x"].asDouble());
		cameraNameKey.location.push_back(cameraElement["location"]["y"].asDouble());
		cameraNameKey.location.push_back(cameraElement["location"]["z"].asDouble());

		cameraNameKey.target.push_back(cameraElement["target"]["x"].asDouble());
		cameraNameKey.target.push_back(cameraElement["target"]["y"].asDouble());
		cameraNameKey.target.push_back(cameraElement["target"]["z"].asDouble());
	}
}

//确保数据正确
void JsonFileParse::ShowAllData()
{
	AllocConsole();

	//输出资产中的数据
	auto assetArray = AssetManage::GetInstance().GetStaticMeshAssets();
	for (auto iter = assetArray.begin(); iter != assetArray.end(); iter++)
	{
		for (auto i = 0; i < iter->first.size(); i++)
			_cprintf("%c", iter->first[i]);
		_cprintf("\n");

		int verticeSize = iter->second.vertices.size();
		_cprintf("vertices's num:%d\n", verticeSize);
		for (auto j = 0; j < verticeSize; j++)
		{
			_cprintf("%f ", iter->second.vertices[j].position.x);
			_cprintf("%f ", iter->second.vertices[j].position.y);
			_cprintf("%f ", iter->second.vertices[j].position.z);
			_cprintf("%f ", iter->second.vertices[j].normal.x);
			_cprintf("%f ", iter->second.vertices[j].normal.y);
			_cprintf("%f ", iter->second.vertices[j].normal.z);
			_cprintf("%f ", iter->second.vertices[j].uv.x);
			_cprintf("%f ", iter->second.vertices[j].uv.y);
		}
		_cprintf("\n");
		int indiceSize = iter->second.indices.size();
		_cprintf("indices's num:%d\n", indiceSize);
		for (auto j = 0; j < indiceSize; j++)
			_cprintf("%d ", iter->second.indices[j]);
		_cprintf("\n");
	}

	//分界线
	_cprintf("================================================================");
	_cprintf("================================================================");
	_cprintf("\n");
	auto actorArray = SceneManage::GetInstance().GetStaticMeshActor();
	for (auto iter = actorArray.begin(); iter != actorArray.end(); iter++)
	{
		for (auto i = 0; i < iter->first.size(); i++)
			_cprintf("%c", iter->first[i]);
		_cprintf("\n");

		auto verticeSize = iter->second.staticMesh.vertices.size();
		for (auto i = 0; i < verticeSize; i++)
		{
			_cprintf("%f ", iter->second.staticMesh.vertices[i].position.x);
			_cprintf("%f ", iter->second.staticMesh.vertices[i].position.y);
			_cprintf("%f ", iter->second.staticMesh.vertices[i].position.z);
			_cprintf("%f ", iter->second.staticMesh.vertices[i].normal.x);
			_cprintf("%f ", iter->second.staticMesh.vertices[i].normal.y);
			_cprintf("%f ", iter->second.staticMesh.vertices[i].normal.z);
			_cprintf("%f ", iter->second.staticMesh.vertices[i].uv.x);
			_cprintf("%f ", iter->second.staticMesh.vertices[i].uv.y);
		}
		_cprintf("\n");
		auto indiceSize = iter->second.staticMesh.indices.size();
		for (auto i = 0; i < indiceSize; i++)
		{
			_cprintf("%d ", iter->second.staticMesh.indices[i]);
		}
		_cprintf("\n");

		for (auto i = 0; i < iter->second.worldMatrix.size(); i++)
		{
			for (auto j = 0; j < iter->first.size(); j++)
				_cprintf("%c", iter->first[j]);
			_cprintf("\n");
			for (auto j = 0; j < iter->second.worldMatrix[i].size(); j++)
			{
				_cprintf("%f ", iter->second.worldMatrix[i][j]);
			}
			_cprintf("\n");
		}
	}

	//获取相机数据
	auto cameraArray = SceneManage::GetInstance().GetCameraActor();
	for (auto iter = cameraArray.begin(); iter != cameraArray.end(); iter++)
	{
		for (auto i = 0; i < iter->first.size(); i++)
			_cprintf("%c", iter->first[i]);
		_cprintf("\n");

		auto locationSize = iter->second.location.size();
		for (auto j = 0; j < locationSize; j++)
			_cprintf("%f ", iter->second.location[j]);
		_cprintf("\n");

		auto targetSize = iter->second.target.size();
		for (auto j = 0; j < locationSize; j++)
			_cprintf("%f ", iter->second.target[j]);
		_cprintf("\n");

		_cprintf("%f ", iter->second.aspect);
	}
	_cprintf("\n");

	auto lightInfo = SceneManage::GetInstance().GetLight();
	for (auto i = 0; i < lightInfo.location.size(); i++)
		_cprintf("%f ", lightInfo.location[i]);
	_cprintf("\n");

	for (auto i = 0; i < lightInfo.direction.size(); i++)
		_cprintf("%f ", lightInfo.direction[i]);
	_cprintf("\n");

	_cprintf("%f ", lightInfo.intensity);
}
