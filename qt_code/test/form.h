#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QTimer>
class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = nullptr);
    ~Form();
protected:
    /* 绘制开关 */
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    /* 鼠标按下事件 */
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    /* 鼠标释放事件 - 切换开关状态、发射toggled()信号 */
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    /* 大小改变事件 */
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

    /* 缺省大小 */
    QSize sizeHint() const Q_DECL_OVERRIDE;
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
private:
    /* 是否选中 */
    bool m_bChecked;

    /* 背景颜色 */
    QColor m_background;

    /* 选中颜色 */
    QColor m_checkedColor;

    /* 不可用颜色 */
    QColor m_disabledColor;

    /* 拇指颜色 */
    QColor m_thumbColor;

    /* 圆角 */
    qreal m_radius;

    /* x点坐标 */
    qreal m_nX;

    /* y点坐标 */
    qreal m_nY;

    /* 高度 */
    qint16 m_nHeight;

    /* 外边距 */
    qint16 m_nMargin;

    /* 定时器 */
    QTimer m_timer;
};

#endif // FORM_H
