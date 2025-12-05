# 项目协作与任务分配

本仓库为“哈夫曼压缩与可视化”项目的代码与 demo。下面给出两位成员的工作分配（其中一位明确不负责前端），以及统一的 git 工作流与构建运行说明，便于多人协作和代码管理。

**Contributors**
- **成员 A（不做前端）**: 负责后端、算法与测试。
- **成员 B（负责前端）**: 负责 UI、可视化与部署。

**总体原则**
- **不修改**: 请不要直接修改 `UIwithPIC.cpp` 的原始逻辑（保持兼容）。前端演示请放到独立目录，如 `demo_qt_charts/`、`demo_huffman/`。
- **代码托管**: 所有改动通过分支 + Pull Request 合并，互相 code review。
- **编码与路径**: Windows 下避免在路径中使用非 ASCII 字符，避免 PowerShell 输出乱码（见下方编码说明）。

**职责分配（细项）**
- **成员 A（后端 / 算法）**:
  - **哈夫曼算法**: `HuffmanTree` 类重构、性能优化、序列化（`.huf` / `.phuf`）、位流打包与解包。
  - **文件 I/O**: 实现并测试 `LoadImageData` / `SaveImageData` 等；实现 `encodeImage`、`decodeImageFromBits` 等函数的健壮性。
  - **压缩效果统计**: 计算并展示压缩比、时空复杂度（用于报告）。
  - **单元测试**: 编写算法与 I/O 的单元测试（可用 Catch2 / GoogleTest），验证边界条件与正确性。
  - **CLI / 后端接口**: 提供可复用的命令行工具或库接口供前端调用。

- **成员 B（前端 / 可视化）**:
  - **Qt UI**: 负责 `demo_qt_charts`（词频实时条形图）与 `demo_huffman`（哈夫曼树构建可视化）相关的界面、事件与动画。
  - **可视化交互**: 缩放、提示、播放/暂停/步进、布局算法可视化优化（避免节点重叠，连线绘制美化）。
  - **打包与部署**: 使用 `windeployqt` 打包 exe 及依赖，准备演示用的 release 包。
  - **UI 性能**: 优化渲染/动画以保证交互流畅。

**如何把 `计划.md` 中的任务分配到两人（建议）**
- 文本与图片读取、统计、哈夫曼树构建、编码/解码、保存为 `.huf/.phuf`：成员 A。
- 将统计过程可视化为柱状图（动态词频）、可视化构建哈夫曼树、前端美化和交互：成员 B。
- 展示压缩前后比率、时空效率的可视化报表：成员 A 负责计算数据，成员 B 负责展示。

**Git 工作流与约定**
- **分支命名**: `feat/<area>-<short>`（新功能），`fix/<area>-<short>`（修复），`chore/<short>`（工具/文档）。
- **提交信息**: 采用简短前缀式，示例：`feat(huffman): add buildForText and encodeText`。
- **Pull Request**: 每次合并到 `main` 前必须创建 PR，由另一位成员至少 review 一次并 approve。
- **合并策略**: 通过 UI 合并（保留合并记录）或使用 rebase 后 fast-forward（两人约定）。
- **冲突处理**: 若两人同时改同一模块，优先沟通，必要时由作者 A/B 约定 owner 再行变更。

**分工示例任务表（从 `计划.md` 映射）**
- **成员 A**:
  - 实现：读取 txt/word 流式统计、图片字节统计（`getByteFrequencySorted`）、哈夫曼树类优化重构、文件保存/加载、解码流程。
  - 输出：提供稳定的编码表序列化格式（兼容旧版本），并给出解码接口。
- **成员 B**:
  - 实现：词频动态柱状图 demo（`demo_qt_charts`）、哈夫曼构建可视化（`demo_huffman`）、UI 美化与导出演示包。
  - 输出：可交互 demo、操作手册截图和发布包（包含 `windeployqt` 的依赖）。

**构建与运行（快捷命令示例，PowerShell）**
请使用官方 Qt 安装的 `qmake`（示例路径按实际修改）：

```powershell
& 'C:\Qt\6.10.1\mingw_64\bin\qmake.exe' demo_qt_charts.pro -spec win32-g++
mingw32-make
```

构建 Huffman demo:

```powershell
& 'C:\Qt\6.10.1\mingw_64\bin\qmake.exe' demo_huffman.pro -spec win32-g++
mingw32-make
```

部署（把运行时 dll 拷到 exe 目录）:

```powershell
& 'C:\Qt\6.10.1\mingw_64\bin\windeployqt.exe' --release path\to\HuffmanDemo.exe
```

**PowerShell 输出乱码临时修复（如遇到中文输出乱码）**
在当前 PowerShell 会话执行：

```powershell
chcp 65001
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8
```

为长期解决，建议使用官方 Qt 安装目录（避免 Anaconda 的 Qt）、或使用 MSYS2/MinGW64 shell，或在 Windows 区域设置中开启“使用 Unicode UTF-8 提供全球语言支持”。

**代码审查与 PR 模板（简要）**
- 每个 PR 请包含：目的、改动文件、如何复现、相关截图/日志（若有）。
- Review 要点：算法正确性、内存/多线程安全（特别是动画停止与 scene 清理）、资源释放、跨平台路径/编码处理。

**沟通与日常流程**
- 每日短会或在 Issue 上同步当日计划（建议 10-15 分钟）。
- 代码冲突或架构更改需提前讨论并在 Issue 留下决议记录。

**下一步（需要你们确认）**
- 请替换本文件开头的 `成员 A / 成员 B` 为实际姓名。
- 确认是否接受以上分工，或告知需要调整的任务边界。
- 需要我可以：
  - 直接把 `demo_*` 的构建脚本加入 `build.ps1`，或
  - 按分工生成 Issue 列表并创建对应分支模板。

已做：我在仓库中新增了 `src/backend/huffman_wrapper.h` 与 `src/backend/huffman_wrapper.cpp`（轻量转发封装），以及 `src/frontend/README_FRONTEND.md` 占位说明，目的是在不改动 `UIwithPIC.cpp` 的前提下，开始把前后端分离为独立目录。请确认是否继续把实现真正迁入 `src/backend/`（我可以接着把实现逐步移过去并提交 PR）。

**代码拆分任务分配（已指派）**

为避免二人重复冲突并让不做前端的成员专注后端实现，现将“前后端拆分（把 `UIwithPIC.cpp` 中的后端逻辑迁入 `src/backend/`）”任务指派给 **成员 A（不做前端）**。下面是细化的任务清单与执行步骤，供成员 A/B 遵循与验收。

-- **主要目标（由成员 A 完成）**
- 将 `UIwithPIC.cpp` 中与哈夫曼算法、编码/解码、图片 I/O、位流打包/解析、序列化相关的函数和类型，按模块提取到 `src/backend/`：
  1. `src/backend/huffman.h` / `huffman.cpp`：包含 `HuffmanTree`、`HuffmanNode`、编码/解码、序列化/反序列化接口。
  2. `src/backend/io_image.h` / `io_image.cpp`：图片加载/保存（BMP 与 GDI+ 支持）与字节频率统计函数（`LoadImageData`、`SaveImageData`、`getByteFrequencySorted`）。
  3. `src/backend/file_format.h` / `file_format.cpp`：`.huf` / `.phuf` 文件格式读写、头部/二进制区的解析与生成（包括 `CODE_TABLE_START` / `CODE_DATA_START` 的约定）。
  4. `src/backend/bitstream.h` / `bitstream.cpp`：位流读写工具（按位读写、缓存末位对齐/填充策略）。

-- **每个子任务的细化步骤（成员 A 执行）**
1) 在本地创建分支并推送：

```powershell
git checkout -b refactor/backend-split
```

2) 复制而非立即移动：先把相关代码从 `UIwithPIC.cpp` 复制到 `src/backend/*.cpp/.h`（保持原地代码不动，确保主分支无中断）。
  - 复制完成后，在 `src/backend` 中建立最小的构建支持（例如 `CMakeLists.txt` 或简单 Makefile），确保单独编译通过。

3) 在 `src/backend` 中实现公共接口：暴露稳定的 API（示例：`buildFromText`、`buildFromBytes`、`encodeText`、`decodeImageFromBits`、`serializeCodes`、`deserializeCodes`）。

4) 为每个迁移文件写单元测试（放到 `tests/backend/`）：
  - 测试用例包括：空输入、单字符/单字节、包含所有 0..255 字节值、异常或损坏的编码表、大小为 1 的图片等。
  - 建议使用 Catch2 或 GoogleTest（任选），在 Windows 下用 MinGW 或 MSVC 编译运行。

5) 本地验证：运行后端的单元测试并在命令行环境中多测试不同文件样例；修复明显内存或边界问题。

6) 替换步骤（渐进替换）：
  - 在 `UIwithPIC.cpp` 中逐步把调用改为封装调用（调用 `src/backend` 的 API），每次改动后都保证编译通过并手动运行基本流程。
  - 建议每个改动形成一个小 PR（例如：`refactor/backend-huffman-extract`），便于 review 与回退。

7) 最后：在所有后端功能稳定后，删除 `UIwithPIC.cpp` 中的重复实现（或把其移动到 `legacy/` 目录保留历史）。

-- **前端（成员 B）需要做的事项**
- 在成员 A 开始迁移后，成员 B 可以并行进行：
  1. 使用 `src/backend/huffman_wrapper.h`（当前已有的轻量转发）进行对接，或在后端 API 稳定后直接包含新的头文件并链接生成的库。
  2. 把 Qt demo（`demo_qt_charts`、`demo_huffman`）改为调用后端 API（不要直接复制算法实现到 front-end）。
  3. 做 UI-线程与后台任务的交互：使用信号/槽或消息回调确保后台计算不会在 UI 线程阻塞，测试文件加载/保存与进度更新。
  4. 更新 `build.ps1` / 打包脚本以按模块化结构构建并打包最终 exe（包含 `windeployqt` 步骤）。

-- **合并和验收标准（双方遵守）**
1) 每次将后端实现从 `UIwithPIC.cpp` 中移除或替换为后端调用前，必须保证：
  - 项目能在本地构建通过（至少后端库和 Qt demo 单元）。
  - 所有后端单元测试通过。
  - 至少一次手动运行关键功能（文本编码生成、图片编码/解码、保存/加载）。

2) PR 要包含：迁移清单、测试步骤、影响范围、回退说明。

3) 推荐 reviewer：对后端实现经验更熟悉的成员（如果 Member B 不熟悉某些底层细节，Member A 应承担主要 reviewer 角色）。

-- **时间与里程碑建议**
- 第一天（分支建立 + 复制模块 + 搭建 `src/backend` 构建）
- 第二天（完成 `huffman` 模块提取 + 基本单元测试）
- 第三天（完成图片 I/O 与 bitstream 工具 + 集成测试）
- 第四天（前端接入测试 + 打包）

如果你同意这份指派，我会把 README 中“成员 A / 成员 B”替换为你们的真实姓名，并把上面的细化任务项转换为仓库 Issue（并在 `refactor/backend-split` 分支上提交第一次微小迁移）。


---
文件位置：项目根目录 `README.md`。
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
