#ifndef MyLabel_H
#define MyLabel_H

#include <QLabel>
#include <QPoint>
#include <QPainter>
#include <QColor>
#include <QPaintEvent>
#include <QPen>
#include<QPainter>
#include <QWidget>

#include <QDebug>
#include <QMouseEvent>

#include <QPixmap>

class LabelPainterTool : public QLabel
{
    //Q_OBJECT
public:
    explicit LabelPainterTool(QWidget *parent = nullptr);
    ~LabelPainterTool();

    void paintEvent(QPaintEvent *event);    //绘制矩形
    void mousePressEvent(QMouseEvent *e);   //鼠标按下
    void mouseMoveEvent(QMouseEvent *e);    //鼠标移动
    void mouseReleaseEvent(QMouseEvent *e); //鼠标抬起
    void addRangle(QRectF rect);
    void addRangle(int x, int y, int h, int w);

private:
    QVector<QRectF> mRects;
};

#endif // MyLabel_H
