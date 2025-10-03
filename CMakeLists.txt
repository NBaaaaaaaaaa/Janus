cmake_minimum_required(VERSION 3.16)
project(Janus VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)


# --- Опции сборки ---
option(BUILD_GUI "Build GUI version" ON)
option(BUILD_CLI "Build Console version" ON)

# --- ОС-специфичные ядра ---
if (WIN32)
    message(STATUS "Building with Windows core")
    set(CORE_SOURCES
        core/windows/src/inject.cpp
        core/windows/src/injectMethods.cpp
    )
    set(CORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core/windows/include)
elseif (APPLE)
    message(STATUS "Building with macOS core")
    set(CORE_SOURCES
        core/macos/src/inject.cpp
        core/macos/src/injectMethods.cpp
    )
    set(CORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core/macos/include)
elseif (UNIX)
    message(STATUS "Building with Linux core")
    set(CORE_SOURCES
        core/linux/src/inject.cpp
        core/linux/src/injectMethods.cpp
    )
    set(CORE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core/linux/include)
endif()


# --- CLI версия ---
if (BUILD_CLI)
    add_executable(JanusCLI
        cli/main.cpp
        ${CORE_SOURCES}
    )

    target_include_directories(JanusCLI PRIVATE 
        core
        cli/include
        ${CORE_INCLUDE_DIR}
    )

endif()

# --- GUI версия ---
if (BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    set(GUI_SOURCES
        gui/src/mainWindow.cpp
        gui/src/leftPanel.cpp
        gui/src/stackPages/files.cpp
        gui/src/stackPages/processes.cpp
        gui/src/stackPages/info.cpp
        gui/src/stackPages/hexFilter.cpp
    )

    qt6_wrap_cpp(MOC_SOURCES
        gui/include/mainWindow.h
        gui/include/leftPanel.h
        gui/include/stackPages/files.h
        gui/include/stackPages/processes.h
        gui/include/stackPages/info.h
        gui/include/stackPages/hexFilter.h
    )

    add_executable(JanusGUI
        gui/main.cpp
        ${CORE_SOURCES}
        ${GUI_SOURCES}
        ${MOC_SOURCES}
    )

    target_link_libraries(JanusGUI PRIVATE Qt6::Widgets)
    target_include_directories(JanusGUI PRIVATE
        core
        gui/include
        ${CORE_INCLUDE_DIR}
    )
endif()

