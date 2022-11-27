#include <QContextMenuEvent>
#include <QDebug>
#include <QPainter>
#include <QStyleOption>

#include "ROISelectionWindow.h"
#include "ui_ROISelectionWindow.h"

int selectedROIRectIndex = -1; // 鼠标点击时、选择ROI的下标索引
double offsetW = 0; // 以中心点为坐标轴时坐标转化参数
double offsetH = 0; // 以中心点为坐标轴时坐标转化参数
double mXPtInterval = 0;    // 移动画面的X偏移
double mYPtInterval = 0;    // 移动画面的y偏移
qreal mZoomValue = 1.0; // 放大缩小系数

QPointF mOldPos;    // 鼠标点击的时候的坐标
QRectF imageRect;

static QVector<QRectF> errROIRects;
static QVector<QRectF> trueROIRects;

bool mPressed = false;
bool isEditROIRect; // 是否处于选取ROI矩形的状态，否的话，就是移动画面

void normalizeInterval(const double& width, const double& height);

inline QPointF ToRelativePos(const QPointF& pos)
{
    return (pos + QPointF(offsetW - mXPtInterval, offsetH - mYPtInterval)) / mZoomValue;
}
inline QPointF ToAbsolutePos(const QPointF& pos)
{
    return pos * mZoomValue - QPointF(offsetW, offsetH);
}

inline QRectF rectToRelativePos(const QRectF& rect)
{
    return QRectF{ToRelativePos(rect.topLeft()), ToRelativePos(rect.bottomRight())};
}

inline QRectF rectToAbsolutePos(const QRectF& rect)
{
    return QRectF{ToAbsolutePos(rect.topLeft()), ToAbsolutePos(rect.bottomRight())};
}

inline QRectF fromCenterPoint(const QPointF& centerPoint, double width, double height)
{
    // 用中心点以及ROI的长宽得到矩形的左上和右下的点
    return {centerPoint.x() - width / 2.0, centerPoint.y() - height / 2.0, width, height};
};

// 一般是自动计算的时候调用这个，会设置ROI的长和宽
ROISelectionWindow::ROISelectionWindow(const QString& imgPath, const bool& isManualFlag,
                                       const int& roiW, const int& roiH,
                                       const QVector<QRectF>& errorROI,
                                       const QVector<QRectF>& trueROI, QWidget* parent)
    : QDialog(parent), ui(new Ui::ROISelectionWindow)
{
    ui->setupUi(this);
    isManual = isManualFlag;
    onLoadImage(imgPath);
    ROIW = roiW;
    ROIH = roiH;

    selectedROIRectIndex = -1;
    isEditROIRect = true;

    offsetW = -mImage.width() / 2.0;
    offsetH = -mImage.height() / 2.0;

    if (!isManualFlag) {
        qDebug() << "is Mannual ";
        for (auto rect : errorROI)
            errROIRects.append(rectToRelativePos(rect));
        for (auto rect : trueROI)
            trueROIRects.append(rectToRelativePos(rect));
        isEditROIRect = false;

        ui->switchPB->setEnabled(false);
        ui->clearAllPB->setEnabled(false);
        ui->UndoPB->setEnabled(false);
    }

    setWindowTitle(QStringLiteral("Img Name:%1").arg(imgPath.right(imgPath.lastIndexOf('/'))));
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
}

// 两个无关紧要的构造函数
ROISelectionWindow::ROISelectionWindow(QWidget*) : ROISelectionWindow(QLatin1String(""), false) {}
// 两个无关紧要的构造函数
ROISelectionWindow::ROISelectionWindow(const QString& imgPath, QWidget*)
    : ROISelectionWindow(imgPath, false)
{
}

// 其实这个构造函数必是手动的时候调用
ROISelectionWindow::ROISelectionWindow(const QString& imgPath, const QRectF& errRect,
                                       const bool& isManualFlag, QWidget* parent)
    : QDialog(parent), ui(new Ui::ROISelectionWindow)
{
    ui->setupUi(this);
    isManual = isManualFlag;
    onLoadImage(imgPath);
    selectedROIRectIndex = -1;
    isEditROIRect = true; // 是否是要移动ROI的操作

    offsetW = -mImage.width() / 2.0;
    offsetH = -mImage.height() / 2.0;
    errROIRects.push_back(rectToRelativePos(errRect));

    setWindowTitle(QStringLiteral("Img Name:%1").arg(imgPath.right(imgPath.lastIndexOf('/'))));
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
}

ROISelectionWindow::~ROISelectionWindow() { delete ui; }

void ROISelectionWindow::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    if (mImage.isNull())
        return QWidget::paintEvent(event);

    int width = mImage.width();
    int height = mImage.height();

    resize(width, height);

    painter.translate(this->width() / 2.0 + mXPtInterval, this->height() / 2.0 + mYPtInterval);

    painter.scale(mZoomValue, mZoomValue);

    QRectF picRect(-width / 2.0, -height / 2.0, width, height);
    painter.drawImage(picRect, mImage);

    painter.setPen(QPen(QColor(255, 255, 0), 1)); // 正在编辑的ROI
    painter.drawRect(QRectF(StartPoint.x(), StartPoint.y(), EndPoint.x() - StartPoint.x(),
                            EndPoint.y() - StartPoint.y()));

    painter.setPen(QPen(QColor(184, 134, 11), 1));
    painter.drawRects(trueROIRects);    // 编辑的、添加的ROI
    painter.setPen(QPen(QColor(255, 0, 0), 2));
    painter.drawRects(errROIRects);// 预定的的ROI视场
}

void ROISelectionWindow::wheelEvent(QWheelEvent* event)
{
    int value = event->angleDelta().y();
    if (value > 0)
        onZoomInImage();
    else
        onZoomOutImage();

    update();
}

void ROISelectionWindow::mousePressEvent(QMouseEvent* event)
{
    mPressed = true;
    selectedROIRectIndex = -1;
    if (!isManual) {
        // 自动处理的情况下
        mOldPos = event->pos(); // 鼠标点击的位置
        QPointF mouseRelativePos = ToRelativePos(event->pos());
        for (int i = 0; i < trueROIRects.length(); i++) {
            // 判断一个点是不是在矩形内
            if (trueROIRects.at(i).contains(mouseRelativePos)) {
                selectedROIRectIndex = i;
                isEditROIRect = true;
                return;
            }
        }
        for (int i = 0; i < errROIRects.length(); i++) {
            if (errROIRects.at(i).contains(mouseRelativePos)) {
                selectedROIRectIndex = i + trueROIRects.length();
                isEditROIRect = true;
                return;
            }
        }
    } else {
        StartPoint = ToRelativePos(event->pos());
        EndPoint = ToRelativePos(event->pos());
        emit StartPointSignal(StartPoint);
    }
}

void ROISelectionWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (!mPressed)
        return QWidget::mouseMoveEvent(event);
    setCursor(Qt::SizeAllCursor);
    QPointF pos = event->pos();
    double xPtInterval = pos.x() - mOldPos.x();
    double yPtInterval = pos.y() - mOldPos.y();
    mOldPos = pos;
    if (!isEditROIRect) {
        // 不是移动ROI操作，那就是移动画面的操作
        mXPtInterval += xPtInterval;
        mYPtInterval += yPtInterval;
        normalizeInterval(width(), height());
    } else {
        if (!isManual) {
            auto len = trueROIRects.length();
            if (selectedROIRectIndex >= len) {
                errROIRects[selectedROIRectIndex - len] =
                    QRectF(errROIRects[selectedROIRectIndex - len].topLeft() + QPointF(xPtInterval, yPtInterval) / mZoomValue,
                           errROIRects[selectedROIRectIndex - len].bottomRight() + QPointF(xPtInterval, yPtInterval) / mZoomValue);
            } else {
                trueROIRects[selectedROIRectIndex] =
                    QRectF(trueROIRects[selectedROIRectIndex].topLeft() + QPointF(xPtInterval, yPtInterval) / mZoomValue,
                           trueROIRects[selectedROIRectIndex].bottomRight() + QPointF(xPtInterval, yPtInterval) / mZoomValue);
            }
        } else {
            EndPoint = ToRelativePos(event->pos());
            emit StopPointSignal(EndPoint);
        }
    }

    update();
}

void ROISelectionWindow::mouseReleaseEvent(QMouseEvent*)
{
    mPressed = false;
    if (!isManual)
        isEditROIRect = false;
    setCursor(Qt::ArrowCursor);

    if (isManual) {
        trueROIRects.append(QRectF(StartPoint, EndPoint));
    }

    update();
}

// 加载图片，给mImage赋值
void ROISelectionWindow::onLoadImage(const QString& image)
{
    QFile file(image);
    if (!file.exists())
        return;

    mImage.load(image);
}

void ROISelectionWindow::onZoomInImage(void)
{
    mZoomValue += 0.2;
    if (mZoomValue >= 2) {
        mZoomValue = 2;
    }
    update();
}

void ROISelectionWindow::onZoomOutImage(void)
{
    mZoomValue -= 0.2;
    if (mZoomValue <= 1) {
        mZoomValue = 1;
        return;
    }

    update();
}

// 从roiPos这个参数中得到所有选中的ROI，方向、矩形、包含像素、偏移
// 这里的roiPos是4*9大小的，其中4表示四个象限，9表示9个视场
QVector<roiRect> ROISelectionWindow::getRoIRects(const QImage& img,
                                                 const QVector<QVector<bool>>& roiPos,
                                                 const int& imgW, const int& imgH,
                                                 const double& roiW, const double& roiH)
{
    QVector<roiRect> roiRects;
    offsetH = -imgH / 2.0;
    offsetW = -imgW / 2.0;
    QRectF ImgRect = QRectF(-imgW / 2.0, -imgH / 2.0, imgW, imgH);

    QVector<QPointF> points = {ImgRect.topRight(), ImgRect.topLeft(), ImgRect.bottomLeft(),
                               ImgRect.bottomRight()};
    for (int row = 0; row < points.size(); row++) {
        // 循环四个点
        for (int col = 0; col < roiPos[row].size(); col++) {
//            循环所有的bool型的roiPos
            roiRect tmp;
            if (roiPos[row][col]) {
                tmp.d = (direction)row;
                tmp.offset = col * 0.1 + 0.1;
                tmp.rect = rectToAbsolutePos(fromCenterPoint(points[row] * tmp.offset, roiW, roiH));
                tmp.img = img.copy(tmp.rect.toRect());
                roiRects.append(tmp);
            }
        }
    }
    return roiRects;
}

void normalizeInterval(const double& width, const double& height)
{
    if (mXPtInterval < -width / 2.0)
        mXPtInterval = -width / 2.0;
    else if (mXPtInterval > width / 2.0)
        mXPtInterval = width / 2.0;
    if (mYPtInterval < -height / 2.0)
        mYPtInterval = -height / 2.0;
    else if (mYPtInterval > height / 2.0)
        mYPtInterval = height / 2.0;
}

void ROISelectionWindow::on_switchPB_clicked()
{
    // 转变模式，拖动和选取ROI
    if (isManual) {
        StartPoint = QPointF(0, 0);
        EndPoint = QPointF(0, 0);
    }
    isManual = !isManual;
    isEditROIRect = !isEditROIRect;
}

void ROISelectionWindow::on_clearAllPB_clicked()
{
    // 删除所有的ROI
    trueROIRects.clear();
    if (isManual) {
        StartPoint = QPointF(0, 0);
        EndPoint = QPointF(0, 0);
    }

    update();
}

void ROISelectionWindow::on_UndoPB_clicked()
{
    // 删除最后一个元素
    if (isManual) {
        StartPoint = QPointF(0, 0);
        EndPoint = QPointF(0, 0);
    }
    if (!trueROIRects.isEmpty()) {
        trueROIRects.pop_back();
    }
    update();
}

void ROISelectionWindow::on_ConfirmPB_clicked()
{
    // 主要是为了得到用户选中的roiRects，一个ROI是一个tmp，需要确定方向，偏移量，ROI对应区域（copy函数）
    QVector<roiRect> roiRects;
    errROIRects.append(trueROIRects);
    for (auto rectR : errROIRects) {
        roiRect tmp;
        if (rectR.center().x() < 0) {
            /*
            if (rectR.center().y() < 0)
                tmp.d = (direction)1;
            else
                tmp.d = (direction)2;
                */
            tmp.d = rectR.center().y() < 0 ? (direction)1 : (direction)2;
        } else {
            /*
            if (rectR.center().y() < 0)
                tmp.d = (direction)0;
            else
                tmp.d = (direction)3;
             */
            tmp.d = rectR.center().y() < 0 ? (direction)0 : (direction)3;
        }
        tmp.offset = -1; // 表示是用户更正的
        tmp.rect = rectToAbsolutePos(fromCenterPoint(rectR.center(), ROIW, ROIH));
        tmp.img = mImage.copy(tmp.rect.toRect());
        tmp.img.save("/Users/wanncye/Desktop/MTF/1.PNG");
        roiRects.append(tmp);
    }

    emit sendConfirmImgs(roiRects);
    close();
}

void ROISelectionWindow::closeEvent(QCloseEvent*)
{
    selectedROIRectIndex = -1;
    offsetW = 0;
    offsetH = 0;
    mXPtInterval = 0;
    mYPtInterval = 0;
    mZoomValue = 1.0;
    mOldPos = QPointF(0, 0);
    imageRect = QRectF(QPointF(0, 0), QPointF(0, 0));

    trueROIRects.clear();
    errROIRects.clear();

    mPressed = false;
    isEditROIRect = false;
}
