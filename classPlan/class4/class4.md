# ✅ 第4讲：从 Python 到 C++ 的迁移

> 本讲目标：理解从 PythonOCC 到 C++ OCCT 的代码迁移方法，掌握两种语言的核心差异，为 C++ CAD 开发打下基础。

---

## 🔄 1. PythonOCC 和 C++ OCCT 是什么关系？

### 📖 简单来说

**PythonOCC** 就像是给 C++ OpenCASCADE 穿了一件 Python 的外衣。底层的几何计算、布尔运算都是同一套 C++ 代码在工作，只是调用的方式不同。

```
你写的 Python 代码 → PythonOCC 翻译 → C++ OCCT 执行 → 返回结果
```

就像你用中文点菜，服务员翻译成英文告诉厨师，厨师还是那个厨师，做的菜也一样。

### 🎯 核心对应关系

| Python OCC | C++ OCCT | 关系 |
|------------|----------|------|
| `from OCC.Core.gp import gp_Pnt` | `#include <gp_Pnt.hxx>` | 同一个点类 |
| `from OCC.Core.BRepPrimAPI import BRepPrimAPI_MakeBox` | `#include <BRepPrimAPI_MakeBox.hxx>` | 同一个立方体构建器 |

**Python 版本：**
```python
from OCC.Core.gp import gp_Pnt
from OCC.Core.BRepPrimAPI import BRepPrimAPI_MakeBox

# 创建一个立方体
point = gp_Pnt(0, 0, 0)
box = BRepPrimAPI_MakeBox(10, 20, 30).Shape()
```

**C++ 版本：**
```cpp
#include <gp_Pnt.hxx>
#include <BRepPrimAPI_MakeBox.hxx>

// 创建一个立方体
gp_Pnt point(0, 0, 0);
TopoDS_Shape box = BRepPrimAPI_MakeBox(10, 20, 30).Shape();
```

看！除了导入方式不同，其他几乎一模一样。

---

## 🔤 2. 主要差异在哪里？

### 💾 2.1 内存管理：最大的不同

#### Python：自动帮你管理
```python
# Python 会自动清理不用的对象，你不用管
box1 = BRepPrimAPI_MakeBox(10, 10, 10).Shape()
box2 = BRepPrimAPI_MakeBox(20, 20, 20).Shape()
# 当 box1, box2 不用时，Python 自动回收内存
```

#### C++：需要你自己注意
```cpp
// 方式1：在栈上创建（推荐简单情况）
BRepPrimAPI_MakeBox maker(10, 10, 10);
TopoDS_Shape box = maker.Shape();
// maker 会在作用域结束时自动销毁

// 方式2：使用智能指针（复杂情况）
auto maker = std::make_shared<BRepPrimAPI_MakeBox>(10, 10, 10);
TopoDS_Shape box = maker->Shape();
// 智能指针会自动管理内存
```

### ⚠️ 2.2 错误检查：C++ 需要更仔细

#### Python：出错会告诉你
```python
try:
    box = BRepPrimAPI_MakeBox(10, 20, 30)
    shape = box.Shape()  # 如果失败，Python 会抛异常
except Exception as e:
    print(f"出错了: {e}")
```

#### C++：需要主动检查
```cpp
try {
    BRepPrimAPI_MakeBox maker(10, 20, 30);
    
    // 必须检查是否成功
    if (maker.IsDone()) {
        TopoDS_Shape shape = maker.Shape();
        std::cout << "创建成功!" << std::endl;
    } else {
        std::cout << "创建失败!" << std::endl;
    }
} catch (const Standard_Failure& e) {
    std::cout << "OCCT错误: " << e.GetMessageString() << std::endl;
}
```

### 📦 2.3 导入 vs 包含

#### Python：import 导入
```python
# 运行时动态加载，很灵活
from OCC.Core.BRepPrimAPI import BRepPrimAPI_MakeBox
from OCC.Core.BRepAlgoAPI import BRepAlgoAPI_Fuse
```

#### C++：#include 包含
```cpp
// 编译时就要确定，需要提前知道用什么
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
```

---

## 🔄 3. 实际迁移例子

### 🎯 例子：创建两个立方体并合并
这里的c++测试环境可以参考class4中OpencascadeLearning.doc中介绍的方式创建occtoverview来测试。

#### Python 版本
```python
from OCC.Core.BRepPrimAPI import BRepPrimAPI_MakeBox
from OCC.Core.BRepAlgoAPI import BRepAlgoAPI_Fuse
from OCC.Core.gp import gp_Pnt

def create_merged_boxes():
    # 创建第一个立方体
    box1 = BRepPrimAPI_MakeBox(20, 20, 20).Shape()
    
    # 创建第二个立方体，位置偏移
    box2 = BRepPrimAPI_MakeBox(gp_Pnt(10, 10, 10), 20, 20, 20).Shape()
    
    # 合并两个立方体
    fusion = BRepAlgoAPI_Fuse(box1, box2)
    result = fusion.Shape()
    
    print("Python: 合并完成!")
    return result
```

#### C++ 版本
```cpp
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <gp_Pnt.hxx>
#include <iostream>

TopoDS_Shape createMergedBoxes() {
    try {
        // 创建第一个立方体
        BRepPrimAPI_MakeBox maker1(20, 20, 20);
        if (!maker1.IsDone()) {
            std::cout << "第一个立方体创建失败!" << std::endl;
            return TopoDS_Shape(); // 返回空形状
        }
        TopoDS_Shape box1 = maker1.Shape();
        
        // 创建第二个立方体，位置偏移
        BRepPrimAPI_MakeBox maker2(gp_Pnt(10, 10, 10), 20, 20, 20);
        if (!maker2.IsDone()) {
            std::cout << "第二个立方体创建失败!" << std::endl;
            return TopoDS_Shape();
        }
        TopoDS_Shape box2 = maker2.Shape();
        
        // 合并两个立方体
        BRepAlgoAPI_Fuse fusion(box1, box2);
        if (!fusion.IsDone()) {
            std::cout << "合并失败!" << std::endl;
            return TopoDS_Shape();
        }
        
        std::cout << "C++: 合并完成!" << std::endl;
        return fusion.Shape();
        
    } catch (const Standard_Failure& e) {
        std::cout << "出错了: " << e.GetMessageString() << std::endl;
        return TopoDS_Shape();
    }
}
```

### 🔍 对比发现的规律

1. **API 调用几乎相同**：`BRepPrimAPI_MakeBox`, `BRepAlgoAPI_Fuse` 用法一样
2. **C++ 需要更多检查**：每次操作后都要用 `IsDone()` 检查
3. **C++ 错误处理更复杂**：需要 try-catch + 状态检查
4. **C++ 类型更严格**：需要明确 `TopoDS_Shape` 类型





## 🎯 4. 迁移建议

### 📋 迁移步骤

#### 第一步：先学会基本对应关系
```python
# Python
from OCC.Core.BRepPrimAPI import BRepPrimAPI_MakeBox
```
对应
```cpp
// C++
#include <BRepPrimAPI_MakeBox.hxx>
```

#### 第二步：加上错误检查
```cpp
BRepPrimAPI_MakeBox maker(10, 10, 10);
if (maker.IsDone()) {
    // 成功时的处理
} else {
    // 失败时的处理
}
```

#### 第三步：处理内存管理
```cpp
// 简单情况：用栈对象
BRepPrimAPI_MakeBox maker(10, 10, 10);

// 复杂情况：用智能指针
auto maker = std::make_shared<BRepPrimAPI_MakeBox>(10, 10, 10);
```



## 🧪 课后作业

### 📝 基础练习

1. **🔄 代码转换**
   - 尝试将第2讲中创建花瓶的 Python 代码转换为 C++ 版本
   - 注意添加适当的错误检查


---

## 📋 下节预告

下节课我们将通过一个完整的实例项目，看看如何在实际开发中应用这些迁移知识，包括项目结构设计和开发流程。