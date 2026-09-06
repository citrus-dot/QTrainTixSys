#include "ticketcard.h"
#include "palette.h"
#include "tableutil.h"
#include <QDateTime>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>

namespace {

// 票面几何常量（按 880×430 扁长设计稿，绘制时按实际宽高比例缩放定位）。
// 真票 86.4×54mm 比例约 1.6，此处按用户偏好拉宽到 2.05 增强屏显设计感。
constexpr double kDesignW = 880.0;
constexpr double kDesignH = 430.0;
constexpr double kBottomBandH = 34.0; // 底部浅蓝色带高度
constexpr double kStubX = 630.0;      // 副联分隔线 x（右侧为二维码/订单区）

} // namespace

// 常见车站拼音（票面第三行小字）。带东西南北后缀的站名拆城市查询后拼接。
QString pinyinOf(const QString &station)
{
    static const QHash<QString, QString> kCity = {
        {QStringLiteral("北京"), "BEIJING"},   {QStringLiteral("上海"), "SHANGHAI"},
        {QStringLiteral("南京"), "NANJING"},   {QStringLiteral("广州"), "GUANGZHOU"},
        {QStringLiteral("深圳"), "SHENZHEN"},  {QStringLiteral("杭州"), "HANGZHOU"},
        {QStringLiteral("武汉"), "WUHAN"},     {QStringLiteral("成都"), "CHENGDU"},
        {QStringLiteral("重庆"), "CHONGQING"}, {QStringLiteral("西安"), "XIAN"},
        {QStringLiteral("苏州"), "SUZHOU"},    {QStringLiteral("天津"), "TIANJIN"},
        {QStringLiteral("济南"), "JINAN"},     {QStringLiteral("郑州"), "ZHENGZHOU"},
        {QStringLiteral("长沙"), "CHANGSHA"},  {QStringLiteral("合肥"), "HEFEI"},
        {QStringLiteral("福州"), "FUZHOU"},    {QStringLiteral("厦门"), "XIAMEN"},
        {QStringLiteral("昆明"), "KUNMING"},   {QStringLiteral("贵阳"), "GUIYANG"},
        {QStringLiteral("兰州"), "LANZHOU"},   {QStringLiteral("太原"), "TAIYUAN"},
        {QStringLiteral("石家庄"), "SHIJIAZHUANG"}, {QStringLiteral("南昌"), "NANCHANG"},
        {QStringLiteral("长春"), "CHANGCHUN"}, {QStringLiteral("沈阳"), "SHENYANG"},
        {QStringLiteral("哈尔滨"), "HAERBIN"}, {QStringLiteral("大连"), "DALIAN"},
        {QStringLiteral("青岛"), "QINGDAO"},   {QStringLiteral("无锡"), "WUXI"},
        {QStringLiteral("常州"), "CHANGZHOU"}, {QStringLiteral("徐州"), "XUZHOU"},
        {QStringLiteral("温州"), "WENZHOU"},   {QStringLiteral("宁波"), "NINGBO"},
        {QStringLiteral("绍兴"), "SHAOXING"},  {QStringLiteral("嘉兴"), "JIAXING"},
        {QStringLiteral("金华"), "JINHUA"},    {QStringLiteral("台州"), "TAIZHOU"},
        {QStringLiteral("镇江"), "ZHENJIANG"}, {QStringLiteral("扬州"), "YANGZHOU"},
        {QStringLiteral("南通"), "NANTONG"},   {QStringLiteral("洛阳"), "LUOYANG"},
        {QStringLiteral("唐山"), "TANGSHAN"},  {QStringLiteral("保定"), "BAODING"},
        {QStringLiteral("蚌埠"), "BENGBU"},    {QStringLiteral("芜湖"), "WUHU"},
        {QStringLiteral("阜阳"), "FUYANG"},    {QStringLiteral("义乌"), "YIWU"},
        {QStringLiteral("温岭"), "WENLING"},   {QStringLiteral("滁州"), "CHUZHOU"},
    };
    static const QHash<QString, QString> kSuffix = {
        {QStringLiteral("东"), "DONG"}, {QStringLiteral("南"), "NAN"},
        {QStringLiteral("西"), "XI"},   {QStringLiteral("北"), "BEI"},
    };
    if (const auto it = kCity.constFind(station); it != kCity.constEnd())
        return *it;
    if (station.size() >= 2) {
        const auto itC = kCity.constFind(station.left(station.size() - 1));
        const auto itS = kSuffix.constFind(station.right(1));
        if (itC != kCity.constEnd() && itS != kSuffix.constEnd())
            return *itC + *itS;
    }
    return QString(); // 未收录的站名不显示拼音行
}

TicketCard::TicketCard(const Train *train, const QString &name, const QString &id,
                       int carriage, int seatNo, QWidget *parent)
    : QWidget(parent)
    , m_train(train)
    , m_name(name)
    , m_id(id)
    , m_carriage(carriage)
    , m_seatNo(seatNo)
{
    setMinimumSize(780, 380);

    // 订单 / 票号 / 支付信息：以「班次地址+座位+当前毫秒」为种子的确定性随机，
    // 同一次售票的票面展示与保存文件内容一致
    const quint64 seed = QDateTime::currentMSecsSinceEpoch()
        ^ (quintptr(train) << 8) ^ (quint32(carriage) << 16) ^ quint32(seatNo);
    QRandomGenerator rng(seed);

    m_ticketNo = QString("E%1").arg(rng.bounded(100000000, 999999999));
    m_saleCode = QString::number(rng.bounded(1, 10)); // 21 位售票码，首位非零
    for (int i = 0; i < 20; ++i)
        m_saleCode += QString::number(rng.bounded(10));
    m_orderNo = QString("OD%1%2")
                    .arg(rng.bounded(100000, 999999))
                    .arg(QDateTime::currentSecsSinceEpoch() % 1000000, 6, 10, QLatin1Char('0'));

    // 支付方式 + 流水号（单字标记与真实车票「网/支/现」购票标记同风格）
    struct Pay { const char *name; const char *prefix; };
    static const Pay kPays[] = {
        {"支付宝", "2018"}, {"微信支付", "4200"}, {"银联云闪付", "6222"}, {"铁路账户", "9901"},
    };
    const Pay &pay = kPays[rng.bounded(4)];
    m_payMethod = QString::fromUtf8(pay.name);
    m_txnNo = QString::fromLatin1(pay.prefix);
    for (int i = 0; i < 18; ++i)
        m_txnNo += QString::number(rng.bounded(10));
    m_payTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}

void TicketCard::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    // 坐标按实际尺寸相对设计稿缩放定位
    const double sx = r.width() / kDesignW;
    const double sy = r.height() / kDesignH;
    const auto X = [sx](double v) { return v * sx; };
    const auto Y = [sy](double v) { return v * sy; };

    // ---------- 底色：浅蓝偏紫渐变 + 细网格底纹（磁介质车票特征） ----------
    QLinearGradient bg(r.topLeft(), r.bottomRight());
    bg.setColorAt(0.0, QColor("#E6ECF8"));
    bg.setColorAt(0.5, QColor("#DCE3F4"));
    bg.setColorAt(1.0, QColor("#D2DBEF"));
    QPainterPath card;
    card.addRoundedRect(r, 10, 10);
    p.setPen(QPen(QColor("#8FA3C8"), 1));
    p.setBrush(bg);
    p.drawPath(card);
    p.setClipping(true);
    p.setClipPath(card);
    p.setPen(QPen(QColor(255, 255, 255, 60), 1));
    for (double x = r.left() + X(10); x < r.right(); x += X(10))
        p.drawLine(QPointF(x, r.top()), QPointF(x, r.bottom()));
    for (double y = r.top() + Y(10); y < r.bottom(); y += Y(10))
        p.drawLine(QPointF(r.left(), y), QPointF(r.right(), y));

    // ---------- 动车组水印：真票标志性的「从右向左飞驰的动车组」图案 ----------
    // 车头朝左的纯白色剪影（半透明），避免与票面文字争夺视觉
    {
        p.save();
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 110));
        const double trainTop = Y(302);
        const double trainH = Y(30);
        const double left = r.left() + X(36);
        const double right = kStubX * sx + r.left() - X(30);
        QPainterPath body;
        // 车身：右侧长方体 + 左侧流线车头（三次曲线收窄）
        body.moveTo(right, trainTop);
        body.lineTo(left + X(100), trainTop);
        body.cubicTo(left + X(38), trainTop, left, trainTop + trainH * 0.35,
                     left, trainTop + trainH * 0.72);
        body.lineTo(left, trainTop + trainH);
        body.lineTo(right, trainTop + trainH);
        body.closeSubpath();
        p.drawPath(body);
        // 车头前窗：小块底色挖窗，点出流线造型
        p.setBrush(QColor(205, 215, 238, 150));
        QPainterPath win;
        win.moveTo(left + X(22), trainTop + trainH * 0.52);
        win.cubicTo(left + X(40), trainTop + trainH * 0.20, left + X(64), trainTop + Y(7),
                    left + X(86), trainTop + Y(7));
        win.lineTo(left + X(86), trainTop + trainH * 0.52);
        win.closeSubpath();
        p.drawPath(win);
        // 尾流线：车身右上方两条渐短线，强化「向左飞驰」方向感
        p.setBrush(Qt::NoBrush);
        QPen streak(QColor(255, 255, 255, 130), Y(4), Qt::SolidLine, Qt::RoundCap);
        p.setPen(streak);
        p.drawLine(QPointF(right - X(120), trainTop - Y(8)), QPointF(right - X(30), trainTop - Y(8)));
        streak.setWidthF(Y(3));
        streak.setColor(QColor(255, 255, 255, 90));
        p.setPen(streak);
        p.drawLine(QPointF(right - X(80), trainTop - Y(16)), QPointF(right - X(20), trainTop - Y(16)));
        p.setPen(Qt::NoPen);
        p.restore();
    }

    // ---------- 底部浅蓝色带（票面下沿色带特征） ----------
    const QRectF band(r.left(), r.bottom() - Y(kBottomBandH), r.width(), Y(kBottomBandH));
    p.fillRect(band, QColor(159, 182, 220, 160));
    p.setPen(QPen(QColor(130, 155, 200, 140), 1));
    p.drawLine(QPointF(r.left(), band.top()), QPointF(r.right(), band.top()));

    const QColor ink = QColor("#16233F");    // 票面主墨色（深藏蓝）
    const QColor subInk = QColor("#46587A"); // 次级墨色
    const double mainRight = r.left() + X(kStubX) - X(26); // 主区右边界

    // ---------- 第一行：红字票号（左） + 检票口（右，按班次号确定） ----------
    QFont f = p.font();
    f.setPixelSize(qMax(12, int(Y(17))));
    f.setBold(true);
    p.setFont(f);
    p.setPen(QColor("#C0392B"));
    p.drawText(QRectF(r.left() + X(28), r.top() + Y(14), X(300), Y(26)),
               Qt::AlignLeft | Qt::AlignVCenter, m_ticketNo);
    quint32 gateBase = 0;
    for (const QChar ch : m_train->no())
        gateBase += ch.unicode();
    const int gate = int(gateBase + quint32(m_carriage)) % 12 + 1;
    p.setPen(ink);
    p.drawText(QRectF(mainRight - X(220), r.top() + Y(14), X(220), Y(26)),
               Qt::AlignRight | Qt::AlignVCenter, QString("检票口:B%1").arg(gate));

    // ---------- 第二/三行：站名大字 + 车次居中 + 拼音小字 ----------
    const QString from = m_train->from() + QStringLiteral("站");
    const QString to = m_train->to() + QStringLiteral("站");
    const QRectF stationRow(r.left() + X(28), r.top() + Y(56), mainRight - r.left() - X(28), Y(62));
    f.setPixelSize(qMax(22, int(Y(44))));
    f.setBold(true);
    p.setFont(f);
    p.setPen(ink);
    p.drawText(stationRow.adjusted(0, 0, -X(180), 0), Qt::AlignLeft | Qt::AlignVCenter, from);
    p.drawText(stationRow, Qt::AlignRight | Qt::AlignVCenter, to);
    f.setPixelSize(qMax(13, int(Y(21))));
    p.setFont(f);
    p.drawText(stationRow, Qt::AlignCenter, m_train->no() + QStringLiteral("次"));

    f.setPixelSize(qMax(9, int(Y(12))));
    f.setBold(false);
    p.setFont(f);
    p.setPen(subInk);
    const QString pyFrom = pinyinOf(m_train->from());
    const QString pyTo = pinyinOf(m_train->to());
    if (!pyFrom.isEmpty() || !pyTo.isEmpty()) {
        p.drawText(stationRow.adjusted(0, 0, -X(180), 0).translated(0, Y(60)),
                   Qt::AlignLeft | Qt::AlignHCenter, pyFrom);
        p.drawText(stationRow.translated(0, Y(60)), Qt::AlignRight | Qt::AlignHCenter, pyTo);
    }

    // ---------- 第四行：发车日期时间 + 车厢座位号 ----------
    // 真票日期格式为 2026年09月05日；数据存储为 YYYY-MM-DD，转换为票面格式
    const QStringList ymd = m_train->date().split(QLatin1Char('-'));
    const QString dateText = ymd.size() == 3
        ? QString("%1年%2月%3日 %4开").arg(ymd[0], ymd[1], ymd[2], m_train->departTime())
        : QString("%1 %2开").arg(m_train->date(), m_train->departTime());
    const QRectF infoCol(r.left() + X(28), r.top() + Y(164), mainRight - r.left() - X(28), Y(160));
    f.setPixelSize(qMax(11, int(Y(17))));
    p.setFont(f);
    p.setPen(ink);
    p.drawText(QRectF(infoCol.left(), infoCol.top(), infoCol.width() - X(70), Y(28)),
               Qt::AlignLeft | Qt::AlignVCenter, dateText);
    f.setBold(true);
    p.setFont(f);
    p.drawText(QRectF(infoCol.right() - X(170), infoCol.top(), X(170), Y(28)),
               Qt::AlignRight | Qt::AlignVCenter,
               QString("%1车%2号").arg(m_carriage).arg(m_seatNo, 3, 10, QLatin1Char('0')));

    // ---------- 第五行：票价（大字加重） + 席别 + 支付标记 ----------
    f.setPixelSize(qMax(13, int(Y(22))));
    p.setFont(f);
    p.setPen(ink);
    p.drawText(QRectF(infoCol.left(), infoCol.top() + Y(42), X(180), Y(30)),
               Qt::AlignLeft | Qt::AlignVCenter,
               QString("¥%1元").arg(m_train->priceOf(m_carriage), 0, 'f', 1));
    f.setPixelSize(qMax(11, int(Y(16))));
    f.setBold(false);
    p.setFont(f);
    p.drawText(QRectF(infoCol.left() + X(160), infoCol.top() + Y(44), X(200), Y(28)),
               Qt::AlignLeft | Qt::AlignVCenter, m_train->carriageClassText(m_carriage));
    const QString payMark = m_payMethod.contains(QStringLiteral("支付宝")) ? QStringLiteral("支")
        : m_payMethod.contains(QStringLiteral("微信"))   ? QStringLiteral("微")
        : m_payMethod.contains(QStringLiteral("银联"))   ? QStringLiteral("云")
                                                         : QStringLiteral("网");
    p.drawText(QRectF(infoCol.left() + X(255), infoCol.top() + Y(44), X(70), Y(28)),
               Qt::AlignLeft | Qt::AlignVCenter, QString("(%1)").arg(payMark));

    // ---------- 第六行：限乘当日当次车 ----------
    f.setPixelSize(qMax(10, int(Y(14))));
    p.setFont(f);
    p.setPen(subInk);
    p.drawText(QRectF(infoCol.left(), infoCol.top() + Y(82), X(300), Y(20)),
               Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("限乘当日当次车"));

    // ---------- 第七行：证件号（打码） + 姓名 ----------
    f.setPixelSize(qMax(11, int(Y(16))));
    f.setBold(true);
    p.setFont(f);
    p.setPen(ink);
    p.drawText(QRectF(infoCol.left(), infoCol.top() + Y(112), infoCol.width() - X(40), Y(26)),
               Qt::AlignLeft | Qt::AlignVCenter,
               QString("%1  %2").arg(maskId(m_id), m_name));

    // ---------- 第八行：服务标语 ----------
    f.setPixelSize(qMax(9, int(Y(12))));
    f.setBold(false);
    p.setFont(f);
    p.setPen(subInk);
    p.drawText(QRectF(r.left() + X(28), band.top() - Y(30), mainRight - r.left() - X(28), Y(20)),
               Qt::AlignLeft | Qt::AlignVCenter,
               QStringLiteral("买票请到12306 发货请到95306 中国铁路祝您旅途愉快"));

    // ---------- 第九行（色带内）：21 位售票码 + 售票站 ----------
    f.setPixelSize(qMax(9, int(Y(13))));
    p.setFont(f);
    p.setPen(ink);
    p.drawText(QRectF(r.left() + X(28), band.top(), mainRight - r.left() - X(28), band.height()),
               Qt::AlignLeft | Qt::AlignVCenter, m_saleCode);
    p.drawText(QRectF(r.right() - X(220), band.top(), X(196), band.height()),
               Qt::AlignRight | Qt::AlignVCenter, m_train->from() + QStringLiteral("南售"));

    // ---------- 副联分隔：竖向齿孔虚线（撕票线设计元素） ----------
    const double stubX = r.left() + X(kStubX);
    QPen dashPen(QColor(120, 145, 190, 170), 1, Qt::DashLine);
    dashPen.setDashOffset(4);
    p.setPen(dashPen);
    p.drawLine(QPointF(stubX, r.top() + Y(10)), QPointF(stubX, band.top() - Y(4)));
    // 齿孔上下端半圆缺口（用票底色系填充模拟打孔）
    p.setPen(QPen(QColor("#8FA3C8"), 1));
    p.setBrush(QColor("#DCE3F4"));
    p.drawEllipse(QPointF(stubX, r.top() + Y(10)), X(5), Y(5));
    p.drawEllipse(QPointF(stubX, band.top() - Y(4)), X(5), Y(5));

    // ---------- 副联区：中国铁路标识 + 二维码 + 订单号 + 支付信息 ----------
    const double stubCX = stubX + (r.right() - stubX) / 2;
    f.setPixelSize(qMax(11, int(Y(15))));
    f.setBold(true);
    p.setFont(f);
    p.setPen(ink);
    p.drawText(QRectF(stubX, r.top() + Y(18), r.right() - stubX, Y(22)),
               Qt::AlignCenter, QStringLiteral("中国铁路"));
    f.setPixelSize(qMax(7, int(Y(9))));
    p.setFont(f);
    p.setPen(subInk);
    p.drawText(QRectF(stubX, r.top() + Y(40), r.right() - stubX, Y(14)),
               Qt::AlignCenter, QStringLiteral("CHINA RAILWAY"));

    // 方形二维码（真票为黑白二维防伪图案），固定正方形
    const double qrSize = qMin(X(128), r.bottom() - Y(120) - (r.top() + Y(64)));
    const QRectF qrArea(stubCX - qrSize / 2, r.top() + Y(64), qrSize, qrSize);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 220));
    p.drawRoundedRect(qrArea.adjusted(-4, -4, 4, 4), 4, 4);
    drawQr(p, qrArea);

    f.setPixelSize(qMax(8, int(Y(11))));
    f.setBold(false);
    p.setFont(f);
    p.setPen(ink);
    p.drawText(QRectF(stubX, qrArea.bottom() + Y(10), r.right() - stubX, Y(16)),
               Qt::AlignCenter, QString("订单号 %1").arg(m_orderNo));
    f.setPixelSize(qMax(8, int(Y(10))));
    p.setFont(f);
    p.setPen(subInk);
    p.drawText(QRectF(stubX, qrArea.bottom() + Y(28), r.right() - stubX, Y(15)),
               Qt::AlignCenter, m_payMethod + QStringLiteral(" · 已支付"));
    f.setPixelSize(qMax(7, int(Y(9))));
    p.setFont(f);
    p.drawText(QRectF(stubX, qrArea.bottom() + Y(44), r.right() - stubX, Y(13)),
               Qt::AlignCenter, m_payTime);

    p.setClipping(false);
}

// 方形二维码：确定性伪 QR——21×21 模块（版本 1），三个 7×7 定位图案 + 时序线，
// 数据区以售票码+座位为种子随机填充。纯装饰，不可扫描。
void TicketCard::drawQr(QPainter &p, const QRectF &area) const
{
    constexpr int N = 21;
    const double m = area.width() / N;
    QRandomGenerator rng(qHash(m_saleCode) ^ (quint64(m_carriage) << 20) ^ quint64(m_seatNo));
    bool cells[N][N] = {};

    const auto finder = [&](int ox, int oy) {
        for (int dy = 0; dy < 7; ++dy)
            for (int dx = 0; dx < 7; ++dx) {
                const bool ring = dx == 0 || dx == 6 || dy == 0 || dy == 6;
                const bool core = dx >= 2 && dx <= 4 && dy >= 2 && dy <= 4;
                cells[oy + dy][ox + dx] = ring || core;
            }
    };
    finder(0, 0);
    finder(N - 7, 0);
    finder(0, N - 7);
    for (int i = 8; i < N - 8; i += 2) {
        cells[6][i] = true;
        cells[i][6] = true;
    }
    for (int y = 0; y < N; ++y)
        for (int x = 0; x < N; ++x)
            if (!cells[y][x])
                cells[y][x] = rng.bounded(2) == 1;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#1B2430"));
    for (int y = 0; y < N; ++y)
        for (int x = 0; x < N; ++x)
            if (cells[y][x])
                p.drawRect(QRectF(area.left() + x * m, area.top() + y * m, m + 0.3, m + 0.3));
}
