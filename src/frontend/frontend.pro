# 项目类型：Qt Widgets应用程序
TEMPLATE = app
TARGET = HuffmanFrontend  # 生成的exe文件名
QT += core gui widgets charts  # 依赖的Qt模块（核心+界面+图表）

# 源文件路径（前端src + 后端src）
SOURCES += \
    src/chartwidget.cpp \
    src/mainwindow.cpp \
    src/main.cpp \
    src/treenodeitem.cpp \
    src/treevisualizer.cpp \
    ../backend/src/HuffmanNode.cpp \
    ../backend/src/HuffmanTree.cpp 

# 头文件路径（前端include + 后端include）
HEADERS += \
    include/chartwidget.h \
    include/mainwindow.h \
    include/treenodeitem.h \
    include/treevisualizer.h \
    ../backend/include/EncodingUtils.h \
    ../backend/include/HuffmanNode.h \
    ../backend/include/HuffmanTree.h 

# 头文件搜索路径（告诉编译器去哪里找头文件，避免路径错误）
INCLUDEPATH += \
    $$PWD/include \
    $$PWD/../backend/include 

# 编译选项（支持C++17，兼容后端代码）
QMAKE_CXXFLAGS += -std=c++17 -Wall  # -Wall显示警告信息（便于调试）