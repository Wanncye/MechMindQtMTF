#include "singleMTFCurve.h"
#include "ui_singleMTFCurve.h"
#include <QStandardItemModel>

#define print(val) qDebug() << #val << val

singleMTFCurve::singleMTFCurve(const QString roiName, const roiRect& roi,
                               const std::vector<double>& mtf,
                               const std::vector<double>& mtfControl, int freq, QWidget* parent)
    : QWidget(parent), ui(new Ui::singleMTFCurve)
{
    ui->setupUi(this);
    // 设置图
    ui->chartView->resetChartSeries();
    ui->chartView->setChartTitle(roiName);
    ui->chartView->setValues(mtf);
    ui->chartView->scaleAxes();

    // 设置显示ROI
    ui->roi->setPixmap(QPixmap::fromImage(roi.img));

    // 显示MTF所有数据
    QTableWidgetItem* headerItem;
    QString headerText = roiName;
    ui->table->setColumnCount(1);
    headerItem = new QTableWidgetItem(headerText);
    ui->table->setHorizontalHeaderItem(0, headerItem);

    ui->table->clearContents();
    ui->table->setRowCount(mtfControl.size() + mtf.size());
    QTableWidgetItem* item;
    for (int row = 0; row < mtfControl.size(); row++) {
        item = new QTableWidgetItem(QString::number(mtfControl.at(row)));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->table->setItem(row, 0, item);
    }
    for (int row = 0; row < mtf.size(); row++) {
        item = new QTableWidgetItem(QString::number(mtf.at(row)));
        if (row == freq) {
            item->setBackground(QBrush(Qt::red));
        }
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->table->setItem(row + mtfControl.size(), 0, item);
        item = new QTableWidgetItem(QString::number(row));
        ui->table->setVerticalHeaderItem(row + mtfControl.size(), item);
    }
    ui->table->setVerticalHeaderItem(0, new QTableWidgetItem("Deg"));
    ui->table->setVerticalHeaderItem(1, new QTableWidgetItem("ROI Top Left x"));
    ui->table->setVerticalHeaderItem(2, new QTableWidgetItem("ROI Top Left y"));
    ui->table->setVerticalHeaderItem(3, new QTableWidgetItem("ROI Bottom Right x"));
    ui->table->setVerticalHeaderItem(4, new QTableWidgetItem("ROI Bottom Right y"));
    ui->table->setVerticalHeaderItem(5, new QTableWidgetItem("Over Exposure Error"));
    ui->table->setVerticalHeaderItem(6, new QTableWidgetItem("Edge Estimation Error"));
    ui->table->setVerticalHeaderItem(7, new QTableWidgetItem("MTF Error"));
    ui->table->setVerticalHeaderItem(8, new QTableWidgetItem("Deg Error"));
    ui->table->setVerticalHeaderItem(9, new QTableWidgetItem("Visual Field"));
    ui->table->setVerticalHeaderItem(10, new QTableWidgetItem("Direction"));
}

singleMTFCurve::~singleMTFCurve() { delete ui; }
