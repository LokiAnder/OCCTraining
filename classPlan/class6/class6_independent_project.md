# ✅ 第6讲：Qt + OCCT 简单实现 - 创建方块并支持点击选择

> 本讲目标：用最简单的方式实现一个Qt应用，创建一个方块(Block)，在3D窗口中显示，并支持鼠标点击获取方块ID。

---

## 🎯 1. 目标效果

我们要实现的功能：
1. ✅ 创建一个Qt窗口
2. ✅ 在窗口中嵌入OCCT 3D显示
3. ✅ 创建一个方块并显示
4. ✅ 鼠标点击方块，控制台输出方块ID

---

## 🏗️ 2. 项目结构（极简版）

```
SimpleCAD/
├── CMakeLists.txt
├── main.cpp
├── OccWidget.h
├── OccWidget.cpp
├── MainWindow.h
└── MainWindow.cpp
```

就这6个文件，足够了！

---

## ⚙️ 3. CMakeLists.txt 配置

```cmake
cmake_minimum_required(VERSION 3.16)
project(SimpleCAD)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# 查找OpenCASCADE
find_package(OpenCASCADE REQUIRED)

# 查找OpenGL
find_package(OpenGL REQUIRED)

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
    
    # 最少必要的OCCT库
    TKernel
    TKMath
    TKBRep
    TKPrim
    TKV3d
    TKOpenGl
    TKService
    
    ${OPENGL_LIBRARIES}
)

# Qt MOC处理
set(CMAKE_AUTOMOC ON)
```

---

## 🖥️ 4. OccWidget.h - 3D显示窗口

```cpp
#pragma once

#include <QWidget>
#include <QMouseEvent>

// OCCT头文件
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>

class OccWidget : public QWidget
{
    Q_OBJECT

public:
    OccWidget(QWidget* parent = nullptr);
    ~OccWidget();
    
    // 创建方块并显示
    int CreateBlock(double x, double y, double z, double width, double height, double depth);
    
    // 适配窗口大小
    void FitAll();

protected:
    // Qt事件处理
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    // 初始化OCCT显示系统
    void InitializeOCC();
    
    // OCCT组件
    Handle(Aspect_DisplayConnection) myDisplayConnection;
    Handle(OpenGl_GraphicDriver) myGraphicDriver;
    Handle(V3d_Viewer) myViewer;
    Handle(V3d_View) myView;
    Handle(AIS_InteractiveContext) myContext;
    
    // 方块管理
    struct Block {
        int id;
        Handle(AIS_Shape) aisShape;
        TopoDS_Shape shape;
    };
    
    std::vector<Block> myBlocks;
    int myNextBlockId;
};
```

---

## 🔧 5. OccWidget.cpp - 实现

```cpp
#include "OccWidget.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <WNT_Window.hxx>
#include <Quantity_Color.hxx>
#include <AIS_Shape.hxx>
#include <gp_Pnt.hxx>

#include <iostream>
#include <QApplication>

OccWidget::OccWidget(QWidget* parent)
    : QWidget(parent)
    , myNextBlockId(1)
{
    // 设置窗口属性
    setBackgroundRole(QPalette::NoRole);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    
    // 初始化OCCT
    InitializeOCC();
}

OccWidget::~OccWidget()
{
    // 清理资源
    myBlocks.clear();
}

void OccWidget::InitializeOCC()
{
    // 1. 创建显示连接
    myDisplayConnection = new Aspect_DisplayConnection();
    
    // 2. 创建图形驱动
    myGraphicDriver = new OpenGl_GraphicDriver(myDisplayConnection);
    
    // 3. 创建查看器
    myViewer = new V3d_Viewer(myGraphicDriver);
    myViewer->SetDefaultLights();
    myViewer->SetLightOn();
    
    // 4. 创建视图
    myView = myViewer->CreateView();
    
    // 5. 创建Qt窗口句柄
    Handle(WNT_Window) window = new WNT_Window((Aspect_Handle)winId());
    myView->SetWindow(window);
    
    // 6. 创建交互上下文
    myContext = new AIS_InteractiveContext(myViewer);
    
    // 7. 设置背景和坐标轴
    myView->SetBackgroundColor(Quantity_NOC_GRAY30);
    myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08);
    
    // 8. 如果窗口已映射，显示它
    if (!window->IsMapped()) {
        window->Map();
    }
    
    std::cout << "OCCT initialized successfully!" << std::endl;
}

int OccWidget::CreateBlock(double x, double y, double z, double width, double height, double depth)
{
    try {
        // 1. 创建方块几何体
        gp_Pnt corner(x, y, z);
        TopoDS_Shape boxShape = BRepPrimAPI_MakeBox(corner, width, height, depth).Shape();
        
        // 2. 创建显示对象
        Handle(AIS_Shape) aisShape = new AIS_Shape(boxShape);
        
        // 3. 设置颜色（每个方块不同颜色）
        Quantity_Color color;
        switch ((myNextBlockId - 1) % 3) {
            case 0: color = Quantity_NOC_RED; break;
            case 1: color = Quantity_NOC_GREEN; break;
            case 2: color = Quantity_NOC_BLUE; break;
        }
        aisShape->SetColor(color);
        
        // 4. 显示方块
        myContext->Display(aisShape, Standard_True);
        
        // 5. 保存方块信息
        Block block;
        block.id = myNextBlockId;
        block.aisShape = aisShape;
        block.shape = boxShape;
        myBlocks.push_back(block);
        
        std::cout << "Block created with ID: " << myNextBlockId << std::endl;
        
        return myNextBlockId++;
        
    } catch (const Standard_Failure& e) {
        std::cerr << "CreateBlock failed: " << e.GetMessageString() << std::endl;
        return -1;
    }
}

void OccWidget::FitAll()
{
    if (!myView.IsNull()) {
        myView->FitAll();
        myView->ZFitAll();
    }
}

void OccWidget::paintEvent(QPaintEvent* /*event*/)
{
    if (!myView.IsNull()) {
        myView->Redraw();
    }
}

void OccWidget::resizeEvent(QResizeEvent* /*event*/)
{
    if (!myView.IsNull()) {
        myView->MustBeResized();
    }
}

void OccWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !myContext.IsNull()) {
        // 1. 将鼠标位置传递给OCCT
        myContext->MoveTo(event->x(), event->y(), myView, Standard_True);
        
        // 2. 执行选择
        myContext->Select(Standard_True);
        
        // 3. 检查是否选中了对象
        if (myContext->HasSelected()) {
            // 4. 获取选中的对象
            Handle(AIS_InteractiveObject) selectedObject = myContext->FirstSelectedObject();
            
            // 5. 查找对应的方块ID
            for (const auto& block : myBlocks) {
                if (block.aisShape == selectedObject) {
                    std::cout << "Clicked Block ID: " << block.id << std::endl;
                    
                    // 可选：高亮显示选中的方块
                    myContext->HilightWithColor(selectedObject, Quantity_NOC_YELLOW, Standard_True);
                    break;
                }
            }
        } else {
            std::cout << "No block selected" << std::endl;
            // 清除所有高亮
            myContext->ClearSelected(Standard_True);
        }
    }
}
```

---

## 🏠 6. MainWindow.h - 主窗口

```cpp
#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class OccWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void CreateBlock();
    void FitAll();

private:
    OccWidget* myOccWidget;
    QPushButton* myCreateBlockButton;
    QPushButton* myFitAllButton;
    QLabel* myStatusLabel;
    
    int myBlockCount;
};
```

---

## 🎨 7. MainWindow.cpp - 主窗口实现

```cpp
#include "MainWindow.h"
#include "OccWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QStatusBar>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , myBlockCount(0)
{
    // 创建中央窗口部件
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 创建OCCT显示窗口
    myOccWidget = new OccWidget(this);
    
    // 创建按钮
    myCreateBlockButton = new QPushButton("创建方块", this);
    myFitAllButton = new QPushButton("适合窗口", this);
    
    // 创建状态标签
    myStatusLabel = new QLabel("准备就绪", this);
    
    // 布局设置
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(myCreateBlockButton);
    buttonLayout->addWidget(myFitAllButton);
    buttonLayout->addStretch(); // 填充空间
    buttonLayout->addWidget(myStatusLabel);
    
    // 主布局
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(myOccWidget, 1); // 给OCCT窗口更多空间
    
    // 连接信号槽
    connect(myCreateBlockButton, &QPushButton::clicked, this, &MainWindow::CreateBlock);
    connect(myFitAllButton, &QPushButton::clicked, this, &MainWindow::FitAll);
    
    // 设置窗口属性
    setWindowTitle("Simple CAD - 方块创建与选择");
    resize(800, 600);
    
    // 状态栏
    statusBar()->showMessage("左键点击方块查看ID");
}

MainWindow::~MainWindow()
{
    // Qt会自动清理子对象
}

void MainWindow::CreateBlock()
{
    // 在不同位置创建方块
    double spacing = 120.0;
    double x = (myBlockCount % 3) * spacing;
    double y = (myBlockCount / 3) * spacing;
    double z = 0.0;
    
    // 创建方块
    int blockId = myOccWidget->CreateBlock(x, y, z, 80, 60, 40);
    
    if (blockId > 0) {
        myBlockCount++;
        myStatusLabel->setText(QString("已创建 %1 个方块").arg(myBlockCount));
        
        // 如果是第一个方块，自动适合窗口
        if (myBlockCount == 1) {
            myOccWidget->FitAll();
        }
    } else {
        myStatusLabel->setText("方块创建失败");
    }
}

void MainWindow::FitAll()
{
    myOccWidget->FitAll();
    myStatusLabel->setText("视图已调整");
}
```

---

## 🚀 8. main.cpp - 程序入口

```cpp
#include <QApplication>
#include "MainWindow.h"

#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    std::cout << "=== Simple CAD Application ===" << std::endl;
    std::cout << "点击'创建方块'按钮添加方块" << std::endl;
    std::cout << "左键点击方块查看ID" << std::endl;
    std::cout << "================================" << std::endl;
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
```

---

## 🔨 9. 编译和运行

### 编译步骤

```bash
# 1. 创建项目目录
mkdir SimpleCAD
cd SimpleCAD

# 2. 复制所有源文件到目录中

# 3. 创建构建目录
mkdir build
cd build

# 4. 配置和编译
cmake ..
make -j4

# 5. 运行
./SimpleCAD
```

### 预期效果

1. **启动程序**：看到一个带按钮的窗口
2. **点击"创建方块"**：在3D视图中出现一个彩色方块
3. **继续点击**：会在不同位置创建更多方块
4. **点击方块**：控制台输出 "Clicked Block ID: X"
5. **点击"适合窗口"**：所有方块都能看到

---

## 🎯 10. 测试验证

### 功能清单

- [ ] 程序能正常启动
- [ ] 能看到3D显示窗口
- [ ] 点击"创建方块"按钮有效果
- [ ] 方块能正常显示
- [ ] 点击方块能输出ID
- [ ] "适合窗口"按钮工作正常

### 控制台输出示例

```
=== Simple CAD Application ===
点击'创建方块'按钮添加方块
左键点击方块查看ID
================================
OCCT initialized successfully!
Block created with ID: 1
Block created with ID: 2
Block created with ID: 3
Clicked Block ID: 2
Clicked Block ID: 1
No block selected
Clicked Block ID: 3
```

---

## 🧪 课后作业

### 📝 基础作业

1. **🔧 成功运行**
   - 编译并运行程序
   - 创建3个方块并测试点击功能
   - 截图展示效果

2. **🎨 小改进**
   - 修改方块的默认尺寸
   - 改变方块的颜色
   - 调整方块的创建位置

### 🚀 进阶作业

3. **✨ 功能扩展**
   - 添加"删除方块"功能（点击右键删除）
   - 显示选中方块的详细信息（尺寸、位置等）
   - 添加方块计数显示

4. **🔍 交互优化**
   - 改进高亮效果
   - 添加鼠标悬停提示
   - 实现方块的拖拽移动

---

## 📋 下节预告

下一讲我们将在这个基础上：
- 添加更多几何体类型（球体、圆柱等）
- 实现几何体的属性编辑
- 添加文件保存/加载功能
- 完善用户界面

---

这个简化版本更实用，代码量大大减少，但功能完整，非常适合入门学习！