
## 🧩 0. 渲染黑屏问题分析：为何 3D 视图窗口会在失焦时变空白？

在第6讲的开发项目中，我们发现当 Qt 程序的主窗口失去焦点（例如点击其他应用）后，再返回窗口时，**嵌入的 3D 视图区域会变成黑色或空白**，无法正确显示已创建的 OCCT 模型。

这并不是你的程序崩溃或逻辑出错，而是一个 **OpenGL 与 Qt 渲染流程冲突** 的常见问题。

---

### 🧠 问题本质：Qt 与 OCCT 同时对控件绘制行为的干扰

在默认行为下，Qt 会试图在控件上绘制自己的背景或边框（通过 `QPaintEngine`）。
而我们的 `OccWidget` 实际上是将 OCCT 的 OpenGL 渲染上下文直接绑定到了该控件上，这意味着：

> **Qt 和 OCCT 会竞争对控件的“绘制控制权”**，尤其在失去焦点或被遮挡后重新渲染时，Qt 仍会尝试绘制，而 OpenGL 并未正确恢复上下文，导致画面空白。

---

### ✅ 正确做法：告诉 Qt “我不需要你来渲染”

解决方案是：**显式关闭 Qt 对该控件的绘图引擎支持**。  
方法是重载控件的 `paintEngine()` 函数，并返回 `nullptr`：

```cpp
QPaintEngine* OccWidget::paintEngine() const
{
    return nullptr;
}
```

这一行代码的含义是：
> 告诉 Qt不要试图为我生成 `QPaintEngine`（Qt 的绘图系统）

这样 Qt 就不会尝试在控件上进行任何图形绘制，从而避免了对 OpenGL 渲染结果的干扰。

---

### 🔍 参考背景：

- 这个问题首次遇到的解决思路是比较OpenCASCADE 的官方示例程序中的view.cpp,这里就明确添加了该函数来避免重绘干扰。

---




# ✅ 第7讲：从观察者模式理解 Qt 信号槽机制 + QSS 基础美化技巧

> 🎯 本讲目标：
> - 从经典设计模式切入，理解 Qt 的信号槽机制
> - 初步掌握 Qt 样式表 QSS 的使用方法，为界面美化做准备

---

## 🧠 1. 什么是观察者模式？（Observer Pattern）

### 📖 概念简述

观察者模式定义了对象之间**一对多的依赖关系**，当一个对象的状态发生变化时，所有依赖它的对象都会被自动通知。

就像公众号推送系统：
- 公众号（主题）发布新文章
- 所有订阅者（观察者）自动收到推送通知

---

### ✅ 观察者模式 C++ 示例代码

```cpp
#include <iostream>
#include <vector>
#include <string>

// 抽象观察者
class Observer {
public:
    virtual void update(const std::string& message) = 0;
    virtual ~Observer() {}
};

// 主题（被观察者）
class Subject {
public:
    void attach(Observer* obs) {
        observers.push_back(obs);
    }

    void detach(Observer* obs) {
        observers.erase(std::remove(observers.begin(), observers.end(), obs), observers.end());
    }

    void notify(const std::string& message) {
        for (Observer* obs : observers) {
            obs->update(message);
        }
    }

private:
    std::vector<Observer*> observers;
};

// 具体观察者A
class UserA : public Observer {
public:
    void update(const std::string& message) override {
        std::cout << "[UserA 收到消息] " << message << std::endl;
    }
};

// 具体观察者B
class UserB : public Observer {
public:
    void update(const std::string& message) override {
        std::cout << "[UserB 收到消息] " << message << std::endl;
    }
};

int main() {
    Subject wechat;         // 模拟一个公众号
    UserA user1;
    UserB user2;

    wechat.attach(&user1);
    wechat.attach(&user2);

    wechat.notify("新文章发布啦");

    wechat.detach(&user2);
    wechat.notify("更新新内容了");

    return 0;
}
```

**输出示例：**

```
[UserA 收到消息] 新文章发布啦
[UserB 收到消息] 新文章发布啦
[UserA 收到消息] 更新新内容啦
```

---

## 🚦 2. Qt 信号槽机制：观察者模式在 Qt 中的实现

Qt 通过 `QObject` + 元对象系统（`QMetaObject`）来实现观察者模式：

```cpp
connect(sender, SIGNAL(signal()), receiver, SLOT(slot()));
```

推荐写法（类型安全，支持 lambda）：

```cpp
connect(button, &QPushButton::clicked, this, &MainWindow::OnButtonClicked);
```

---

### 🔁 项目中信号槽的例子

```cpp
connect(myCreateBoxButton, &QPushButton::clicked, this, &MainWindow::OnCreateBox);
connect(myOccWidget, &OccWidget::ShapeSelected, this, &MainWindow::OnShapeSelected);
```

---

## 🧵 3. 自定义信号槽结构

#### `.h` 中定义：

```cpp
signals:
    void DataChanged(int newValue);

private slots:
    void HandleData(int value);
```

#### `.cpp` 中连接：

```cpp
connect(this, &MyClass::DataChanged, this, &MyClass::HandleData);
```

#### 发射信号：

```cpp
emit DataChanged(123);
```

---

## 🎨 4. 初识 QSS：用样式表美化你的 Qt 界面

### 📦 QSS 是什么？

Qt Style Sheet（QSS）是 Qt 提供的样式系统，语法类似 HTML 的 CSS。

### ✍️ 设置方式

```cpp
button->setStyleSheet("background-color: red; color: white;");
```

### ✅ 全局加载 `.qss` 文件

```cpp
QFile file(":/style/mytheme.qss");
file.open(QFile::ReadOnly);
qApp->setStyleSheet(file.readAll());
```

---

## 🧾 5. 常见 QSS 控件与属性



### 🖌️ 常用样式属性

| 属性              | 功能说明            |
|-------------------|---------------------|
| `background-color` | 背景色             |
| `color`            | 字体颜色            |
| `border`           | 边框样式            |
| `border-radius`    | 圆角半径            |
| `font-size`        | 字号                |
| `font-family`      | 字体                |
| `padding`          | 内边距              |
| `margin`           | 外边距              |

---

### 🌈 示例：按钮三态样式

```css
QPushButton {
    background-color: #3498db;
    color: white;
    border-radius: 6px;
    padding: 8px 16px;
}
QPushButton:hover {
    background-color: #2980b9;
}
QPushButton:pressed {
    background-color: #1c5980;
}
```

---

## 🧪 课后作业

### ✅ 基础练习

1. 自定义一个信号和槽，点击按钮触发 `qDebug()` 输出
2. 使用 `setStyleSheet` 美化一个按钮
3. 设置按钮三态颜色：默认 / 悬浮 / 按下

### 🚀 进阶挑战

1. 编写一个 `.qss` 文件并加载应用
2. 实现“Fusion360 风格”界面主题：蓝灰配色，扁平化设计
3. 使用 `QPushButton` 和 `QLabel` 搭配构建一个美观的工具栏
