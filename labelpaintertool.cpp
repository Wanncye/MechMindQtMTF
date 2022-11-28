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
    painter.translate(windowsWidth / 2.0 + mRectProcessor->mXPtInterval,
                      windowsHeight / 2.0 + mRectProcessor->mYPtInterval);

    painter.scale(mRectProcessor->mZoomValue, mRectProcessor->mZoomValue);

    // windowRect要按当前的坐标系来计算，imgRect要按图片的坐标系计算
    // 且这里不能使用QLabel的setPixmap
    QRectF windowRect(-windowsWidth / 2.0, -windowsHeight / 2.0, windowsWidth, windowsHeight);
    QRectF imgRect(0, 0, imgWidth, imgHeight);
    painter.drawPixmap(windowRect, QPixmap::fromImage(mImage), imgRect);
    setPixmap(QPixmap::fromImage(mImage));

    // 十三个视场绘制
    painter.setPen(QPen(Qt::red, 4));
    for (const auto& roi : mFieldRects) {
        painter.drawRect(roi.rect);
        if (roi.checked)
            painter.fillRect(roi.rect, Qt::red);
    }

    // 中心点
    painter.drawRect(QRectF(-1, -1, 2, 2));

    // 手动选择的视场绘制
    for (const auto& rect : mManualRects)
        painter.drawRect(rect);
}

//鼠标按下
void LabelPainterTool::mousePressEvent(QMouseEvent* event)
{
    mPressed = true;
    switch (mOpMode) {
    case choose:
    {
        QPointF mouseRelativePos = mRectProcessor->ToRelativePos(event->pos());
        for (auto& roi : mFieldRects) {
            if (roi.rect.contains(mouseRelativePos)) {
                roi.checked = !roi.checked;
                break;
            }
        }
        break;
    }
    case edit:
    {
        mMouseStartPoint = mRectProcessor->ToRelativePos(event->pos());
        mMouseEndPoint = mRectProcessor->ToRelativePos(event->pos());
        break;
    }
    default:
        break;
    }
}

//鼠标移动
void LabelPainterTool::mouseMoveEvent(QMouseEvent* event)
{
    if (!mPressed)
        return QWidget::mouseMoveEvent(event);
    setCursor(Qt::CrossCursor);

    switch (mOpMode) {
    case choose:
        break;
    case edit:
    {
        mMouseEndPoint = mRectProcessor->ToRelativePos(event->pos());
        break;
    }
    default:
        break;
    }
    //    QPointF pos = event->pos();
    //    double xPtInterval = pos.x() - mMouseStartPoint.x();
    //    double yPtInterval = pos.y() - mMouseStartPoint.y();
    //    mOldPos = pos;
    //    if (!isEditROIRect) {
    //        // 不是移动ROI操作，那就是移动画面的操作
    //        mXPtInterval += xPtInterval;
    //        mYPtInterval += yPtInterval;
    //        normalizeInterval(width(), height());
    //    } else {
    //        if (!isManual) {
    //            auto len = trueROIRects.length();
    //            if (selectedROIRectIndex >= len) {
    //                errROIRects[selectedROIRectIndex - len] =
    //                    QRectF(errROIRects[selectedROIRectIndex - len].topLeft() +
    //                               QPointF(xPtInterval, yPtInterval) / mZoomValue,
    //                           errROIRects[selectedROIRectIndex - len].bottomRight() +
    //                               QPointF(xPtInterval, yPtInterval) / mZoomValue);
    //            } else {
    //                trueROIRects[selectedROIRectIndex] =
    //                    QRectF(trueROIRects[selectedROIRectIndex].topLeft() +
    //                               QPointF(xPtInterval, yPtInterval) / mZoomValue,
    //                           trueROIRects[selectedROIRectIndex].bottomRight() +
    //                               QPointF(xPtInterval, yPtInterval) / mZoomValue);
    //            }
    //        } else {
    //            EndPoint = ToRelativePos(event->pos());
    //            emit StopPointSignal(EndPoint);
    //        }
    //    }

    update();
}

//鼠标抬起
void LabelPainterTool::mouseReleaseEvent(QMouseEvent* event)
{
    mPressed = false;
    setCursor(Qt::ArrowCursor);

    switch (mOpMode) {
    case choose:
        break;
    case edit:
    {
        mManualRects.append(QRectF(mMouseStartPoint, mMouseEndPoint));
        break;
    }
    default:
        break;
    }

    update();
}

void LabelPainterTool::onZoomInImage(void)
{
    mRectProcessor->mZoomValue += 0.2;
    if (mRectProcessor->mZoomValue >= 2) {
        mRectProcessor->mZoomValue = 2;
    }
    update();
}

void LabelPainterTool::onZoomOutImage(void)
{
    mRectProcessor->mZoomValue -= 0.2;
    if (mRectProcessor->mZoomValue <= 0.2) {
        mRectProcessor->mZoomValue = 0.2;
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
            newRect.checked = false;
            roiRects.push_back(newRect);
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
