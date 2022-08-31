#include "stdafx.h"
#include "DxApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	DxApp instance(hInstance);
	instance.Init();

	//窗口的主循环
	instance.Run();
}