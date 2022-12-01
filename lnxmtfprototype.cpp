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

inline void warnMoveROI()
{
    QMessageBox::warning(nullptr, LNXMTFPrototype::tr("Calcuate Failure"),
                         LNXMTFPrototype::tr("Failed to calcuate MTF, please edit(move) ROI."));
}

LNXMTFPrototype::LNXMTFPrototype(QWidget* parent) : QWidget(parent), ui(new Ui::LNXMTFPrototype)
{
    ui->setupUi(this);
    ui->roiWidth->setValue(50);
    ui->roiHeight->setValue(80);
    ui->pixelSize->setValue(5.0);
    setWindowState(Qt::WindowMaximized);
    ui->zoomIn->setEnabled(false);
    ui->zoomOut->setEnabled(false);
    ui->editRoi->setEnabled(false);
    connect(ui->imgView, &LabelPainterTool::sendFieldRects, this,
            &LNXMTFPrototype::recieveFieldRects);
    ui->NW01->setChecked(true);
    ui->NW02->setChecked(true);
    ui->NW05->setChecked(true);
    ui->NW09->setChecked(true);
    ui->feild01->setRangeY(-0.1, 1.1);
    ui->feild02->setRangeY(-0.1, 1.1);
    ui->feild05->setRangeY(-0.1, 1.1);
    ui->feild09->setRangeY(-0.1, 1.1);
    ui->feild01->resetChartSeries({"NE", "NW", "SW", "SE"});
    ui->feild02->resetChartSeries({"NE", "NW", "SW", "SE"});
    ui->feild05->resetChartSeries({"NE", "NW", "SW", "SE"});
    ui->feild09->resetChartSeries({"NE", "NW", "SW", "SE"});
    ui->feild01->setChartTitle("Field 0.1");
    ui->feild02->setChartTitle("Field 0.2");
    ui->feild05->setChartTitle("Field 0.5");
    ui->feild09->setChartTitle("Field 0.9");
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
    ui->imgView->setParentScrollArea(ui->scrollArea); // 这句先留着，后续修改scrollAreabug的时候用
    ui->imgView->setImg(img);

    // 要在图上画选择的视场
    ui->imgView->setRectProcessor(new myRectProcessor);
    QVector<QVector<bool>> roiBool = {
        {ui->EN01->isChecked(), ui->EN02->isChecked(), false, false, ui->EN05->isChecked(), false,
         false, false, ui->EN09->isChecked()},
        {ui->NW01->isChecked(), ui->NW02->isChecked(), false, false, ui->NW05->isChecked(), false,
         false, false, ui->NW09->isChecked()},
        {ui->WS01->isChecked(), ui->WS02->isChecked(), false, false, ui->WS05->isChecked(), false,
         false, false, ui->WS09->isChecked()},
        {ui->SE01->isChecked(), ui->SE02->isChecked(), false, false, ui->SE05->isChecked(), false,
         false, false, ui->SE09->isChecked()}};
    mFieldRects = ui->imgView->getRectProcessor()->getRoIRects(
        img, roiBool, img.width(), img.height(), ui->roiWidth->value(), ui->roiHeight->value());

    ui->imgView->addFieldRectangle(mFieldRects);

    // 使能编辑图片相关按钮
    ui->zoomIn->setEnabled(true);
    ui->zoomOut->setEnabled(true);
    ui->editRoi->setEnabled(true);
}

QString genSeriesNames(const roiRect& rects)
{
    QString direction;
    switch (rects.d) {
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
        direction = "NULL";
        break;
    }
    return direction;
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

// 调用python脚本计算roiRects里面的MTF值
bool LNXMTFPrototype::calcMTF(const std::vector<roiRect>& roiRects, const QString& imgPath,
                              bool isSave)
{
    std::vector<std::vector<std::vector<double>>> img;
    for (const auto& roi : qAsConst(roiRects)) {
        img.push_back(QimageToArrayLNX(roi.img));
    }
    std::vector<std::vector<double>> information;
    for (const auto& roi : qAsConst(roiRects)) {
        information.push_back({roi.rect.topLeft().x(), roi.rect.topLeft().y(),
                               roi.rect.bottomRight().x(), roi.rect.bottomRight().y(), roi.offset,
                               (double)roi.d, (double)isSave});
    }

    const std::string imgFileName =
        imgPath.mid(imgPath.lastIndexOf(QLatin1String("/")) + 1).toStdString();
    print(imgPath.mid(imgPath.lastIndexOf(QLatin1String("/")) + 1));
    const std::string savefileName = "result.xlsx";
    QVector<int> errRoiId(callPythonReturnMTFData(img, information, imgFileName, savefileName,
                                                  ui->pixelSize->value(), mtfData));
    return errRoiId.isEmpty();
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
    calcMTF(mFieldRects, "D:/验收软件支持/沙姆MTF/03.bmp", false);

    if (mtfData.empty()) {
        warnMoveROI();
        return;
    }

    //    drawChart();

    ui->feild01->setAxisTitle("x", "MTF");
    for (int i = 0; i < mtfData.size(); ++i) {
        if (mFieldRects[i].offset == 0.1) {
            print(ui->feild01->setValues(genSeriesNames(mFieldRects[i]), mtfData[i]));
            ui->feild01->scaleAxisX();
        } else if (mFieldRects[i].offset == 0.2) {
            print(ui->feild02->setValues(genSeriesNames(mFieldRects[i]), mtfData[i]));
            ui->feild02->scaleAxisX();
        } else if (mFieldRects[i].offset == 0.5) {
            print(ui->feild05->setValues(genSeriesNames(mFieldRects[i]), mtfData[i]));
            ui->feild05->scaleAxisX();
        } else if (mFieldRects[i].offset == 0.9) {
            print(ui->feild09->setValues(genSeriesNames(mFieldRects[i]), mtfData[i]));
            ui->feild09->scaleAxisX();
        }
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
