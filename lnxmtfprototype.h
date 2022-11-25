#ifndef LNXMTFPROTOTYPE_H
#define LNXMTFPROTOTYPE_H

#include <QWidget>

namespace Ui {
class LNXMTFPrototype;
}

class LNXMTFPrototype : public QWidget
{
    Q_OBJECT

public:
    explicit LNXMTFPrototype(QWidget *parent = nullptr);
    ~LNXMTFPrototype();

public slots:
    void on_loadImg_clicked(); // 加载图片
    void on_calcMTF_clicked();

private:
    Ui::LNXMTFPrototype *ui;
};

#endif // LNXMTFPROTOTYPE_H
