# Setup the basic CMake environment
cmake_minimum_required(VERSION 3.20)

project(spectrogram 
    VERSION 1.0.0 
    LANGUAGES CXX
)

# Enforce modern C++ standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define the main target (Build as a library)
add_library(spectrogram_lib STATIC 
    src/audio_capture.cpp
    src/canvas.cpp
    src/processor.cpp
    src/visualizer.cpp
)

# Bind the header search paths to your library target
target_include_directories(spectrogram_lib 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Do platform specific stuff
if(WIN32)
    target_sources(spectrogram_lib PRIVATE src/platform/win32.cpp)
elseif(APPLE)
    target_sources(spectrogram_lib PRIVATE src/platform/macos.mm)
    target_link_libraries(spectrogram_lib PUBLIC "-framework Cocoa")
else()
    target_sources(spectrogram_lib PRIVATE src/platform/linux.cpp)
    target_link_libraries(spectrogram_lib PUBLIC X11)
endif()

# Crucial for static SFML: Tells CMake to use static targets
set(SFML_STATIC_LIBRARIES TRUE)

# Get SFML and TGUI
find_package(SFML 3.0.2 COMPONENTS Graphics Window System CONFIG REQUIRED EXACT)
find_package(TGUI 1 CONFIG REQUIRED)

target_link_libraries(spectrogram_lib PUBLIC 
    SFML::Graphics 
    SFML::Window 
    SFML::System
    TGUI::TGUI
)

# Tell CMake to output the final executable to your main project root directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Define the executable application entry point
add_executable(spectrogram 
    src/main.cpp
)

# target_include_directories(spectogram PRIVATE 
#     "$<BUILD_INTERFACE:${SHADER_HEADER_DIR}>"
# )

# Link your application to your core library logic
target_link_libraries(spectrogram 
    PRIVATE 
        spectrogram_lib
)
