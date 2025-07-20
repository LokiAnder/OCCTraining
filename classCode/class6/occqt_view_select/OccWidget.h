#pragma once

#include <QWidget>
#include <QMouseEvent>

// OpenCASCADE 核心
#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <AIS_ViewController.hxx>

#ifdef _WIN32
#include <WNT_Window.hxx>
#else
#include <Xw_Window.hxx>
#endif

#include <TopoDS_Shape.hxx>
#include <map>

class OccWidget : public QWidget,protected AIS_ViewController
{
    Q_OBJECT

public:
    explicit OccWidget(QWidget* parent = nullptr);
    ~OccWidget();

    // 初始化3D视图
    void InitializeOcc();

    // 添加立方体并返回ID
    int AddBox(double width, double height, double depth, double x = 0, double y = 0, double z = 0);

    // 清除所有对象
    void ClearAll();
    virtual QPaintEngine* paintEngine() const;

protected:
    // Qt事件处理
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private slots:
    void OnSelectionChanged();

signals:
    void ShapeSelected(int shapeId);

private:
    // OCCT组件
    Handle(Aspect_DisplayConnection) myDisplayConnection;
    Handle(OpenGl_GraphicDriver) myGraphicDriver;
    Handle(V3d_Viewer) myViewer;
    Handle(V3d_View) myView;
    Handle(AIS_InteractiveContext) myContext;

    // 形状管理
    std::map<int, Handle(AIS_Shape)> myShapes;
    int myNextShapeId;

    // 交互状态
    bool myViewInitialized;
    QPoint myLastMousePos;

    // 初始化函数
    void CreateViewer();
    void CreateView();
    void SetupInteraction();
};