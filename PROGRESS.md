# 列车客运售票管理系统 — 优化迭代计划（PROGRESS）

> 本文档记录系统优化迭代计划、实现细节与决策点。后续所有更新在新仓库 `dev` 分支上进行。

## 一、项目状态

| 项目 | 说明 |
|------|------|
| 当前版本 | V12（结构重构，已推送 dev） |
| 技术栈 | Qt 6.5+ Widgets / C++17 / CMake |
| 代码结构 | `Seat`/`Train`/`TrainSystem` 数据层 + `MainWindow` + 四个对话框（新增/售票/车票/查询）；V12 后新增 `TrainListView`/`SeatTableView` 两个组件 |
| 开发分支 | 新仓库 `summer2026class1-213251605` 的 `dev` 分支 |
| 现有功能 | 班次增删、售票/退票、余票与可选座位查询、中间停靠、数据文件读写；V7 含日期/票价/座位等级与输入校验；V8 售票/退票座位可视化网格；V9 售票后车票展示与导出；V10 班次筛选与搜索；审查精简：座位登记表仅显示已售、查询可选座位按车厢显示；V11 数据层单元测试（Qt Test + CTest）；V12 组件化重构（`TrainListView`/`SeatTableView`），MainWindow 精简 |

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
| 6 | V12 | 结构重构（组件化） | 代码质量 | 迭代 5 |
| 7 | V13 | 快捷操作与统计 | 易用性 | 迭代 6 |
| 8 | V14 | 修改班次 | 功能补齐 | 迭代 5、6 |
| 9 | V15 | 未保存提示 + 窗口记忆 | 工程细节 | 迭代 8 |

> 扩展原则（2026-09-02）：测试先行（重构前建安全网）；一个功能对应一次提交，每次提交独立可验证；功能求实用，代码求简单易懂。

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

### 迭代 4（V10）：筛选与搜索 —— 已完成并推送 dev

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

### 代码审查与精简（V10 后维护，已推送 dev）

**目标**：修复细节问题、删除死代码、提升实用性与可读性。

**改动**：

1. 删除死代码 `Train::setPrices()`（声明/定义均无调用）。
2. `Train::availableSeats()` 改为 `availableSeats(int carriage)`：按车厢返回可选座位，消除跨车厢座位号重复歧义。
3. `QueryDialog` 可选座位按车厢显示（"X号车厢 Y号座"）。
4. 座位登记表只显示已售座位，不再铺满空座（大班次下表格更实用）。
5. 新建/打开文件时清空搜索框，避免旧关键字过滤新数据。
6. `AddTrainDialog` 增加发车/终点城市非空校验。

**验收**：编译通过；启动正常；查询可选座位带车厢信息；座位登记表仅含已售记录。

---

### 迭代 5（V11）：单元测试 —— 已完成并推送 dev

**目标**：为数据层建立自动化测试安全网（V12 重构、V14 修改班次都依赖它）。

**涉及文件**：新增 `tests/tst_train.cpp`、`tests/tst_system.cpp`、`CMakeLists.txt`

**提交拆分**：

1. **提交 1 `V11: 核心类单元测试`** — `tests/tst_train.cpp`
   - `Seat`：构造/`isEmpty`/`setPassenger`/`clearPassenger`
   - `Train`：售票成功/座位重复/车厢座位越界、退票成功/空座退票/越界、余票计算、`availableSeats(carriage)` 按厢查询、`priceOf`/`carriageClassText` 等级票价、`isValidTime`/`isValidDate`/`isValidId` 校验
2. **提交 2 `V11: 系统类单元测试与CTest`** — `tests/tst_system.cpp` + CMake
   - `TrainSystem`：新增/班次号重复、删除、`findTrain`
   - 文件读写往返（`QTemporaryDir` 临时文件，不污染项目目录）：保存后重新加载逐字段一致
   - 旧格式文件（无日期/票价字段）向后兼容加载
   - CMake：`enable_testing()` + `qt_add_executable` 两个测试目标 + `add_test`

**执行步骤**：

- 步骤 1：创建 `tests/` 目录与 `tests/tst_train.cpp` 骨架（`QTEST_MAIN` + 空测试类，先保证可编译）
- 步骤 2：编写 `Seat` 测试用例（构造、`isEmpty`、`setPassenger`、`clearPassenger`）
- 步骤 3：编写 `Train` 售票/退票测试（成功、重复、越界、空座退票）
- 步骤 4：编写 `Train` 余票/按厢可选座位/等级票价测试
- 步骤 5：编写 `Train` 静态校验测试（`isValidTime`/`isValidDate`/`isValidId`）
- 步骤 6：编译并运行 `tst_train`，全部通过后提交 1
- 步骤 7：编写 `tests/tst_system.cpp`（增删查、文件读写往返、旧格式兼容）
- 步骤 8：更新 `CMakeLists.txt`：`enable_testing()` + 两个测试目标 + `add_test`
- 步骤 9：编译并运行 `ctest`，全部通过后提交 2

**决策点**：
- **D9 测试框架**：A. Qt Test（建议，与 Qt 集成好）；B. 简单 main + 断言。
- **D10 测试代码位置**：A. `tests/` 子目录（建议）；B. 与源码同目录。

**验收**：`ctest` 全部通过；测试不依赖 GUI（不链接 Widgets）。

---

### 迭代 6（V12）：结构重构（组件化）

**目标**：MainWindow 瘦身——把两个表格区块封装为独立组件，MainWindow 只做菜单/文件/对话框调度。每步重构后跑 V11 测试 + 手动冒烟。

**涉及文件**：新增 `trainlistview.h/cpp`、`seattableview.h/cpp`；改 `mainwindow.h/cpp/ui`、`CMakeLists.txt`

**提交拆分**：

1. **提交 1 `V12: 抽取班次列表组件`** — `trainlistview.h/cpp`
   - `TrainListView(QWidget)`：内含搜索框 + 班次表格 + 过滤刷新逻辑
   - 对外接口：`setTrains(const QVector<Train>&)`、`currentTrainNo()`
   - 对外信号：`trainSelected(const QString &no)`（选中变化通知 MainWindow）
   - MainWindow 中对应的成员与刷新逻辑迁移进来
2. **提交 2 `V12: 抽取座位登记组件`** — `seattableview.h/cpp`
   - `SeatTableView(QWidget)`：内含座位表格与已售座位填充逻辑
   - 对外接口：`setTrain(const Train*)`（nullptr 清空）
3. **提交 3 `V12: MainWindow 收尾精简`**
   - 颜色/尺寸等常量集中定义（如 `style.h` 或组件内 `constexpr`）
   - 删除迁移后的死代码，MainWindow 最终只保留：文件操作、菜单/工具栏接线、对话框调度
   - 全量验证：ctest + 手动过一遍 V7–V10 功能清单

**执行步骤**：

- 步骤 1：创建 `trainlistview.h/cpp`（搜索框 + 班次表格成员，先搭 UI 骨架）
- 步骤 2：迁移过滤刷新逻辑到 `TrainListView::setTrains()`，暴露 `currentTrainNo()` 与 `trainSelected` 信号
- 步骤 3：MainWindow 替换为 `TrainListView` 成员，接线选中信号，删除旧刷新代码
- 步骤 4：编译 + ctest + 手动验证搜索/选中/售票，通过后提交 1
- 步骤 5：创建 `seattableview.h/cpp`（座位表格成员 + `setTrain(const Train*)` 填充已售座位）
- 步骤 6：MainWindow 替换为 `SeatTableView` 成员，删除旧刷新代码
- 步骤 7：编译 + ctest + 手动验证座位登记，通过后提交 2
- 步骤 8：常量集中定义、清理死代码与多余 include
- 步骤 9：全量验证（ctest + V7–V10 功能清单手动过一遍），通过后提交 3

**决策点**：
- **D12 拆分方式**：A. 组件化拆分（建议，Qt 惯用组合方式）；B. 调度层抽取（Controller，偏重）；C. 仅函数级拆短。

**验收**：MainWindow 代码量明显下降；全部现有功能不变；ctest 通过。

---

### 迭代 7（V13）：快捷操作与统计

**目标**：缩短操作路径、增强全局信息感知。每项独立提交、独立可验证。

**涉及文件**：`mainwindow.ui/cpp`、`train.h/cpp`、`ticketview.cpp`

**提交拆分**：

1. **提交 1 `V13: 工具栏补全业务按钮`**
   - 工具栏加入 新增/删除/售票/退票/查询（现有"打开/保存"保留），复用现有 QAction
2. **提交 2 `V13: 双击班次直接售票`**
   - 班次表格 `cellDoubleClicked` → 打开售票对话框（双击行不再需要先点菜单）
3. **提交 3 `V13: 班次表列头排序`**
   - `trainTable->setSortingEnabled(true)`；刷新时用 `setSortingEnabled(false)` 包裹避免插入错乱
4. **提交 4 `V13: 状态栏统计`**
   - 状态栏常驻显示：班次总数 / 余票总数 / 已售票总数（数据变化时同步刷新）
5. **提交 5 `V13: 身份证号脱敏显示`**
   - 新增 `Train::maskedId()`（静态，中间 8 位替换为 `*`）
   - 应用于：座位登记表、TicketView 车票展示与导出的 .txt（数据文件仍存完整号）

**执行步骤**：

- 步骤 1：`mainwindow.ui` 工具栏添加 新增/删除/售票/退票/查询 按钮（复用现有 QAction）
- 步骤 2：编译 + 手动验证工具栏各按钮，通过后提交 1
- 步骤 3：连接班次表格 `cellDoubleClicked` → 打开售票对话框
- 步骤 4：编译 + 手动验证双击售票，通过后提交 2
- 步骤 5：`trainTable->setSortingEnabled(true)`，刷新时用 `setSortingEnabled(false)` 包裹
- 步骤 6：编译 + 手动验证各列排序，通过后提交 3
- 步骤 7：`MainWindow` 增加统计方法（班次总数/余票总数/已售总数），刷新时更新状态栏
- 步骤 8：编译 + 手动验证统计随数据变化，通过后提交 4
- 步骤 9：`train.h/cpp` 新增静态 `Train::maskedId(const QString &id)`
- 步骤 10：座位登记表改用脱敏号；`TicketView` 展示与 .txt 导出改用脱敏号
- 步骤 11：编译 + 手动验证界面与导出均打码、数据文件仍完整，通过后提交 5

**决策点**：
- **D13 双击行为**：A. 双击=售票（建议，最高频操作）；B. 双击=查看详情。
- **D14 身份证显示**：A. 脱敏显示（建议，符合真实系统习惯，中间 8 位打码）；B. 完整显示。

**验收**：常用操作一键/双击可达；列头点击排序正确；状态栏统计与数据一致；界面与导出车票中身份证已打码。

---

### 迭代 8（V14）：修改班次

**目标**：支持编辑已有班次（当前只能删除后重建），售票记录不丢失。

**涉及文件**：`addtraindialog.h/cpp`、`trainsystem.h/cpp`、`mainwindow.h/cpp/ui`

**提交拆分**：

1. **提交 1 `V14: 修改班次对话框与数据层`**
   - `AddTrainDialog` 增加编辑模式：构造时传入 `Train`，字段回填，标题改"修改班次"
   - `TrainSystem::updateTrain(const QString &oldNo, const Train &t)`：按 `oldNo` 定位替换；新班次号查重（排除自身）；车厢/座位数变更时，若已售座位超出新范围则拒绝修改
2. **提交 2 `V14: 主窗口修改班次入口`**
   - "操作"菜单新增"修改班次..."（位于新增/删除之间），复用 `currentTrain()`

**执行步骤**：

- 步骤 1：`AddTrainDialog` 增加编辑模式构造（传入 `Train` 回填字段，标题改"修改班次"）
- 步骤 2：`TrainSystem::updateTrain(oldNo, t)`：按 `oldNo` 定位替换，新号查重（排除自身），已售座位超新范围时拒绝
- 步骤 3：编译 + ctest（补 `updateTrain` 测试用例），通过后提交 1
- 步骤 4：`mainwindow.ui` "操作"菜单加"修改班次..."，`mainwindow.cpp` 接线（取当前班次 → 编辑对话框 → updateTrain）
- 步骤 5：编译 + 手动验证修改/改号查重/座位范围拦截，通过后提交 2

**决策点**：
- **D15 修改范围**：A. 允许改班次号（建议，保存时查重排除自身）；B. 班次号只读。
- **D16 修改入口**：A. 菜单项（建议）；B. 右键菜单；C. 工具栏按钮。

**验收**：修改后表格/文件数据一致；改号查重生效；座位数缩小到已售范围外时被拦截并提示。

---

### 迭代 9（V15）：未保存提示 + 窗口记忆

**目标**：防止误关丢失数据；记住窗口状态。

**涉及文件**：`mainwindow.h/cpp`、`main.cpp`

**提交拆分**：

1. **提交 1 `V15: 未保存改动提示`**
   - `MainWindow` 增加 `bool m_dirty`，任何数据改动（增删改班次、售票退票）置 true，保存/新建/打开后置 false
   - 重写 `closeEvent()`：`m_dirty` 时弹 `QMessageBox`（保存/不保存/取消）
2. **提交 2 `V15: 窗口几何记忆`**
   - `QSettings("SEU", "TrainSystem")` 保存/恢复窗口大小与位置
3. **提交 3 `V15: 自动打开上次文件`**
   - QSettings 记录上次打开的文件路径，启动时存在则自动加载

**执行步骤**：

- 步骤 1：`MainWindow` 增加 `bool m_dirty`，数据改动处（增删改、售票退票）置 true，保存/新建/打开后置 false
- 步骤 2：重写 `closeEvent()`：`m_dirty` 时弹 保存/不保存/取消 三选
- 步骤 3：编译 + 手动验证三种关闭选择，通过后提交 1
- 步骤 4：`QSettings("SEU", "TrainSystem")` 保存窗口大小/位置，`closeEvent` 中写、构造后读
- 步骤 5：编译 + 手动验证重启后窗口几何恢复，通过后提交 2
- 步骤 6：QSettings 记录上次文件路径，`MainWindow` 构造后自动加载（文件存在时）
- 步骤 7：编译 + 手动验证重启自动打开上次文件，通过后提交 3

**决策点**：
- **D11 是否自动打开上次文件**：A. 自动打开（建议，演示方便）；B. 不自动打开。

**验收**：有未保存改动时关闭弹提示；重启后窗口大小/位置恢复且自动加载上次数据文件。

---

## 五、决策点汇总

| # | 决策点 | 选项 | 结论 |
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
| D12 | MainWindow 瘦身方式 | 组件化拆分 / 调度层 / 函数拆短 | 组件化拆分 |
| D13 | 双击班次行为 | 售票 / 查看详情 | 售票 |
| D14 | 身份证显示 | 脱敏 / 完整 | 脱敏（中间 8 位打码，数据文件仍存完整号） |
| D15 | 修改班次能否改号 | 允许（查重排除自身）/ 只读 | 允许 |
| D16 | 修改班次入口 | 菜单项 / 右键菜单 / 工具栏 | 菜单项 |

## 六、分支与提交策略

- 所有迭代在 `dev` 分支开发并提交，`main` 分支保持稳定（受保护，需 MR 合并）
- **一个功能对应一次提交**：每个提交独立可编译、可验证，commit message 格式 `V11: 核心类单元测试`（沿用 V1–V10 风格）
- 每次提交前本地编译 + 相关验证（V11 起含 ctest），一个迭代全部提交完成后统一 push 到 `dev`
- 迭代间相互独立，可随时暂停

## 七、后续可选（不在本次计划内）

- **Model/View 重构**（`QAbstractTableModel` + `QSortFilterProxyModel`）：表格数据自动驱动，工程量较大，收益偏架构（当前 QTableWidget 原生排序已够用）
- **多选批量操作**：批量售票/退票
