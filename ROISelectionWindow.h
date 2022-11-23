#pragma once

#include <QDialog>
#include <QImage>

namespace Ui {
class ROISelectionWindow;
}

enum direction { ne = 0, nw = 1, sw, se };

struct roiRect
{
    double offset = 0;
    direction d = (direction)0;
    QImage img;
    QRectF rect;
};

class ROISelectionWindow : public QDialog
{
    Q_OBJECT

public:
    ROISelectionWindow(QWidget* parent = nullptr);
    explicit ROISelectionWindow(const QString& imgPath, QWidget* parent = nullptr);
    explicit ROISelectionWindow(const QString& imgPath, const bool& isManualFlag = false,
                                const int& roiW = 50, const int& roiH = 80,
                                const QVector<QRectF>& errorROI = {},
                                const QVector<QRectF>& trueROI = {}, QWidget* parent = nullptr);
    explicit ROISelectionWindow(const QString& imgPath, const QRectF& errRect,
                                const bool& isManualFlag = false, QWidget* parent = nullptr);

    ~ROISelectionWindow() override;
    static QVector<roiRect> getRoIRects(const QImage& img, const QVector<QVector<bool>>& roiPos,
                                        const int& imgW, const int& imgH, const double& roiW,
                                        const double& roiH);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void closeEvent(QCloseEvent*) override;

private:
    void startEmitROIImg();

    Ui::ROISelectionWindow* ui;
    bool isManual;

    QImage mImage;
    int ROIW = 50;
    int ROIH = 80;
    QVector<roiRect> roiRects;

    QPointF StartPoint;
    QPointF EndPoint;

signals:
    void StartPointSignal(QPointF p);
    void StopPointSignal(QPointF p);
    void sendConfirmImgs(QVector<roiRect>);

private slots:
    void onLoadImage(const QString& image);
    void onZoomInImage(void);
    void onZoomOutImage(void);

    void on_switchPB_clicked();
    void on_clearAllPB_clicked();
    void on_UndoPB_clicked();
    void on_ConfirmPB_clicked();
};
