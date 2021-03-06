cmake_minimum_required(VERSION 3.16)

project(Raytracer)

set(CMAKE_BUILD_TYPE Debug)

set(SRCS
	src/stb_image.cpp
	src/stb_image_write.cpp
	src/main.cpp
	src/window.cpp
	src/shader_light.cpp
	src/camera.cpp
	src/mesh.cpp
	src/object.cpp
	src/grid_axis.cpp
	src/skybox.cpp
	src/scene.cpp
	src/framebuffer.cpp
	src/application.cpp
	src/graphics.cpp
	src/clprogram.cpp)

set(HEADERS
	include/stb_image.h
	include/stb_image_write.h
	include/color.hpp
	include/window.hpp
	include/shader_light.hpp
	include/camera.hpp
	include/mesh.hpp
	include/object.hpp
	include/grid_axis.hpp
	include/skybox.hpp
	include/scene.hpp
	include/framebuffer.hpp
	include/application.hpp
	include/graphics.hpp
	include/clprogram.hpp)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

include_directories(${PROJECT_SOURCE_DIR}/include /usr/local/cuda/include)

link_directories(/usr/local/cuda/lib64)

add_compile_options(-lOpenCL)

add_executable(${PROJECT_NAME} ${SRCS} ${HEADERS})

cmake_policy(SET CMP0072 NEW)

find_package(SDL2 REQUIRED)
if(SDL2_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})
else()
	message(FATAL_ERROR "sdl2 not found")
endif()

find_package(GLEW REQUIRED)
if(GLEW_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
else()
	message(FATAL_ERROR "glew not found")
endif()

find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARY})
else()
	message(FATAL_ERROR "OpenGL not found")
endif()

find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
	target_link_libraries(${PROJECT_NAME} ${OpenMP_LD_FLAGS})
else()
	message(FATAL_ERROR "OpenMP not found")
endif()

find_package(OPENCL REQUIRED)
if(OPENCL_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${OPENCL_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${OPENCL_LIBRARIES})
else()
	message(FATAL_ERROR "OpenCL not found")
endif()

find_package(ASSIMP REQUIRED)
if(ASSIMP_FOUND)
	target_include_directories(${PROJECT_NAME} PUBLIC ${ASSIMP_INCLUDE_DIR})
	target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES})
	target_link_libraries(${PROJECT_NAME} -fopenmp)
else()
	message(FATAL_ERROR "Assimp not found")
endif()
