#include "lnxmtfprototype.h"
#include "ui_lnxmtfprototype.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>

LNXMTFPrototype::LNXMTFPrototype(QWidget* parent) : QWidget(parent), ui(new Ui::LNXMTFPrototype)
{
    ui->setupUi(this);
    ui->roiWidth->setValue(50);
    ui->roiHeight->setValue(80);
    setWindowState(Qt::WindowMaximized);
    ui->zoomIn->setEnabled(false);
    ui->zoomOut->setEnabled(false);
    ui->chooseRoi->setEnabled(false);
    ui->editRoi->setEnabled(false);
    ui->viewersTabs->setCurrentWidget(ui->imgTab);
}

LNXMTFPrototype::~LNXMTFPrototype() { delete ui; }

void LNXMTFPrototype::on_loadImg_clicked()
{
    // clear操作
    clear();
    // 选择图片界面
    QFileDialog fileDlg(this);
    fileDlg.setWindowTitle(QStringLiteral("Choose Pictures"));
    const QStringList qstrFilters(QStringLiteral("Image files(*.jpg *.png *.bmp)"));
    fileDlg.setNameFilters(qstrFilters);
    QImage img;
    if (fileDlg.exec() == QDialog::Rejected)
        return;
    QStringList imgPath = fileDlg.selectedFiles();
    if (imgPath.isEmpty())
        return;

    // 设置图片到label标签中
    if (!img.load(imgPath.front())) {
        QMessageBox::information(this, tr("Open Failed"), tr("Open image Failed!"));
        return;
    }
    ui->imgView->setParentScrollArea(ui->scrollArea);
    ui->imgView->setImg(img);
    //    ui->imgView->setPixmap(QPixmap::fromImage(img));

    // 要在图上画好13个ROI框
    ui->imgView->setRectProcessor(new myRectProcessor);
    QVector<QVector<bool>> roiBool = {{true, true, false, false, true, false, false, false, true},
                                      {true, true, false, false, true, false, false, false, true},
                                      {true, true, false, false, true, false, false, false, true},
                                      {true, true, false, false, true, false, false, false, true}};
    QVector<roiRect> allROI = ui->imgView->getRectProcessor()->getRoIRects(
        img, roiBool, img.width(), img.height(), ui->roiWidth->value(), ui->roiHeight->value());

    ui->imgView->addFieldRectangle(allROI);

    // 使能编辑图片相关按钮
    ui->zoomIn->setEnabled(true);
    ui->zoomOut->setEnabled(true);
    ui->chooseRoi->setEnabled(true);
    ui->chooseRoi->setChecked(true);
    ui->editRoi->setEnabled(true);
}

void LNXMTFPrototype::on_calcMTF_clicked()
{
    //    ui->lineChart->resetChartSeries(genSeriesNames(3));
    //    ui->viewersTabs->setCornerWidget(ui->mtfCurveTab);
    //    ui->lineChart->resetChartSeries({"123", "456"});
    //    ui->lineChart->setAxisTitle("x", "y");
}

void LNXMTFPrototype::on_zoomIn_clicked() { ui->imgView->onZoomInImage(); }
void LNXMTFPrototype::on_zoomOut_clicked() { ui->imgView->onZoomOutImage(); }
void LNXMTFPrototype::on_chooseRoi_clicked(bool checked)
{
    if (checked) {
        ui->editRoi->setChecked(false);
        ui->imgView->setOperateMode(choose);
    } else {
        ui->editRoi->setChecked(true);
        ui->imgView->setOperateMode(edit);
    }
}
void LNXMTFPrototype::on_editRoi_clicked(bool checked)
{
    if (checked) {
        ui->chooseRoi->setChecked(false);
        ui->imgView->setOperateMode(edit);
    } else {
        ui->chooseRoi->setChecked(true);
        ui->imgView->setOperateMode(choose);
    }
}

void LNXMTFPrototype::clear() { ui->imgView->clearFieldRect(); }
