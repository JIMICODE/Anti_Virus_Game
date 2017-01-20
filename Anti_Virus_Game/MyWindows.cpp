#include "MyDirectX.h"
using namespace std;

bool gameover = false;
//For old D3DX lib
int (WINAPIV * __vsnprintf)(char *, size_t, const char*, va_list) = _vsnprintf;

//Windows event handler
LRESULT WINAPI WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		gameover = true;
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

//Windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//initialize window settings
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "MainWindowsClass";
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);
	//create new window
	HWND window = CreateWindow
	(
		"MainWindowsClass",
		APPTITLE.c_str(),
		WS_EX_TOPMOST | WS_POPUP,
		0,
		0,
		SCREENW,
		SCREENH,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!window) return 0;

	//display the window
	ShowWindow(window, nCmdShow);
	UpdateWindow(window);
	//initialize the game
	if (!Game_Init(window)) return 0;
	//main message loop
	MSG message;
	while (!gameover)
	{
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		//process game loop
		Game_Run(window);
	}
	//shutdown
	Game_End();

	return message.wParam;
}