#pragma once
#include "Win.h"
#include "Renderer.h"
#include "JsonFileParse.h"

class TEngine
{
private:
	bool isRunning = true;
	JsonFileParse jsonFileParse;

	Renderer renderer;
private:
	void GameTick();
	bool messageLoop();
public:
	void Init();
	void Run();
};

