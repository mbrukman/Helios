# Helios

A Experimental, C++20 & DX12 renderer made for learning and trying out various graphics / rendering techniques.

# Features
* Bindless Rendering (Using SM 6.6's Resource / Sampler descriptor Heap).
* Normal Mapping.
* Diffuse and Specular IBL.
* Physically based rendering (PBR).
* Blinn-Phong Shading.
* Deferred Shading.
* HDR and Tone Mapping.
* Instanced rendering.
* Compute Shader mip map generation.
* Editor (ImGui Integration).

# Gallery
> PBR and IBL
![](Assets/Screenshots/IBL1.png)
![](Assets/Screenshots/IBL2.png)
![](Assets/Screenshots/IBL3.png)

> Deferred Shading
![](Assets/Screenshots/Deferred1.png)

> Editor (using ImGui)
![](Assets/Screenshots/Editor1.png)

# Building
+ This project uses CMake as a build system, and all third party libs are setup using CMake's FetchContent().
+ After cloning the project, build the project using CMake (Open folder does not currently work due to a bug in the working directory)
+ Run the setup.bat file, which will install the DirectXAgility SDK. 
+ Ensure you have installed the DirectXShaderCompiler (must support atleast SM 6.6).
+ Shaders are ~~automatically compiled after build process~~, to be compiled by running the CompileShaders.bat (or alternatively the .py file).

