#pragma once

#include <QWidget>
#include "labelpaintertool.h"

namespace Ui {
class singleMTFCurve;
}

class singleMTFCurve : public QWidget
{
    Q_OBJECT

public:
    explicit singleMTFCurve(const QString roiName, const roiRect& roi,
                            const std::vector<double>& mtf, const std::vector<double>& tmfControl,
                            int freq, QWidget* parent = nullptr);
    ~singleMTFCurve();

private:
    Ui::singleMTFCurve* ui;
};
