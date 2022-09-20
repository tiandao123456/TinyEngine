#include "stdafx.h"
#include "SceneManage.h"
int SceneManage::colorChoose = 0;

const staticMeshActor SceneManage::GetStaticMeshActorData(std::string actorName)
{
	staticMeshActor staticMeshData;
	std::ifstream strJsonContent(filePath, std::ios::binary);
	//负责读取Json文件
	reader.parse(strJsonContent, value);
	Json::Value staticMeshDataArray = value["staticMeshData"];

	for (auto i = 0; i < staticMeshDataArray.size(); i++)
	{
		Json::Value temp = staticMeshDataArray[i];
		if (temp["staticMeshName"] == actorName)
		{
			//顶点数据
			auto keyValue = temp["vertices"];
			for (auto j = 0; j < keyValue.size();)
			{
				//直接读取三个数
				float positionX = keyValue[j++].asDouble();
				float positionY = keyValue[j++].asDouble();
				float positionZ = keyValue[j++].asDouble();

				float normalX = keyValue[j++].asDouble();
				float normalY = keyValue[j++].asDouble();
				float normalZ = keyValue[j++].asDouble();

				float textureU = keyValue[j++].asDouble();
				float textureV = keyValue[j++].asDouble();
				if (colorChoose == 0)
					staticMeshData.vertices.push_back({ DirectX::XMFLOAT3(positionX, positionY, positionZ),
														DirectX::XMFLOAT3(normalX,normalY, normalZ),
														DirectX::XMFLOAT2(textureU,textureV),
														DirectX::XMFLOAT3(1.0f,0.0f,0.0f) });
				else if (colorChoose == 1)
					staticMeshData.vertices.push_back({ DirectX::XMFLOAT3(positionX, positionY, positionZ),
														DirectX::XMFLOAT3(normalX,normalY, normalZ),
														DirectX::XMFLOAT2(textureU,textureV),
														DirectX::XMFLOAT3(0.0f,1.0f,0.0f) });
				else
					staticMeshData.vertices.push_back({ DirectX::XMFLOAT3(positionX, positionY, positionZ),
														DirectX::XMFLOAT3(normalX,normalY, normalZ),
														DirectX::XMFLOAT2(textureU,textureV),
														DirectX::XMFLOAT3(0.0f,0.0f,1.0f) });
			}
			//索引数据
			keyValue = temp["indices"];
			for (auto j = 0; j < keyValue.size(); j++)
			{
				std::uint16_t temp = keyValue[j].asUInt();
				staticMeshData.indices.push_back(temp);
			}
			//世界位移矩阵
			keyValue = temp["modelMatrix"];
			for (auto j = 0; j < keyValue.size(); j++)
			{
				staticMeshData.modelMatrix.push_back(keyValue[j].asDouble());
			}
			//索引数量
			staticMeshData.indiceNum = temp["indices"].size();
			++colorChoose;
			break;
		}
	}
	return staticMeshData;
}

const cameraInfo SceneManage::GetCameraActorData(std::string actorName)
{
	cameraInfo cameraData;
	std::ifstream strJsonContent(filePath, std::ios::binary);
	//负责读取Json文件
	reader.parse(strJsonContent, value);
	Json::Value cameraDataArray = value["cameraInfo"];

	for (auto i = 0; i < cameraDataArray.size(); i++)
	{
		Json::Value temp = cameraDataArray[i];
		if (temp["cameraName"] == actorName)
		{
			auto keyValue = temp["location"];
			cameraData.location.push_back(keyValue["x"].asDouble());
			cameraData.location.push_back(keyValue["y"].asDouble());
			cameraData.location.push_back(keyValue["z"].asDouble());

			keyValue = temp["target"];
			cameraData.target.push_back(keyValue["x"].asDouble());
			cameraData.target.push_back(keyValue["y"].asDouble());
			cameraData.target.push_back(keyValue["z"].asDouble());

			break;
		}
	}

	return cameraData;
}
