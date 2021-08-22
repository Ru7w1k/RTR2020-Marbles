// headers
#include "main.h"
#include "helper.h"
#include "logger.h"
#include "audio.h"

// shaders
#include "shader.h"

#include "letters.h"

// scenes
#include "scene.h"
#include "scene-intro.h"
#include "scene-marbles.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "openal32.lib")
#pragma comment(lib, "assimp-vc142-mtd.lib")

// macros
#define WIN_WIDTH  800
#define WIN_HEIGHT 600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variables
HDC   ghdc = NULL;
HGLRC ghrc = NULL;

bool gbFullscreen = false;
bool gbActiveWindow = false;

HWND  ghwnd = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

int gWidth  = 0;
int gHeight = 0;

bool bPause = false;

float mouseX = 0.0f;
float mouseY = 0.0f;

float prevMouseX = 0.0f;
float prevMouseY = 0.0f;

bool firstMouse   = false;
bool mousePressed = false;


// WinMain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function declarations
	void initialize(void);
	void display(void);
	void update(float);

	// variable declarations
	bool bDone = false;
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyApp");
	float delta;

	// code

	// initialization of WNDCLASSEX
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(RMC_ICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(RMC_ICON));

	// register above class
	RegisterClassEx(&wndclass);

	// get the screen size
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);

	// create window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("OpenGL | Marbles"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(width / 2) - (WIN_WIDTH / 2),
		(height / 2) - (WIN_HEIGHT / 2),
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	initialize();

	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Game Loop!
	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			// get the time diff between this frame and last frame
			delta = GetTimeDeltaMS();

			if (gbActiveWindow == true)
			{
				// call update() here for OpenGL rendering
				if (!bPause) update(delta);
				// call display() here for OpenGL rendering
				display();
			}
		}
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function declaration
	void display(void);
	void resize(int, int);
	void uninitialize();
	void ToggleFullscreen(void);

	Scene scene;
	short delta;

	// code
	switch (iMsg)
	{

	case WM_SETFOCUS:
		gbActiveWindow = true;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = false;
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 0x46:
		case 0x66:
			ToggleFullscreen();
			break;

		case VK_SPACE:
			bPause = !bPause;
			break;

		case VK_LEFT:
			if (GetScene(scene) && scene.Camera != NULL)
				ProcessKeyboard(scene.Camera, CAM_LEFT);
			break;

		case VK_RIGHT:
			if (GetScene(scene) && scene.Camera != NULL)
				ProcessKeyboard(scene.Camera, CAM_RIGHT);
			break;

		case VK_UP:
			if (GetScene(scene) && scene.Camera != NULL)
				ProcessKeyboard(scene.Camera, CAM_FOREWARD);
			break;

		case VK_DOWN:
			if (GetScene(scene) && scene.Camera != NULL)
				ProcessKeyboard(scene.Camera, CAM_BACKWARD);
			break;
		default:
			break;
		}
		break;

	case WM_CHAR:
		switch (wParam)
		{
		case '.':
		case '>':
			NextScene();
			if (GetScene(scene)) scene.ResizeFunc(gWidth, gHeight);
			break;

		case ',':
		case '<':
			PrevScene();
			if (GetScene(scene)) scene.ResizeFunc(gWidth, gHeight);
			break;

		case 'C':
		case 'c':
			if (GetScene(scene) && scene.Camera != NULL)
				Print(scene.Camera);
			break;

		case 'a':
		case 'A':
			if (GetScene(scene) && scene.Camera != NULL)
				ProcessKeyboard(scene.Camera, CAM_LEFT);
			break;

		case 'd':
		case 'D':
			if (GetScene(scene) && scene.Camera != NULL)
				ProcessKeyboard(scene.Camera, CAM_RIGHT);
			break;

		case 'w':
		case 'W':
			if (GetScene(scene) && scene.Camera != NULL)
				ProcessKeyboard(scene.Camera, CAM_FOREWARD);
			break;

		case 's':
		case 'S':
			if (GetScene(scene) && scene.Camera != NULL)
				ProcessKeyboard(scene.Camera, CAM_BACKWARD);
			break;

		case 'r':
		case 'R':
			if (GetScene(scene) && scene.ResetFunc != NULL)
				scene.ResetFunc();
			break;

		default:
			break;
		}
		break;

	case WM_MOUSEWHEEL:
		// -120 scroll down
		//  120 scroll up
		delta = (short)HIWORD(wParam)/120;
		if (GetScene(scene) && scene.Camera != NULL)
		{
			Zoom(scene.Camera, -2.0f * delta);
		}
		break;

	case WM_MOUSEMOVE:
		if (mousePressed)
		{
			mouseX = (float)GET_X_LPARAM(lParam);
			mouseY = (float)GET_Y_LPARAM(lParam);

			if (firstMouse)
			{
				prevMouseX = mouseX;
				prevMouseY = mouseY;
				firstMouse = false;
			}

			if (GetScene(scene) && scene.Camera != NULL)
			{
				ProcessMouse(scene.Camera, mouseX - prevMouseX, mouseY - prevMouseY);
				prevMouseX = mouseX;
				prevMouseY = mouseY;
			}
		}
		break;

	case WM_LBUTTONDOWN:
		prevMouseX = (float)GET_X_LPARAM(lParam);
		prevMouseY = (float)GET_Y_LPARAM(lParam);
		mousePressed = true;
		break;

	case WM_LBUTTONUP:
		mousePressed = false;
		break;

	case WM_ERASEBKGND:
		return(0);

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
	// local variables
	MONITORINFO mi = { sizeof(MONITORINFO) };

	// code
	if (gbFullscreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(ghwnd, &wpPrev) &&
				GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
		gbFullscreen = true;
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
		ShowCursor(TRUE);
		gbFullscreen = false;
	}
}


void initialize(void)
{
	// function declarations
	void resize(int, int);
	void uninitialize(void);

	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	// code
	ghdc = GetDC(ghwnd);

	ZeroMemory((void*)&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		LogE("ChoosePixelFormat() failed..");
		DestroyWindow(ghwnd);
		return;
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		LogE("SetPixelFormat() failed..");
		DestroyWindow(ghwnd);
		return;
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		LogE("wglCreateContext() failed..");
		DestroyWindow(ghwnd);
		return;
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		LogE("wglMakeCurrent() failed..");
		DestroyWindow(ghwnd);
		return;
	}

	// glew initialization for programmable pipeline
	GLenum glew_error = glewInit();
	if (glew_error != GLEW_OK)
	{
		LogE("glewInit() failed..");
		DestroyWindow(ghwnd);
		return;
	}

	// init logger
	InitLogger();
	
	// init audio
	if (!InitOpenAL())
	{
		LogE("InitOpenAL() failed..");
		DestroyWindow(ghwnd);
		return;
	}

	// fetch OpenGL related details
	LogI("OpenGL Vendor:   %s", glGetString(GL_VENDOR));
	LogI("OpenGL Renderer: %s", glGetString(GL_RENDERER));
	LogI("OpenGL Version:  %s", glGetString(GL_VERSION));
	LogI("GLSL Version:    %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// fetch OpenGL enabled extensions
	GLint numExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

	LogD("==== OpenGL Extensions ====");
	for (int i = 0; i < numExtensions; i++)
	{
		LogD("  %s", glGetStringi(GL_EXTENSIONS, i));
	}
	LogD("===========================\n");

	// Compile all shaders
	if (!InitAllShaders())
	{
		LogE("InitAllShaders() failed..");
		DestroyWindow(ghwnd);
		return;
	}

	// load all letters
	LoadLetters();

	// initialize scene queue
	InitSceneQueue();

	// add scenes
	AddScene(GetIntroScene());
	AddScene(GetMarblesScene());

	// set clear color and clear depth
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	// depth test 
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// blend test
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// cull back face
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// textures
	glEnable(GL_TEXTURE_2D);

	// initialize all scenes
	for (int i = 0; i < GetSceneCount(); i++)
	{
		Scene scene;
		if (GetSceneAt(scene, i))
		{
			if (!scene.InitFunc())
			{
				LogE("Scene %s Init() failed..", scene.Name);
				DestroyWindow(ghwnd);
				return;
			}
			LogD("Scene %s Init() done..", scene.Name);

			// warm-up resize
			scene.ResizeFunc(WIN_WIDTH, WIN_HEIGHT);
			LogD("Scene %s Resize() done..", scene.Name);

			scene.ResetFunc();
			LogD("Scene %s Reset() done..", scene.Name);
		}
	}

	// clock for syncing animation speed
	InitClock();

	// warm-up resize call
	resize(WIN_WIDTH, WIN_HEIGHT);
}

void resize(int width, int height)
{
	// code
	if (height == 0)
		height = 1;

	gWidth  = width;
	gHeight = height;

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	// resize active scene
	Scene scene;
	if (GetScene(scene)) scene.ResizeFunc(width, height);
}

void display(void)
{
	// code

	// draw active scene
	Scene scene;
	if (GetScene(scene)) scene.DisplayFunc();

	SwapBuffers(ghdc);
}

void update(float delta)
{
	// code	

	// update active scene
	Scene scene;
	if (GetScene(scene))
	{
		if (scene.UpdateFunc(delta))
		{
			// current scene is completed
			// move to the next scene
			LogD("Scene %s Update() done..", scene.Name);
			RemoveScene();

			// if no next scene, terminate
			if (!GetScene(scene))
			{
				DestroyWindow(ghwnd);
				return;
			}

			// warm-up resize call to next scene
			scene.ResizeFunc(gWidth, gHeight);
			LogD("Scene %s Resize() done..", scene.Name);
		}
	}
}

void uninitialize(void)
{
	// code
	if (gbFullscreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
	}

	// uninit audio
	UninitOpenAL();

	// uninit all shaders
	UninitAllShaders();

	// unload all letters
	UnloadLetters();

	// uninit all scenes
	for (int i = 0; i < GetSceneCount(); i++)
	{
		Scene scene;
		if (GetSceneAt(scene, i))
		{
			LogD("Scene %s Uninit() calling..", scene.Name);
			scene.UninitFunc();
		}
	}

	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	UninitLogger();
}

