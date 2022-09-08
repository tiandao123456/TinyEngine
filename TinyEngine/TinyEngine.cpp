#include "stdafx.h"
#include "DxApp.h"
#include "JsonfileParse.h"
using namespace std;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	//Json解析模块
	//JsonfileParse parse;
	//parse.GetStaticMeshActorData("camp");
	
//执行某函数获取平台
//UINT GetPlatform()
	UINT platform = 0/*= GetPlatform()*/;

#if Windows == platform
	//执行初始化Windows的逻辑
	//下述同理
	DxApp instance(hInstance);
	instance.Init();

	//窗口的主循环
	instance.Run();
#endif

#if Mac == platform
#endif

#if Android == platform
#endif

#if Ios == platform
#endif 
}