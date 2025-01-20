# Rastertoy
`Rastertoy` is a software rasteriser I built as a learning exercise. It features:
* Wireframe and Solid rendering mode
* OBJ file loading
* Three shading modes: Flat, Gouraud and Phong

## Building guide
**Note:** This program uses SDL2 for windowing to make cross platform compilation as seamless as possible. However, the program is only tested and provides .lib files for `SDL2 version 2.30.9-VC`. If you want to compile on linux you will need to install the library independently.

**Windows (recommended):**

1. Install the [Visual Studio SDK](https://learn.microsoft.com/en-us/visualstudio/extensibility/installing-the-visual-studio-sdk?view=vs-2022).
2. Compile the porgram by running the following commands:
* `.\build.bat release` for the release build.
* `.\build.bat debug` for the debug build.

The resulting executables will be placed in the `.\release\` or `.\debug\` folder.

**Troubleshooting:** <br>
If you encounter issues with `cl` (the MSVC compiler), you may need to enable the Microsoft C++ toolset from the command line. One simple trick is to use the **Developer Command Prompt for VS**. <br>
For further instructions, refer to [Microsoft's official documentation](https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170).

**Linux (Ubuntu/Debian):** 

1. Install SDL2 by running:<br>
```bash
sudo apt-get install libsdl2-dev
```
2. Compile the program by running the following commands:
* `./build.bat release` for the release build.
* `./build.bat debug` for the debug build.

The resulting executables will be placed in the `./release` or `./debug` folder.

**Note:** This program was written on windows and was only tested minimally on Linux(Ubuntu) using `g++`. While it should work, additional debugging may be required. `clang` is currently not supported.

## Running Rastertoy
**Windows:**

Run:
 ```batch
 .\release\sld2_rastertoy.exe
 ``` 

**Linux:**

Run:
```bash
./release/sdl2_rastertoy
```
This will start the application with a default cube at the center of the screen. To bring up the controls press `h` and a help string will be printed to the terminal.<br>

To load in your own models:
1. create a `data` directory in `rastertoy` and place you models inside.
2. Run the program with the modles specified as arguments:
```batch
.\release\sdl2_rastertoy.exe [obj1 obj2 obj3 ...]
```
You can switch between models with keys `0-9`.<br>
Sample models can be found at:
* [McGuire Computer Graphics Archive](https://casual-effects.com/data/)
* [Florida State University: OBJ Files A 3D Object Format](https://people.sc.fsu.edu/~jburkardt/data/obj/obj.html)
* [github.com: common-3d-test-models repository](https://github.com/alecjacobson/common-3d-test-models)
**Note:** Currently only `OBJ` file formats are supported for importing.

## Disclaimer
`Ratertoy`'s current implementation is not complete and has known bugs, using it outside the prescribed parameters may result in undefined behaviour.