#pragma once
#include "Common/Singleton.h"
class Win:public Singleton<Win> 
{
public:
	//window
	HINSTANCE mhAppInst = nullptr;
	std::wstring mMainWndCaption = L"d3d App";
	static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	bool InitMainWindow();

public:
	static bool lButtonDown;
	static bool rButtonDown;
	static HWND mhMainWnd;
};

