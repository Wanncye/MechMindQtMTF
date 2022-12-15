#ifndef LNXMTFPROTOTYPE_H
#define LNXMTFPROTOTYPE_H

#include <QWidget>
#include <QTableWidgetItem>

namespace Ui {
class LNXMTFPrototype;
}

struct roiRect;
// tset
class LNXMTFPrototype : public QWidget
{
    Q_OBJECT

public:
    explicit LNXMTFPrototype(QWidget* parent = nullptr);
    ~LNXMTFPrototype();

public slots:
    void on_loadImg_clicked(); // 加载图片
    void on_loadTVImg_clicked();
    void on_calcTVBtn_clicked();
    void on_calcMTF_clicked();
    void on_zoomIn_clicked();
    void on_zoomOut_clicked();
    void on_editRoi_clicked(bool checked);

    void recieveFieldRects(std::vector<roiRect>& rects);
    void showSingleMTFCurve(int index);

private:
    void clear();
    bool calcMTF(const std::vector<roiRect>& roiRects, const QString& imgPath, bool isSave);
    void showTable();
    void showChart();
    std::vector<roiRect> getSpecificFieldRect(double offset);

    QString mTVImgPath;
    Ui::LNXMTFPrototype* ui;
    std::vector<roiRect> mFieldRects;
    std::vector<std::vector<double>> mMtfData;
    std::vector<std::vector<double>> mMtfControlInformation;
};

#endif // LNXMTFPROTOTYPE_H
