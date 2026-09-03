# 列车客运售票管理系统 - V16（前端全面优化）

## 版本说明

V16 完成前端全面优化，界面从"功能可用"提升到"生产级软件"标准：

- **浅色现代扁平主题**：品牌蓝 `#2F6FED` + 中性灰 + 圆角卡片设计，统一全部控件风格
- **左侧导航栏**：品牌区 + 「班次管理」「查询」导航 + 「关于」，选中项高亮
- **顶部操作栏**：打开/保存 + 新增/删除/售票/退票 全部按钮化并配图标，核心功能不再藏在菜单栏
- **查询页**：原模态查询对话框整合为独立页面，随数据自动刷新
- **淡入动效**：所有对话框显示时 160ms 淡入，交互反馈自然
- **空状态引导**：无数据时显示操作引导文案（如"点击上方「新增班次」"）
- **座位可视化**：圆角座位按钮网格，空座绿/已售红/已选蓝三态区分
- **状态栏统计**：班次总数 / 余票总数 / 已售总数常驻显示

## 功能清单（截至 V16）

| 功能 | 版本 | 说明 |
|------|------|------|
| 新建/打开/保存数据文件 | 基础 | `.dat` 格式，可读写所有班次和旅客信息 |
| 新增班次 | 基础 | 输入班次号/日期/时间/城市/车厢/座位/票价/停靠站 |
| 删除班次 | 基础 | 删除选中班次 |
| 售票（座位可视化选座） | V8 | 绿色=空座，红色=已售；点击选座，支持按车厢切换 |
| 退票 | V8 | 点击已售座位退票 |
| 售票成功弹出车票 | V9 | 显示完整车票信息，可保存为 `.txt` 文件 |
| 班次搜索筛选 | V10 | 搜索框输入关键字（班次号/城市）实时过滤 |
| 查询余票和可选座位 | 基础 | 独立查询页，按车厢列出可选座位 |
| 输入校验 | V7 | 时间 HH:MM、日期 YYYY-MM-DD、身份证 18 位格式校验 |
| 文件格式向后兼容 | V7 | 新版可读取旧版无日期/票价的文件 |
| 单元测试（Qt Test + CTest） | V11 | 数据层 20 个测试用例全覆盖 |
| 组件化重构 | V12 | `TrainListView`/`SeatTableView` 独立组件 |
| 前端全面优化 | V16 | 浅色扁平主题、左侧导航栏、顶部操作栏、查询页、淡入动效、空状态引导、SVG 图标 |

## 代码结构

```
数据层：
  seat.h/cpp                座位类：存储旅客信息（姓名/身份证/车厢/座位号）
  train.h/cpp               班次类：班次信息 + 所有座位 + 售票/退票操作 + 输入校验
  trainsystem.h/cpp         系统类：管理所有班次 + 文件读写

UI 组件：
  sidebarwidget.h/cpp       左侧导航栏：品牌区 + 页面导航 + 关于
  trainlistview.h/cpp       班次列表组件：搜索框 + 表格 + 实时过滤 + 空状态引导
  seattableview.h/cpp       座位登记表组件：显示已售座位 + 空状态引导
  querypage.h/cpp/ui        查询页：余票/可选座位/停靠站展示
  fadedialog.h/cpp          对话框基类：显示时淡入动效
  mainwindow.h/cpp          主窗口：侧边栏 + 顶部操作栏 + 页面切换 + 调度
  mainwindow.ui             主窗口布局

对话框：
  addtraindialog.h/cpp/ui   新增班次对话框
  ticketdialog.h/cpp/ui     售票/退票对话框（含座位网格）
  ticketview.h/cpp/ui       车票展示对话框（可保存为 txt）

主题与资源：
  theme.qss                 全局主题（浅色现代扁平）
  resources.qrc             资源清单（图标 + 主题）
  icons/*.svg               9 个线性 SVG 图标

测试：
  tests/tst_train.cpp       Seat/Train 单元测试
  tests/tst_system.cpp      TrainSystem 单元测试 + 文件读写测试

构建：
  CMakeLists.txt            CMake 构建配置
  .gitignore               Git 忽略规则（build/ 等）

文档：
  README.md                本文件（版本说明 + 功能 + 文件说明 + 编译运行）
  PROGRESS.md              迭代计划与进度
  testdata.dat              示例测试数据
```

## 编译运行

```bash
# 配置（首次）
/Users/orange/Qt/Tools/CMake/CMake.app/Contents/bin/cmake -S . -B build -G Ninja \
  -DCMAKE_PREFIX_PATH=/Users/orange/Qt/6.11.2/macos \
  -DCMAKE_MAKE_PROGRAM=/Users/orange/Qt/Tools/Ninja/ninja

# 编译
/Users/orange/Qt/Tools/CMake/CMake.app/Contents/bin/cmake --build build

# 运行
open build/09025208_3.app

# 单元测试
ctest --test-dir build --output-on-failure
```

## 测试

1. **运行单元测试**：`ctest --test-dir build --output-on-failure`
2. **打开数据**：顶部「打开」→ `testdata.dat`
3. **新增班次**：顶部「新增班次」→ 填写信息 → 确定
4. **售票**：选中班次 → 顶部「售票」（或双击班次）→ 点击空座 → 填姓名身份证 → 确定
5. **退票**：选中班次 → 顶部「退票」→ 点击已售座位 → 确定
6. **查询**：左侧导航「查询」页，选择班次查看余票/可选座位/停靠站
7. **车票导出**：售票成功弹出车票 → 点"保存车票" → 保存为 .txt
