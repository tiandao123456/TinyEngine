#pragma once
#include "SceneManage.h"
#include "DxApp.h"

class TEngine
{
private:
	int cameraSwitchFlag = 1;
	bool isRunning = true;
	SceneManage sceneManage;
	DxApp render;
	//window
	HINSTANCE mhAppInst = nullptr;
	std::wstring mMainWndCaption = L"d3d App";
	static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	void GetSceneDatas();
	bool InitMainWindow();
	void GameTick();
	bool messageLoop();
public:
	static HWND mhMainWnd;
	//存放场景数据的容器
	static std::vector<staticMeshActor> staticMeshDatas;
	static cameraInfo cameraData;
	static std::vector<std::vector<float>> modelMatrixDatas;
	static lightInfo lightInfoData;
public:
	void Init();
	void Run();
};

