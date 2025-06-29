# 从 PythonOCC 到 C++ OpenCASCADE 教学计划
考虑到两位同学都熟悉python，这里我考虑从pythonocc入手先教会两位告诉两位occ编程的思考方式，然后通过对照python代码以及c++代码，学习如何在c++环境下使用occ开发。


---

## 📘 教学阶段概览

| 阶段 | 核心目标 | 教学形式 |
|------|----------|-----------|
| 阶段 0 | 环境搭建与测试 | Jupyter Notebook |
| 阶段 1 | BRep 原理与 Python 建模 | Notebook + 可视化 |
| 阶段 2 | Python 实战与结构分析 | Notebook + 项目实践 |
| 阶段 3 | 迁移到 C++ OCCT | C++ 工程 + 对照讲义 |
| 阶段 4 | C++ 工程化实战 | Qt/VTK 可视化开发 |

---

## 🗂️ 教学计划分节安排

### ✅ 第0讲：环境搭建与测试
- 使用 Anaconda 创建环境
- 安装 pythonocc-core、jupyter
- 测试 JupyterRenderer
- 输出一个 3D Box 测试窗口

---

### ✅ 第1讲：什么是 BRep？
- BRep（边界表示）原理介绍
- 几何 vs 拓扑
- BRep 层级结构：Vertex → Edge → Wire → Face → Solid

---