#pragma once
#include "WindowContainer.h"
class Engine : WindowContainer
{
public:
	bool Initialize(HINSTANCE hInstace, std::string window_title, std::string window_class, int width, int height);
	bool ProcessMessages();
	void Update();
	void RenderFrame();
public:
	Engine();
	~Engine();
};

