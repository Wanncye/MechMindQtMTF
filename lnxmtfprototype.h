#ifndef LNXMTFPROTOTYPE_H
#define LNXMTFPROTOTYPE_H

#include <QWidget>

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
    void on_chooseRoi_clicked(bool checked);
    void on_editRoi_clicked(bool checked);

    void recieveFieldRects(std::vector<roiRect>& rects);

private:
    void clear();
    Ui::LNXMTFPrototype* ui;
    std::vector<roiRect> mFieldRects;
};

#endif // LNXMTFPROTOTYPE_H
