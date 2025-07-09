cmake_minimum_required(VERSION 3.15)

# 强制使用 MSVC 2017 工具集（v141）
set(CMAKE_GENERATOR_TOOLSET "v141" CACHE STRING "Platform Toolset" FORCE)

project(MyCADApp)
set(CMAKE_CXX_STANDARD 17)

# ------------------------------------------------------------------
# 明确设置 Qt 路径
# ------------------------------------------------------------------
set(Qt5_DIR "D:/Qt/Qt5.9.7/5.9.7/msvc2017_64/lib/cmake/Qt5")
set(CMAKE_PREFIX_PATH "D:/Qt/Qt5.9.7/5.9.7/msvc2017_64")

# 调试信息
message(STATUS "Qt5_DIR: ${Qt5_DIR}")
message(STATUS "CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

# 检查 Qt 配置文件是否存在
if(NOT EXISTS "${Qt5_DIR}/Qt5Config.cmake")
    message(FATAL_ERROR "Qt5 config file not found at: ${Qt5_DIR}/Qt5Config.cmake")
endif()

# ------------------------------------------------------------------
# 查找 Qt5 - 先只找基础组件
# ------------------------------------------------------------------
find_package(Qt5 5.9 REQUIRED COMPONENTS Core Widgets)

# 验证 Qt5 是否找到
if(NOT Qt5_FOUND)
    message(FATAL_ERROR "Qt5 not found!")
endif()

message(STATUS "Qt5_FOUND: ${Qt5_FOUND}")
message(STATUS "Qt5_VERSION: ${Qt5_VERSION}")
message(STATUS "Qt5Core_LIBRARIES: ${Qt5Core_LIBRARIES}")
message(STATUS "Qt5Widgets_LIBRARIES: ${Qt5Widgets_LIBRARIES}")

# ------------------------------------------------------------------
# 现在添加 OpenCASCADE
# ------------------------------------------------------------------
set(OpenCASCADE_DIR "C:/Program Files/OCCT/cmake")

# 检查 OpenCASCADE 配置文件是否存在
if(NOT EXISTS "${OpenCASCADE_DIR}/OpenCASCADEConfig.cmake")
    message(FATAL_ERROR "OpenCASCADE config file not found at: ${OpenCASCADE_DIR}/OpenCASCADEConfig.cmake")
endif()

find_package(OpenCASCADE REQUIRED CONFIG)

# 验证 OpenCASCADE 是否找到
if(NOT OpenCASCADE_FOUND)
    message(FATAL_ERROR "OpenCASCADE not found!")
endif()

message(STATUS "OpenCASCADE_FOUND: ${OpenCASCADE_FOUND}")
message(STATUS "OpenCASCADE_INCLUDE_DIR: ${OpenCASCADE_INCLUDE_DIR}")
message(STATUS "OpenCASCADE_LIBRARIES: ${OpenCASCADE_LIBRARIES}")

# ------------------------------------------------------------------
# 创建可执行文件
# ------------------------------------------------------------------
add_executable(MyCADApp
    src/main.cpp
)

# ------------------------------------------------------------------
# Windows 特定设置
# ------------------------------------------------------------------
if(WIN32 AND MSVC)
    # 确保使用正确的运行时库
    set_property(TARGET MyCADApp PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    # 添加必要的定义
    target_compile_definitions(MyCADApp PRIVATE
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
    )
    
    # 设置为 Windows 应用程序
    set_target_properties(MyCADApp PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# ------------------------------------------------------------------
# 包含目录
# ------------------------------------------------------------------
target_include_directories(MyCADApp PRIVATE
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${OpenCASCADE_INCLUDE_DIR}
)

# ------------------------------------------------------------------
# 链接库 - 现在包含 OpenCASCADE
# ------------------------------------------------------------------
target_link_libraries(MyCADApp PRIVATE
    Qt5::Core
    Qt5::Widgets
    ${OpenCASCADE_LIBRARIES}
)

# 确保自动 MOC
set(CMAKE_AUTOMOC ON)

# ------------------------------------------------------------------
# 调试：打印链接信息
# ------------------------------------------------------------------
get_target_property(LINK_LIBS MyCADApp LINK_LIBRARIES)
message(STATUS "Final linked libraries: ${LINK_LIBS}")

# ------------------------------------------------------------------
# 部署 Qt DLL - 使用更安全的方式
# ------------------------------------------------------------------
if(WIN32)
    # 查找 windeployqt
    find_program(QT_DEPLOYQT_EXECUTABLE windeployqt
        HINTS "D:/Qt/Qt5.9.7/5.9.7/msvc2017_64/bin"
    )
    
    if(QT_DEPLOYQT_EXECUTABLE)
        message(STATUS "Found windeployqt: ${QT_DEPLOYQT_EXECUTABLE}")
        # 暂时注释掉自动部署，避免构建错误
        # add_custom_command(TARGET MyCADApp POST_BUILD
        #     COMMAND ${QT_DEPLOYQT_EXECUTABLE} $<TARGET_FILE:MyCADApp>
        #     COMMENT "Deploying Qt libraries"
        # )
        message(STATUS "You can manually run: ${QT_DEPLOYQT_EXECUTABLE} $<TARGET_FILE:MyCADApp>")
    else()
        message(WARNING "windeployqt not found!")
    endif()
    
    # 手动复制必要的 Qt DLL
    add_custom_command(TARGET MyCADApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/Qt/Qt5.9.7/5.9.7/msvc2017_64/bin/Qt5Core$<$<CONFIG:Debug>:d>.dll"
            $<TARGET_FILE_DIR:MyCADApp>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/Qt/Qt5.9.7/5.9.7/msvc2017_64/bin/Qt5Widgets$<$<CONFIG:Debug>:d>.dll"
            $<TARGET_FILE_DIR:MyCADApp>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/Qt/Qt5.9.7/5.9.7/msvc2017_64/bin/Qt5Gui$<$<CONFIG:Debug>:d>.dll"
            $<TARGET_FILE_DIR:MyCADApp>
        COMMENT "Copying Qt libraries manually"
    )
    
    # 创建平台插件目录并复制插件
    add_custom_command(TARGET MyCADApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:MyCADApp>/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "D:/Qt/Qt5.9.7/5.9.7/msvc2017_64/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            $<TARGET_FILE_DIR:MyCADApp>/platforms/
        COMMENT "Copying Qt platform plugins"
    )
endif()