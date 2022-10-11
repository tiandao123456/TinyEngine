#include "stdafx.h"
#include "JsonFileParse.h"

void JsonFileParse::FillStaticMeshDatas(const Json::Value& arrayElement)
{
	std::string arrayElementStaticMeshName = arrayElement["staticMeshName"].asString();

	//ע��˴������ã�������������õĻ����Ǿֲ�����
	auto &StaticMeshAssetsNameKeyToValue = AssetManage::GetInstance().GetStaticMeshAssets()[arrayElementStaticMeshName];

	//��䶥������
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

	//�����������
	auto arrayElementindices = arrayElement["indices"];
	for (auto i = 0; i < arrayElementindices.size(); i++)
	{
		std::uint16_t indiceValue = arrayElementindices[i].asUInt();
		StaticMeshAssetsNameKeyToValue.indices.push_back(indiceValue);
	}
}

//����world����
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
	//�����ȡJson�ļ�
	reader.parse(strJsonContent, value);
	Json::Value staticMeshDataArray = value["staticMeshData"];
	//������������������
	std::vector<std::string> staticMeshNameArray;
	for (auto i = 0; i < staticMeshDataArray.size(); i++)
	{
		//����ת����string��push��vector��
		Json::Value arrayElement = staticMeshDataArray[i];
		std::string arrayElementStaticMeshName = arrayElement["staticMeshName"].asString();

		//�ж�staticMeshName�ظ����
		if (staticMeshNameArray.size() == 0)
		{
			staticMeshNameArray.push_back(arrayElementStaticMeshName);
			//���StaticMesh����
			FillStaticMeshDatas(arrayElement);

			auto &staticMeshNameKeyToValue = SceneManage::GetInstance().GetStaticMeshActor()[arrayElementStaticMeshName];
			
			//���staticMeshActor��StaticMesh�ṹ��
			staticMeshNameKeyToValue.staticMesh
				= AssetManage::GetInstance().GetStaticMeshAssets()[arrayElementStaticMeshName];

			staticMeshNameKeyToValue.indiceNum
				= AssetManage::GetInstance().GetStaticMeshAssets()[arrayElementStaticMeshName].indices.size();

			//world����������
			staticMeshNameKeyToValue.worldMatrix.push_back(
				FillWorldMatrixDatas(arrayElement));
		}
		else
		{
			for (auto j = 0; j < staticMeshNameArray.size(); j++)
			{
				//����ܹ����������ҵ�������
				if (arrayElementStaticMeshName == staticMeshNameArray[j])
				{
					//ֱ�Ӹ��������±����world����ֵ
					SceneManage::GetInstance().GetStaticMeshActor()[arrayElementStaticMeshName].worldMatrix.push_back(
						FillWorldMatrixDatas(arrayElement));
					//���ݴ����ɺ�ֱ��break���ٽ��к����ѭ��
					break;
				}
				//�ڸ��������Ҳ���������
				else
				{
					staticMeshNameArray.push_back(arrayElementStaticMeshName);
					//���StaticMesh����
					FillStaticMeshDatas(arrayElement);

					auto &staticMeshNameKeyToValue = SceneManage::GetInstance().GetStaticMeshActor()[arrayElementStaticMeshName];

					//���staticMeshActor��StaticMesh�ṹ��
					staticMeshNameKeyToValue.staticMesh
						= AssetManage::GetInstance().GetStaticMeshAssets()[arrayElementStaticMeshName];

					staticMeshNameKeyToValue.indiceNum
						= AssetManage::GetInstance().GetStaticMeshAssets()[arrayElementStaticMeshName].indices.size();
					
					//world����������
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

	//��ȡ���ߵ�λ�á�����ǿ��
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

		//ע��˴�Ϊ���ã���Ȼ����õ�ֵֻ��洢��cameraNameKey������洢��ֵ����
		auto &cameraNameKey = SceneManage::GetInstance().GetCameraActor()[cameraName];
		cameraNameKey.aspect = cameraElement["aspect"].asDouble();
		
		//������location�Լ�target
		cameraNameKey.location.push_back(cameraElement["location"]["x"].asDouble());
		cameraNameKey.location.push_back(cameraElement["location"]["y"].asDouble());
		cameraNameKey.location.push_back(cameraElement["location"]["z"].asDouble());

		cameraNameKey.target.push_back(cameraElement["target"]["x"].asDouble());
		cameraNameKey.target.push_back(cameraElement["target"]["y"].asDouble());
		cameraNameKey.target.push_back(cameraElement["target"]["z"].asDouble());
	}
}

//ȷ��������ȷ
void JsonFileParse::ShowAllData()
{
	AllocConsole();

	//����ʲ��е�����
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

	//�ֽ���
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

	//��ȡ�������
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
