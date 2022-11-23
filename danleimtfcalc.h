#ifndef DANLEIMTFCALC_H
#define DANLEIMTFCALC_H

#include <QWidget>
#include "ROISelectionWindow.h"

namespace Ui {
class DanLeiMTFCalc;
}

class DanLeiMTFCalc : public QWidget
{
    Q_OBJECT

public:
    explicit DanLeiMTFCalc(QWidget *parent = nullptr);
    ~DanLeiMTFCalc();
private slots:
    void on_pushButton_clicked();
    void on_isManualRB_clicked();
    void on_isAutoRB_clicked();
    void on_CorPRB_clicked();
    void on_LWOrPHRB_clicked();
    void on_ipOrMmRB_clicked(bool checked);
    void getNewImgs(const QVector<roiRect>& imgs);
    int processCode();
    bool calcMTF(const QVector<roiRect>& roiRects, const QString& strPath, bool isSave = true);

private:
    Ui::DanLeiMTFCalc* ui;
    ROISelectionWindow* roiSelectionWindow{};
    QList<QImage> m_qlistLoadImgs; // 用来存放用户选择的图片
    QVector<roiRect> newConfirmImgs;
    // QVector<QRectF> newConfirmRects;
    // QVector<QPoint> newConfirmImgsCenterPoints;//用来储存用户选择完confirm后的roi框的中心点
    QPixmap* m_crtPixmap = nullptr; // 显示其中某张图片用
    double isSaveToExcel;           // double是为了方便给python模块传参数 1或者 0
};

#endif // DANLEIMTFCALC_H
