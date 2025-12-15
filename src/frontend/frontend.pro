# 声明Qt依赖的模块（根据你的前端功能选）
QT       += core gui widgets charts

# 项目名称（生成的可执行文件会叫这个名字）
TARGET = HuffmanFrontend
# 项目类型是可执行程序
TEMPLATE = app

# ========== 前端源代码文件 ==========
SOURCES += \
    src/chartwidget.cpp \
    src/mainwindow.cpp \
    src/treenodeitem.cpp \
    src/treevisualizer.cpp \
    # 前端的main函数（你需要在frontend/src下创建main.cpp，用来启动程序）
    src/main.cpp

# ========== 前端头文件 ==========
HEADERS += \
    include/chartwidget.h \
    include/mainwindow.h \
    include/treenodeitem.h \
    include/treevisualizer.h

# ========== 后端代码（关联backend的头文件和源文件） ==========
# 包含backend的头文件路径（让前端能#include "HuffmanTree.h"）
INCLUDEPATH += \
    ../backend/include

# 后端的源文件（需要编译到项目里）
SOURCES += \
    ../backend/src/HuffmanNode.cpp \
    ../backend/src/HuffmanTree.cpp \
    ../backend/src/EncodingUtils.cpp  # 如果backend有这个文件的话

# 后端的头文件（可选，写在这里方便Qt Creator识别）
HEADERS += \
    ../backend/include/EncodingUtils.h \
    ../backend/include/HuffmanNode.h \
    ../backend/include/HuffmanTree.h