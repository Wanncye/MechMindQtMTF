#include "labelpaintertool.h"
#include <QStyleOption>
LabelPainterTool::LabelPainterTool(QWidget *parent) : QLabel(parent){}
LabelPainterTool::~LabelPainterTool(){}

void LabelPainterTool::paintEvent(QPaintEvent *event){
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if (mImage.isNull())
        return QWidget::paintEvent(event);

    const int width = mImage.width();
    const int height = mImage.height();

    // 这一句相当于以左上角为原点，将坐标原点往右下角平移这么多像素
    painter.translate(this->width() / 2.0 + rectProcessor->mXPtInterval,
                      this->height() / 2.0 + rectProcessor->mYPtInterval);

    painter.scale(rectProcessor->mZoomValue, rectProcessor->mZoomValue);

    QRectF picRect(-width / 2.0, -height / 2.0, width, height);
    painter.drawImage(picRect, mImage);

    painter.setPen(QPen(Qt::red, 4));
    for(const auto& rect : mFieldRects)
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

void LabelPainterTool::onZoomInImage(void){
    rectProcessor->mZoomValue += 0.2;
    if (rectProcessor->mZoomValue >= 2) {
        rectProcessor->mZoomValue = 2;
    }
    update();
}

void LabelPainterTool::onZoomOutImage(void){
    rectProcessor->mZoomValue -= 0.2;
    if (rectProcessor->mZoomValue <= 0.2) {
        rectProcessor->mZoomValue = 0.2;
    }
    update();
}

void LabelPainterTool::addFieldRectangle(QVector<roiRect> roiRects){
    for(auto& roi : roiRects)
        mFieldRects.push_back(roi.rect);
    update();
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
    qDebug() << "ImgRect.topRight(): " << ImgRect.topRight().x() << ImgRect.topRight().y();
    qDebug() << "ImgRect.topLeft(): " << ImgRect.topLeft().x() << ImgRect.topLeft().y();
    qDebug() << "ImgRect.bottomLeft(): " << ImgRect.bottomLeft().x() << ImgRect.bottomLeft().y();
    qDebug() << "ImgRect.bottomRight(): " << ImgRect.bottomRight().x() << ImgRect.bottomRight().y();
    for (int row = 0; row < points.size(); row++) {
        for (int col = 0; col < roiPos[row].size(); col++) {
            if(!roiPos[row][col])
                continue;
            roiRect newRect;
            newRect.d = static_cast<direction>(row);
            newRect.offset = (col + 1) * 0.1;
            newRect.rect = rectToAbsolutePos(fromCenterPoint(points[row] * newRect.offset * mZoomValue, roiW, roiH));
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
