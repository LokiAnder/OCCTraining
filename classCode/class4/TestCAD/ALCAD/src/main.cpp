#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

// OpenCASCADE 基础头文件
#include <BRepPrimAPI_MakeBox.hxx>
#include <TopoDS_Shape.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <Standard_Failure.hxx>

#include <iostream>

class SimpleCADWidget : public QWidget
{
public:
    SimpleCADWidget(QWidget* parent = nullptr) : QWidget(parent)
    {
        setupUI();
        createAndDisplayBlock();
    }

private:
    void setupUI()
    {
        auto* layout = new QVBoxLayout(this);

        // 标题
        auto* titleLabel = new QLabel("Qt + OpenCASCADE 简单测试", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 10px;");
        layout->addWidget(titleLabel);

        // 信息显示区域
        infoDisplay = new QTextEdit(this);
        infoDisplay->setReadOnly(true);
        infoDisplay->setMaximumHeight(200);
        infoDisplay->setStyleSheet(
            "QTextEdit {"
            "  background-color: #f8f9fa;"
            "  border: 1px solid #dee2e6;"
            "  padding: 10px;"
            "  font-family: 'Consolas', monospace;"
            "}"
        );
        layout->addWidget(infoDisplay);

        // 创建立方体按钮
        auto* createButton = new QPushButton("重新创建立方体", this);
        createButton->setStyleSheet(
            "QPushButton {"
            "  background-color: #007bff;"
            "  color: white;"
            "  border: none;"
            "  padding: 10px 20px;"
            "  font-size: 14px;"
            "  border-radius: 5px;"
            "}"
            "QPushButton:hover {"
            "  background-color: #0056b3;"
            "}"
        );
        layout->addWidget(createButton);

        // 连接按钮事件
        connect(createButton, &QPushButton::clicked, [this]() {
            createAndDisplayBlock();
            });

        setWindowTitle("Qt + OpenCASCADE 立方体测试");
        resize(400, 350);
    }

    void createAndDisplayBlock()
    {
        try {
            // 清空之前的信息
            infoDisplay->clear();

            addMessage("=== OpenCASCADE 立方体创建测试 ===");
            addMessage("");

            // 使用 OpenCASCADE 创建立方体
            addMessage("正在创建立方体...");
            TopoDS_Shape box = BRepPrimAPI_MakeBox(10.0, 5.0, 3.0).Shape();
            addMessage("✓ 立方体创建成功！");
            addMessage("");

            // 显示立方体信息
            addMessage("立方体参数:");
            addMessage("  长度 (X): 10.0 单位");
            addMessage("  宽度 (Y): 5.0 单位");
            addMessage("  高度 (Z): 3.0 单位");
            addMessage("");

            // 计算几何属性
            addMessage("正在计算几何属性...");
            GProp_GProps geometricProps;
            BRepGProp::VolumeProperties(box, geometricProps);

            // 获取体积
            double volume = geometricProps.Mass();
            addMessage(QString("✓ 体积计算完成: %1 立方单位").arg(volume));
            addMessage(QString("  理论体积: %1 立方单位").arg(10.0 * 5.0 * 3.0));

            // 获取重心
            gp_Pnt centerOfMass = geometricProps.CentreOfMass();
            addMessage("");
            addMessage("重心位置:");
            addMessage(QString("  X: %1").arg(centerOfMass.X()));
            addMessage(QString("  Y: %1").arg(centerOfMass.Y()));
            addMessage(QString("  Z: %1").arg(centerOfMass.Z()));

            // 计算表面积
            GProp_GProps surfaceProps;
            BRepGProp::SurfaceProperties(box, surfaceProps);
            double surfaceArea = surfaceProps.Mass();
            addMessage("");
            addMessage(QString("✓ 表面积: %1 平方单位").arg(surfaceArea));
            addMessage(QString("  理论表面积: %1 平方单位").arg(2 * (10 * 5 + 10 * 3 + 5 * 3)));

            addMessage("");
            addMessage("=== 测试完成 ===");
            addMessage("Qt 界面和 OpenCASCADE 几何引擎工作正常！");

            // 在控制台也输出信息
            std::cout << "立方体创建成功，体积: " << volume << std::endl;
            std::cout << "重心位置: (" << centerOfMass.X() << ", "
                << centerOfMass.Y() << ", " << centerOfMass.Z() << ")" << std::endl;

        }
        catch (const Standard_Failure& e) {
            addMessage("");
            addMessage("❌ 错误发生:");
            addMessage(QString("OpenCASCADE 错误: %1").arg(e.GetMessageString()));
            addMessage("");
            addMessage("请检查 OpenCASCADE 库是否正确安装和链接。");

            std::cout << "OpenCASCADE 错误: " << e.GetMessageString() << std::endl;
        }
        catch (const std::exception& e) {
            addMessage("");
            addMessage("❌ 系统错误:");
            addMessage(QString("标准异常: %1").arg(e.what()));

            std::cout << "系统错误: " << e.what() << std::endl;
        }
    }

    void addMessage(const QString& message)
    {
        infoDisplay->append(message);
        infoDisplay->ensureCursorVisible();
    }

private:
    QTextEdit* infoDisplay;
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    std::cout << "=== Qt + OpenCASCADE 简单测试程序启动 ===" << std::endl;
    std::cout << "Qt 版本: " << QT_VERSION_STR << std::endl;

    try {
        SimpleCADWidget widget;
        widget.show();

        std::cout << "程序界面已显示" << std::endl;
        std::cout << "OpenCASCADE 立方体测试已开始..." << std::endl;

        return app.exec();

    }
    catch (const std::exception& e) {
        std::cout << "程序启动失败: " << e.what() << std::endl;
        return -1;
    }
}