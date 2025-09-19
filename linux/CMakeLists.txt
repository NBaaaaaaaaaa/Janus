cmake_minimum_required(VERSION 3.16)
project(JanusLV VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake/Qt6")
find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(CORE_SOURCES
    src/core/inject.cpp
    src/core/injectMethods.cpp
)

set(GUI_SOURCES
    src/gui/mainWindow.cpp
    src/gui/leftPanel.cpp
    src/gui/stackPages/files.cpp
    src/gui/stackPages/processes.cpp
    src/gui/stackPages/info.cpp
    src/gui/stackPages/hexFilter.cpp
)

qt6_wrap_cpp(MOC_SOURCES
    include/gui/mainWindow.h
    include/gui/leftPanel.h
    include/gui/stackPages/files.h
    include/gui/stackPages/processes.h
    include/gui/stackPages/info.h
    include/gui/stackPages/hexFilter.h
    include/core/inject.h
    include/core/injectMethods.h
)

add_executable(JanusLV src/main.cpp ${CORE_SOURCES} ${GUI_SOURCES} ${MOC_SOURCES})

target_link_libraries(JanusLV PRIVATE Qt6::Widgets)
target_include_directories(JanusLV PRIVATE include)
