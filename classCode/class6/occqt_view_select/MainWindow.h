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
    void OnCreateBox();
    void OnClearAll();
    void OnShapeSelected(int shapeId);

private:
    OccWidget* myOccWidget;
    QPushButton* myCreateBoxButton;
    QPushButton* myClearButton;
    QLabel* myStatusLabel;

    void SetupUI();
};