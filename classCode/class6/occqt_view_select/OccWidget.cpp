#include "OccWidget.h"

// OpenCASCADE
#include <BRepPrimAPI_MakeBox.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <Quantity_Color.hxx>
#include <AIS_Shape.hxx>
#include <V3d_DirectionalLight.hxx>
#include <V3d_AmbientLight.hxx>
#include <Aspect_TypeOfTriedronPosition.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <StdSelect_BRepOwner.hxx>

#include <QApplication>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <iostream>

OccWidget::OccWidget(QWidget* parent)
    : QWidget(parent)
    , myNextShapeId(1)
    , myViewInitialized(false)
{
    // 设置Qt widget属性
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
}
QPaintEngine* OccWidget::paintEngine() const
{
    return nullptr;
}

void OccWidget::paintEvent(QPaintEvent* event)
{
    if (!myView.IsNull()) {
        myView->Redraw();  // 或 myView->RedrawImmediate();
    }
}


OccWidget::~OccWidget()
{
    if (!myContext.IsNull()) {
        myContext->RemoveAll(Standard_False);
    }
}

void OccWidget::InitializeOcc()
{
    if (myViewInitialized) return;

    try {
        CreateViewer();
        CreateView();
        SetupInteraction();

        myViewInitialized = true;
        std::cout << "OccWidget initialized successfully" << std::endl;
        myViewer->Redraw();

    }
    catch (const Standard_Failure& e) {
        std::cerr << "Failed to initialize OccWidget: " << e.GetMessageString() << std::endl;
    }
}

void OccWidget::CreateViewer()
{
    // 创建显示连接
    myDisplayConnection = new Aspect_DisplayConnection();

    // 创建图形驱动
    myGraphicDriver = new OpenGl_GraphicDriver(myDisplayConnection);

    // 创建查看器
    myViewer = new V3d_Viewer(myGraphicDriver);

    // 设置默认光照
    myViewer->SetDefaultLights();
    myViewer->SetLightOn();

    // 创建交互上下文
    myContext = new AIS_InteractiveContext(myViewer);
}

void OccWidget::CreateView()
{
    // 创建窗口句柄
    Handle(Aspect_Window) window;

#ifdef _WIN32
    window = new WNT_Window((Aspect_Handle)winId());
#else
    window = new Xw_Window(myDisplayConnection, (Window)winId());
#endif

    // 创建视图
    myView = myViewer->CreateView();
    myView->SetWindow(window);

    if (!window->IsMapped()) {
        window->Map();
    }

    // 设置背景和显示
    myView->SetBackgroundColor(Quantity_NOC_GRAY30);
    myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_ZBUFFER);

    // 设置投影模式
    myView->Camera()->SetProjectionType(Graphic3d_Camera::Projection_Perspective);
}

void OccWidget::SetupInteraction()
{
    // 设置选择模式
    myContext->SetPixelTolerance(5);

    // 连接选择信号（注意：这是简化版本，实际中需要更复杂的处理）
    // connect(this, &OccWidget::ShapeSelected, this, &OccWidget::OnSelectionChanged);
}

int OccWidget::AddBox(double width, double height, double depth, double x, double y, double z)
{
    try {
        // 创建立方体
        BRepPrimAPI_MakeBox boxMaker(width, height, depth);
        TopoDS_Shape box = boxMaker.Shape();

        // 如果需要移动位置
        if (x != 0 || y != 0 || z != 0) {
            gp_Trsf transform;
            transform.SetTranslation(gp_Vec(x, y, z));
            BRepBuilderAPI_Transform transformer(box, transform);
            box = transformer.Shape();
        }

        // 创建可显示对象
        Handle(AIS_Shape) aisShape = new AIS_Shape(box);

        // 设置颜色
        Quantity_Color color(0.7, 0.5, 0.3, Quantity_TOC_RGB); // 棕色
        aisShape->SetColor(color);
        aisShape->SetDisplayMode(AIS_Shaded);

        // 显示对象
        myContext->Display(aisShape, Standard_True);

        // 保存到映射表
        int shapeId = myNextShapeId++;
        myShapes[shapeId] = aisShape;

        // 适配视图
        myView->FitAll();

        std::cout << "Box created with ID: " << shapeId << std::endl;
        return shapeId;

    }
    catch (const Standard_Failure& e) {
        std::cerr << "Failed to create box: " << e.GetMessageString() << std::endl;
        return -1;
    }
}

void OccWidget::ClearAll()
{
    if (!myContext.IsNull()) {
        myContext->RemoveAll(Standard_True);
        myShapes.clear();
        myNextShapeId = 1;
        std::cout << "All shapes cleared" << std::endl;
    }
}

//void OccWidget::paintEvent(QPaintEvent* /*event*/)
//{
//    if (!myView.IsNull()) {
//        myView->Redraw();
//    }
//}

void OccWidget::resizeEvent(QResizeEvent* /*event*/)
{
    if (!myView.IsNull()) {
        myView->MustBeResized();
    }
}

void OccWidget::mousePressEvent(QMouseEvent* event)
{
    if (myContext.IsNull()) return;

    myLastMousePos = event->pos();

    if (event->button() == Qt::LeftButton) {
        // 执行选择
        myContext->MoveTo(event->x(), event->y(), myView, Standard_True);
        myContext->Select(Standard_True);

        // 检查是否有选择的对象
        if (myContext->NbSelected() > 0) {
            // 遍历选择的对象，找到对应的ID
            for (myContext->InitSelected(); myContext->MoreSelected(); myContext->NextSelected()) {
                Handle(AIS_InteractiveObject) selectedObj = myContext->SelectedInteractive();

                // 查找对应的ID
                for (const auto& pair : myShapes) {
                    if (pair.second == selectedObj) {
                        std::cout << "Selected shape ID: " << pair.first << std::endl;
                        emit ShapeSelected(pair.first);
                        break;
                    }
                }
            }
        }
        else {
            std::cout << "No shape selected" << std::endl;
        }
    }

    QWidget::mousePressEvent(event);
}

void OccWidget::wheelEvent(QWheelEvent* event)
{
    if (myView.IsNull()) return;

    const double zoomFactor = 1.1;

    if (event->angleDelta().y() > 0) {
        // 放大
        myView->SetZoom(zoomFactor);
    }
    else {
        // 缩小
        myView->SetZoom(1.0 / zoomFactor);
    }

    QWidget::wheelEvent(event);
}

void OccWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (myView.IsNull()) return;

    if (event->buttons() & Qt::MiddleButton) {
        // 平移
        QPoint delta = event->pos() - myLastMousePos;
        myView->Pan(delta.x(), -delta.y());

    }
    else if (event->buttons() & Qt::RightButton) {
        // 旋转
        QPoint delta = event->pos() - myLastMousePos;
        myView->Turn(delta.x() * 0.01, delta.y() * 0.01);
    }

    myLastMousePos = event->pos();
    QWidget::mouseMoveEvent(event);
}

void OccWidget::OnSelectionChanged()
{
    // 处理选择变化的槽函数
    std::cout << "Selection changed" << std::endl;
}

#include "OccWidget.moc"