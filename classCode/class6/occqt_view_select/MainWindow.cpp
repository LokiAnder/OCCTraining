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
    , myOccWidget(nullptr)
    , myCreateBoxButton(nullptr)
    , myClearButton(nullptr)
    , myStatusLabel(nullptr)
{
    SetupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::SetupUI()
{
    // 创建中央widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建OCC widget
    myOccWidget = new OccWidget(this);
    myOccWidget->setMinimumSize(800, 600);

    // 创建按钮
    myCreateBoxButton = new QPushButton("创建立方体", this);
    myClearButton = new QPushButton("清除所有", this);

    // 创建状态标签
    myStatusLabel = new QLabel("准备就绪", this);

    // 布局
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(myCreateBoxButton);
    buttonLayout->addWidget(myClearButton);
    buttonLayout->addStretch(); // 添加弹性空间

    // 主布局
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(myOccWidget, 1); // 占据大部分空间

    // 状态栏
    statusBar()->addWidget(myStatusLabel);

    // 连接信号
    connect(myCreateBoxButton, &QPushButton::clicked, this, &MainWindow::OnCreateBox);
    connect(myClearButton, &QPushButton::clicked, this, &MainWindow::OnClearAll);
    connect(myOccWidget, &OccWidget::ShapeSelected, this, &MainWindow::OnShapeSelected);

    // 设置窗口属性
    setWindowTitle("简单的occ查看器");
    resize(1000, 800);

    // 初始化OCC
    myOccWidget->InitializeOcc();
}

void MainWindow::OnCreateBox()
{
    if (myOccWidget) {
        // 创建一个随机位置的立方体
        static int count = 0;
        double offset = count * 60; // 避免重叠

        int shapeId = myOccWidget->AddBox(50, 50, 50, offset, 0, 0);

        if (shapeId >= 0) {
            myStatusLabel->setText(QString("创建立方体，ID: %1").arg(shapeId));
            count++;
        }
        else {
            myStatusLabel->setText("创建立方体失败");
        }
    }
}

void MainWindow::OnClearAll()
{
    if (myOccWidget) {
        myOccWidget->ClearAll();
        myStatusLabel->setText("已清除所有对象");
    }
}

void MainWindow::OnShapeSelected(int shapeId)
{
    myStatusLabel->setText(QString("选中立方体，ID: %1").arg(shapeId));
}