#include "stdafx.h"
#include "DxApp.h"
#include "TEngine.h"
using namespace std;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{	
//执行某函数获取平台
//UINT GetPlatform()
	UINT platform = 0/*= GetPlatform()*/;

#if Windows == platform

	TEngine tEngine;
	tEngine.Init();
	tEngine.Run();

	//执行初始化Windows的逻辑
	//下述同理
#endif

#if Mac == platform
#endif

#if Android == platform
#endif

#if Ios == platform
#endif 
}