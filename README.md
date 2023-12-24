# Solar System Game
Solar System Game in OpenGL

### *University project*
---


## Getting Started

1. Install [Visual Studio](https://visualstudio.microsoft.com/vs/getting-started/) with *Desktop development with C++*.
2. Open `SolarSystem.sln`.
3. **View** -> **Solution Explorer**.
4. Right click **Solar System** and go to **Properties**.
5. Set `Configuration` to `All Configurations` and platform `x64`.
6. **C/C++** -> **General**
7. Copy/Paste this to *Additional Include Directories*:
    * `$(ProjectDir)Dependencies\GLM;$(ProjectDir)Dependencies\GLEWbin\include;$(ProjectDir)Dependencies\GLFWx64\include;%(AdditionalIncludeDirectories)`
8. **Linker** -> **General**
9. Copy/Paste this to *Additional Library Directories*:
    * `$(ProjectDir)Dependencies\GLEWbin\lib\Release\x64;$(ProjectDir)Dependencies\GLFWx64\lib-vc2022;%(AdditionalLibraryDirectories)`
10. **Linker** -> **Input**
11. Copy/Paste this to *Additional Dependencies*:
    * `opengl32.lib;glfw3.lib;glfw3dll.lib;glew32.lib;%(AdditionalDependencies)`
12. Click **Apply**
13. **Run**.
