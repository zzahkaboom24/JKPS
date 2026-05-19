cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(JKPS CXX)

file(GLOB_RECURSE SRC_SOURCES "Source/*.cpp")
file(GLOB_RECURSE SRC_HEADERS "Headers/*.hpp")

if(WIN32)
    configure_file(${CMAKE_SOURCE_DIR}/Resources/icon.rc.in ${CMAKE_BINARY_DIR}/icon.rc @ONLY)
    set(WIN32_RESOURCES ${CMAKE_BINARY_DIR}/icon.rc)
endif()

add_executable(${PROJECT_NAME} WIN32 ${SRC_SOURCES} ${SRC_HEADERS} ${WIN32_RESOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE Headers)

if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /permissive-
    )
    # Use mainCRTStartup so int main() works as entry point with WIN32 subsystem
    target_link_options(${PROJECT_NAME} PRIVATE /ENTRY:mainCRTStartup)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wctor-dtor-privacy
        -Wenum-compare
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wredundant-decls
        -Wsign-conversion
        -Wsign-promo
    )
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC PROGRAM_VERSION="0.5.0")

find_package(Threads REQUIRED)
find_package(SFML 3 REQUIRED COMPONENTS System Window Graphics)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SFML::System
    SFML::Window
    SFML::Graphics
    Threads::Threads
)