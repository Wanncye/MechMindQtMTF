#include "lnxmtfprototype.h"
#include "labelpaintertool.h"
#include "singleMTFCurve.h".h "
#include "ui_lnxmtfprototype.h"
#include "mtfCaculation/callPythonScripts.h"

#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

#define print(val) qDebug() << #val << val

constexpr double maxYAxies = 1.1;
constexpr double minYAxies = -0.1;

inline void warnMoveROI()
{
    QMessageBox::warning(nullptr, LNXMTFPrototype::tr("Calcuate Failure"),
                         LNXMTFPrototype::tr("Failed to calcuate MTF, please edit(move) ROI."));
}

QString genSeriesName(const roiRect& rect)
{
    QString dst;
    QString direction;
    switch (rect.d) {
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
    QString offset = QString::number(rect.offset);
    dst.push_back(QStringLiteral("%1%2").arg(direction, offset));
    return dst;
}

QStringList genSeriesNames(const std::vector<roiRect>& rects)
{
    QStringList dst;
    for (auto& rect : rects)
        dst.push_back(genSeriesName(rect));
    return dst;
}

LNXMTFPrototype::LNXMTFPrototype(QWidget* parent) : QWidget(parent), ui(new Ui::LNXMTFPrototype)
{
    ui->setupUi(this);
    mTimer = new QTimer(this);
    ui->roiWidth->setValue(50);
    ui->roiHeight->setValue(80);
    ui->pixelSize->setValue(5.0);
    ui->frequency->setValue(55);
    ui->criteria->setValue(0.35);
    setWindowState(Qt::WindowMaximized);
    ui->zoomIn->setEnabled(false);
    ui->zoomOut->setEnabled(false);
    ui->editRoi->setEnabled(false);
    connect(ui->imgView, &LabelPainterTool::sendFieldRects, this,
            &LNXMTFPrototype::recieveFieldRects);
    connect(ui->errorTable->horizontalHeader(), &QHeaderView::sectionDoubleClicked, this,
            &LNXMTFPrototype::showSingleMTFCurve);
    connect(mTimer, &QTimer::timeout, this, &LNXMTFPrototype::timerWorker);
    ui->NW01->setChecked(true);
    ui->NW02->setChecked(true);
    ui->NW05->setChecked(true);
    ui->feild01->setAxisTitle("x", "MTF");
    ui->feild02->setAxisTitle("x", "MTF");
    ui->feild05->setAxisTitle("x", "MTF");
    ui->feild09->setAxisTitle("x", "MTF");
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
    if (fileDlg.exec() == QDialog::Rejected)
        return;
    QStringList imgPath = fileDlg.selectedFiles();
    if (imgPath.isEmpty())
        return;

    // 设置图片到label标签中
    if (!mImg.load(imgPath.front())) {
        QMessageBox::information(this, tr("Open Failed"), tr("Open image Failed!"));
        return;
    }
    ui->imgView->setParentScrollArea(ui->scrollArea); // 这句先留着，后续修改scrollAreabug的时候用
    ui->imgView->setImg(mImg);

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
        mImg, roiBool, mImg.width(), mImg.height(), ui->roiWidth->value(), ui->roiHeight->value());
    ui->imgView->addFieldRectangle(mFieldRects);

    ui->feild01->resetChartSeries(genSeriesNames(getSpecificFieldRect(0.1)), false);
    ui->feild02->resetChartSeries(genSeriesNames(getSpecificFieldRect(0.2)), false);
    ui->feild05->resetChartSeries(genSeriesNames(getSpecificFieldRect(0.5)), false);
    ui->feild09->resetChartSeries(genSeriesNames(getSpecificFieldRect(0.9)), false);

    // 使能编辑图片相关按钮
    ui->zoomIn->setEnabled(true);
    ui->zoomOut->setEnabled(true);
    ui->editRoi->setEnabled(true);
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
bool LNXMTFPrototype::calcMTF(const std::vector<roiRect>& roiRects)
{
    std::vector<std::vector<std::vector<double>>> img;
    for (const auto& roi : qAsConst(roiRects)) {
        img.push_back(QimageToArrayLNX(roi.img));
    }
    std::vector<std::vector<double>> information;
    for (const auto& roi : qAsConst(roiRects)) {
        information.push_back({roi.rect.topLeft().x(), roi.rect.topLeft().y(),
                               roi.rect.bottomRight().x(), roi.rect.bottomRight().y(), roi.offset,
                               (double)roi.d});
    }
    QVector<int> errRoiId(callPythonReturnMTFData(img, information, ui->pixelSize->value(),
                                                  mMtfData, mMtfControlInformation));
    return errRoiId.isEmpty();
}
void LNXMTFPrototype::showSingleMTFCurve(int index)
{
    print(index);
    print(ui->errorTable->horizontalHeaderItem(index)->text());
    const auto singleMTFData = mMtfData[index];
    const auto singleRect = mFieldRects[index];
    const auto singleMTFControlInformation = mMtfControlInformation[index];

    auto singleMTFCurveWindows =
        new singleMTFCurve(ui->errorTable->horizontalHeaderItem(index)->text(), singleRect,
                           singleMTFData, singleMTFControlInformation, ui->frequency->value());
    singleMTFCurveWindows->show();
}
void LNXMTFPrototype::showTable()
{
    if (mMtfControlInformation.empty() || mMtfData.empty())
        return;
    // 设置表格
    QTableWidgetItem* headerItem;
    auto headerText = genSeriesNames(mFieldRects);
    ui->errorTable->setColumnCount(headerText.count());
    for (int i = 0; i < ui->errorTable->columnCount(); i++) {
        headerItem = new QTableWidgetItem(headerText.at(i));
        // 可以设置表头的格式
        ui->errorTable->setHorizontalHeaderItem(i, headerItem);
    }
    ui->errorTable->clearContents();
    // 这一块赋值的地方可以缩减一下
    std::vector<double> freqMTF(mMtfData.size(), 0);
    std::vector<double> grayError(mMtfControlInformation.size(), 0);
    std::vector<double> edgeErro(mMtfControlInformation.size(), 0);
    std::vector<double> mtfError(mMtfControlInformation.size(), 0);
    for (int i = 0; i < mMtfData.size(); i++) {
        freqMTF[i] = mMtfData[i][ui->frequency->value()];
    }
    for (int i = 0; i < mMtfControlInformation.size(); i++) {
        grayError[i] = mMtfControlInformation[i][5];
        edgeErro[i] = mMtfControlInformation[i][8];
        mtfError[i] = mMtfControlInformation[i][7];
    }
    QTableWidgetItem* item;
    for (int col = 0; col < ui->errorTable->columnCount(); col++) {
        item = new QTableWidgetItem(QString::number(freqMTF.at(col)));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        if (grayError[col] == 1. || edgeErro[col] == 1. || mtfError[col] == 1.) {
            // 黑色背景，白字，有错误
            item->setBackground(QBrush(Qt::black));
            item->setTextColor(Qt::white);
        } else if (freqMTF.at(col) > ui->criteria->value()) {
            // 绿色背景，高于标准
            item->setBackground(QBrush(Qt::green));
        } else {
            // 红色背景，低于标准
            item->setBackground(QBrush(Qt::red));
        }
        ui->errorTable->setItem(0, col, item);

        item = new QTableWidgetItem(QString::number(grayError.at(col)));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->errorTable->setItem(1, col, item);

        item = new QTableWidgetItem(QString::number(edgeErro.at(col)));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->errorTable->setItem(2, col, item);

        item = new QTableWidgetItem(QString::number(mtfError.at(col)));
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        ui->errorTable->setItem(3, col, item);
    }
}

void LNXMTFPrototype::showChart()
{
    if (mMtfData.empty())
        return;
    // 设置图
    for (int i = 0; i < mMtfData.size(); ++i) {
        if (mFieldRects[i].offset == 0.1) {
            ui->feild01->setValues(genSeriesName(mFieldRects[i]), mMtfData[i]);
            ui->feild01->setRangeY(0, maxYAxies);
            ui->feild01->scaleAxisX();
        } else if (mFieldRects[i].offset == 0.2) {
            ui->feild02->setValues(genSeriesName(mFieldRects[i]), mMtfData[i]);
            ui->feild02->setRangeY(0, maxYAxies);
            ui->feild02->scaleAxisX();
        } else if (mFieldRects[i].offset == 0.5) {
            ui->feild05->setValues(genSeriesName(mFieldRects[i]), mMtfData[i]);
            ui->feild05->setRangeY(0, maxYAxies);
            ui->feild05->scaleAxisX();
        } else if (mFieldRects[i].offset == 0.9) {
            ui->feild09->setValues(genSeriesName(mFieldRects[i]), mMtfData[i]);
            ui->feild09->setRangeY(0, maxYAxies);
            ui->feild09->scaleAxisX();
        }
    }
}

// 感觉这个可以写成STL的算法
std::vector<roiRect> LNXMTFPrototype::getSpecificFieldRect(double offset)
{
    std::vector<roiRect> dst;
    for (auto& rect : mFieldRects) {
        if (rect.offset == offset)
            dst.push_back(rect);
    }
    return dst;
}

void LNXMTFPrototype::on_stopCalc_clicked()
{
    mTimer->stop();
    ui->calcMTF->setEnabled(true);
}

void LNXMTFPrototype::timerWorker()
{
    calcMTF(mFieldRects);
    if (mMtfData.empty() || mMtfControlInformation.empty()) {
        warnMoveROI();
        return;
    }
    showChart();
    showTable();
}

void LNXMTFPrototype::on_calcMTF_clicked()
{
    print("click calcMTF");
    if (mFieldRects.empty()) {
        qDebug() << "mFieldRects NULL";
        return;
    }
    mTimer->start(500);
    ui->calcMTF->setEnabled(false);
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
