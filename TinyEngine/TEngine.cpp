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
	//在此处可以更新相机的位置

	//以及更新光源的位置
	SceneManage::GetInstance().GetLight().location[0] += 0.5f;
}