cmake_minimum_required(VERSION 3.10)
project(raycaster C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)

# Try to find SDL2_mixer using pkg-config
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(SDL2_MIXER SDL2_mixer)
endif()

include_directories(${SDL2_INCLUDE_DIRS})
if(SDL2_MIXER_FOUND)
    include_directories(${SDL2_MIXER_INCLUDE_DIRS})
endif()
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/assets)

# Collect all source files
file(GLOB_RECURSE SOURCES
    "src/*.c"
)

# Create executable
add_executable(raycaster ${SOURCES})

# Link libraries
if(SDL2_MIXER_FOUND)
    target_link_libraries(raycaster ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARIES} m)
else()
    target_link_libraries(raycaster ${SDL2_LIBRARIES} m)
    message(WARNING "SDL2_mixer not found - sound will be disabled")
endif()
