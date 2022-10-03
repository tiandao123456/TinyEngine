#include "stdafx.h"
#include "TEngine.h"

//存放场景数据的容器
std::vector<staticMeshActor> TEngine::staticMeshDatas = {};
cameraInfo TEngine::cameraData = {};
std::vector<std::vector<float>> TEngine::modelMatrixDatas = {};
lightInfo TEngine::lightInfoData = {};
XMMATRIX TEngine::shadowTransform = {};
HWND TEngine::mhMainWnd = nullptr;

//回调函数,有鼠标或键盘消息就触发
LRESULT CALLBACK TEngine::MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_PAINT:
		//绘制将不在此处处理
		PAINTSTRUCT ps;
		BeginPaint(TEngine::mhMainWnd, &ps);
		EndPaint(TEngine::mhMainWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//窗口的初始化
bool TEngine::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"MainWnd", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, 0);

	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

	return true;
}

void TEngine::Init()
{
	GetSceneDatas();
	InitMainWindow();
	render.InitDirectx12();
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
		render.RenderTick();
	}
}

void TEngine::GetSceneDatas()
{
	staticMeshDatas.push_back(sceneManage.GetStaticMeshActorData("SM_Chair"));
	staticMeshDatas.push_back(sceneManage.GetStaticMeshActorData("Cube"));
	cameraData = sceneManage.GetCameraActorData("CameraActor_2");
	lightInfoData = sceneManage.GetLightInfo();
	modelMatrixDatas.clear();
	//存储从模型本地坐标到世界坐标转换的矩阵
	for (auto i = 0; i < staticMeshDatas.size(); i++)
		modelMatrixDatas.push_back(staticMeshDatas[i].modelMatrix);
}
void TEngine::GameTick()
{
	//在此处可以更新相机的位置

	//以及更新光源的位置
	TEngine::lightInfoData.location[0] += 0.1;
}