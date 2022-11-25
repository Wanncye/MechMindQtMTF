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

private:
    Ui::LNXMTFPrototype *ui;
};

#endif // LNXMTFPROTOTYPE_H
