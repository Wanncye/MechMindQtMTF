#include "labelpaintertool.h"
#include <QStyleOption>
#include <QScrollArea>
LabelPainterTool::LabelPainterTool(QWidget* parent) : QLabel(parent) {}
LabelPainterTool::~LabelPainterTool() {}

void LabelPainterTool::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    if (mImage.isNull())
        return QWidget::paintEvent(event);

    const int imgWidth = mImage.width();
    const int imgHeight = mImage.height();
    const int windowsWidth = this->width();
    const int windowsHeight = this->height();

    // 以左上角为原点，将坐标原点往右下角平移这么多像素
    painter.translate(windowsWidth / 2.0 + rectProcessor->mXPtInterval,
                      windowsHeight / 2.0 + rectProcessor->mYPtInterval);

    painter.scale(rectProcessor->mZoomValue, rectProcessor->mZoomValue);

    // windowRect要按当前的坐标系来计算，imgRect要按图片的坐标系计算
    // 且这里不能使用QLabel的setPixmap
    QRectF windowRect(-windowsWidth / 2.0, -windowsHeight / 2.0, windowsWidth, windowsHeight);
    QRectF imgRect(0, 0, imgWidth, imgHeight);
    painter.drawPixmap(windowRect, QPixmap::fromImage(mImage), imgRect);
    setPixmap(QPixmap::fromImage(mImage));

    // 十三个视场绘制
    painter.setPen(QPen(Qt::red, 4));
    for (const auto& rect : mFieldRects)
        painter.drawRect(rect.rect);
    painter.drawRect(QRectF(-1, -1, 2, 2));

    // 手动选择的视场绘制
    for (const auto& rect : mManualRects)
        painter.drawRect(rect.rect);
}

//鼠标按下
void LabelPainterTool::mousePressEvent(QMouseEvent* e)
{
    //    auto mOldPos = e->pos();
    //    addRangle(mOldPos.x(), mOldPos.y(), 90, 90);
    //    update();
}
void LabelPainterTool::mouseMoveEvent(QMouseEvent* e) {}    //鼠标移动
void LabelPainterTool::mouseReleaseEvent(QMouseEvent* e) {} //鼠标抬起

void LabelPainterTool::onZoomInImage(void)
{
    rectProcessor->mZoomValue += 0.2;
    if (rectProcessor->mZoomValue >= 2) {
        rectProcessor->mZoomValue = 2;
    }
    update();
}

void LabelPainterTool::onZoomOutImage(void)
{
    rectProcessor->mZoomValue -= 0.2;
    if (rectProcessor->mZoomValue <= 0.2) {
        rectProcessor->mZoomValue = 0.2;
    }
    update();
}

void LabelPainterTool::addFieldRectangle(QVector<roiRect>& roiRects)
{
    for (auto& roi : roiRects)
        mFieldRects.push_back(roi);
    update();
}

QVector<roiRect> myRectProcessor::getRoIRects(const QImage& img,
                                              const QVector<QVector<bool>>& roiPos, const int& imgW,
                                              const int& imgH, const double& roiW,
                                              const double& roiH)
{
    QVector<roiRect> roiRects;
    mOffsetH = -imgH / 2.0;
    mOffsetW = -imgW / 2.0;
    QRectF ImgRect = QRectF(-imgW / 2.0, -imgH / 2.0, imgW, imgH);

    // 得到图像的四个角的坐标，用于计算ROI的offset
    QVector<QPointF> points = {ImgRect.topRight(), ImgRect.topLeft(), ImgRect.bottomLeft(),
                               ImgRect.bottomRight()};
    for (int row = 0; row < points.size(); row++) {
        for (int col = 0; col < roiPos[row].size(); col++) {
            if (!roiPos[row][col])
                continue;
            roiRect newRect;
            newRect.d = static_cast<direction>(row);
            newRect.offset = (col + 1) * 0.1;
            newRect.rect = fromCenterPoint(points[row] * newRect.offset * mZoomValue, roiW, roiH);
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
