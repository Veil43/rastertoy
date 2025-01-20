#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <cassert>

#include "platform.h"

// TODO: add help function
struct sdl_render_resources
{
    SDL_Window *window;
    SDL_Renderer *sdlRenderer;
    SDL_Texture *sdlTexture;
    void *sdlBufferMemory;
    unsigned int sdlPixelFromat;
    int bytesPerPixel;
    int positionX;
    int positionY;
    int windowWidth;
    int windowHeight;
    float aspectRatio;
};

static PlatformState WindowState = WINDOW_RUNNING;
static bool SDLCursorShown = true;
static const int32 WINDOW_WIDTH = 1280;
static const real32 WINDOW_ASPECT_RATIO = 16.0f / 9.0f;
static const std::string CORRECT_USAGE_STRING = "Correct Usage: ./sdl2_rastertoy.exe [list of obj files]";

static bool SDLInitializeVideo();
[[nodiscard]] static sdl_render_resources SDLCreateRenderingResourses
(
    const char *windowName, int32 positionX,int32 positionY, int32 windowWidth, real32 aspectRatio
);
static void SDLRenderBackBuffer(const sdl_render_resources& sdlResources);
static void SDLReleaseResources(sdl_render_resources& sdlResources);
static void SDLSendKeyboardState();
static std::vector<std::string> ParseCommandLineArgs(int argc, char **argv);
static void PrintCorrectUsage();

// Entry point ********************************************************************
int
main(int argc, char *argv[])
{
    if (argc < 2)
    {
        PrintCorrectUsage();
    }
    std::vector<std::string> objFiles = ParseCommandLineArgs(argc, argv);

    // Resource Acquisition -----------------------------------------------------
    bool initSuccessful = SDLInitializeVideo();
    assert(initSuccessful);
    std::string windowTitle = "Raster Toy";
    sdl_render_resources sdlRenderResources =
    SDLCreateRenderingResourses(windowTitle.c_str(), 150, 150, WINDOW_WIDTH, WINDOW_ASPECT_RATIO);
    assert(sdlRenderResources.window != nullptr);
    assert(sdlRenderResources.sdlRenderer != nullptr);
    assert(sdlRenderResources.sdlTexture != nullptr);
    assert(sdlRenderResources.sdlBufferMemory != nullptr);

    // Startup Operations -----------------------------------------------------
    PlatformScreenDevice ScreenDevice =
    CreatePlatformScreenDevice(sdlRenderResources.sdlBufferMemory,
                               sdlRenderResources.windowWidth,
                               sdlRenderResources.windowHeight,
                               sdlRenderResources.aspectRatio,
                               sdlRenderResources.bytesPerPixel);

    rastertoy::OnLaunchSetup(ScreenDevice, objFiles);

    // Main Loop ---------------------------------------------------------------
    unsigned long long lastCounter = SDL_GetPerformanceCounter();
    unsigned long long counterFrequency = SDL_GetPerformanceFrequency();
    float  deltaTime = 0;
    while(WindowState == WINDOW_RUNNING)
    {
        // Renderer update ----------------------------------------------------
        SDLSendKeyboardState();
        rastertoy::UpdateRenderLoop(deltaTime / 1000.0f);

        // SDL Event handling ------------------------------------------------
        SDL_Event SDLEvent;
        while(SDL_PollEvent(&SDLEvent))
        {
            // SDL Specific events
            if (SDLEvent.type == SDL_QUIT)
            {
                WindowState = WINDOW_QUIT;
            }
            if (SDLEvent.type == SDL_WINDOWEVENT)
            {
            }
        }

        // Buffer Presentation -----------------------------------------------
        SDLRenderBackBuffer(sdlRenderResources);

        // Timing ------------------------------------------------------------
        unsigned long long EndCounter = SDL_GetPerformanceCounter();

        unsigned long long CounterDiff = EndCounter - lastCounter;

        int  MSPerFrame = 1000 * CounterDiff / counterFrequency;
        int FPS = counterFrequency / CounterDiff;

        std::string newTitle = windowTitle + " | Time: " + std::to_string(MSPerFrame) + "ms";
        SDL_SetWindowTitle(sdlRenderResources.window, newTitle.c_str());
#if defined(DEBUG_ON) && defined(PERF_ON)
        std::cout << "Frame Time: " << MSPerFrame << "ms | FPS: " << FPS << "\n";
#endif
        lastCounter = EndCounter;
        deltaTime = MSPerFrame;
    }

    // Resource Release -----------------------------------------------------
    SDLReleaseResources(sdlRenderResources);
    SDL_Quit();

    return 0;
}

// SDL-Specific functions --------------------------------------------------

static bool
SDLInitializeVideo()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "[ERROR]: SDL Could Not Initialize Video -- SDLERROR::" << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

[[nodiscard]] static sdl_render_resources
SDLCreateRenderingResourses(const char *windowName,
                int positionX,int positionY,
                int windowWidth, float aspectRatio)
{
    int windowHeight = ((int) windowWidth / aspectRatio);
    SDL_Window *sdlWindow =
    SDL_CreateWindow(windowName,
                    positionX, positionY,
                    windowWidth, windowHeight,
                    SDL_WINDOW_MAXIMIZED);

    if (!sdlWindow)
    {
        std::cerr << "[ERROR]: SDL Could Not Create a Window -- SDLERROR::" << SDL_GetError() << std::endl;
        SDL_DestroyWindow(sdlWindow);
        return {nullptr, nullptr, nullptr, nullptr};
    }

    SDL_Renderer *sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_SOFTWARE);
    if (!sdlRenderer)
    {
        std::cerr << "[ERROR]: SDL Could Not Create a Renderer -- SDLERROR::" << SDL_GetError() << std::endl;
        SDL_DestroyWindow(sdlWindow);
        SDL_DestroyRenderer(sdlRenderer);
        return {nullptr, nullptr, nullptr, nullptr};
    }

    int bytesPerPixel = sizeof(unsigned int);
    void *bufferMemory = operator new(windowWidth * windowHeight * bytesPerPixel);
    unsigned int sdlPixelFromat = SDL_PIXELFORMAT_RGBA8888;

    if (!bufferMemory)
    {
        std::cerr << "[ERROR]: Failed To Allocate Back Buffer Memory" << std::endl;
        SDL_DestroyWindow(sdlWindow);
        SDL_DestroyRenderer(sdlRenderer);
        operator delete(bufferMemory);
        return {nullptr, nullptr, nullptr, nullptr};
    }

    SDL_Texture *sdlTexture =
    SDL_CreateTexture(sdlRenderer,
                      sdlPixelFromat,
                      SDL_TEXTUREACCESS_STREAMING,
                      windowWidth, windowHeight);

    if (!sdlTexture)
    {
        std::cerr << "[ERROR]: SDL Failed to Create Texture -- SDLERROR::" << SDL_GetError() << std::endl;
        SDL_DestroyWindow(sdlWindow);
        SDL_DestroyRenderer(sdlRenderer);
        operator delete(bufferMemory);
        SDL_DestroyTexture(sdlTexture);
        return {nullptr, nullptr, nullptr};
    }

    return
    {
        sdlWindow,
        sdlRenderer,
        sdlTexture,
        bufferMemory,
        sdlPixelFromat,
        bytesPerPixel,
        positionX,
        positionY,
        windowWidth,
        windowHeight,
        aspectRatio
    };
}

[[nodiscard]] static PlatformScreenDevice
CreatePlatformScreenDevice(void * backBufferMemory,
                     int32 width, int32 height,
                     real32 aspectRatio, int32 bytesPerPixel)
{
    return
    {
        backBufferMemory,
        aspectRatio,
        width,
        height,
        width * bytesPerPixel,
        bytesPerPixel
    };
}

static void
SDLRenderBackBuffer(const sdl_render_resources& sdlResources)
{
    SDL_SetRenderDrawColor(sdlResources.sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlResources.sdlRenderer);

    SDL_UpdateTexture(sdlResources.sdlTexture, nullptr, sdlResources.sdlBufferMemory, sdlResources.windowWidth * sdlResources.bytesPerPixel);

    SDL_RenderCopy(sdlResources.sdlRenderer, sdlResources.sdlTexture, nullptr, nullptr);

    SDL_RenderPresent(sdlResources.sdlRenderer);
}

static void
SDLReleaseResources(sdl_render_resources& sdlResources)
{
    SDL_DestroyWindow(sdlResources.window);
    SDL_DestroyRenderer(sdlResources.sdlRenderer);
    operator delete(sdlResources.sdlBufferMemory);
    SDL_DestroyTexture(sdlResources.sdlTexture);
}

// Platform API compliance functions -----------------------------------------

// NOTE(Reuel): What we've learned here going from a single event
// multiple event handling.
static
void SDLSendKeyboardState()
{
    const Uint8 *keyState = SDL_GetKeyboardState(NULL);

    if (keyState[SDL_SCANCODE_ESCAPE])
    {
        WindowState = WINDOW_QUIT;
    }

    if (keyState[SDL_SCANCODE_W])
    {
        rastertoy::ProcessInput(KEY_W);
    }

    if (keyState[SDL_SCANCODE_G])
    {
        rastertoy::ProcessInput(KEY_G);
    }

    if (keyState[SDL_SCANCODE_F])
    {
        rastertoy::ProcessInput(KEY_F);
    }

    if (keyState[SDL_SCANCODE_S])
    {
        rastertoy::ProcessInput(KEY_S);
    }

    if (keyState[SDL_SCANCODE_D])
    {
        rastertoy::ProcessInput(KEY_D);
    }

    if (keyState[SDL_SCANCODE_Q])
    {
        rastertoy::ProcessInput(KEY_Q);
    }

    if (keyState[SDL_SCANCODE_E])
    {
        rastertoy::ProcessInput(KEY_E);
    }

    if (keyState[SDL_SCANCODE_N])
    {
        rastertoy::ProcessInput(KEY_N);
    }

    if (keyState[SDL_SCANCODE_H])
    {
        rastertoy::ProcessInput(KEY_H);
    }

    if (keyState[SDL_SCANCODE_P])
    {
        rastertoy::ProcessInput(KEY_P);
    }

    if (keyState[SDL_SCANCODE_UP])
    {
        rastertoy::ProcessInput(KEY_UP);
    }

    if (keyState[SDL_SCANCODE_DOWN])
    {
        rastertoy::ProcessInput(KEY_DOWN);
    }

    if (keyState[SDL_SCANCODE_LEFT])
    {
        rastertoy::ProcessInput(KEY_LEFT);
    }

    if (keyState[SDL_SCANCODE_RIGHT])
    {
        rastertoy::ProcessInput(KEY_RIGHT);
    }

    if (keyState[SDL_SCANCODE_SPACE])
    {
        rastertoy::ProcessInput(KEY_SPACE);
    }

    if (keyState[SDL_SCANCODE_LCTRL])
    {
        rastertoy::ProcessInput(KEY_LCTRL);
    }

    if (keyState[SDL_SCANCODE_0])
    {
        rastertoy::ProcessInput(KEY_0);
    }

    if (keyState[SDL_SCANCODE_1])
    {
        rastertoy::ProcessInput(KEY_1);
    }

    if (keyState[SDL_SCANCODE_2])
    {
        rastertoy::ProcessInput(KEY_2);
    }

    if (keyState[SDL_SCANCODE_3])
    {
        rastertoy::ProcessInput(KEY_3);
    }

    if (keyState[SDL_SCANCODE_4])
    {
        rastertoy::ProcessInput(KEY_4);
    }

    if (keyState[SDL_SCANCODE_5])
    {
        rastertoy::ProcessInput(KEY_5);
    }

    if (keyState[SDL_SCANCODE_6])
    {
        rastertoy::ProcessInput(KEY_6);
    }

    if (keyState[SDL_SCANCODE_7])
    {
        rastertoy::ProcessInput(KEY_7);
    }

    if (keyState[SDL_SCANCODE_8])
    {
        rastertoy::ProcessInput(KEY_8);
    }

    if (keyState[SDL_SCANCODE_9])
    {
        rastertoy::ProcessInput(KEY_9);
    }
}

static std::vector<std::string> ParseCommandLineArgs(int argc, char **argv)
{
    std::vector<std::string> strings{};
    for (size_t i = 1; i < argc; ++i)
    {
        strings.push_back(argv[i]);
    }
    return strings;
}

static void PrintCorrectUsage()
{
    std::cerr << CORRECT_USAGE_STRING << std::endl;
}
