#include "Engine.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "CoInitialize 를 호출하지 못했습니다.");
		return -1;
	}

	Engine engine;
	if (engine.Initialize(hInstance, "WoohyukEngine", "MyWindowClass", 800, 600))
	{

		while (engine.ProcessMessages() == true)
		{
			engine.Update();
			engine.RenderFrame();
		}

	}
	return 0;
}

