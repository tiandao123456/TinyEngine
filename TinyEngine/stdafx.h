#pragma once

#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include "Common/UploadHeapBuffer.h"
#include "Common/UploadHeapConstantBuffer.h"
#include "d3dx12.h"
#include "DxHelper.h"
#include "include/json/json.h"


#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <fstream>
#include <conio.h>
#include <memory.h>

#define mClientWidth 800
#define mClientHeight 600
#define Pi 3.1415926535

#define Windows 0
#define Mac 1
#define Android 2
#define Ios 3