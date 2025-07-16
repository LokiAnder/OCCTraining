cmake_minimum_required(VERSION 3.16)
project(SimpleCAD)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置OpenCASCADE的安装路径
set(OpenCASCADE_DIR "C:/Program Files/OCCT/cmake")

# 查找Qt
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL)

# 查找OpenCASCADE
find_package(OpenCASCADE REQUIRED)

# 查找OpenGL
find_package(OpenGL REQUIRED)

# 设置Qt自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 包含目录
include_directories(${OpenCASCADE_INCLUDE_DIR})

# 源文件
set(SOURCES
    main.cpp
    OccWidget.cpp
    MainWindow.cpp
)

set(HEADERS
    OccWidget.h
    MainWindow.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 链接库
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::OpenGL
    
    # OCCT核心库
    TKernel
    TKMath
    TKBRep
    TKTopAlgo
    TKPrim
    TKBool
    
    # OCCT显示库
    TKV3d
    TKOpenGl
    TKService
    
    ${OpenGL_LIBRARIES}
)

# Windows特殊处理
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()