#include "labelpaintertool.h"
#include <QStyleOption>
#include <QScrollArea>

LabelPainterTool::LabelPainterTool(QWidget* parent) : QLabel(parent) {}
LabelPainterTool::~LabelPainterTool() {}

#define print(val) qDebug() << #val << val

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

    // 视场绘制
    painter.setPen(QPen(Qt::red, 4));
    for (const auto& roi : mFieldRects) {
        painter.drawRect(roi.rect);
    }

    // 中心点
    painter.drawRect(QRectF(-1, -1, 2, 2));
}

//鼠标按下
void LabelPainterTool::mousePressEvent(QMouseEvent* event)
{
    mPressed = true;
    mSelectedROIRectIndex = -1;
    print(event->pos());
    mMouseStartPoint = mRectProcessor->ToRelativePos(event->pos());
    print(mMouseStartPoint);
    print(mRectProcessor->ToAbsolutePos(mMouseStartPoint));
    switch (mOpMode) {
    case choose:
    {
        break;
    }
    case edit:
    {
        for (int i = 0; i < mFieldRects.size(); i++) {
            if (mFieldRects.at(i).rect.contains(mMouseStartPoint)) {
                mSelectedROIRectIndex = i;
                break;
            }
        }

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

    QPointF mouseCurrentPos = mRectProcessor->ToRelativePos(event->pos());
    double xPtInterval = mouseCurrentPos.x() - mMouseStartPoint.x();
    double yPtInterval = mouseCurrentPos.y() - mMouseStartPoint.y();
    mMouseStartPoint = mouseCurrentPos;
    switch (mOpMode) {
    case choose:
        break;
    case edit:
    {
        // 移动ROI
        mFieldRects[mSelectedROIRectIndex].rect =
            QRectF(mFieldRects[mSelectedROIRectIndex].rect.topLeft() +
                       QPointF(xPtInterval, yPtInterval) / mRectProcessor->mZoomValue,
                   mFieldRects[mSelectedROIRectIndex].rect.bottomRight() +
                       QPointF(xPtInterval, yPtInterval) / mRectProcessor->mZoomValue);
        break;
    }
    default:
        break;
    }

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
        mFieldRects[mSelectedROIRectIndex].img = mImage.copy(
            mRectProcessor->rectToAbsolutePos(mFieldRects[mSelectedROIRectIndex].rect).toRect());
        print(mFieldRects[mSelectedROIRectIndex].rect.width());
        print(mFieldRects[mSelectedROIRectIndex].rect.height());
        print(mRectProcessor->rectToAbsolutePos(mFieldRects[mSelectedROIRectIndex].rect).width());
        print(mRectProcessor->rectToAbsolutePos(mFieldRects[mSelectedROIRectIndex].rect).height());
        print(mRectProcessor->rectToAbsolutePos(mFieldRects[mSelectedROIRectIndex].rect).toRect());

        mFieldRects[mSelectedROIRectIndex].img.save(
            "D:/MechMindQtMTF/img/img_" +
            QString::number(static_cast<int>(mFieldRects[mSelectedROIRectIndex].d)) +
            QString::number(mFieldRects[mSelectedROIRectIndex].offset) + ".png");
        emit sendFieldRects(mFieldRects);
        break;
    }
    default:
        break;
    }

    update();
}

void LabelPainterTool::onZoomInImage(void)
{
    mRectProcessor->mZoomValue += 0.1;
    if (mRectProcessor->mZoomValue >= 2) {
        mRectProcessor->mZoomValue = 2;
    }
    update();
}

void LabelPainterTool::onZoomOutImage(void)
{
    mRectProcessor->mZoomValue -= 0.1;
    if (mRectProcessor->mZoomValue <= 0.1) {
        mRectProcessor->mZoomValue = 0.1;
    }
    update();
}

void LabelPainterTool::addFieldRectangle(std::vector<roiRect>& roiRects)
{
    for (auto& roi : roiRects)
        mFieldRects.push_back(roi);

    update();
}

void LabelPainterTool::clearFieldRect() { mFieldRects.clear(); }

// 得到的是移动坐标原点至图中心后的矩形ROI
std::vector<roiRect> myRectProcessor::getRoIRects(const QImage& img,
                                                  const QVector<QVector<bool>>& roiPos,
                                                  const int& imgW, const int& imgH,
                                                  const double& roiW, const double& roiH)
{
    std::vector<roiRect> roiRects;
    roiRects.reserve(16);
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
            newRect.img = img.copy(rectToAbsolutePos(newRect.rect).toRect());
            newRect.img.save("D:/MechMindQtMTF/img/img_" + QString::number(row) + "_" +
                             QString::number(col) + ".png");
            print("after save roi copy");
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
