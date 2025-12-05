# Qt Charts 实时词频条形图 Demo

说明：这是一个最小的 Qt + Qt Charts Demo，使用随机数据模拟词频，并以条形图实时更新显示。演示文件未修改现有的 `UIwithPIC.cpp`。

文件：
- `demo_qt_charts.pro` - qmake 工程文件
- `main.cpp` - 程序入口
- `chartwidget.h` / `chartwidget.cpp` - 实现实时更新的条形图窗口

构建（Windows + MinGW64，假设您已经安装好 Qt 并把 `qmake` 和 `mingw32-make` 放入 PATH）：

在 PowerShell 中运行：

```powershell
cd "c:\Users\77955\Desktop\挑战课题-课程内设计数据结构"
qmake demo_qt_charts.pro
mingw32-make
.\QtChartsDemo.exe
```

注意事项：
- 需要安装 Qt 的 `Qt Charts` 模块（通常通过官方安装器选择 Qt Charts）。
- 如果使用 Qt Creator，可以直接打开 `demo_qt_charts.pro` 并以 MinGW kit 构建运行。

如果你希望我也生成一个 CMake 构建脚本，或把 demo 集成到现有工程里，请告诉我。
