#include "lnxmtfprototype.h"
#include "labelpaintertool.h"
#include "ui_lnxmtfprototype.h"
#include "mtfCaculation/callPythonScripts.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
#include <QThread>

#define print(val) qDebug() << #val << val

LNXMTFPrototype::LNXMTFPrototype(QWidget* parent) : QWidget(parent), ui(new Ui::LNXMTFPrototype)
{
    ui->setupUi(this);
    ui->roiWidth->setValue(50);
    ui->roiHeight->setValue(80);
    setWindowState(Qt::WindowMaximized);
    ui->zoomIn->setEnabled(false);
    ui->zoomOut->setEnabled(false);
    ui->editRoi->setEnabled(false);
    connect(ui->imgView, &LabelPainterTool::sendFieldRects, this,
            &LNXMTFPrototype::recieveFieldRects);
    PythonInit();
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
    ui->editRoi->setEnabled(true);
}

QStringList genSeriesNames(const std::vector<roiRect>& rects)
{
    QStringList dst;
    for (auto& roi : rects) {
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

std::vector<std::vector<double>> QimageToArrayLNX(const QImage& image)
{
    int width = image.width();
    int height = image.height();

    std::vector<std::vector<double>> pixelMat;

    pixelMat.resize(height);
    for (int row = 0; row < height; row++) {
        pixelMat[row].resize(width);
        const auto* data = reinterpret_cast<const quint8*>(image.scanLine(row));
        for (int column = 0; column < width; column++) {
            pixelMat[row][column] = data[column];
        }
    }

    return pixelMat;
}

void LNXMTFPrototype::on_calcMTF_clicked()
{
    //    while (1) {
    print("click calcMTF");
    if (mFieldRects.empty()) {
        qDebug() << "mFieldRects NULL";
        return;
    }

    // 模拟一下MTF数据， 将他显示出来
    // 这个事要拿另外的线程来做，要不然会阻塞IO
    std::vector<std::vector<std::vector<double>>> img;
    std::vector<std::vector<double>> mtfData;
    std::vector<roiRect> checkedROI;
    int ifsf = 0;
    for (const auto& roi : qAsConst(mFieldRects)) {
        if (!roi.checked)
            continue;
        checkedROI.push_back(roi);
        img.push_back(QimageToArrayLNX(roi.img));
        ifsf++;
    }
    std::vector<std::vector<double>> information;
    for (const auto& roi : qAsConst(mFieldRects)) {
        if (!roi.checked)
            continue;
        // 为什么information中是否保存这一项设置为true时，会出现闪退的情况？
        information.push_back({roi.rect.topLeft().x(), roi.rect.topLeft().y(),
                               roi.rect.bottomRight().x(), roi.rect.bottomRight().y(), roi.offset,
                               (double)roi.d, (double)false});
    }
    const std::string fileName = "test.png";
    const std::string savefileName = "result.xlsx";
    const double pixelSize = 5;
    print("Begin call python");
    callPythonReturnMTFData(img, information, savefileName, fileName, pixelSize, mtfData);
    print("Finish call python");
    auto seriesNames = genSeriesNames(checkedROI);
    if (seriesNames.isEmpty()) {
        qDebug() << "seriesNames NULL";
        return;
    }
    ui->feild01->resetChartSeries(seriesNames);
    ui->feild01->setAxisTitle("x", "MTF");
    for (int i = 0; i < mtfData.size(); ++i) {
        print(seriesNames[i]);
        print(ui->feild01->setValues(seriesNames[i], mtfData[i]));
        ui->feild01->scaleAxes();
    }
    //    }
}

void LNXMTFPrototype::on_zoomIn_clicked() { ui->imgView->onZoomInImage(); }
void LNXMTFPrototype::on_zoomOut_clicked() { ui->imgView->onZoomOutImage(); }

void LNXMTFPrototype::on_editRoi_clicked(bool checked)
{
    if (checked) {
        ui->imgView->setOperateMode(edit);
    } else {
        ui->imgView->setOperateMode(choose);
    }
}

void LNXMTFPrototype::clear() { ui->imgView->clearFieldRect(); }

void LNXMTFPrototype::recieveFieldRects(std::vector<roiRect>& rects) { mFieldRects = rects; }
