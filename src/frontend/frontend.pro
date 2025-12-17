# 项目类型：Qt Widgets应用程序
TEMPLATE = app
TARGET = HuffmanFrontend  # 生成的exe文件名
QT += core gui widgets charts  # 依赖的Qt模块（核心+界面+图表）

# 源文件路径（前端src目录下的所有源文件）
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/textencodedecodewindow.cpp \
    src/imageencodedecodewindow.cpp

# 头文件路径（前端include目录下的所有头文件）
HEADERS += \
    include/mainwindow.h \
    include/textencodedecodewindow.h \
    include/imageencodedecodewindow.h \
    ../backend/include/backend_api.h

# 头文件搜索路径（告诉编译器去哪里找头文件）
INCLUDEPATH += \
    $$PWD/include \
    $$PWD/../backend/include

# 库文件搜索路径和连接配置
LIBS += -L$$PWD/../backend -lbackend

# 编译选项（支持C++17，兼容后端代码）
QMAKE_CXXFLAGS += -std=c++17 -Wall  # -Wall显示警告信息（便于调试）

# 确保QT_CORE_LIB宏被定义，这样后端的Qt适配函数才能被编译
DEFINES += QT_CORE_LIB