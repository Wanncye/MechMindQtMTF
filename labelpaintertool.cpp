#include "labelpaintertool.h"
#include <QStyleOption>
#include <QScrollArea>

LabelPainterTool::LabelPainterTool(QWidget* parent) : QLabel(parent) {}
LabelPainterTool::~LabelPainterTool() {}

#define print(val) qDebug() << #val << val

QString genSeriesName(const roiRect& rect);

void LabelPainterTool::paintEvent(QPaintEvent* event)
{
//    QPainter painter(this);

//    if (mImage.isNull())
//        return QWidget::paintEvent(event);

//    const int imgWidth = mImage.width();
//    const int imgHeight = mImage.height();
//    const int windowsWidth = this->width();
//    const int windowsHeight = this->height();

//    // 以左上角为原点，将坐标原点往右下角平移这么多像素
//    painter.translate(windowsWidth / 2.0 + mXPtInterval,
//                      windowsHeight / 2.0 + mYPtInterval);

//    painter.scale(mZoomValue, mZoomValue);

//    // windowRect要按当前的坐标系来计算，imgRect要按图片的坐标系计算
//    // 且这里不能使用QLabel的setPixmap
//    QRectF windowRect(-windowsWidth / 2.0, -windowsHeight / 2.0, windowsWidth, windowsHeight);
//    QRectF imgRect(0, 0, imgWidth, imgHeight);
//    painter.drawPixmap(windowRect, QPixmap::fromImage(mImage), imgRect);
//    setPixmap(QPixmap::fromImage(mImage));

//    // 视场绘制
//    painter.setPen(QPen(Qt::red, 1));
//    for (const auto& roi : mFieldRects) {
//        painter.drawRect(roi.rect);
//        painter.drawText(roi.rect, Qt::AlignCenter, genSeriesName(roi));
//    }

//    // 中心点
//    painter.drawRect(QRectF(-1, -1, 2, 2));


    // 绘制样式
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if (mImage.isNull())
        return QWidget::paintEvent(event);

    // 根据窗口计算应该显示的图片的大小
    int width = mImage.width();
    int height = mImage.height();
//    int width = qMin(mImage.width(), this->width());
//    int height = width * 1.0 / (mImage.width() * 1.0 / mImage.height());
//    height = qMin(height, this->height());
//    width = height * 1.0 * (mImage.width() * 1.0 / mImage.height());


    // 平移
    painter.translate(this->width() / 2 + mXPtInterval, this->height() / 2 + mYPtInterval);

    // 缩放
    painter.scale(mZoomValue, mZoomValue);

    // 绘制图像
    QRect picRect(-width / 2, -height / 2, width, height);
    painter.drawImage(picRect, mImage);

    // 视场绘制
    painter.setPen(QPen(Qt::red, 1));
    for (const auto& roi : mFieldRects) {
        painter.drawRect(roi.rect);
        painter.drawText(roi.rect, Qt::AlignCenter, genSeriesName(roi));
    }

    // 中心点
    painter.drawRect(QRectF(-1, -1, 2, 2));
}

void LabelPainterTool::wheelEvent(QWheelEvent *event)
{
    int value = event->delta();
    if (value > 0)
        onZoomInImage();
    else
        onZoomOutImage();

    this->update();
}

//鼠标按下
void LabelPainterTool::mousePressEvent(QMouseEvent* event)
{
    print("-----mouse press-----");
    mPressed = true;
    mSelectedROIRectIndex = -1;
    mMouseStartPoint = event->pos();
    switch (mOpMode) {
    case choose:
    {
        break;
    }
    case edit:
    {
        for (int i = 0; i < mFieldRects.size(); i++) {
            print(mMouseStartPoint);
            QPointF curRelativePos = WidgetToImgRelativePos(mMouseStartPoint);
            print(curRelativePos);
            print(mFieldRects.at(i).rect);
            if (mFieldRects.at(i).rect.contains(curRelativePos)) {
                mSelectedROIRectIndex = i;
                print(i);
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

    QPointF mouseCurrentPos = event->pos();
    double xPtInterval = mouseCurrentPos.x() - mMouseStartPoint.x();
    double yPtInterval = mouseCurrentPos.y() - mMouseStartPoint.y();
    mMouseStartPoint = mouseCurrentPos;
    switch (mOpMode) {
    case choose:
        this->setCursor(Qt::SizeAllCursor);
        mXPtInterval += xPtInterval;
        mYPtInterval += yPtInterval;
        break;
    case edit:
    {
        // 移动ROI
        mFieldRects[mSelectedROIRectIndex].rect =
            QRectF(mFieldRects[mSelectedROIRectIndex].rect.topLeft() +
                       QPointF(xPtInterval, yPtInterval) / mZoomValue,
                   mFieldRects[mSelectedROIRectIndex].rect.bottomRight() +
                       QPointF(xPtInterval, yPtInterval) / mZoomValue);
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
    print("-----mouse mouseReleaseEvent-----");
    mPressed = false;
    setCursor(Qt::ArrowCursor);

    switch (mOpMode) {
    case choose:
        break;
    case edit:
    {
        mFieldRects[mSelectedROIRectIndex].img = mImage.copy(
            rectToImgAbsolutePos(mFieldRects[mSelectedROIRectIndex].rect).toRect());
        print(rectToImgAbsolutePos(mFieldRects[mSelectedROIRectIndex].rect).toRect());
        if(!mFieldRects[mSelectedROIRectIndex].img.save(
            "/Users/wanncye/Desktop/MTF/img/img_" +
            QString::number(static_cast<int>(mFieldRects[mSelectedROIRectIndex].d)) +
            QString::number(mFieldRects[mSelectedROIRectIndex].offset) + ".png"));
            print("save img roi failed.");
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
    mZoomValue += 0.1;
    if (mZoomValue >= 10) {
        mZoomValue = 10;
    }
    update();
}

void LabelPainterTool::onZoomOutImage(void)
{
    mZoomValue -= 0.1;
    if (mZoomValue <= 0.1) {
        mZoomValue = 0.1;
    }
    update();
}

void LabelPainterTool::addFieldRectangle(std::vector<roiRect>& roiRects)
{
    //    for (auto& roi : roiRects)
    //        mFieldRects.push_back(roi);
    mFieldRects = roiRects;
    update();
}

void LabelPainterTool::clearFieldRect() { mFieldRects.clear(); }

// 得到的是移动坐标原点至图中心后的矩形ROI
std::vector<roiRect> LabelPainterTool::getRoIRects(const QImage& img,
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
            newRect.img = img.copy(rectToImgAbsolutePos(newRect.rect).toRect());
            newRect.img.save("D:/MechMindQtMTF/img/img_" + QString::number(row) + "_" +
                             QString::number(col) + ".png");
            roiRects.push_back(newRect);
        }
    }
    return roiRects;
}

QPointF LabelPainterTool::WidgetToImgRelativePos(const QPointF& pos)
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!
    return (pos + QPointF(-this->width() / 2 - mXPtInterval, -this->height() / 2 - mYPtInterval)) / mZoomValue;
}
QPointF LabelPainterTool::ToImgAbsolutePos(const QPointF& pos)
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!
    return pos - QPointF(mOffsetW, mOffsetH);
}

QRectF LabelPainterTool::rectToRelativePos(const QRectF& rect)
{
    return QRectF{WidgetToImgRelativePos(rect.topLeft()), WidgetToImgRelativePos(rect.bottomRight())};
}

QRectF LabelPainterTool::rectToImgAbsolutePos(const QRectF& rect)
{
    return QRectF{ToImgAbsolutePos(rect.topLeft()), ToImgAbsolutePos(rect.bottomRight())};
}

QRectF LabelPainterTool::fromCenterPoint(const QPointF& centerPoint, double width, double height)
{
    return {centerPoint.x() - width / 2.0, centerPoint.y() - height / 2.0, width, height};
};
