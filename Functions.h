#pragma once
void			CreateGameFont(HFONT* _Font, wchar_t* font_name, int font_size);
WNDCLASSEXW		CreateWindowClass(void);
BOOL			CreateGameWindow(void);
BOOL			GameIsAlreadyRunning(void);
BOOL			InitializeBackBuffer(GAMEBITMAP* bitmap);
BOOL			SetWindowToFullscreen(HWND WindowHandle);
void			ProcessPlayerInput(void);
void			DrawDebugInfo(HDC DeviceContext);
void			RenderFrameGraphics(void);
void			InitializePlayer(PLAYER* player);
void			DrawSprite(int x, int y, int width, int height);
void			DrawSprite2(int x, int y, int width, int height);
void			ClearScreen(void);
void			ClearColor(int red, int green, int blue);
void			InitializeGameMap(void);
void			InitializeGame(void);
BOOL			SetGameToHighPriority(HANDLE ProcessHandle, HANDLE ThreadHandle);
LRESULT			CALLBACK MainWindowProc(_In_ HWND WindowHandle, _In_ unsigned int WindowMessage, _In_ WPARAM WindowParameterWide, _In_ LPARAM WindowParameter);