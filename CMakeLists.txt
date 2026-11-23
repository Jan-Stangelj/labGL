cmake_minimum_required(VERSION 3.16)

project("labGL" LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL3
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL_UNINSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory("vendor/sdl" EXCLUDE_FROM_ALL)
add_subdirectory("vendor/glm")
add_subdirectory("vendor/glad")
add_subdirectory("vendor/stb")
add_subdirectory("vendor/spdlog")
add_subdirectory("vendor/fastgltf")

file(GLOB_RECURSE "SOURCES" CONFIGURE_DEPENDS "src/*.cpp")
add_executable("labGL" "${SOURCES}")

# Enable ALL warnings
if(MSVC)
  target_compile_options("labGL" PRIVATE /W4)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options("labGL" PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options("labGL" PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories("labGL" PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Keep trace/debug logging available in Debug builds, compile it out everywhere else.
target_compile_definitions("labGL" PRIVATE $<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>,SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE,SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO>)

target_link_libraries("labGL" PRIVATE SDL3::SDL3-static glm::glm glad stb spdlog::spdlog fastgltf::fastgltf)