#ifndef LNXMTFPROTOTYPE_H
#define LNXMTFPROTOTYPE_H

#include <QWidget>
#include <QTableWidgetItem>

namespace Ui {
class LNXMTFPrototype;
}

struct roiRect;

class LNXMTFPrototype : public QWidget
{
    Q_OBJECT

public:
    explicit LNXMTFPrototype(QWidget* parent = nullptr);
    ~LNXMTFPrototype();

public slots:
    void on_loadImg_clicked(); // 加载图片
    void on_calcMTF_clicked();
    void on_zoomIn_clicked();
    void on_zoomOut_clicked();
    void on_editRoi_clicked(bool checked);
    void on_stopCalc_clicked();
    void on_saveToExcel_clicked();

    void recieveFieldRects(std::vector<roiRect>& rects);
    void showSingleMTFCurve(int index);
    void timerWorker();
    void resetROIField(int state);
    void valueChangedResetROIField(double val);

private:
    void clear();
    void calcMTF(const std::vector<roiRect>& roiRects);
    void showTable();
    void showChart();
    void updateRoiRectByParameters(int state);
    std::vector<roiRect> getSpecificFieldRect(double offset);

    QImage mImg;

    QTimer* mTimer;
    Ui::LNXMTFPrototype* ui;
    std::vector<roiRect> mFieldRects;
    std::vector<std::vector<double>> mMtfData;
    std::vector<std::vector<double>> mMtfControlInformation;
};

#endif // LNXMTFPROTOTYPE_H
