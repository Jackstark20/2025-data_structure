# Huffman Tree Visualization Demo (Qt)

这是一个使用 Qt Widgets 和 QGraphicsView 实现的 Huffman 树构建过程可视化 Demo。

构建与运行（假设已安装 Qt 并且 `qmake` 指向你要用的版本）：

```powershell
cd "c:\Users\77955\Desktop\挑战课题-课程内设计数据结构"
& 'C:\Qt\6.10.1\mingw_64\bin\qmake.exe' -spec win32-g++ demo_huffman.pro
& 'C:\Qt\Tools\mingw1120_64\bin\mingw32-make.exe'
& 'C:\Qt\6.10.1\mingw_64\bin\windeployqt.exe' --release .\release\HuffmanDemo.exe
.\release\HuffmanDemo.exe
```

功能简介：
- 初始示例数据在 `HuffmanWidget` 中定义（可修改为自定义输入）。
- 支持 Step / Play / Reset 控制，自动或手动逐步可视化合并最小两个节点的过程。
