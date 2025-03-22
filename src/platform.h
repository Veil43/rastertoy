#ifndef PLATFORM_H
#define PLATFORM_H

// #define DEBUG_ON
// #define PERF_ON

#include <vector>
#include <string>

typedef uint8_t u8;
typedef u8 byte;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

/*
28 bytes
*/
struct PlatformScreenDevice
{
    void *BufferMemory;
    f32 aspectRatio;
    i32 width;
    i32 height;
    i32 pitch;
    i32 bytesPerPixel;
};

enum PlatformState
{
    WINDOW_QUIT,
    WINDOW_RUNNING
};

enum KeyCode
{
    KEY_W, KEY_F, KEY_S, KEY_D, KEY_H,
    KEY_G, KEY_Q, KEY_E, KEY_N, KEY_P,
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
    KEY_SPACE, KEY_LCTRL,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
    KEY_5, KEY_6, KEY_7, KEY_8, KEY_9
};

struct PlatformMouseDevice
{
    f32 x, y;
};

// For the application to implement -----------------------------------------------------------
namespace rastertoy
{
void UpdateRenderLoop(f32 DeltaTime); // DeltaTime in seconds
void OnLaunch(PlatformScreenDevice Screen, const std::vector<std::string>& objects);
void ProcessInput(KeyCode Key);
}

// For the platform to implement ---------------------------------------------------------------
[[nodiscard]] static PlatformScreenDevice CreatePlatformScreenDevice
(
    void * backBufferMemory, i32 width, i32 height, f32 aspectRatio, i32 bytesPerPixel
);

#endif // PLATFORM_H