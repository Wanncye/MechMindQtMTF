#ifndef MTFCALC_H
#define MTFCALC_H

#include <QWidget>

namespace Ui {
class MTFCalc;
}

class MTFCalc : public QWidget
{
    Q_OBJECT

public:
    explicit MTFCalc(QWidget *parent = nullptr);
    ~MTFCalc();

private:
    Ui::MTFCalc *ui;
};

#endif // MTFCALC_H
