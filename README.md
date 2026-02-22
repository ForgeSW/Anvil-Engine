# Anvil Engine
#### A lightweight 3D game engine made in C++
![Anvil Engine](image.png)
Anvil is a custom game engine built in C++ focusing on BSP (Binary space partition) or in this case ABSP (Anvil Binary Space Partition). It is designed for developers who prefer controlling everything from code instead of an editor

## Tech Stack
Anvil is built on top of the following technologies:
* Rendering: Glad and GLFW
* Math: GLM (OpenGL mathematics)
* Physics: ReactPhysics3d
* Assets: Assimp (models) stb_image (textures)

## Getting Started
### Prerequisites
* C++20 Standard
* Visual Studio 2022/2026 (for .slnx support)
* Git

### Installation
* Clone the ```https://github.com/microsoft/vcpkg.git``` repository to a path like ```C:\dev\vcpkg```
* Open the terminal and run ```.\bootstrap-vcpkg.bat```
* Run ```.\vcpkg install glad glm glfw reactphysics3d assimp stb_image```
* Run ``` .\vcpkg integrate install```
* Open the ```Anvil Engine.slnx``` solution file and hit CTRL+SHIFT+B
* Go to the SampleModels folder and copy the files to the build directory
* Open the terminal and run ```.\Anvil_Compile mesh YOURMESHNAME.glb``` or any other model extention you'd like, you can also run ```.\Anvil_Compile map MAPNAME.map```, the map should be only in the Quake Standard format
* Run the ```Anvil_Engine.exe``` file
