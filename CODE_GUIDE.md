# 列车客运售票管理系统 · 代码详解

> 适用版本 v0.3.0（V18）。本文逐文件、逐函数说明项目代码：每个函数做什么、依靠什么实现、被谁调用。
> 阅读顺序建议：先看「1. 分层总览」，再按 数据层 → 工具层 → 主窗口 → 页面 → 对话框 的顺序阅读。

---

## 1. 分层总览

项目为纯 Qt Widgets 桌面应用（Qt 6.5+ / C++17 / CMake），无第三方依赖。分为四层，依赖方向自上而下、不允许反向：

```
┌─ 入口层 ──── main.cpp
├─ 主窗口层 ── MainWindow（组织导航/工具栏/页面栈/状态栏/文件IO/脏标记）
│    ├─ 导航 ── SidebarWidget（侧边栏按钮组 + 数据概览卡）
│    ├─ 页面 ── TrainListView（班次管理） / QueryPage（查询） / StatisticsPage（统计）
│    │             └─ DonutChart / SeatMapPopup（统计与查询的自绘子组件）
│    └─ 对话框层
│         ├─ AddTrainDialog（新增/修改班次）
│         ├─ TicketDialog（班次详情：座位网格 + 售退票）
│         ├─ SellDialog / RefundDialog / SeatTableDialog（表格式售/退/登记）
│         │    └─ SeatStatusTable / SeatTableView（两种座位表格视图）
│         ├─ RegisterDialog（旅客登记） / TicketView → TicketCard（车票展示与保存）
│         └─ FadeDialog（淡入基类，所有对话框继承它）
├─ 工具层 ──── palette.h（颜色） / tableutil.h（表格/座位网格/脱敏辅助）
└─ 数据层 ──── TrainSystem（班次集合 + 文件IO） → Train（单个班次） → Seat（座位）
```

**核心数据流**：所有 UI 对数据的修改都收敛到 `Train::sellTicket / refundTicket` 与 `TrainSystem::add/update/remove`，完成后通过 `dataChanged` 信号逐层上抛，最终由 `MainWindow::refreshTrainList()` 统一刷新列表、查询页与状态栏。

**文件清单（28 个源文件）**

| 层 | 文件 | 职责 |
|---|---|---|
| 数据 | `seat.h/cpp`、`train.h/cpp`、`trainsystem.h/cpp` | 座位、班次、班次集合与 .dat 文件 IO |
| 工具 | `palette.h`、`tableutil.h` | 颜色调色板；表格/座位网格/身份证脱敏共享辅助 |
| 基类 | `fadedialog.h/cpp` | 对话框淡入动画基类 |
| 主窗口 | `main.cpp`、`mainwindow.h/cpp/ui` | 程序入口、全局样式、主窗口 |
| 导航 | `sidebarwidget.h/cpp` | 侧边栏导航与数据概览 |
| 页面 | `trainlistview.h/cpp`、`querypage.h/cpp/ui`、`statisticspage.h/cpp` | 三个功能页 |
| 自绘 | `donutchart.h/cpp`、`ticketcard.h/cpp`、`seatmappopup.h/cpp` | 环形图、仿真车票、座位浮窗 |
| 对话框 | `addtraindialog.*`、`ticketdialog.*`、`ticketview.*`、`selldialog.*`、`refunddialog.*`、`registerdialog.*`、`seattabledialog.*`、`seattableview.*`、`seatstatustable.*` | 各功能弹窗 |
| 资源 | `theme.qss`、`resources.qrc`、`icons/`、`testdata.dat` | 全局样式、图标、示例数据 |
| 构建/测试 | `CMakeLists.txt`、`tests/tst_train.cpp`、`tests/tst_system.cpp` | 构建脚本与单元测试 |

---

## 2. 数据层（无 UI，可独立测试）

### 2.1 `seat.h/cpp` —— 座位类

一个座位 = 座位坐标 + 可选的旅客信息。**以「姓名是否为空」作为空座判据**（`m_name` 空 = 未售出），不设单独的 bool 标志，避免两份状态不同步。

| 成员 | 说明 |
|---|---|
| `Seat()` / `Seat(name, id, carriage, seatNo)` | 默认构造产生空座；带参构造直接登记旅客 |
| `isEmpty()` | 座位是否空闲。实现：`m_name.isEmpty()` |
| `name()/id()/carriage()/seatNo()` | 四个只读访问器 |
| `setPassenger(name, id)` | 售票：写入旅客姓名与身份证号 |
| `clearPassenger()` | 退票：清空姓名与身份证号 |

### 2.2 `train.h/cpp` —— 班次类（核心领域对象）

**座位存储模型**：所有座位放在一条扁平数组 `m_seats` 中（`carriages × seatsPerCarriage` 个），座位 `(c, s)` 的下标 = `(c-1) * seatsPerCarriage + (s-1)`。构造时一次性生成全部空座并预填坐标。**车厢等级是逐车厢的向量** `m_carriageClass`（1=一等 2=二等），默认全 2，因此一等厢可位于任意节。

| 函数 | 实现要点 |
|---|---|
| `Train(9参构造)` | resize 座位数组、逐个填坐标、`m_carriageClass.fill(2, carriages)` |
| `sellTicket(name, id, carriage, seatNo)` | 先做车厢/座位号范围检查 → 下标定位 → `isEmpty()` 判重 → `setPassenger`。返回 false 表示范围非法或已售 |
| `refundTicket(carriage, seatNo)` | 与上对称：范围检查 → 已售才允许 `clearPassenger` |
| `remainingSeats()` | 遍历座位数组统计 `isEmpty()` |
| `availableSeats(carriage)` | 指定车厢内未售座位号列表；车厢号非法返回空表 |
| `carriageClass(c)` | 按车厢号查等级向量，**越界兜底返回 2**（二等），保证 UI 永远拿到安全值 |
| `setCarriageClass(classes)` | 直接替换向量；长度不足时补 2 |
| `carriageClassText(c)` | `carriageClass(c)==1 ? "一等座" : "二等座"` |
| `priceOf(c)` | 按该节等级返回一等/二等票价 |
| `isSeatOccupied(c, s)` | 范围检查后返回 `!m_seats[index].isEmpty()` |
| `seatAt(c, s)` | 越界安全的只读访问：非法范围返回静态空座位引用 |
| `isValidTime(t)`（static） | 正则 `^([01]\d|2[0-3]):[0-5]\d$` 校验 HH:MM |
| `isValidDate(d)`（static） | `QDate::fromString(d, "yyyy-MM-dd").isValid()` |
| `isValidId(id)`（static） | 正则 `^\d{17}[0-9Xx]$`（18 位身份证，末位可为 X/x） |

其余为纯访问器（`no/date/departTime/from/to/carriages/seatsPerCarriage/seats/stops` 等），一行实现。

### 2.3 `trainsystem.h/cpp` —— 班次集合 + 文件持久化

文档类，管理 `QVector<Train> m_trains`，不继承 QObject（纯值语义，可整体赋值备份/恢复）。

| 函数 | 实现要点 |
|---|---|
| `addTrain(t)` | `findTrain` 查重后 append |
| `updateTrain(oldNo, newTrain)` | **V14 核心**。三步：① `indexOf(oldNo)` 找原班次，新号与他班冲突即拒绝；② 遍历原已售座位，任何一张超出新编组范围（车厢数/每厢座位数变小）即整体拒绝——保证不丢数据；③ 把保留的售座逐张 `sellTicket` 迁移到新班次后替换 |
| `removeTrain(no)` | `indexOf` 后 removeAt |
| `indexOf(no)` / `findTrain(no)` | 线性查找；findTrain 返回指针供对话框直接读写班次 |
| `saveToFile(path)` | 文本格式：第一行班次数；每班次两段——字段行 + 已售座位行。**新格式**带「日期 一等票价 二等票价 各车厢等级...」，**旧格式**（无日期的班次）按旧格式写回，往返无损 |
| `loadFromFile(path)` | 逐班次解析。向后兼容：读字段行时预读一个 token，`isValidDate` 为真走新格式（继续读票价与逐节等级），否则该 token 就是旧格式的停靠站数。座位逐张 `sellTicket` 回填 |

> 文件格式注释见 `trainsystem.cpp:63`；`tests/tst_system.cpp` 的 `oldFormatCompatibility` 用例固定了这一往返行为。

---

## 3. 工具层（头文件实现，多个组件共用）

### 3.1 `palette.h` —— 全局调色板

`namespace Palette` 内一组 `inline const QColor`，与 `theme.qss` 的品牌色系人工同步（QSS 无变量机制）。分四组：基础色（背景/边框/交替行）、文字四级灰阶、品牌三色（蓝/危险红/成功绿）、环形图淡色切片。**新增颜色前先查是否已有同义色**——注释中明文约束。

### 3.2 `tableutil.h` —— 共享辅助（全 inline）

| 函数 | 用途 | 依靠 |
|---|---|---|
| `maskId(id)` | 身份证脱敏展示：15 位以上号码第 11-14 位换 `*`（真票规则）；短号保留前 6 后 4。**仅展示层使用，数据文件仍存全号** | 纯 QString 操作 |
| `makeCenteredItem(text)` | 生成居中对齐的 `QTableWidgetItem` | QTableWidgetItem |
| `computeColumnWeights(table, weights)` | 数据填充后调用一次：切换 `Interactive` 列宽模式、`resizeColumnsToContents` 逐列测量内容宽作为权重 | QHeaderView |
| `applyWeightedColumnWidths(table, weights)` | 按权重把列宽铺满视口（最后一列兜底吃掉余数）；`showEvent`/`resizeEvent` 时调用。权重为空或行数为 0 直接返回 | viewport 宽度 |
| `clearLayout(layout)` | 清空布局所有子项并 `deleteLater` 控件（座位网格重建用） | QLayout::takeAt |
| `seatGridRow(seatRow, aisleAfter)` | 座位排 → 网格实际行号：过道之后的排整体 +1 | 纯算术 |
| `makeAisleFrame(parent, height=12)` | 过道分隔条：`objectName="aisleFrame"` + `WA_StyledBackground`，QSS 中是上下两条 1px 浅灰线 | QFrame + QSS |
| `addAisleRow(grid, rows, cols, parent, h)` | rows ≥ 2 时把过道插到倒数第二排之后（2 排→正中，3 排→二/三排之间） | makeAisleFrame |

### 3.3 `fadedialog.h/cpp` —— 对话框淡入基类

唯一的 `showEvent` 重写：每次显示时创建 `QPropertyAnimation(windowOpacity)` 200ms 从 0 到 1，`DeleteWhenStopped` 自动释放。`SellDialog/RefundDialog/RegisterDialog/TicketDialog/TicketView/AddTrainDialog/SeatTableDialog` 均继承它，获得统一的开窗过渡。

---

## 4. 入口与主窗口

### 4.1 `main.cpp`

1. 构造 `QApplication` 并设置应用名。
2. **强制浅色 QPalette**：逐角色写入 Palette 色值，防止 macOS 深色模式下未覆盖控件显示为深色。
3. 读 `:/theme.qss`（qrc 资源）设为全局样式表。
4. 构造并显示 `MainWindow`，进入事件循环。

### 4.2 `mainwindow.h/cpp` —— 应用壳

**匿名命名空间内两个辅助**（仅本文件可见）：
- `HoverIconFilter`：事件过滤器，`Enter/Leave` 时切换按钮的灰/白图标（工具栏按钮悬停变白的实现），`setupHoverIcon()` 负责装配。
- `setupClickAnimation(btn)`：连接 clicked → `QPropertyAnimation(minimumWidth)` 100ms 缩 4px 再复原的按压反馈。

**构造函数职责（按序）**：
1. `setupUi`；连接文件按钮（打开/保存）与 8 个操作按钮（新增/修改/删除/售票/退票/座位登记…）。
2. 保留系统菜单 action（新建/打开/保存/退出/关于）的快捷键连接。
3. 连接侧边栏 `pageSelected → onPageSelected`、班次列表 `trainSelected/trainDoubleClicked`。
4. `queryPage->setSystem(&m_system)` 把数据源（指针）交给查询页。
5. 逐个装配悬停图标与点击动画。
6. 状态栏加永久 `QLabel`（全局统计文字）。
7. 默认进入第 0 页；`refreshTrainList()` + 初始按钮禁用态。
8. `restoreWindowState()`：恢复几何并自动打开上次文件。

**主要私有槽/函数**：

| 函数 | 说明 |
|---|---|
| `loadFile(path)`（public） | 不弹对话框直接加载数据文件并刷新，供冒烟测试用；普通打开走 `onOpenFile` |
| `onNewFile/onOpenFile/onSaveFile` | 文件三件套：重建空系统 / `QFileDialog` 选文件 + `loadFromFile` / 有路径直接存。三者都维护 `m_dirty` 复位与窗口标题 |
| `onAddTrain/onEditTrain/onRemoveTrain` | 班次 CRUD。`onEditTrain` 用 `AddTrainDialog(editTrain)` 编辑模式构造，失败原因（改号冲突/缩编组）在提示框中说明。删除前保存班次号——`removeTrain` 后原指针失效 |
| `onSellTicket/onRefundTicket` | 弹对应对话框并连接 `dataChanged → markDirty + refreshTrainList` |
| `onTrainDoubleClicked` | 双击列表行直接开 `TicketDialog`（班次详情） |
| `onSeatTable` | 弹 `SeatTableDialog`（座位登记） |
| `onTrainSelectionChanged(no)` | 无选中班次时禁用 5 个依赖按钮 |
| `onPageSelected(index)` | 切 `contentStack` 页 + 入场动画；第 2 页（统计）额外 `setData` 刷新 |
| `animatePageIn(page)` | 260ms 淡入（`QGraphicsOpacityEffect`，结束移除避免常驻合成开销）+ 8px 上浮（动画布局顶部 margin，不动 geometry）。`m_pageAnim` 是 QPointer：快速连点时停掉上一场动画，以最后点击为准 |
| `refreshTrainList()` | 统一刷新入口：列表 `setTrains` + 查询页 `refresh` + `updateStats` |
| `updateStats()` | 汇总容量/余票/已售 → 状态栏文字 + 侧边栏概览卡 |
| `currentTrain()` | 列表当前行班次号 → `findTrain` 取指针 |
| `markDirty()` | 置脏（所有数据改动路径都会调用） |
| `closeEvent(event)` | V15：脏时弹 保存/不保存/取消 三选框——取消则 `event->ignore()`；选保存后**若保存被取消或失败（仍为脏）同样中止关闭**；随后把几何与 `m_filePath` 写入 `QSettings("SEU","TrainSystem")` |
| `restoreWindowState()` | `restoreGeometry`；lastFile 存在且能加载则自动打开并提示 |

### 4.3 `sidebarwidget.h/cpp` —— 侧边栏

- 构造：品牌区（图标+名称+副标题）→「功能导航」分组标题 → 3 个可勾选导航按钮（`QButtonGroup` 独占，idClicked 直接转 `pageSelected` 信号）→ stretch → 数据概览卡 → 关于按钮 + 版本号。
- `setCurrentPage(index)`：按 id 勾选按钮（程序化切页时用）。
- `currentPage()`：返回 `checkedId()`。
- `updateStats(班次, 余票, 已售)`：刷新概览卡文字。

---

## 5. 三个功能页

### 5.1 `trainlistview.h/cpp` —— 班次管理页核心（列表）

**内部类**（匿名命名空间）：`NumericItem : QTableWidgetItem`——构造时居中；重载 `operator<` 按 `text().toDouble()` 比较，使车厢数/座位数/余票三列按数值而非字典序排序（否则 "99" 排在 "100" 之后）。

| 函数 | 说明 |
|---|---|
| 构造 | 标题 + 搜索框 + 8 列表格（排序开启）+ `QStackedLayout` 中的空状态引导页（图标+提示语）；连接 textChanged / itemSelectionChanged / cellDoubleClicked |
| `setTrains(trains)` | 缓存数据副本 → `applyFilter()` |
| `currentTrainNo()` | 当前行第 0 列 item 的 `Qt::UserRole` 数据（存原始班次号，排序后依然正确） |
| `applyFilter()` | 核心：班次号/发车/终点城市大小写不敏感过滤。**填充期间关闭排序**（避免 insertRow 被当前排序规则重排导致行错乱），`setRowCount` 预分配；填完恢复排序 → 按旧班次号恢复选中 → 切换空状态页 → `computeColumnWeights` + 按权重铺列宽 |
| `applyColumnWidths()` | 转调 `applyWeightedColumnWidths` |
| `showEvent` | `QTimer::singleShot(0)` 延迟分配列宽——等事件循环完成布局、视口宽度就绪 |
| `resizeEvent` | 窗口缩放时重新铺列宽 |
| `onSelectionChanged / onDoubleClicked` | 转发为 `trainSelected(no)` / `trainDoubleClicked(no)` 信号 |

### 5.2 `querypage.h/cpp` —— 查询页

| 函数 | 说明 |
|---|---|
| `setSystem(const TrainSystem*)` | 绑定数据源（只读指针，由 MainWindow 在构造时传入） |
| `refresh()` | 重建班次下拉框（blockSignals 防抖），尽量恢复之前选中的班次号，最后 `onTrainChanged()` |
| `onTrainChanged()` | 无数据时九个展示位全填 "-"；有数据时：日期/时间/城市/余票/双票价 → 一等车厢号列表（`carriageClass(c)==1` 收集）→ 余票逐张填入座位列表（UserRole 存车厢号、UserRole+1 存座位号）→ 停靠站列表（UserRole 存序号） |
| `onSeatClicked(item)` | 取出车厢/座位号 → 销毁旧浮窗 → 新建 `SeatMapPopup` → `showNear(QCursor::pos())` 就近弹出 |
| `onStopClicked(item)` | QToolTip 显示「第 N 站 · 途经站」 |

### 5.3 `statisticspage.h/cpp` —— 统计页

**内部结构** `ClassStats`：一/二等车厢数与座位数统计。

| 函数 | 说明 |
|---|---|
| 构造 | 纯代码布局（无 .ui）：标题 → 双栏卡片（左环形图 / 右等级条）→ 班次详情表格卡。`makeBar` lambda 统一造等级行；**进度条量程 0-1000**——量程 100 时 OutCubic 尾段每帧增量 <1 被量化，最后一帧肉眼可见跳 3px；1000 量程下步进约 0.3px |
| `calcClassStats(trains)` | 遍历班次×车厢按等级累加 |
| `animateBarTo(bar, target)` | 等级条入场动画：先停该 bar 上旧动画（防抢占），target≤0 直接归零；否则 `QPropertyAnimation(bar,"value")` 600ms OutCubic，值 = 百分比 ×10（匹配 0-1000 量程） |
| `setData(trains)` | 每次切入统计页调用：① 算总容量/已售/余票；② 环形图 setNote（总班次）+ setSlices（余票绿在前、已售红在后）+ animate；③ 等级条文字与动画；④ 班次详情表格逐行填（**上座率 >50% 文字标红**）；⑤ 空数据时用 `setSpan` 合并整行显示提示 |
| `applyColumnWidths / showEvent / resizeEvent` | 与 TrainListView 相同的权重列宽模式 |

### 5.4 `donutchart.h/cpp` —— 自绘环形图（统计页核心组件）

纯 QPainter 绘制，两个 Q_PROPERTY 动画属性：`animationProgress`（入场展开 0→100）与 `hoverProgress`（悬停加粗进度）。

| 函数 | 说明 |
|---|---|
| `setSlices(slices)` | 存数据并复位悬停状态（from/to 向量清空）；**不动 animationProgress**——动画节奏完全由 `animate()` 控制，避免先画终态再归零的闪帧 |
| `animate()` | 停掉所有运行中的 QPropertyAnimation 后，800ms OutCubic 从 0 到 100 驱动展开 |
| `chartGeometry()` | **绘图与命中共用的几何**：top = 标题/注释行累计高；`chartSize = min(width-150, height-top-10)`；弧宽 = size/4、半径 = (size-弧宽)/2。共用保证 hit-test 与绘制坐标系一致 |
| `sliceAt(pos)` | 屏幕坐标 → 切片下标：动画未结束直接返回 -1（弧还在生长）；环带命中检测（±2px 余量）；角度归一到 [270°, -90°) 区间后按「切片顺时针排列、角度递减」线性扫描命中段 |
| `hoverStrength(i)` / `currentStrengths()` | 第 i 段当前加粗强度 = from/to 向量按 `hoverProgress` 插值；过冲时强度可短暂 >1（OutBack 回弹感） |
| `animateHoverTo(target)` | 悬停弹性动画核心：把**当前各段强度固化为新起点** `m_hoverFrom`，终点向量只有目标段为 1；420ms OutBack 驱动 hoverProgress 0→1。这样悬停从绿段滑到红段时旧段平滑回缩、新段弹性弹出，**两头都无跳变** |
| `mouseMoveEvent` | `sliceAt` 变化时更新 `m_hoverIndex` + `animateHoverTo`；命中段弹 QToolTip 明细 |
| `leaveEvent` | 悬停复位（目标 -1，全部回缩） |
| `paintEvent` | 绘制流程：标题/注释 → 空数据灰环 → **预计算最终分布**（`starts/draws/fulls` 三向量，末段终点固定越过 6 点接缝 kExt=10°，动画只是让末端匀速收敛到该预计算终点——终点即最终分布，根治跳变）→ 逐段 `drawArcSeg`（RoundCap 圆头弧）→ 首段交接处补画 stub（绿帽压过红段起点，压入量随红段揭示增长）→ 悬停段按强度加粗（最高 +5px）最后绘制盖在相邻段上 → 环心总座位数 → 引出线注释（径向射线至文字行水平线处折点、再转横线，文字就近放四角） |

---

## 6. 对话框层

### 6.1 `addtraindialog.h/cpp` —— 新增/修改班次（V14）

| 函数 | 说明 |
|---|---|
| `AddTrainDialog(parent)` | 新建模式：日期默认今天、日历箭头图标装配 |
| `AddTrainDialog(editTrain, parent)` | 编辑模式：委托构造后回填全部字段——**一等车厢号列表**（如 `2,3`）替代旧的“一等车厢数”，可表达一等厢在中后部的任意布局 |
| `toggleCalendar/openCalendar/closeCalendar` | 无边框日历弹窗：首次点击时惰性创建（QCalendarWidget 包在 QDialog 里），定位到日期字段下方；`qApp->installEventFilter` 全局监听点击以实现“点外部关闭” |
| `eventFilter` | 日历可见时点击弹窗外且不在箭头按钮上 → closeCalendar |
| `updateCalendarIcon` | 箭头图标随展开状态切换（chevron-up/down.png） |
| `getTrain()` | 表单 → Train：切分「一等车厢号」文本逐个置 1（越界忽略），停靠站按逗号切分 |
| `accept()` | 完整校验后放行：班次号/城市非空、时间正则、**一等车厢号必须是 1..车厢数 内不重复整数**、票价 > 0 |

### 6.2 `ticketdialog.h/cpp` —— 班次详情（双击列表打开）

座位网格 + 单击选中 + 底部动作按钮（空座→售票、已售→退票）。`SeatsPerRow = 5` 常量与测试数据的 10/15 座对应（2 排/3 排）。

| 函数 | 说明 |
|---|---|
| `onCarriageChanged` | 切车厢 → 清选中 → 重建网格 |
| `onSeatClicked(seatNo)` | 再点同一座位取消选中；重刷全部按钮样式 + 信息行 + 动作按钮 |
| `rebuildSeatGrid()` | `clearLayout` 清旧按钮 → 按座位数算排数与过道位置 → **座位图节奏**：座位块高 36、垂直间距 = 18（座位高一半）、上下留白 30（比座位块略短）、过道带高 36 → `addAisleRow` 插过道 → **按网格实际高度固定滚动区与窗口高度**（`setFixedHeight(sizeHint())`），不多留空 |
| `styleSeatButton(btn, seatNo)` | 设置 `state`（selected/occupied/empty）与 `class`（一等 first/二等 second）动态属性 → `unpolish+polish` 强制 QSS 重算 |
| `updateInfoLabel / updateActionButton` | 选中已售显示“已售给 X”；选中空座显示席别与实时票价（`priceOf`）；按钮文字/可用性随选中状态切换 |
| `onAction()` | 空座：RegisterDialog 登记成功 → `sellTicket` → 刷新 → emit dataChanged → 弹 `TicketView` 出票；已售：确认后 `refundTicket` → 刷新 → dataChanged |

### 6.3 `ticketview.h/cpp / ticketview.ui` —— 车票弹窗

- 构造：创建 `TicketCard` 插入布局首位 → `adjustSize()` 让弹窗包住票面（.ui 里的 geometry 是遗留小值）→ 保存按钮标 `primary` 属性并强制重算 QSS。
- `onSaveTicket()`：`QFileDialog` 选路径 → 写电子客票格式文本——**订单/支付数据取自 `m_card`（TicketCard 构造时生成）**，票面展示与保存文件共用同一份数据；身份证经 `maskId` 脱敏。

### 6.4 `ticketcard.h/cpp` —— 仿真磁介质车票（纯自绘）

**文件内自由函数** `pinyinOf(station)`：50 城市拼音映射 + 东西南北后缀拆解（“南京南” = 城市“南京” + 后缀“南”），未收录返回空（票面省略拼音行）。

**构造函数**：以「时钟毫秒 ^ 班次指针 ^ 车厢 << 8 ^ 座位 << 16」为种子生成 `QRandomGenerator`——同一次售票 determinstic，票面与保存文件一致。生成：红字票号（E+8位）、21 位售票码、订单号（OD+6位+秒级后6位）、支付方式四选一 + 22 位流水号（前缀模拟 2018=支付宝/4200=微信/6222=银联/9901=铁路账户）、支付时间。

| 函数 | 说明 |
|---|---|
| `paintEvent` | 按 880×430 设计稿等比缩放定位（X/Y lambda）。绘制顺序：① 浅蓝渐变底 + 圆角边框 + clip 后细网格底纹；② **动车组水印**——证件行与标语行空带上的低矮白色剪影（车头朝左三次曲线 + 前窗挖窗 + 两条尾流线），不压文字；③ 底部浅蓝色带；④ 九行票面文字（红字票号/检票口 → 站名大字+车次居中+拼音小字 → 日期座位 → 票价+席别+支付标记 → 限乘当日当次车 → 证件打码+姓名 → 标语 → 色带内售票码+售票站）；⑤ 副联分隔——竖向齿孔虚线（DashLine + 上下打孔半圆缺口）；⑥ 副联区——中国铁路标识 + 二维码 + 订单号 + 支付信息 |
| `drawQr(p, area)` | 装饰性伪 QR：21×21 模块（版本 1 规格），三个 7×7 定位图案（ring+core）+ 时序线，数据区以售票码/车厢/座位为种子随机填充。**纯装饰，不可扫描** |

### 6.5 售/退/登记三个流程弹窗

| 文件 | 函数与说明 |
|---|---|
| `selldialog.cpp` | 构造：内嵌 `SeatStatusTable`（sellMode=true，仅空座可点）+ 确认按钮（无选中禁用）。`onSell`：登记旅客 → `sellTicket` → 刷表格 → dataChanged → 弹 `TicketView` 出票 |
| `refunddialog.cpp` | 对称：sellMode=false（仅已售可点）。`onRefund`：确认框 → `refundTicket` → 刷新 → dataChanged |
| `registerdialog.cpp` | 姓名+身份证表单；`accept()` 校验姓名非空、`Train::isValidId` 格式合法才放行；`name()/id()` 返回 trimmed 文本 |
| `seattabledialog.cpp` | 薄壳：内嵌 `SeatTableView`，转发其 `dataChanged` 信号 |

### 6.6 `seatstatustable.h/cpp` —— 座位状态表格（售票/退票用）

| 函数 | 说明 |
|---|---|
| `setTrain / setSellMode` | 存状态 → `rebuild()` |
| `rebuild()` | 全量重建行：状态列用 **"■" 字符 + 前景色**画圆角色块（已售红/空座绿，16pt 字体统一大小，行高 30），不可操作行整行灰化并剥掉 `ItemIsSelectable`。一等节行底浅灰蓝、二等白底。填完 `computeColumnWeights` + 铺列宽 |
| `selectedCarriage / selectedSeat` | 当前行第 1/2 列文本 toInt（无选中返回 0） |
| `showEvent / resizeEvent` | 延迟/实时铺列宽（同前） |

### 6.7 `seattableview.h/cpp` —— 座位登记视图（座位登记弹窗内）

按钮式座位网格（非表格）。5 列布局、10 座 2 排 / 15 座 3 排、过道经 `addAisleRow` 插入。

| 函数 | 说明 |
|---|---|
| `setTrain(train)` | 重建车厢下拉（每项标注等级），默认第 1 节 |
| `rebuildGrid()` | `clearLayout` 清旧按钮 → 按排/列生成 52×34 座位按钮 → 过道 → 空状态切换 → `updateStats` |
| `onSeatClicked(seatNo)` | 已售：QToolTip 显示旅客与脱敏身份证；空座：RegisterDialog → `sellTicket` → 刷网格 → dataChanged |
| `styleSeatButton / updateStats` | 状态属性重算 QSS；“本车厢已售 N / M 座” 统计行 |

### 6.8 `seatmappopup.h/cpp` —— 座位分布浮窗（查询页）

Qt::Popup 无边框半透明窗口，外层透明、内层白底卡片。

- 构造：标题（车厢号+等级）→ QLabel 座位格（34×26，`state` 属性 empty/occupied/selected）+ 过道 → 三色图例。
- `showNear(globalPos)`：`adjustSize` 后按光标方位镜像翻转防出屏，最后 150ms 淡入显示。

---

## 7. 资源、样式与数据

### 7.1 `theme.qss`（约 12k 字符，全局样式）

分区：全局 `QWidget/QLabel` 基础 → 按钮体系（`#primary` 主按钮、`[nav]` 导航、`[seat]` 座位块的状态×等级矩阵）→ 表格/输入框/卡片容器 → 侧边栏 → 弹窗（`#seatMapPopup`、`#seatCell`）→ 空状态 → `#aisleFrame`（过道上下 1px 浅灰线）。所有 objectName/动态属性（`state`、`class`、`accent`、`primary`、`seat`）与 C++ 中 `setProperty` 一一对应。

### 7.2 `resources.qrc` + `icons/`

22 个资源：14 个工具栏图标 ×灰/白两态（悬停切换）+ 3 个导航图标 + about + chevron-up/down.png（日历箭头）。全部被引用，无孤儿资源。

### 7.3 `testdata.dat`（示例数据）

5 个班次 275 座已售 96（34.9%，各班次 32%~35% 随机偏移）：
- G1（一等第 1 节·车头）、D2206（一等第 2 节·中间）、G1371（一等第 3 节·靠尾）——一等厢三态布局覆盖；
- Z281（0 停靠站 + 全二等，边界用例）；C2015（16/30=53.3%，演示统计页 >50% 红色高亮）。
- 格式与解析规则见 `trainsystem.cpp:63` 注释；加载/往返由 `tst_system` 固定。

### 7.4 `CMakeLists.txt`

- 主程序 `qt_add_executable(ticket_system WIN32 MACOSX_BUNDLE)`，`AUTORCC ON` 编译 qrc；输出名「列车售票系统.app」，版本 0.3.0。
- 链接 `Qt::Core Qt::Widgets`；仅 Core+Widgets，无网络/无多媒体依赖。
- 两个测试目标 `tst_train`（Train/Seat 单元）与 `tst_system`（TrainSystem CRUD + 文件往返），`add_test` 注册给 CTest。

### 7.5 `tests/`

- `tst_train.cpp`：座位模型（售票判重、退票、余票统计、等级行为、一等厢在第 3 节的场景、时间/日期/身份证校验）。
- `tst_system.cpp`：增删改查、**updateTrain 三条失败路径与售座迁移**、新格式文件往返、**旧格式（无日期）读取→按旧格式保存的兼容回归**。

---

## 8. 关键设计决策速查

| 决策 | 位置 | 理由 |
|---|---|---|
| 座位扁平数组 + 下标换算 | train.cpp | 避免二维 QVector 的双重间接；一次换算函数式复用 |
| 车厢等级为逐节向量 | train.cpp:75 | 一等厢可在任意节，UI 全部按 `carriageClass(c)` 查询 |
| updateTrain 整体拒绝而非部分迁移 | trainsystem.cpp:13 | 防止缩编组时静默丢已售数据 |
| 旧格式往返兼容 | trainsystem.cpp:63 | 老数据文件可继续使用，保存不升级格式 |
| 环形图「预计算终态」动画 | donutchart.cpp:paintEvent | 动画终点即最终分布，根治结束跳变 |
| 悬停逐段强度向量插值 | donutchart.cpp:animateHoverTo | 跨段滑动时旧段回缩/新段弹出双平滑 |
| 进度条量程 0-1000 | statisticspage.cpp:makeBar | int 属性动画量化跳变的分辨率对策 |
| 一等厢号文本输入替代数量 Spin | addtraindialog | 可表达一等厢在中后部的任意布局 |
| 所有数据修改 → dataChanged → refreshTrainList | mainwindow | 单一刷新入口，杜绝多处手动同步遗漏 |
| 展示层统一 maskId 脱敏 | tableutil.h | 数据文件存全号，票面/导出/浮窗三处展示一致打码 |
