#pragma once

// #define DEBUG_ON
// #define PERF_ON

#include <vector>
#include <string>

typedef uint8_t uint8;
typedef uint8 byte;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef float real32;
typedef double real64;

/*
28 bytes
*/
struct PlatformScreenDevice
{
    void *BufferMemory;
    real32 aspectRatio;
    int32 width;
    int32 height;
    int32 pitch;
    int32 bytesPerPixel;
};

enum PlatformState
{
    WINDOW_QUIT,
    WINDOW_RUNNING
};

enum KeyCode
{
    KEY_W, KEY_F, KEY_S, KEY_D,
    KEY_G, KEY_Q, KEY_E, KEY_N, KEY_P,
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_SPACE, KEY_LCTRL,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
    KEY_5, KEY_6, KEY_7, KEY_8, KEY_9
};

struct PlatformMouseDevice
{
    real32 x, y;
};

// For the application to implement -----------------------------------------------------------
namespace rastertoy
{
void UpdateRenderLoop(real32 DeltaTime); // DeltaTime in seconds
void OnLaunchSetup(PlatformScreenDevice Screen, const std::vector<std::string>& objects);
void ProcessInput(KeyCode Key);
}

// For the platform to implement ---------------------------------------------------------------
[[nodiscard]] static PlatformScreenDevice CreatePlatformScreenDevice
(
    void * backBufferMemory, int32 width, int32 height, real32 aspectRatio, int32 bytesPerPixel
);
