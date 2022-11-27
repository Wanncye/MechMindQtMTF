#include "labelpaintertool.h"

LabelPainterTool::LabelPainterTool(QWidget *parent) : QLabel(parent){}
LabelPainterTool::~LabelPainterTool(){}

void LabelPainterTool::paintEvent(QPaintEvent *event){
    QLabel::paintEvent(event);
    QPainter painter(this);
    painter.setPen(QPen(Qt::red,10));
    for(const auto& rect : mRects)
        painter.drawRect(rect);
}
//鼠标按下
void LabelPainterTool::mousePressEvent(QMouseEvent *e){
//    auto mOldPos = e->pos();
//    addRangle(mOldPos.x(), mOldPos.y(), 90, 90);
//    update();
}
void LabelPainterTool::mouseMoveEvent(QMouseEvent *e){}    //鼠标移动
void LabelPainterTool::mouseReleaseEvent(QMouseEvent *e){} //鼠标抬起
void LabelPainterTool::wheelEvent(QWheelEvent* event)
{
    int value = event->angleDelta().y();
    if (value > 0)
        onZoomInImage();
    else
        onZoomOutImage();
    update();
}

void LabelPainterTool::onZoomInImage(void){
    rectProcessor->mZoomValue += 0.2;
    if (rectProcessor->mZoomValue >= 2) {
        rectProcessor->mZoomValue = 2;
    }
}

void LabelPainterTool::onZoomOutImage(void){
    rectProcessor->mZoomValue -= 0.2;
    if (rectProcessor->mZoomValue <= 1) {
        rectProcessor->mZoomValue = 1;
    }
}

void LabelPainterTool::addRectangle(QRectF rect){
    mRects.push_back(rect);
}

void LabelPainterTool::addRectangle(int x, int y, int w, int h){
    addRectangle(QRectF(x,y,w,h));
}

void LabelPainterTool::addRectangle(QVector<roiRect> roiRects){
    for(auto& roi : roiRects)
        mRects.push_back(roi.rect);
}

QVector<roiRect> myRectProcessor::getRoIRects(const QImage& img,
                                                 const QVector<QVector<bool>>& roiPos,
                                                 const int& imgW, const int& imgH,
                                                 const double& roiW, const double& roiH)
{
    QVector<roiRect> roiRects;
    mOffsetH = -imgH / 2.0;
    mOffsetW = -imgW / 2.0;
    QRectF ImgRect = QRectF(-imgW / 2.0, -imgH / 2.0, imgW, imgH);

    // 得到图像的四个角的坐标，用于计算ROI的offset
    QVector<QPointF> points = {ImgRect.topRight(), ImgRect.topLeft(), ImgRect.bottomLeft(), ImgRect.bottomRight()};
    for (int row = 0; row < points.size(); row++) {
        for (int col = 0; col < roiPos[row].size(); col++) {
            if(!roiPos[row][col])
                continue;
            roiRect newRect;
            newRect.d = static_cast<direction>(row);
            newRect.offset = (col + 1) * 0.1;
            newRect.rect = rectToAbsolutePos(fromCenterPoint(points[row] * newRect.offset, roiW, roiH));
            newRect.img = img.copy(newRect.rect.toRect());
            roiRects.append(newRect);
        }
    }
    return roiRects;
}

QPointF myRectProcessor::ToRelativePos(const QPointF& pos)
{
    return (pos + QPointF(mOffsetW - mXPtInterval, mOffsetH - mYPtInterval)) / mZoomValue;
}
QPointF myRectProcessor::ToAbsolutePos(const QPointF& pos)
{
    return pos * mZoomValue - QPointF(mOffsetW, mOffsetH);
}

QRectF myRectProcessor::rectToRelativePos(const QRectF& rect)
{
    return QRectF{ToRelativePos(rect.topLeft()), ToRelativePos(rect.bottomRight())};
}

QRectF myRectProcessor::rectToAbsolutePos(const QRectF& rect)
{
    return QRectF{ToAbsolutePos(rect.topLeft()), ToAbsolutePos(rect.bottomRight())};
}

QRectF myRectProcessor::fromCenterPoint(const QPointF& centerPoint, double width, double height)
{
    return {centerPoint.x() - width / 2.0, centerPoint.y() - height / 2.0, width, height};
};
