cmake_minimum_required(VERSION 3.10)
project(CHIP8_Emulator C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")

# Find SDL2. If this fails, the user will need to configure the SDL2_DIR
find_package(SDL2 REQUIRED)

include_directories(include)

file(GLOB SOURCES "src/*.c")

add_executable(chip8 ${SOURCES})

# Target linking
target_link_libraries(chip8 ${SDL2_LIBRARIES})
target_include_directories(chip8 PRIVATE ${SDL2_INCLUDE_DIRS})
