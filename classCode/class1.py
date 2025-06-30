import tkinter as tk
from OCC.Display.SimpleGui import init_display
from OCC.Core.gp import gp_Pnt
from OCC.Core.BRepBuilderAPI import (
    BRepBuilderAPI_MakeEdge,
    BRepBuilderAPI_MakeWire,
    BRepBuilderAPI_MakeFace,
    BRepBuilderAPI_Sewing,
    BRepBuilderAPI_MakeSolid,
)

# 初始化 tkinter 后端
display, start_display, add_menu, add_function_to_menu = init_display("tk")

# 模拟 BRep 建模流程状态
build_stage = 0
shape_data = {}

# 每次回车触发一步建模
def on_key_press():
    global build_stage, shape_data
    display.EraseAll()
    
    dx, dy, dz = 100, 60, 30
    
    def make_face(p1, p2, p3, p4):
        e1 = BRepBuilderAPI_MakeEdge(p1, p2).Edge()
        e2 = BRepBuilderAPI_MakeEdge(p2, p3).Edge()
        e3 = BRepBuilderAPI_MakeEdge(p3, p4).Edge()
        e4 = BRepBuilderAPI_MakeEdge(p4, p1).Edge()
        wire = BRepBuilderAPI_MakeWire(e1, e2, e3, e4).Wire()
        return BRepBuilderAPI_MakeFace(wire).Face()
    
    if build_stage == 0:
        # 顶点阶段
        print("Step 0: 初始化顶点")
        shape_data['p0'] = gp_Pnt(0, 0, 0)
        shape_data['p1'] = gp_Pnt(dx, 0, 0)
        shape_data['p2'] = gp_Pnt(dx, dy, 0)
        shape_data['p3'] = gp_Pnt(0, dy, 0)
        shape_data['p4'] = gp_Pnt(0, 0, dz)
        shape_data['p5'] = gp_Pnt(dx, 0, dz)
        shape_data['p6'] = gp_Pnt(dx, dy, dz)
        shape_data['p7'] = gp_Pnt(0, dy, dz)
        print("点击菜单 'Next Step' 进入下一步：底面")
            
    elif build_stage == 1:
        # 底面
        print("Step 1: 构建底面")
        face = make_face(shape_data['p0'], shape_data['p1'], shape_data['p2'], shape_data['p3'])
        shape_data['face_bottom'] = face
        display.DisplayShape(face, update=True)
        print("点击菜单 'Next Step' 进入下一步：顶面")
    
    elif build_stage == 2:
        # 顶面
        print("Step 2: 构建顶面")
        face = make_face(shape_data['p4'], shape_data['p5'], shape_data['p6'], shape_data['p7'])
        shape_data['face_top'] = face
        display.DisplayShape(face, update=True)
        print("点击菜单 'Next Step' 进入下一步：四个侧面")
    
    elif build_stage == 3:
        print("Step 3: 构建四个侧面")
        f1 = make_face(shape_data['p0'], shape_data['p1'], shape_data['p5'], shape_data['p4'])  # front
        f2 = make_face(shape_data['p1'], shape_data['p2'], shape_data['p6'], shape_data['p5'])  # right
        f3 = make_face(shape_data['p2'], shape_data['p3'], shape_data['p7'], shape_data['p6'])  # back
        f4 = make_face(shape_data['p3'], shape_data['p0'], shape_data['p4'], shape_data['p7'])  # left
        shape_data['sides'] = [f1, f2, f3, f4]
        for f in [f1, f2, f3, f4]:
            display.DisplayShape(f, update=False)
        display.DisplayShape(shape_data['face_bottom'], update=False)
        display.DisplayShape(shape_data['face_top'], update=True)
        print("点击菜单 'Next Step' 进入下一步：缝合为Shell")
    
    elif build_stage == 4:
        print("Step 4: 缝合为 Shell")
        sewing = BRepBuilderAPI_Sewing()
        for f in shape_data['sides']:
            sewing.Add(f)
        sewing.Add(shape_data['face_bottom'])
        sewing.Add(shape_data['face_top'])
        sewing.Perform()
        shell = sewing.SewedShape()
        shape_data['shell'] = shell
        display.DisplayShape(shell, update=True)
        print("点击菜单 'Next Step' 进入最后一步：构建 Solid")
    
    elif build_stage == 5:
        print("Step 5: Shell → Solid")
        solid = BRepBuilderAPI_MakeSolid(shape_data['shell']).Solid()
        display.DisplayShape(solid, update=True)
        print("✅ 完成：Solid 实体已构建！")
    
    else:
        print("🔁 所有步骤已完成。")
    
    build_stage += 1

# 添加菜单项来触发下一步
add_menu('BRep Demo')
add_function_to_menu('BRep Demo', on_key_press)

# 启动显示窗口
print("程序启动成功。请点击菜单 'BRep Demo' -> 'on_key_press'，逐步执行 BRep 构造。")
start_display()