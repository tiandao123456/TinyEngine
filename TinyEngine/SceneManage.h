#pragma once
#pragma comment(lib, "json_vc71_libmtd.lib")

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT3 color;
};

struct cameraInfo
{
	std::vector<float> location;
	std::vector<float> target;
};

//静态网格体数据
struct staticMeshActor
{
	std::vector<Vertex> vertices;
	std::vector<std::uint16_t> indices;
	std::vector<float> modelMatrix;
	int indiceNum;
};

struct lightInfo
{
	std::vector<float> location;
	std::vector<float> direction;
	float intensity;
};

class SceneManage
{
private:
	//负责读取Json文件
	Json::Reader reader;
	Json::Value value;
	std::string filePath = "SceneMessage.json";

public:
	static int colorChoose;
	//根据传入的名称进行匹配
	//静态网格体actor对应的顶点数据，索引数据，世界变换矩阵
	const staticMeshActor GetStaticMeshActorData(std::string staticMeshName);

	//相机的view矩阵
	const cameraInfo GetCameraActorData(std::string cameraName);

	const lightInfo GetLightInfo();
};

