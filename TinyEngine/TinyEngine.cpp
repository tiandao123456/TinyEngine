#include "stdafx.h"
#include "DxApp.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
//执行某函数获取平台
//UINT GetPlatform()
	UINT platform /*= GetPlatform()*/;

#if Windows == platform
	//执行初始化Windows的逻辑
	//下述同理
#endif

#if Mac == platform
#endif

#if Android == platform
#endif

#if Ios == platform
#endif 

	DxApp instance(hInstance);
	instance.Init();

	//窗口的主循环
	instance.Run();
}