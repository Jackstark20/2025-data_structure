前端（Qt）占位目录

本目录用于放置 Qt 前端工程：`demo_qt_charts`、`demo_huffman` 等。

说明：目前后端逻辑仍存在于 `UIwithPIC.cpp`；为了平滑迁移，我们添加了 `src/backend` 的包装接口，前端应引用 `src/backend` 中的头文件以便逐步替换原始实现。
