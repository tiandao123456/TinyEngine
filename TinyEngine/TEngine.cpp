#include "stdafx.h"
#include "TEngine.h"

void TEngine::Init()
{
	jsonFileParse.ParseStaticMeshDatas();
	jsonFileParse.ParseCameraDatas();
	jsonFileParse.ParseLightDatas();
	//jsonFileParse.ShowAllData();
	
	Win::GetInstance().InitMainWindow();
	renderer.Init();
}

bool TEngine::messageLoop()
{
	bool quit = false;
	MSG Msg = {};
	while (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);

		if (Msg.message == WM_QUIT)
			quit = true;
	}
	return !quit;
}

void TEngine::Run()
{
	while (isRunning && messageLoop())
	{
		GameTick();
		renderer.RenderTick();
	}
}

void TEngine::GameTick()
{
	//�ڴ˴����Ը��������λ�ã����������vp�����
	//������ɺ���Ȼ��api�����䣬���Ǹ��µ��߼��ڴ˴�
	auto &camera = SceneManage::GetInstance().GetCameraActor();
	auto &theNumOneCamera = camera[camera.begin()->first];

	if (Win::GetInstance().lButtonDown)
	{
		theNumOneCamera.location[0] -= 1.0f;
		theNumOneCamera.location[1] += 1.0f;
		Win::GetInstance().lButtonDown = false;
	}
	if (Win::GetInstance().rButtonDown)
	{
		theNumOneCamera.location[0] += 1.0f;
		theNumOneCamera.location[1] -= 1.0f;
		Win::GetInstance().rButtonDown = false;
	}

	//�Լ����¹�Դ��λ��
	SceneManage::GetInstance().GetLight().location[0] += 0.5f;
}