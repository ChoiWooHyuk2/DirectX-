#include "Engine.h"



bool Engine::Initialize(HINSTANCE hInstace, std::string window_title, std::string window_class, int width, int height)
{
	if (!this->render_window.Initialize(this, hInstace, window_title, window_class, width, height))
		return false;
	if (!gfx.Initialize(this->render_window.GetHWND(), width, height))
		return false;

	return true;
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update()
{
	//GameUpdate
}

void Engine::RenderFrame()
{
	this->gfx.RenderFrame();
}

Engine::Engine()
{
}


Engine::~Engine()
{
}
