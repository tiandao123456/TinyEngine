#pragma once

#include <windows.h>

#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#include "Common/d3d12.h"
#include "Common/d3dx12.h"
#include "Common/UploadHeapBuffer.h"
#include "Common/UploadHeapConstantBuffer.h"
#include "Common/DxHelper.h"
#include "include/json/json.h"
#include "DDSTextureLoader.h"
#include "CommonStruct.h"

#include <wtypes.h>
#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <fstream>
#include <conio.h>
#include <memory.h>
#include <vector>

#define mClientWidth 1000
#define mClientHeight 800
#define Pi 3.1415926535
#define totalNums 2

#define Windows 0
#define Mac 1
#define Android 2
#define Ios 3