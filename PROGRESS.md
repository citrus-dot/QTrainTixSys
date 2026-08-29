# 列车客运售票管理系统 — 优化迭代计划（PROGRESS）

> 本文档记录系统优化迭代计划、实现细节与决策点。后续所有更新在新仓库 `dev` 分支上进行。

## 一、项目状态

| 项目 | 说明 |
|------|------|
| 当前版本 | V9（车票展示，已推送 dev） |
| 技术栈 | Qt 6.5+ Widgets / C++17 / CMake |
| 代码结构 | `Seat`/`Train`/`TrainSystem` 数据层 + `MainWindow` + 三个对话框（新增/售票/查询） |
| 开发分支 | 新仓库 `summer2026class1-213251605` 的 `dev` 分支 |
| 现有功能 | 班次增删、售票/退票、余票与可选座位查询、中间停靠、数据文件读写；V7 含日期/票价/座位等级与输入校验；V8 售票/退票座位可视化网格；V9 售票后车票展示与导出 |

## 二、优化目标

- 实用、可实现性高，不引入外部依赖（保持 Qt Widgets 原生）
- 兼顾"演示亮点"（座位图、车票）与"工程严谨"（校验、测试）
- 每次迭代独立提交到 `dev` 分支，可单独验证

## 三、迭代总览

| 迭代 | 版本 | 主题 | 类型 | 依赖 |
|------|------|------|------|------|
| 1 | V7 | 数据模型扩展与校验 | 数据层 | 无 |
| 2 | V8 | 座位可视化网格 | UI 亮点 | 迭代 1 |
| 3 | V9 | 车票展示 | UI 亮点 | 迭代 1 |
| 4 | V10 | 筛选与搜索 | 易用性 | 无 |
| 5 | V11 | 单元测试 | 工程 | 迭代 1 |
| 6 | V12 | 未保存提示 + 窗口记忆 | 工程细节 | 无 |

---

## 四、迭代详情

### 迭代 1（V7）：数据模型扩展与校验

**目标**：为车票/票价/座位等级打数据基础，并加强输入校验。

**涉及文件**：`train.h/cpp`、`trainsystem.h/cpp`、`addtraindialog.ui/cpp`、`querydialog.ui/cpp`、`mainwindow.cpp`、`testdata.dat`

**实现细节**：

1. `Train` 新增字段：
   - `QString m_date`（发车日期，如 `2026-08-27`）
   - `double m_firstClassPrice`、`double m_secondClassPrice`（一等/二等票价）
   - `QVector<int> m_carriageClass`（每车厢等级，1=一等 2=二等），长度 = 车厢数
   - 新增访问器与 `setCarriageClass()` 辅助方法
2. 新增方法：
   - `bool isSeatOccupied(int carriage, int seatNo) const`（供座位图用）
   - `double priceOf(int carriage) const`（按车厢等级返回票价）
   - `QString carriageClassText(int carriage) const`（"一等座"/"二等座"）
3. 数据校验（放在 `Train` 构造或 `TrainSystem::addTrain`）：
   - 发车时间格式 `HH:MM`（`QRegularExpression("^([01]\\d|2[0-3]):[0-5]\\d$")`）
   - 日期格式 `YYYY-MM-DD`
   - 票价 > 0
   - 身份证号：18 位（17 位数字 + 数字或 X/x），至少校验位数
4. 文件格式扩展：在班次行追加 `日期 一等票价 二等票价 各车厢等级...`，读取时对旧文件缺字段给默认值（向后兼容）。
5. `AddTrainDialog` 增加：日期输入（`QDateEdit`）、一等/二等票价（`QDoubleSpinBox`）、每车厢等级（下拉或按车厢设置）。
6. `QueryDialog` 增加票价与等级显示。

**决策点**：
- **D1 文件格式**：A. 保持空格分隔，仅追加新字段（建议，向后兼容）；B. 改为更健壮的 `|` 分隔。
- **D2 座位等级建模**：A. 按车厢分等级（建议，简单直观）；B. 按座位号范围；C. 每座独立。
- **D3 是否加日期字段**：A. 加（建议，车票更完整）；B. 不加。

**验收**：新增班次可填日期/票价/等级；非法时间/日期/票价被拦截；旧 `testdata.dat` 仍能打开。

---

### 迭代 2（V8）：座位可视化网格

**目标**：售票/退票对话框用座位网格图替代手动输入，直观展示座位状态。

**涉及文件**：`ticketdialog.ui/cpp/h`、`train.h/cpp`

**实现细节**：

1. `TicketDialog` 布局改造：
   - 顶部保留 售票/退票 单选 + 姓名/身份证输入
   - 中部：车厢选择 `QComboBox` + 座位网格 `QTableWidget`（每车厢一屏）
   - 底部：已选座位信息 + 确认按钮
2. 座位网格：
   - 行 = 每排座位（如每排 5 座），列 = 座位号
   - 已售座位：红色背景、禁用；空座：绿色背景、可点选
   - 点击空座 → 自动填充车厢号/座位号，高亮选中
3. 退票模式：只允许点击已售座位（红色），点击后填充车厢/座位号。
4. `Train` 增加按车厢查询：`QVector<Seat> seatsOfCarriage(int carriage) const` 或复用 `isSeatOccupied`。
5. 座位数较多时网格可滚动（`QScrollArea` 或表格自带滚动）。

**决策点**：
- **D4 网格实现**：A. `QTableWidget` 单元格着色（建议，简单）；B. `QGridLayout` + 自定义按钮。
- **D5 座位图位置**：A. 售票对话框内嵌（建议）；B. 独立座位图窗口。

**验收**：售票时能直观看到已售/空座；点击空座完成选座；退票时点击已售座位完成选座。

---

### 迭代 3（V9）：车票展示 —— 已完成并推送 dev

**目标**：售票成功后弹出"车票"，可查看并导出。

**涉及文件**：新增 `ticketviewdialog.h/cpp/ui`、`mainwindow.cpp`、`CMakeLists.txt`

**实现细节**：

1. 新增 `TicketViewDialog`（只读展示）：
   - 内容：日期、车次、发车时间、发车城市 → 终点城市、乘车人、身份证号、车厢号、座位号、等级、票价
   - 排版：`QFormLayout` + 大号字体，模拟车票样式
2. 售票成功路径：`MainWindow::onSellTicket` 在 `sellTicket` 成功后弹出车票对话框。
3. 可选：车票"保存为文本文件"按钮（`QFileDialog` 写出车票信息）。
4. 车票数据来源：售票时 `TicketDialog` 已持有姓名/身份证/车厢/座位，加上 `Train` 的日期/票价/等级。

**决策点**：
- **D6 车票是否需要"保存为文件"**：A. 加（建议，演示效果好）；B. 只展示不导出。
- **D7 车票是否含票价**：依赖迭代 1 是否做票价（D2/D3 联动）。

**验收**：售票成功弹出车票；车票信息与输入一致；可导出为文本。

---

### 迭代 4（V10）：筛选与搜索

**目标**：班次表格上方加搜索框，快速定位班次。

**涉及文件**：`mainwindow.ui/cpp/h`

**实现细节**：

1. 主窗口班次表格上方加 `QLineEdit`（placeholder："输入班次号/发车城市/终点城市筛选"）。
2. `textChanged` 信号触发 `refreshTrainTable()`，按关键字过滤：
   - 匹配班次号、发车城市、终点城市（`QString::contains`，不区分大小写）
3. 保持现有手动刷新模式，不引入 Model/View 重构（工程量小、风险低）。

**决策点**：
- **D8 筛选实现**：A. 手动过滤刷新（建议，符合当前架构）；B. `QSortFilterProxyModel`（需先做 Model/View 重构）。

**验收**：输入关键字后表格实时过滤；清空恢复全部。

---

### 迭代 5（V11）：单元测试

**目标**：为核心类写自动化测试，验证业务逻辑正确性。

**涉及文件**：新增 `tests/` 目录、`CMakeLists.txt`

**实现细节**：

1. 使用 Qt Test（`QTest`），新增 `tests/tst_core.cpp`：
   - `Seat`：设置/清除旅客、`isEmpty`
   - `Train`：售票成功/重复/越界、退票成功/空座/越界、余票计算、可选座位、按车厢等级票价
   - `TrainSystem`：新增/重复班次号、删除、查找、文件读写往返
2. CMake 配置：
   - `enable_testing()`
   - `qt_add_executable(tst_core tests/tst_core.cpp seat.cpp train.cpp trainsystem.cpp)`
   - `target_link_libraries(tst_core Qt::Test)`
   - `add_test(NAME tst_core COMMAND tst_core)`
3. 测试数据用临时文件（`QTemporaryDir`），不污染项目目录。

**决策点**：
- **D9 测试框架**：A. Qt Test（建议，与 Qt 集成好）；B. 简单 main + 断言。
- **D10 测试代码位置**：A. `tests/` 子目录（建议）；B. 与源码同目录。

**验收**：`ctest` 或 Qt Creator 运行全部测试通过。

---

### 迭代 6（V12）：未保存提示 + 窗口记忆

**目标**：防止误关丢失数据；记住窗口状态。

**涉及文件**：`mainwindow.h/cpp`、`main.cpp`

**实现细节**：

1. 未保存提示：
   - `MainWindow` 增加 `bool m_dirty` 标记，任何数据改动（增删班次、售票退票）置 true，保存/新建后置 false
   - 重写 `closeEvent()`：`m_dirty` 时弹 `QMessageBox`（保存/不保存/取消）
2. 窗口记忆：
   - `QSettings("SEU", "TrainSystem")` 保存窗口大小/位置
   - 保存上次打开的文件路径，启动时若存在则自动打开（可选）
3. `main.cpp` 在 `MainWindow` 构造后应用保存的几何信息。

**决策点**：
- **D11 是否自动打开上次文件**：A. 自动打开（建议，演示方便）；B. 不自动打开。

**验收**：有未保存改动时关闭弹提示；重启后窗口大小/位置恢复。

---

## 五、决策点汇总

| # | 决策点 | 选项 | 建议 |
|---|--------|------|------|
| D1 | 文件格式 | 空格分隔追加字段 / `\|` 分隔 | 空格分隔追加字段（向后兼容） |
| D2 | 座位等级建模 | 按车厢 / 按座位号范围 / 每座独立 | 按车厢分等级 |
| D3 | 是否加日期字段 | 加 / 不加 | 加 |
| D4 | 座位网格实现 | QTableWidget / QGridLayout+按钮 | QTableWidget |
| D5 | 座位图位置 | 售票对话框内嵌 / 独立窗口 | 内嵌 |
| D6 | 车票是否可导出 | 可导出 / 仅展示 | 可导出为文本 |
| D7 | 车票是否含票价 | 含 / 不含 | 含（联动 D2/D3） |
| D8 | 筛选实现 | 手动过滤 / 代理模型 | 手动过滤 |
| D9 | 测试框架 | Qt Test / 简单断言 | Qt Test |
| D10 | 测试代码位置 | tests/ 子目录 / 同目录 | tests/ 子目录 |
| D11 | 是否自动打开上次文件 | 自动 / 不自动 | 自动 |

## 六、分支与提交策略

- 所有迭代在 `dev` 分支开发并提交，`main` 分支保持稳定（受保护，需 MR 合并）
- 每次迭代一个提交，commit message 格式：`V7: 数据模型扩展与校验`（沿用 V1–V6 风格）
- 每次迭代完成先本地编译 + 手动验证，再 push 到 `dev`
- 迭代间相互独立，可随时暂停

## 七、后续可选（不在本次计划内）

- **Model/View 重构**（`QAbstractTableModel` + `QSortFilterProxyModel`）：表格数据自动驱动，工程量较大，收益偏架构
- **排序功能**：点击列头排序（依赖 Model/View）
- **多选批量操作**：批量售票/退票
