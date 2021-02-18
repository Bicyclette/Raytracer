# RAYTRACER
TME_1 IG3D

# Rendered images
Are located in the assets/render folder

# install dependencies (fedora 33)
Must be performed as root:
+ dnf install SDL2-devel.x86_64
+ dnf install glew-devel.x86_64
+ dnf install assimp-devel.x86_64
Now you need to install OpenCL
I personnaly have a nvidia graphic card so I have done the following steps as explained here : <br/>
<https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html#fedora-installation>

Install CUDA
+ sudo dnf module install nvidia-driver:latest-dkms
+ sudo dnf install cuda

The CUDA driver installation may fail if the RPMFusion non-free repository is enabled. In this case, CUDA installations should temporarily disable the RPMFusion non-free repository:

+ sudo dnf --disablerepo="rpmfusion-nonfree*" install cuda

It may be necessary to rebuild the grub configuration files, particularly if you use a non-default partition scheme. <br/>
If so, then run this below command, and reboot the system:

+ sudo grub2-mkconfig -o /boot/grub2/grub.cfg

Remember to reboot the system.

# how to build (GNU/linux systems)
1) in the root directory, create a build directory: mkdir build
2) then type: cmake -G "Unix Makefiles" -B build -S .
3) checkout to build directory: cd build
4) compile: make
5) run: ./Raytracer

# how to use
File main.cpp, line 12:<br/>You can choose which scene you wish to render, by setting the int parameter for the function **raytracer->setActiveScene(int scene)** to either 0 (lightweight scene) or 1(heavy scene, runs better on GPU) !<br/><br/>
File main.cpp, line 31:<br/>You can also choose wether or not you want to render on CPU or GPU, by setting the boolean for the function **raytracer->renderScene(client, true)** it is set on true by default, so it uses the GPU.<br/><br/>
- Ctrl + R to start rendering an image
- Ctrl + S to save an **img.jpg** image inside the folder assets/render

# Known issues
The render view window will not show anything, even if the render job is finished.<br/>
That is because I have commented the SDL instruction which paints the result on it.<br/>
Why have I done that ? Because for some reason, the **SDL_RenderPresent()** instruction induces a segfault
in the application.<br/>
It is in the file **application.cpp**, on line 451.<br/>
Just take a look at the terminal messages, when the message "Raytracing finished" appears, you can safely save the image or close the window to go back to the main window<br/>
and change your point of view, or whatever.

# Additional informations
The scene with an angel takes approximatively 3 min 40 seconds to render on my nvidia GTX 1660 TI.<br/>
I haven't tested how long it takes on CPU, since that mesh got like 170000 triangles, it would take ages to render on CPU.<br/>
