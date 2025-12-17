# 项目类型：Qt Widgets应用程序
TEMPLATE = app
TARGET = HuffmanFrontend  # 生成的exe文件名
QT += core gui widgets charts  # 依赖的Qt模块（核心+界面+图表）

# 源文件路径（仅前端src）
SOURCES += \
    src/mainwindow.cpp \
    src/main.cpp 

# 头文件路径（前端include + 后端include）
HEADERS += \
    include/mainwindow.h \
    ../backend/include/backend_api.h 

# 头文件搜索路径（告诉编译器去哪里找头文件，避免路径错误）
INCLUDEPATH += \
    $$PWD/include \
    $$PWD/../backend/include 

# 库文件搜索路径
LIBS += -L$$PWD/../backend -lbackend

# 编译选项（支持C++17，兼容后端代码）
QMAKE_CXXFLAGS += -std=c++17 -Wall  # -Wall显示警告信息（便于调试）

# 确保QT_CORE_LIB宏被定义，这样后端的Qt适配函数才能被编译
DEFINES += QT_CORE_LIB