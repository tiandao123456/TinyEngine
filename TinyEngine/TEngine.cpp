#include "stdafx.h"
#include "TEngine.h"

//��ų������ݵ�����
std::vector<staticMeshActor> TEngine::staticMeshDatas = {};
cameraInfo TEngine::cameraData = {};
std::vector<std::vector<float>> TEngine::modelMatrixDatas = {};
lightInfo TEngine::lightInfoData = {};
XMMATRIX TEngine::shadowTransform = {};
HWND TEngine::mhMainWnd = nullptr;

//�ص�����,�����������Ϣ�ʹ���
LRESULT CALLBACK TEngine::MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_PAINT:
		//���ƽ����ڴ˴�����
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

//���ڵĳ�ʼ��
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
	//�洢��ģ�ͱ������굽��������ת���ľ���
	for (auto i = 0; i < staticMeshDatas.size(); i++)
		modelMatrixDatas.push_back(staticMeshDatas[i].modelMatrix);
}
void TEngine::GameTick()
{
	//�ڴ˴����Ը��������λ��

	//�Լ����¹�Դ��λ��
	TEngine::lightInfoData.location[0] += 0.1;
}