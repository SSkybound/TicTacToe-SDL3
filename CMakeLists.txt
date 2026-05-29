cmake_minimum_required(VERSION 3.2...4.2)
project(TicTacToe_SDL3 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.8
    GIT_SHALLOW    TRUE
)
set(SDL_SHARED ON  CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
    SDL3_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG        release-3.2.2
    GIT_SHALLOW    TRUE
)
set(SDLMIXER_VENDORED ON  CACHE BOOL "" FORCE)
set(SDLMIXER_SAMPLES  OFF CACHE BOOL "" FORCE)
set(SDLMIXER_INSTALL  OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3_mixer)

file(GLOB_RECURSE CORE_SOURCES core/src/*.cpp)
file(GLOB_RECURSE CORE_HEADERS core/lib/*.h)

add_library(TicTacToeCore SHARED ${CORE_SOURCES} ${CORE_HEADERS})

target_include_directories(TicTacToeCore
    PUBLIC  core/lib
    PRIVATE core/src
)

target_link_libraries(TicTacToeCore PUBLIC SDL3::SDL3 SDL3_mixer::SDL3_mixer)

set_target_properties(TicTacToeCore PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

add_executable(app app/main.cpp)

target_link_libraries(app PRIVATE TicTacToeCore)

set_target_properties(app PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(TicTacToeCore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

if(APPLE)
    target_link_libraries(TicTacToeCore PUBLIC
        "-framework IOKit"
        "-framework Cocoa"
        "-framework OpenGL"
    )
endif()

if(WIN32)
    set_target_properties(app PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()

if(WIN32)
    add_custom_command(TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared>
            $<TARGET_FILE:SDL3_mixer::SDL3_mixer>
            $<TARGET_FILE_DIR:app>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying SDL3 DLLs next to app.exe"
    )
endif()

add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/bin/assets
    COMMENT "Copying assets to bin/"
)