#pragma once
#pragma comment(lib, "json_vc71_libmtd.lib")

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

struct cameraInfo
{
	std::vector<float> location;
	std::vector<float> target;
};

//��̬����������
struct staticMeshActor
{
	std::vector<Vertex> vertices;
	std::vector<std::uint16_t> indices;
	std::vector<float> modelMatrix;
	int indiceNum;
};

class SceneManage
{
private:
	//�����ȡJson�ļ�
	Json::Reader reader;
	Json::Value value;
	std::string filePath = "SceneMessage.json";

public:
	static int colorChoose;
	//���ݴ�������ƽ���ƥ��
	//��̬������actor��Ӧ�Ķ������ݣ��������ݣ�����任����
	const staticMeshActor GetStaticMeshActorData(std::string staticMeshName);

	//�����view����
	const cameraInfo GetCameraActorData(std::string cameraName);
};

