#include "form.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
Form::Form(QWidget *parent) :
    QWidget(parent),
    m_bChecked(false),
    m_background(Qt::gray),
    m_checkedColor(34, 131, 246),
    m_disabledColor(190, 190, 190),
    m_thumbColor(Qt::gray),
    m_radius(12.5),
    m_nHeight(16),
    m_nMargin(3)
{
    /* 鼠标滑过光标形状 - 手型 */
    setCursor(Qt::PointingHandCursor);
}

Form::~Form()
{

}

void Form::paintEvent(QPaintEvent *event)
{
    qDebug()<<"paintEvent";
    Q_UNUSED(event)
    m_nX = m_nHeight / 2;
    m_nY = m_nHeight / 2;

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    QColor background;
    QColor thumbColor;
    qreal dOpacity;
    /* 可用状态 */
    if (isEnabled()) {
        /* 打开状态 */
        if (m_bChecked) {
            background = m_checkedColor;
            thumbColor = m_checkedColor;
            dOpacity = 0.600;
            /* 关闭状态 */
        } else {
            background = m_background;
            thumbColor = m_thumbColor;
            dOpacity = 0.800;
        }
        /* 不可用状态 */
    } else {
        background = m_background;
        dOpacity = 0.260;
        thumbColor = m_disabledColor;
    }
    /* 绘制大椭圆 */
    painter.setBrush(background);
    painter.setOpacity(dOpacity);
    path.addRoundedRect(QRectF(m_nMargin,
                               m_nMargin, width() - 2 * m_nMargin,
                               height() - 2 * m_nMargin),
                        m_radius, m_radius);
    painter.drawPath(path.simplified());

    /* 绘制小椭圆 */
    painter.setBrush(thumbColor);
    painter.setOpacity(1.0);
    painter.drawEllipse(QRectF(m_nX - (m_nHeight / 2),
                               m_nY - (m_nHeight / 2),
                               height(),
                               height()));
}

void Form::mousePressEvent(QMouseEvent *event)
{

}

void Form::mouseReleaseEvent(QMouseEvent *event)
{

}

void Form::resizeEvent(QResizeEvent *event)
{
    qDebug()<<"resizeEvent";
    QWidget::resizeEvent(event);
}

QSize Form::sizeHint() const
{
//    return minimumSizeHint();
}

QSize Form::minimumSizeHint() const
{
//    return QSize(2 * (m_nHeight + m_nMargin),
//                 m_nHeight + 2 * m_nMargin);
}
