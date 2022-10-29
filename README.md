# RAYTRACER

## Dependencies

- [VCPKG](https://vcpkg.io/)
- [CMake](https://cmake.org/)

## Build

```
cmake -DCMAKE_TOOLCHAIN_FILE=${PATH_TO_VCPKG}/scripts/buildsystems/vcpkg.cmake -B build -S .
```

## Raytraced Owl

<img alt="owl" src="assets/render/owl.jpg"/>

## how to use
- Ctrl + R to start rendering an image
- Ctrl + S to save an **img.jpg** image inside the folder assets/render
