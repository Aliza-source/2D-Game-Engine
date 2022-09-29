#pragma once
#define CONCAT_STRINGS(stringA, strinB) (StringA stringB);
#define GAME_ROOT_FOLDER				"..\\..\\"
#define GAME_NAME						L"Game_Name"
#define GAME_WIDTH						384
#define GAME_HEIGHT						240 
#define GAME_COLOR_DEPTH				32
#define GAME_PIXEL_COUNT				GAME_WIDTH * GAME_HEIGHT
#define GAME_BACKBUFFER_SIZE			(GAME_PIXEL_COUNT * (GAME_COLOR_DEPTH / 8))
#define GAME_DEBUG_FONT_NAME			L"Arial"
#define GAME_DEBUG_FONT_SIZE			19
#define GAME_DEBUG_WINDOW_MARGIN		4
#define GAME_UI_FONT_NAME				L"Arial"
#define GAME_UI_FONT_SIZE				32
#define WINDOW_BACKGROUND_COLOR			RGB(255, 0, 255)
#define WINDOW_TITLE					L"Window Title"
#define WINDOW_CLASS					GAME_NAME L"_WindowClass"
#define DEFAULT_WINDOW_WIDTH			640
#define DEFAULT_WINDOW_HEIGHT			480
#define TARGET_FRAMES_PER_SECOND		144
#define TARGET_MICROSECONDS_PER_FRAME	(1000000 / TARGET_FRAMES_PER_SECOND)
#define GET_AVG_FPS_EVERY				(TARGET_FRAMES_PER_SECOND * 2)
#define MICROSECONDS_TO_SECONDS(microseconds) ((microseconds) * 0.000001f)
#define MILLISECONDS_TO_SECONDS(milliseconds) ((milliseconds) * 0.001f)
#define SECONDS_TO_MICROSECONDS(seconds) ((seconds) * 1000000)