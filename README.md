# 列车客运售票管理系统 - V18（统计可视化 + 仿真车票 + 功能补齐）

## 版本说明

V18（v0.3.0）补齐 V13–V15 搁置功能：修改班次、未保存提示、窗口几何与上次文件记忆、班次列表列头排序、身份证脱敏显示统一。V17 在 V16 前端优化基础上，完成数据可视化与票面仿真升级：

- **统计页环形图**（`DonutChart`）：余票/已售分布自绘环形图，入场顺时针逐段展开动画、悬停弹性加粗（OutBack 缓动 + 逐段强度插值，无跳变）、注释线引出至就近角落
- **仿真磁介质车票**（`TicketCard`）：参照 12306 真票样式全 QPainter 自绘——浅蓝底纹、红字票号、站名拼音、动车组剪影水印、齿孔撕边副联区、伪二维码；含订单号、支付方式/流水/时间等凭据，可保存为电子客票格式 txt
- **全局调色板**（`palette.h`）：统一色值常量（品牌蓝/危险红/成功绿等），消除同义异值散落
- **表格工具**（`tableutil.h`）：居中条目、按内容权重分配列宽、清空布局，三处表格复用
- **切页过渡动画**：260ms 淡入 + 8px 上浮，快速连点侧边栏时以最后一次点击为准
- **浅色现代扁平主题**：品牌蓝 `#2F6FED` + 中性灰 + 圆角卡片，QSS 统一全部控件风格
- **左侧导航栏**：品牌区 + 功能导航 + 信息卡 + 版本号，选中项高亮
- **顶部操作栏**：打开/保存 + 新增/删除/售票/退票/座位登记全部按钮化并配图标
- **座位可视化**：圆角座位按钮网格，空座绿/已售红/已选蓝三态；一等座金色描边
- **对话框淡入**：所有对话框显示时 200ms 淡入（`FadeDialog` 基类）
- **状态栏统计**：班次总数 / 余票总数 / 已售总数常驻显示

## 功能清单

| 功能 | 版本 | 说明 |
|------|------|------|
| 新建/打开/保存数据文件 | 基础 | `.dat` 格式，可读写所有班次和旅客信息 |
| 新增班次 | 基础 | 输入班次号/日期/时间/城市/车厢/座位/票价/停靠站 |
| 删除班次 | 基础 | 删除选中班次 |
| 修改班次 | V18 | 选中班次 →「修改班次」，改号查重、已售座位超新编组时拦截 |
| 未保存提示 + 窗口记忆 | V18 | 关闭时保存/不保存/取消；重启恢复窗口几何并自动打开上次文件 |
| 班次列表排序与身份证脱敏 | V18 | 列头点击排序（数值列按数值比较）；界面与导出身份证打码，数据文件存完整号 |
| 售票（座位可视化选座） | V8 | 绿色=空座，红色=已售；点击选座，支持按车厢切换 |
| 退票 | V8 | 点击已售座位退票 |
| 售票成功弹出车票 | V9/V17 | 仿真磁介质票面，可保存为含订单/支付凭据的 `.txt` |
| 班次搜索筛选 | V10 | 搜索框输入关键字（班次号/城市）实时过滤 |
| 查询余票和可选座位 | 基础 | 独立查询页，按车厢列出可选座位 |
| 统计可视化 | V17 | 余票分布环形图（动画+悬停交互）+ 座位等级分布条 + 班次表格 |
| 输入校验 | V7 | 时间 HH:MM、日期 YYYY-MM-DD、身份证 18 位格式校验 |
| 文件格式向后兼容 | V7 | 新版可读取旧版无日期/票价的文件 |
| 单元测试（Qt Test + CTest） | V11 | 数据层测试用例（含旧格式往返回归） |
| 组件化重构 | V12 | `TrainListView`/`SeatTableView` 独立组件 |
| 前端全面优化 | V16 | 浅色扁平主题、左侧导航栏、顶部操作栏、查询页、淡入动效、空状态引导 |

## 代码结构

```
数据层：
  seat.h/cpp                座位类：存储旅客信息（姓名/身份证/车厢/座位号）
  train.h/cpp               班次类：班次信息 + 所有座位 + 售票/退票操作 + 输入校验
  trainsystem.h/cpp         系统类：管理所有班次 + 文件读写（旧格式向后兼容）

UI 组件：
  sidebarwidget.h/cpp       左侧导航栏：品牌区 + 页面导航 + 信息卡 + 关于
  trainlistview.h/cpp       班次列表组件：搜索框 + 表格 + 实时过滤 + 空状态引导
  seattableview.h/cpp       座位登记表组件：显示已售座位 + 空状态引导
  seattabledialog.h/cpp     座位登记弹窗（承载 SeatTableView）
  seatmappopup.h/cpp        座位周边分布浮窗（查询页可选座位点击）
  querypage.h/cpp/ui        查询页：余票/可选座位/停靠站展示
  statisticspage.h/cpp      统计页：环形图 + 等级分布条 + 班次表格
  donutchart.h/cpp          自绘环形图：入场动画 + 弹性悬停 + 注释线
  mainwindow.h/cpp          主窗口：侧边栏 + 顶部操作栏 + 页面切换 + 调度
  mainwindow.ui             主窗口布局
  fadedialog.h/cpp          对话框基类：显示时淡入动效

对话框：
  addtraindialog.h/cpp/ui   新增班次对话框
  ticketdialog.h/cpp/ui     售票/退票对话框（含座位网格）
  ticketview.h/cpp/ui       车票展示对话框（承载 TicketCard，可保存 txt）
  ticketcard.h/cpp          仿真磁介质车票自绘卡片（含订单/支付凭据）
  registerdialog.h/cpp      乘机人/旅客信息登记对话框
  selldialog.h/cpp          售票入口对话框
  refunddialog.h/cpp        退票入口对话框

公共工具：
  palette.h                 全局色值常量（namespace Palette）
  tableutil.h               表格 UI 工具（居中条目/权重列宽/清空布局）

主题与资源：
  theme.qss                 全局主题（浅色现代扁平）
  resources.qrc             资源清单（图标 + 主题）
  icons/*.svg               线性 SVG 图标（含悬停白色变体）

测试：
  tests/tst_train.cpp       Seat/Train 单元测试
  tests/tst_system.cpp      TrainSystem 单元测试 + 文件读写往返测试

构建：
  CMakeLists.txt            CMake 构建配置（含两个测试目标）
  .gitignore                Git 忽略规则（build/ 等）

文档与数据：
  README.md                 本文件
  PROGRESS.md               迭代计划与进度
  testdata.dat              示例测试数据（5 班次，覆盖边界用例）
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
open build/列车售票系统.app

# 单元测试
ctest --test-dir build --output-on-failure
```

## 快速上手

1. **运行单元测试**：`ctest --test-dir build --output-on-failure`
2. **打开数据**：顶部「打开」→ `testdata.dat`
3. **新增班次**：顶部「新增班次」→ 填写信息 → 确定
4. **售票**：选中班次 → 顶部「售票」（或双击班次）→ 点击空座 → 填姓名身份证 → 确定 → 弹出仿真车票可保存
5. **退票**：选中班次 → 顶部「退票」→ 点击已售座位 → 确定
6. **查询**：左侧导航「查询」页，选择班次查看余票/可选座位/停靠站
7. **统计**：左侧导航「统计」页，查看余票分布环形图与等级分布
