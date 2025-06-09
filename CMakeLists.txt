cmake_minimum_required(VERSION 3.16)
#set(CMAKE_PREFIX_PATH  "C:\\Qt\\6.9.0\\mingw_64")

project(JSONTool VERSION 0.1 LANGUAGES CXX)
message(STATUS "=============start complile=============")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        src/main.cpp
        src/jsonmainwindow.cpp
        src/jsonmainwindow.h
        src/jsonmainwindow.ui
        src/codeeditor.h
        src/codeeditor.cpp
        src/jsonhighlighter.h
        src/jsonhighlighter.cpp
        src/utils.h
        src/utils.cpp
        src/jsonwidget.h
        src/jsonwidget.cpp
        src/jsonwidget.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(JSONTool
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        src/jsonrec.qrc
        src/app.rc
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET JSONTool APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(JSONTool SHARED ${PROJECT_SOURCES})
    else()
        add_executable(JSONTool ${PROJECT_SOURCES})
    endif()
endif()

target_compile_definitions(JSONTool PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(JSONTool PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.JSONTool)
endif()
set_target_properties(JSONTool PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
message(STATUS "当前源码目录: ${PROJECT_NAME}")
include(GNUInstallDirs)
install(TARGETS JSONTool
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(JSONTool)
endif()

message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "PROJECT_NAME_autogen: ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_autogen")

message(STATUS "codeeditor: ${CMAKE_CURRENT_SOURCE_DIR}/src/codeeditor.h")
# 确保目录存在（添加存在性检查）
if(NOT EXISTS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_autogen/include/)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_autogen/include/)
endif()

# 在CMake配置阶段复制文件
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/codeeditor.h
     DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_autogen/include/)

# Windows平台图标设置
if(WIN32)
    # 复制图标文件到构建目录
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/json.ico
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    
    # 设置资源编译选项
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
endif()
