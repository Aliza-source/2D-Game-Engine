#pragma comment(lib, "winmm.lib")
#pragma warning(push, 3)
#include <stdio.h>
#include <windows.h>
#include <stdint.h>
#include <emmintrin.h>
#include <Psapi.h>
#pragma warning(pop)
#include "Main.h"



static BOOL gGameIsRunning;
static HWND gWindowHandle;
static STATISTICS gStatistics = { {sizeof(MONITORINFO)}, {DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT}, {0}, {0}, FALSE };
static GAME_FONTS gGameFonts = { 0 };
static GRAPHICS gGraphics = { 0 };
static PLAYER gPlayer = { 0 };
static GAME_MAP gMap = { 0 };


int WINAPI WinMain(_In_ HINSTANCE WindowInstance, _In_opt_ HINSTANCE PreviousInstance, _In_ PSTR CommandLine, _In_ int CommandShow) {
	UNREFERENCED_PARAMETER(WindowInstance);
	UNREFERENCED_PARAMETER(PreviousInstance);
	UNREFERENCED_PARAMETER(CommandLine);
	UNREFERENCED_PARAMETER(CommandShow);

	gStatistics.Debug.DisplayInfo = TRUE;
	CreateGameFont(&gGameFonts.Debug, (wchar_t*)(GAME_DEBUG_FONT_NAME), GAME_DEBUG_FONT_SIZE);

	SystemTimer Timer;
	MSG WindowMessage = { 0 };

	const HANDLE ProcessHandle = GetCurrentProcess();
	const HANDLE ThreadHandle = GetCurrentThread();

	int64_t ProcessCreationTime = 0;
	int64_t ProcessExitTime = 0;
	int64_t CurrentKernelTime = 0;
	int64_t PreviousKernelTime = 0;
	int64_t CurrentUserCPUTime = 0;
	int64_t PreviousUserCPUTime = 0;

	struct FRAME {
		int64_t Count;
		int64_t Start;
		int64_t End;
		int64_t ElapsedMicroseconds;
		int64_t DeltaTimeUnscaled;
		int64_t DeltaTime;
	} Frame = { 0 };

	GetSystemInfo(&gStatistics.System);

	GetSystemTimeAsFileTime(
		(LPFILETIME)&gStatistics.Time.PreviousSystemTime
	);
	if (GameIsAlreadyRunning()) {
		goto Exit;
	}
	if (!Timer.Initialize()) {
		goto Exit;
	}
	if (!Timer.SetResolution(1)) {
		goto Exit;
	}
	if (!SetGameToHighPriority(ProcessHandle, ThreadHandle)) {
		goto Exit;
	}
	if (!CreateGameWindow()) {
		goto Exit;
	}
	if (!SetWindowToFullscreen(gWindowHandle)) {
		goto Exit;
	};
	if (!InitializeBackBuffer(&gGraphics.BackBuffer)) {
		goto Exit;
	}

	Timer.GetResolution (
		&gStatistics.Timer.MinimumResolution,
		&gStatistics.Timer.MaximumResolution,
		&gStatistics.Timer.CurrentResolution
	);

	QueryPerformanceFrequency (
		(LARGE_INTEGER*)&gStatistics.Debug.PerformanceFrequency
	);

	if (!InitializeGame()) {
		goto Exit;
	}

	gGameIsRunning = TRUE;

	while (gGameIsRunning) {
		while (PeekMessageW(&WindowMessage, gWindowHandle, 0, 0, PM_REMOVE)) {
			DispatchMessageW(&WindowMessage);
		}
		QueryPerformanceCounter((LARGE_INTEGER*)&Frame.Start);

		ProcessPlayerInput();

		RenderFrameGraphics();

		QueryPerformanceCounter((LARGE_INTEGER*)&Frame.End);

		Frame.ElapsedMicroseconds =	 SECONDS_TO_MICROSECONDS(Frame.End - Frame.Start) / gStatistics.Debug.PerformanceFrequency;
		Frame.DeltaTimeUnscaled =	 Frame.DeltaTimeUnscaled + Frame.ElapsedMicroseconds;
		Frame.Count = 				 Frame.Count + 1;

		while (Frame.ElapsedMicroseconds < TARGET_MICROSECONDS_PER_FRAME) {
			Frame.ElapsedMicroseconds = SECONDS_TO_MICROSECONDS(Frame.End - Frame.Start) / gStatistics.Debug.PerformanceFrequency;
			QueryPerformanceCounter((LARGE_INTEGER*)&Frame.End);

			if (Frame.ElapsedMicroseconds < (TARGET_MICROSECONDS_PER_FRAME - (gStatistics.Timer.CurrentResolution * 0.4))) {
				Sleep(1);
			}
			else {
				Sleep(0);
			}
		}

		Frame.DeltaTime += Frame.ElapsedMicroseconds;

		if ((Frame.Count % GET_AVG_FPS_EVERY) == 0) {
			GetSystemTimeAsFileTime(
				(LPFILETIME)&gStatistics.Time.CurrentSystemTime
			);

			GetProcessTimes(
				ProcessHandle,
				(LPFILETIME)&ProcessCreationTime,
				(LPFILETIME)&ProcessExitTime,
				(LPFILETIME)&CurrentKernelTime,
				(LPFILETIME)&CurrentUserCPUTime
			);

			gStatistics.CPU.Usage =
				(float)((CurrentKernelTime - PreviousKernelTime)
					+ (CurrentUserCPUTime - PreviousUserCPUTime))
				/ (gStatistics.Time.CurrentSystemTime - gStatistics.Time.PreviousSystemTime)
				/ gStatistics.System.dwNumberOfProcessors
				* 100;

			GetProcessHandleCount(
				ProcessHandle,
				&gStatistics.Debug.HandleCount
			);

			K32GetProcessMemoryInfo(
				ProcessHandle,
				(PROCESS_MEMORY_COUNTERS*)&gStatistics.Debug.MemoryInfo,
				sizeof(gStatistics.Debug.MemoryInfo)
			);

			gStatistics.Time.DeltaTime =  (float) MICROSECONDS_TO_SECONDS(Frame.DeltaTime / GET_AVG_FPS_EVERY);
			gStatistics.Render.FPS = (int)(1.0f / MICROSECONDS_TO_SECONDS(Frame.DeltaTime / GET_AVG_FPS_EVERY));
			gStatistics.Render.VFR = (int)(1.0f / MICROSECONDS_TO_SECONDS(Frame.DeltaTimeUnscaled / GET_AVG_FPS_EVERY));
			gStatistics.Render.VariableFrameCount = Frame.Count;
			gStatistics.Time.PreviousSystemTime = gStatistics.Time.CurrentSystemTime;

			PreviousKernelTime = CurrentKernelTime;
			PreviousUserCPUTime = CurrentUserCPUTime;
			Frame.DeltaTime = 0;
			Frame.DeltaTimeUnscaled = 0;
		}
	}
	Exit: {
		return 0;
	}
}


LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ unsigned int WindowMessage, _In_ WPARAM WindowParameterWide, _In_ LPARAM WindowParameter) {
	LRESULT Result = 0;
	switch (WindowMessage) {
	case WM_CLOSE: {
		gGameIsRunning = FALSE;
		PostQuitMessage(0);
		break;
	}
	case WM_ACTIVATE: {
		if (WindowParameterWide == 0) {
			gStatistics.Window.HasFocus = FALSE;
		}
		else {
			gStatistics.Window.HasFocus = TRUE;
		}
		ShowCursor(FALSE);

		break;
	}
	default: {
		Result = DefWindowProcA(WindowHandle, WindowMessage, WindowParameterWide, WindowParameter);
		break;
	}
	}
	return Result;
}

void CreateGameFont(HFONT* _Font, wchar_t *font_name, int font_size) {
	*_Font = CreateFontW(
		font_size, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, font_name
	);
	if (*_Font == NULL) {
		*_Font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	}
}
WNDCLASSEXW CreateWindowClass(void) {
	WNDCLASSEXW WindowClass = { 0 };
	WindowClass.cbSize = sizeof(WNDCLASSEXW);
	WindowClass.style = 0;
	WindowClass.lpfnWndProc = MainWindowProc;
	WindowClass.cbClsExtra = 0;
	WindowClass.hInstance = GetModuleHandleA(0);
	WindowClass.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	WindowClass.hIconSm = LoadIconW(NULL, IDI_APPLICATION);
	WindowClass.hCursor = LoadIconW(NULL, IDC_ARROW);
	WindowClass.hbrBackground = CreateSolidBrush(WINDOW_BACKGROUND_COLOR);
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = WINDOW_CLASS;
	WindowClass.hIconSm = LoadIconW(NULL, IDI_APPLICATION);
	return WindowClass;
}

void DisplayLastError(const wchar_t* Message) {
	int ErrorCode = (int)GetLastError();
	wchar_t ErrorHeader[10];
	swprintf_s(ErrorHeader, 10, L"Error %i", ErrorCode);
	wchar_t ErrorMessage[256];
	swprintf_s(ErrorHeader, 256, L"%s\n /s", Message, SystemErrors[ErrorCode]);
	MessageBoxW(NULL, ErrorMessage, ErrorHeader, MB_ICONEXCLAMATION | MB_OK);
}

BOOL SetGameToHighPriority(HANDLE ProcessHandle, HANDLE ThreadHandle) {
	if (SetPriorityClass(ProcessHandle, HIGH_PRIORITY_CLASS) == 0) {
		MessageBoxW(NULL, L"Failed to set the process priority!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	if (SetThreadPriority(ThreadHandle, THREAD_PRIORITY_HIGHEST) == 0) {
		MessageBoxW(NULL, L"Failed to set the thread priority!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	return TRUE;
}

BOOL Load32BitmapFromFile(_In_ const char* Filename, _Inout_ GAMEBITMAP* Bitmap) {
	return TRUE;
}


BOOL CreateGameWindow(void) {

	WNDCLASSEXW WindowClass = CreateWindowClass();

	if (RegisterClassExW(&WindowClass) == 0) {
		DisplayLastError();
		return FALSE;
	}

	gWindowHandle = CreateWindowExW(
		0, WINDOW_CLASS, WINDOW_TITLE,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		gStatistics.Window.Width,
		gStatistics.Window.Height,
		NULL, NULL, GetModuleHandleA(0), NULL
	);

	if (gWindowHandle == NULL) {
		DisplayLastError();
		return FALSE;
	}

	

	return TRUE;
}


BOOL InitializeBackBuffer(GAMEBITMAP* bitmap) {
	bitmap->Bitmap.bmiHeader.biSize = sizeof(gGraphics.BackBuffer.Bitmap.bmiHeader);
	bitmap->Bitmap.bmiHeader.biWidth = GAME_WIDTH;
	bitmap->Bitmap.bmiHeader.biHeight = GAME_HEIGHT;
	bitmap->Bitmap.bmiHeader.biBitCount = GAME_COLOR_DEPTH;
	bitmap->Bitmap.bmiHeader.biCompression = BI_RGB;
	bitmap->Bitmap.bmiHeader.biPlanes = 1;
	bitmap->Memory = VirtualAlloc(NULL, GAME_BACKBUFFER_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (bitmap->Memory == NULL) {
		MessageBoxW(NULL, L"Failed to Alocate memory for backbuffer!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}
	return TRUE;
}
BOOL GameIsAlreadyRunning(void) {
	HANDLE Mutex = NULL;
	Mutex = CreateMutexW(NULL, FALSE, GAME_NAME "_GameMutex");

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBoxW(NULL, L"Other instance of this program is already running!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return TRUE;
	}
	else {
		return FALSE;
	}
}
BOOL SetWindowToFullscreen(HWND WindowHandle) {
	if (GetMonitorInfoW(MonitorFromWindow(WindowHandle, MONITOR_DEFAULTTOPRIMARY), &gStatistics.Monitor) == 0) {
		MessageBoxW(NULL, L"Couldnt retrive monitor info, running in default resolution", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	};

	if (SetWindowLongPtrW(WindowHandle, GWL_STYLE, WS_VISIBLE) == 0) {
		MessageBoxW(NULL, L"Unable to set window style to borderless", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	int MonitorWidth = gStatistics.Monitor.rcMonitor.right - gStatistics.Monitor.rcMonitor.left,
		MonitorHeight = gStatistics.Monitor.rcMonitor.bottom - gStatistics.Monitor.rcMonitor.top;

	if (SetWindowPos(WindowHandle, HWND_TOP, gStatistics.Monitor.rcMonitor.left, gStatistics.Monitor.rcMonitor.top, MonitorWidth, MonitorHeight, SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0) {
		MessageBoxW(NULL, L"Unable to set window position or size", L"Error", MB_ICONEXCLAMATION | MB_OK);
		return FALSE;
	}

	gStatistics.Window.Width = MonitorWidth;
	gStatistics.Window.Height = MonitorHeight;

	return TRUE;
}

BOOL InitializeGame(void) {
	ClearColor(128, 50, 0);

	if (!InitializeGameMap()) {
		DisplayLastError();
		return FALSE;
	}
	if (!InitializePlayer(&gPlayer)) {
		DisplayLastError();
		return FALSE;
	}
	return TRUE;
}

BOOL InitializeGameMap(void) {
	gMap.Bounds.Bottom = 0;
	gMap.Bounds.Top = GAME_HEIGHT;
	gMap.Bounds.Left = 0;
	gMap.Bounds.Right = GAME_WIDTH;
	return TRUE;
}

BOOL InitializePlayer(PLAYER* player) {
	player->Stats.MovementSpeed = 120.0f;
	player->ScreenPosition.X = 25;
	player->ScreenPosition.Y = 25;
	player->Size.Width = 35;
	player->Size.Height = 35;
	Load32BitmapFromFile(
		"..\\..\\Assets\\Hero_Suit0_Down_Standing.bmpx",
		&player->Sprites.GameStart[FACING_DOWN_0]
	);
	return TRUE;
}

void ProcessPlayerInput(void) {
	if (gStatistics.Window.HasFocus == FALSE) {
		return;
	}

	static SHORT DebugKeyWasDown;

	SHORT EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);
	SHORT DebugKeyIsDown = GetAsyncKeyState(VK_F3);
	SHORT LeftKeyIsDown = GetAsyncKeyState(VK_LEFT) | GetAsyncKeyState('A');
	SHORT RightKeyIsDown = GetAsyncKeyState(VK_RIGHT) | GetAsyncKeyState('D');
	SHORT UpKeyIsDown = GetAsyncKeyState(VK_UP) | GetAsyncKeyState('W');
	SHORT DownKeyIsDown = GetAsyncKeyState(VK_DOWN) | GetAsyncKeyState('S');

	if (DebugKeyIsDown && !DebugKeyWasDown) {
		gStatistics.Debug.DisplayInfo = !gStatistics.Debug.DisplayInfo;
	}

	if (EscapeKeyIsDown) {
		SendMessageA(gWindowHandle, WM_CLOSE, 0, 0);
	}

	if (LeftKeyIsDown) {
		float positionX = gPlayer.ScreenPosition.X - (gPlayer.Stats.MovementSpeed * gStatistics.Time.DeltaTime);

		if (positionX > gMap.Bounds.Left) {
			gPlayer.ScreenPosition.X = positionX;
		}
		else {
			gPlayer.ScreenPosition.X = (float)gMap.Bounds.Left;
		}
	}

	if (RightKeyIsDown) {
		float positionX = gPlayer.ScreenPosition.X + (gPlayer.Stats.MovementSpeed * gStatistics.Time.DeltaTime);

		if (positionX + gPlayer.Size.Width < gMap.Bounds.Right) {
			gPlayer.ScreenPosition.X = positionX;
		}
		else {
			gPlayer.ScreenPosition.X = (float)gMap.Bounds.Right - gPlayer.Size.Width;
		}
	}

	if (UpKeyIsDown) {
		float positionY = gPlayer.ScreenPosition.Y + (gPlayer.Stats.MovementSpeed * gStatistics.Time.DeltaTime);

		if ((positionY + gPlayer.Size.Height) < gMap.Bounds.Top) {
			gPlayer.ScreenPosition.Y = positionY;
		}
		else {
			gPlayer.ScreenPosition.Y = (float)gMap.Bounds.Top - gPlayer.Size.Height;
		}
	}

	if (DownKeyIsDown) {
		float positionY = gPlayer.ScreenPosition.Y - (gPlayer.Stats.MovementSpeed * gStatistics.Time.DeltaTime);

		if (positionY > gMap.Bounds.Bottom) {
			gPlayer.ScreenPosition.Y = positionY;
		}
		else {
			gPlayer.ScreenPosition.Y = (float)gMap.Bounds.Bottom;
		}
	}

	DebugKeyWasDown = DebugKeyIsDown;
}

void RenderFrameGraphics(void) {
	HDC DeviceContext = GetDC(gWindowHandle);
	ClearScreen();
	DrawSprite((int)gPlayer.ScreenPosition.X, (int)gPlayer.ScreenPosition.Y, gPlayer.Size.Width, gPlayer.Size.Height);
	StretchDIBits(DeviceContext, 0, 0, gStatistics.Window.Width, gStatistics.Window.Height, 0, 0, GAME_WIDTH, GAME_HEIGHT, gGraphics.BackBuffer.Memory, &gGraphics.BackBuffer.Bitmap, DIB_RGB_COLORS, SRCCOPY);

	if (gStatistics.Debug.DisplayInfo) {
		DrawDebugInfo(DeviceContext);
	}

	ReleaseDC(gWindowHandle, DeviceContext);
}


void DrawSprite2(int x, int y, int width, int height) {
	uintptr_t StartingPixel = (((uintptr_t)GAME_WIDTH * (uintptr_t)GAME_HEIGHT) - GAME_WIDTH) - ((uintptr_t)GAME_WIDTH * y) + x;
	for (int pixelX = 0; pixelX < width; pixelX++) {
		for (int pixelY = 0; pixelY < height; pixelY++) {
			memset(
				(PIXEL32*)gGraphics.BackBuffer.Memory + StartingPixel + pixelX - ((uintptr_t)GAME_WIDTH * pixelY), 255, sizeof(PIXEL32)
			);
		}
	}
}

void DrawSprite(int x, int y, int width, int height) {
	for (int pixelX = 0; pixelX < width; pixelX++) {
		for (int pixelY = 1; pixelY < height + 1; pixelY++) {
			memset(
				(PIXEL32*)gGraphics.BackBuffer.Memory + ((uintptr_t)GAME_WIDTH * ((uintptr_t)y + height)) + x + pixelX - ((uintptr_t)GAME_WIDTH * pixelY), 255, sizeof(PIXEL32)
			);
		}
	}
}

void DrawDebugInfo(HDC DeviceContext) {
	SelectObject(DeviceContext, gGameFonts.Debug);
	wchar_t DebugStringBuffer[64] = { 0 };

	swprintf_s(DebugStringBuffer, 64, L"Raw FPS: %i", gStatistics.Render.VFR);
	TextOutW(DeviceContext, GAME_DEBUG_WINDOW_MARGIN, GAME_DEBUG_WINDOW_MARGIN, DebugStringBuffer, (int)wcslen(DebugStringBuffer));
	swprintf_s(DebugStringBuffer, 64, L"FPS: %i", gStatistics.Render.FPS);
	TextOutW(DeviceContext, GAME_DEBUG_WINDOW_MARGIN, GAME_DEBUG_FONT_SIZE + GAME_DEBUG_WINDOW_MARGIN, DebugStringBuffer, (int)wcslen(DebugStringBuffer));
	swprintf_s(DebugStringBuffer, 64, L"Delta Time: %f", gStatistics.Time.DeltaTime);
	TextOutW(DeviceContext, GAME_DEBUG_WINDOW_MARGIN, GAME_DEBUG_FONT_SIZE * 2 + GAME_DEBUG_WINDOW_MARGIN, DebugStringBuffer, (int)wcslen(DebugStringBuffer));
	swprintf_s(DebugStringBuffer, 64, L"Handle Count: %lu", gStatistics.Debug.HandleCount);
	TextOutW(DeviceContext, GAME_DEBUG_WINDOW_MARGIN, GAME_DEBUG_FONT_SIZE * 3 + GAME_DEBUG_WINDOW_MARGIN, DebugStringBuffer, (int)wcslen(DebugStringBuffer));
	swprintf_s(DebugStringBuffer, 64, L"Current Timer Resolution: %f ms", (float)gStatistics.Timer.CurrentResolution / 10000);
	TextOutW(DeviceContext, GAME_DEBUG_WINDOW_MARGIN, GAME_DEBUG_FONT_SIZE * 4 + GAME_DEBUG_WINDOW_MARGIN, DebugStringBuffer, (int)wcslen(DebugStringBuffer));
	swprintf_s(DebugStringBuffer, 64, L"Maximum Timer Resolution: %f ms", (float)gStatistics.Timer.MaximumResolution / 10000);
	TextOutW(DeviceContext, GAME_DEBUG_WINDOW_MARGIN, GAME_DEBUG_FONT_SIZE * 5 + GAME_DEBUG_WINDOW_MARGIN, DebugStringBuffer, (int)wcslen(DebugStringBuffer));
	swprintf_s(DebugStringBuffer, 64, L"Minimum Timer Resolution: %f ms", (float)gStatistics.Timer.MinimumResolution / 10000);
	TextOutW(DeviceContext, GAME_DEBUG_WINDOW_MARGIN, GAME_DEBUG_FONT_SIZE * 6 + GAME_DEBUG_WINDOW_MARGIN, DebugStringBuffer, (int)wcslen(DebugStringBuffer));
	swprintf_s(DebugStringBuffer, 64, L"Memory Usage: %i KB", (int)gStatistics.Debug.MemoryInfo.PrivateUsage / 1024);
	TextOutW(DeviceContext, GAME_DEBUG_WINDOW_MARGIN, GAME_DEBUG_FONT_SIZE * 7 + GAME_DEBUG_WINDOW_MARGIN, DebugStringBuffer, (int)wcslen(DebugStringBuffer));
	swprintf_s(DebugStringBuffer, 64, L"CPU Usage: %f %%", (float)gStatistics.CPU.Usage);
	
	TextOutW(DeviceContext, GAME_DEBUG_WINDOW_MARGIN, GAME_DEBUG_FONT_SIZE * 8 + GAME_DEBUG_WINDOW_MARGIN, DebugStringBuffer, (int)wcslen(DebugStringBuffer));
}

#ifdef SIMD
void ClearColor( int red,  int green,  int blue) {
	gGraphics.ClearColor128.m128i_i8[0] = (char)blue;
	gGraphics.ClearColor128.m128i_i8[1] = (char)green;
	gGraphics.ClearColor128.m128i_i8[2] = (char)red;
	gGraphics.ClearColor128.m128i_i8[3] = (char)255;

	gGraphics.ClearColor128.m128i_i8[4] = (char)blue;
	gGraphics.ClearColor128.m128i_i8[5] = (char)green;
	gGraphics.ClearColor128.m128i_i8[6] = (char)red;
	gGraphics.ClearColor128.m128i_i8[7] = (char)255;

	gGraphics.ClearColor128.m128i_i8[8] = (char)blue;
	gGraphics.ClearColor128.m128i_i8[9] = (char)green;
	gGraphics.ClearColor128.m128i_i8[10] = (char)red;
	gGraphics.ClearColor128.m128i_i8[11] = (char)255;

	gGraphics.ClearColor128.m128i_i8[12] = (char)blue;
	gGraphics.ClearColor128.m128i_i8[13] = (char)green;
	gGraphics.ClearColor128.m128i_i8[14] = (char)red;
	gGraphics.ClearColor128.m128i_i8[15] = (char)255;
}

__forceinline void ClearScreen(void) {
	for (int i = 0, len = GAME_PIXEL_COUNT / 4; i < len; i++) {
		_mm_store_si128((__m128i*)gGraphics.BackBuffer.Memory + i, gGraphics.ClearColor128);
	}
}

#else
void ClearColor(int red, int green, int blue) {
	gGraphics.ClearColor = (PIXEL32){
		blue, green, red, 255,
	};
}
__forceinline void ClearScreen(void) {
	for (int i = 0; i < GAME_PIXEL_COUNT; i++) {
		memcpy((PIXEL32*)gGraphics.BackBuffer.Memory + i, &gGraphics.ClearColor, sizeof(PIXEL32));
	}
}
#endif

