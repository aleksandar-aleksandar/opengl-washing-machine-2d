cmake_minimum_required(VERSION 3.20)
project(WashingMachine CXX)              # project name, language C++
set(CMAKE_CXX_STANDARD 17)               # use the C++17 version of the language

find_package(glfw3 REQUIRED)             # "find GLFW on this PC, stop if missing"
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

# build WashingMachine.exe from all our .cpp files
add_executable(WashingMachine
    src/main.cpp      # window setup + the main loop
    src/scene.cpp     # the washing machine: states, dragging, spinning, water, pink, drawing it
    src/cloth.cpp     # one piece of clothing + its < > buttons
    src/draw.cpp      # drawing shapes (the 1x1 square + uniforms)
    src/texture.cpp   # loading PNGs (textures + cursor)
    src/shader.cpp    # loading + compiling shaders
)
target_include_directories(WashingMachine PRIVATE external)   # where to find stb_image.h
target_link_libraries(WashingMachine PRIVATE glfw GLEW::GLEW OpenGL::GL)  # the linking stage
