#ifndef PALETTE_H
#define PALETTE_H

#include <QColor>

// 全局调色板：与 theme.qss 的品牌色系保持一致（QSS 无变量机制，两侧需人工同步）。
// 命名按语义；新增颜色前先确认是否已有同义色，禁止引入近似色变体。
namespace Palette {

// 基础色
inline const QColor kWhite       = QColor("#FFFFFF");
inline const QColor kWindowBg    = QColor("#F4F6FA"); // 页面背景
inline const QColor kTableAlt    = QColor("#F8FAFD"); // 表格交替行
inline const QColor kBorder      = QColor("#E3E7EF"); // 卡片/分隔线边框

// 文字
inline const QColor kText        = QColor("#1B2430"); // 主文字
inline const QColor kTextMuted   = QColor("#667085"); // 次要文字（表头、图例）
inline const QColor kTextFaint   = QColor("#98A1B3"); // 弱文字（副标题）
inline const QColor kTextGhost   = QColor("#A8B0BF"); // 占位/禁用文字

// 品牌
inline const QColor kBrand       = QColor("#2F6FED"); // 品牌蓝（选中/高亮）
inline const QColor kDanger      = QColor("#E5484D"); // 危险/已售红
inline const QColor kSuccess     = QColor("#1E7B45"); // 成功/空座绿

// 环形图淡色切片
inline const QColor kDonutSold     = QColor("#F2A6A1"); // 已售·淡红
inline const QColor kDonutRemain   = QColor("#A5D0AF"); // 余票·淡绿
inline const QColor kDonutCallout  = QColor("#B6BDCB"); // 引出线灰

} // namespace Palette

#endif // PALETTE_H
