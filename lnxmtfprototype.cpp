#include "lnxmtfprototype.h"
#include "labelpaintertool.h"
#include "ui_lnxmtfprototype.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>

#define print(val) qDebug() << #val << val

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
    connect(ui->imgView, &LabelPainterTool::sendFieldRects, this,
            &LNXMTFPrototype::recieveFieldRects);
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
    std::vector<roiRect> allROI = ui->imgView->getRectProcessor()->getRoIRects(
        img, roiBool, img.width(), img.height(), ui->roiWidth->value(), ui->roiHeight->value());

    ui->imgView->addFieldRectangle(allROI);

    // 使能编辑图片相关按钮
    ui->zoomIn->setEnabled(true);
    ui->zoomOut->setEnabled(true);
    ui->chooseRoi->setEnabled(true);
    ui->chooseRoi->setChecked(true);
    ui->editRoi->setEnabled(true);
    ui->viewersTabs->setCurrentWidget(ui->imgTab);
}

QStringList genSeriesNames(const std::vector<roiRect>& rects)
{
    QStringList dst;
    for (auto& roi : rects) {
        if (!roi.checked)
            continue;
        QString direction;
        switch (roi.d) {
        case ne:
            direction = "NE";
            break;
        case nw:
            direction = "NW";
            break;
        case sw:
            direction = "SW";
            break;
        case se:
            direction = "SE";
            break;
        default:
            break;
        }
        QString offset = QString::number(roi.offset);
        dst.push_back(QStringLiteral("%1%2").arg(direction, offset));
    }
    return dst;
}

void LNXMTFPrototype::on_calcMTF_clicked()
{
    auto seriesNames = genSeriesNames(mFieldRects);
    if (seriesNames.isEmpty()) {
        qDebug() << "seriesNames NULL";
        return;
    }
    ui->lineChart->resetChartSeries(seriesNames);
    ui->lineChart->setAxisTitle("x", "MTF");
    // 模拟一下MTF数据， 将他显示出来

    ui->viewersTabs->setCurrentWidget(ui->mtfCurveTab);
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

void LNXMTFPrototype::recieveFieldRects(std::vector<roiRect>& rects) { mFieldRects = rects; }
