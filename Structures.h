typedef struct PLAYER_SIZE {
	int Width;
	int Height;
} PLAYER_SIZE;

typedef struct PLAYER_POSITION {
	float X;
	float Y;
} PLAYER_POSITION;

typedef struct PLAYER_STATS {
	float MovementSpeed;
	int Health;
	int Strength;
	int MagicPoints;
} PLAYER_STATS;

typedef struct PLAYER {
	char Name[12];
	PLAYER_POSITION ScreenPosition;
	PLAYER_POSITION WorldPosition;
	PLAYER_SIZE Size;
	PLAYER_STATS Stats;
} PLAYER;

typedef struct GAMEBITMAP {
	BITMAPINFO Bitmap;
	void* Memory;
} GAMEBITMAP;

typedef struct PIXEL32 {
	BYTE Blue;
	BYTE Green;
	BYTE Red;
	BYTE Alpha;
} PIXEL32;

typedef struct WINDOW_STATISTICS {
	int Width;
	int Height;
	BOOL HasFocus;
} WINDOW;

typedef struct RENDER_STATISTICS {
	uint64_t VariableFrameCount;
	uint64_t FrameCount;
	int VFR;
	int FPS;
} RENDER;

typedef struct TIME_STATISTICS {
	int64_t CurrentSystemTime;
	int64_t PreviousSystemTime;
	float DeltaTime;
	float DeltaTimeRaw;
} TIME;


typedef struct CPU_STATISTICS {
	float Usage;
	int CoreCount;
} CPU;

typedef struct TIMER_INFO {
	ULONG MinimumResolution;
	ULONG MaximumResolution;
	ULONG CurrentResolution;
} TIMER_INFO;

typedef struct DEBUG_STATISTICS {
	int64_t PerformanceFrequency;
	BOOL DisplayInfo;
	DWORD HandleCount;
	PROCESS_MEMORY_COUNTERS_EX MemoryInfo;
} DEBUG;

typedef struct GAME_STATISTICS {
	MONITORINFO Monitor;
	SYSTEM_INFO System;
	WINDOW Window;
	RENDER Render;
	TIME Time;
	CPU CPU;
	TIMER_INFO Timer;
	DEBUG Debug;
} STATISTICS;

typedef struct GAME_FONTS {
	HFONT Debug;
	HFONT UI;
} GAME_FONTS;

typedef struct GRAPHICS {
	__m128i ClearColor128;
	GAMEBITMAP BackBuffer;
	PIXEL32 ClearColor;
} GRAPHICS;

typedef struct GAME_BOUNDS {
	int Left;
	int Right;
	int Top;
	int Bottom;
	int Width;
	int Height;
} GAME_BOUNDS;

typedef struct GAME_MAP {
	GAME_BOUNDS Bounds;
}GAME_MAP;

typedef LONG(NTAPI* _NtQueryTimerResolution)(
	OUT PULONG MinimumResolution,
	OUT PULONG MaximumResolution,
	OUT PULONG CurrentResolution
);

_NtQueryTimerResolution NtQueryTimerResolution;
