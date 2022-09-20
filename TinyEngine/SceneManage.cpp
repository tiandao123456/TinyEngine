#include "stdafx.h"
#include "SceneManage.h"
int SceneManage::colorChoose = 0;

const staticMeshActor SceneManage::GetStaticMeshActorData(std::string actorName)
{
	staticMeshActor staticMeshData;
	std::ifstream strJsonContent(filePath, std::ios::binary);
	//�����ȡJson�ļ�
	reader.parse(strJsonContent, value);
	Json::Value staticMeshDataArray = value["staticMeshData"];

	for (auto i = 0; i < staticMeshDataArray.size(); i++)
	{
		Json::Value temp = staticMeshDataArray[i];
		if (temp["staticMeshName"] == actorName)
		{
			//��������
			auto keyValue = temp["vertices"];
			for (auto j = 0; j < keyValue.size();)
			{
				//ֱ�Ӷ�ȡ������
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
			//��������
			keyValue = temp["indices"];
			for (auto j = 0; j < keyValue.size(); j++)
			{
				std::uint16_t temp = keyValue[j].asUInt();
				staticMeshData.indices.push_back(temp);
			}
			//����λ�ƾ���
			keyValue = temp["modelMatrix"];
			for (auto j = 0; j < keyValue.size(); j++)
			{
				staticMeshData.modelMatrix.push_back(keyValue[j].asDouble());
			}
			//��������
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
	//�����ȡJson�ļ�
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
