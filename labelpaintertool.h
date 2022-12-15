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
#include <QScrollArea>

#include <QDebug>
#include <QMouseEvent>

#include <QPixmap>

enum direction { ne = 0, nw = 1, sw, se };
enum operateMode { choose = 0, edit = 1 };

struct roiRect
{
    double offset = 0;                       // 8
    direction d = static_cast<direction>(0); // 4
    QImage img;
    QRectF rect;
};

class LabelPainterTool : public QLabel
{
    Q_OBJECT
public:
    explicit LabelPainterTool(QWidget* parent = nullptr);
    ~LabelPainterTool() override;

    void paintEvent(QPaintEvent* event) override;        //绘制矩形
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent* event) override;   //鼠标按下
    void mouseMoveEvent(QMouseEvent* event) override;    //鼠标移动
    void mouseReleaseEvent(QMouseEvent* event) override; //鼠标抬起

    void onZoomInImage(void);
    void onZoomOutImage(void);

    void addFieldRectangle(std::vector<roiRect>& roiRects);

    void setImg(QImage& img) { mImage = img; }
    QImage getImg() const { return mImage; }
    void setOperateMode(const operateMode& op) { mOpMode = op; }
    void clearFieldRect();
    std::vector<roiRect> getRoIRects(const QImage& img, const QVector<QVector<bool>>& roiPos,
                                     const int& imgW, const int& imgH, const double& roiW,
                                     const double& roiH);

signals:
    void sendFieldRects(std::vector<roiRect>& rects);

private:

    inline QPointF WidgetToImgRelativePos(const QPointF& pos);
    // 将矩形从以左上角为原点的坐标系坐标，转化为以图像中心为原点的坐标系坐标
    inline QRectF rectToRelativePos(const QRectF& rect);
    inline QPointF ToImgAbsolutePos(const QPointF& pos);
    // 将矩形从以图像中心为原点的坐标系坐标，转化为以左上角为原点的坐标系坐标
    inline QRectF rectToImgAbsolutePos(const QRectF& rect);
    // 从一个中心点以及长宽，得到一个矩形
    inline QRectF fromCenterPoint(const QPointF& centerPoint, double width, double height);

    inline void setOffsetW(double val) { mOffsetW = val; }
    inline void setOffsetH(double val) { mOffsetH = val; }
    inline void setXPtInterval(double val) { mXPtInterval = val; }
    inline void setYPtInterval(double val) { mYPtInterval = val; }
    inline void setZoomValueW(double val) { mZoomValue = val; }

    double mOffsetW = 0;     // 以中心点为坐标轴时坐标转化参数
    double mOffsetH = 0;     // 以中心点为坐标轴时坐标转化参数
    double mXPtInterval = 0; // 移动画面的X偏移
    double mYPtInterval = 0; // 移动画面的y偏移
    qreal mZoomValue = 1.0;  // 放大缩小系数

    std::vector<roiRect> mFieldRects; // 十三个视场矩形
    QImage mImage;

    // 状态相关
    bool mPressed = false;
    operateMode mOpMode = choose;
    QPointF mMouseStartPoint;
    QPointF mMouseEndPoint;
    int mSelectedROIRectIndex; // 用于拖动ROI
};

#endif // MyLabel_H
