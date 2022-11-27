#ifndef MyLabel_H
#define MyLabel_H

#include <QLabel>
#include <QPoint>
#include <QPainter>
#include <QColor>
#include <QPaintEvent>
#include <QPen>
#include <QPainter>
#include <QWidget>

#include <QDebug>
#include <QMouseEvent>

#include <QPixmap>

enum direction { ne = 0, nw = 1, sw, se };

struct roiRect
{
    double offset = 0;
    direction d = (direction)0;
    QImage img;
    QRectF rect;
};

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
    void addRectangle(QRectF rect);
    void addRectangle(int x, int y, int h, int w);
    void addRectangle(QVector<roiRect> roiRects);

private:
    QVector<QRectF> mRects;
};

class myRectProcessor{
public:
    myRectProcessor() = default;
    // 根据roiPos的布尔值得到img对应的所有ROI区域
    QVector<roiRect> getRoIRects(const QImage& img, const QVector<QVector<bool>>& roiPos,
                       const int& imgW, const int& imgH, const double& roiW, const double& roiH);
    inline QPointF ToRelativePos(const QPointF& pos);
    // 将矩形从以左上角为原点的坐标系坐标，转化为以图像中心为原点的坐标系坐标
    inline QRectF rectToRelativePos(const QRectF& rect);
    inline QPointF ToAbsolutePos(const QPointF& pos);
    // 将矩形从以图像中心为原点的坐标系坐标，转化为以左上角为原点的坐标系坐标
    inline QRectF rectToAbsolutePos(const QRectF& rect);
    // 从一个中心点以及长宽，得到一个矩形
    inline QRectF fromCenterPoint(const QPointF& centerPoint, double width, double height);

    inline void setOffsetW(double val){mOffsetW = val;}
    inline void setOffsetH(double val){mOffsetH = val;}
    inline void setOffset(double w, double h){mOffsetW = w; mOffsetH = h;}
    inline void setXPtInterval(double val){mXPtInterval = val;}
    inline void setYPtInterval(double val){mYPtInterval = val;}
    inline void setInterval(double x, int y){mXPtInterval = x; mYPtInterval = y;}
    inline void setZoomValueW(double val){mZoomValue = val;}

private:
    double mOffsetW = 0; // 以中心点为坐标轴时坐标转化参数
    double mOffsetH = 0; // 以中心点为坐标轴时坐标转化参数
    double mXPtInterval = 0;    // 移动画面的X偏移
    double mYPtInterval = 0;    // 移动画面的y偏移
    qreal mZoomValue = 1.0; // 放大缩小系数
};

#endif // MyLabel_H
