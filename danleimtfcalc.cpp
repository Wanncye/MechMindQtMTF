#include "danleimtfcalc.h"
#include "ROISelectionWindow.h"
#include "mtfCaculation/callPythonScripts.h"
#include "ui_danleimtfcalc.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

//#include <fstream>
//#include <iostream>
//#include <QHash>

bool isFirst = true;
std::vector<std::vector<double>> QimageToArray(const QImage& image);

DanLeiMTFCalc::DanLeiMTFCalc(QWidget* parent) : QWidget(parent), ui(new Ui::DanLeiMTFCalc)
{
    ui->setupUi(this);
    setWindowState(Qt::WindowMaximized);
    this->setWindowTitle(QStringLiteral("MTF Tool"));

    ui->c0CB->setEnabled(false);
    ui->ne1CB->setChecked(true);
    ui->nw2CB->setChecked(true);
    ui->sw3CB->setChecked(true);
    ui->se9CB->setChecked(true);
    ui->sw3CB->setChecked(true);
    ui->sw8CB->setChecked(true);
    ui->isSaveCheck->setChecked(true);
    ui->isAutoRB->setChecked(true);
    ui->visualFieldAssistCB->setEnabled(false);
    ui->ipOrMmRB->setChecked(true);
    ui->pixelSizeDB->setEnabled(true);
    ui->ROIWSB->setValue(50);
    ui->ROIHSB->setValue(80);
    ui->pixelSizeDB->setValue(5.86);
    PythonInit();
}

DanLeiMTFCalc::~DanLeiMTFCalc() { delete ui; }

void DanLeiMTFCalc::on_pushButton_clicked()
{
    bool isManual = ui->isManualRB->isChecked();
    int ROIW = ui->ROIWSB->value();
    int ROIH = ui->ROIHSB->value();
    double pixelSize = ui->pixelSizeDB->value();
    isSaveToExcel = ui->isSaveCheck->isChecked() ? 1 : 0;
    QVector<QVector<bool>> visualFieldMatrix(4, QVector<bool>(9, false));

    QVector<bool> pos = {ui->ne1CB->isChecked(), ui->ne2CB->isChecked(), ui->ne3CB->isChecked(),
                         ui->ne4CB->isChecked(), ui->ne5CB->isChecked(), ui->ne6CB->isChecked(),
                         ui->ne7CB->isChecked(), ui->ne8CB->isChecked(), ui->ne9CB->isChecked(),
                         ui->nw1CB->isChecked(), ui->nw2CB->isChecked(), ui->nw3CB->isChecked(),
                         ui->nw4CB->isChecked(), ui->nw5CB->isChecked(), ui->nw6CB->isChecked(),
                         ui->nw7CB->isChecked(), ui->nw8CB->isChecked(), ui->nw9CB->isChecked(),
                         ui->sw1CB->isChecked(), ui->sw2CB->isChecked(), ui->sw3CB->isChecked(),
                         ui->sw4CB->isChecked(), ui->sw5CB->isChecked(), ui->sw6CB->isChecked(),
                         ui->sw7CB->isChecked(), ui->sw8CB->isChecked(), ui->sw9CB->isChecked(),
                         ui->se1CB->isChecked(), ui->se2CB->isChecked(), ui->se3CB->isChecked(),
                         ui->se4CB->isChecked(), ui->se5CB->isChecked(), ui->se6CB->isChecked(),
                         ui->se7CB->isChecked(), ui->se8CB->isChecked(), ui->se9CB->isChecked()};
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 9; j++)
            visualFieldMatrix[i][j] = pos[i * 9 + j];

    QFileDialog fileDlg(this);
    fileDlg.setWindowTitle(QStringLiteral("Choose Pictures"));
    QStringList qstrFilters(
        QStringLiteral("Image files(*.jpg *.png *.bmp)")); // ????????????????????????list
    fileDlg.setNameFilters(qstrFilters);                   // ?????????????????????
    fileDlg.setFileMode(QFileDialog::ExistingFiles);       // ???????????????????????????

    if (fileDlg.exec() == QDialog::Accepted) {
        QStringList strPathList = fileDlg.selectedFiles(); // ?????????????????????????????????????????????list
        for (int i = 0; i < strPathList.count(); i++) {
            QImage image(strPathList[i]);
            // image.load(strPathList[i]);
            m_qlistLoadImgs.push_back(image);

            QVector<roiRect> roiRects; // ??????????????????roi
            if (!isManual) {
                roiRects = ROISelectionWindow::getRoIRects(m_qlistLoadImgs[i], visualFieldMatrix,
                                                           m_qlistLoadImgs[i].width(),
                                                           m_qlistLoadImgs[i].height(), ROIW, ROIH);
            } else {
                roiSelectionWindow =
                    new ROISelectionWindow(strPathList.at(i), isManual, ROIW, ROIH);
                connect(roiSelectionWindow, &ROISelectionWindow::sendConfirmImgs, this,
                        &DanLeiMTFCalc::getNewImgs);
                roiSelectionWindow->exec();
                qDebug() << "OK";
                roiRects = newConfirmImgs;
                newConfirmImgs.clear();
            }

            // mtf?????????
            if (roiRects.empty())
                return;

            std::vector<std::vector<std::vector<double>>> img;
            for (const auto& roi : qAsConst(roiRects)) {
                img.push_back(QimageToArray(roi.img));
            }
            std::vector<std::vector<double>> information;
            for (const auto& roi : qAsConst(roiRects)) {
                information.push_back({roi.rect.topLeft().x(), roi.rect.topLeft().y(),
                                       roi.rect.bottomRight().x(), roi.rect.bottomRight().y(),
                                       roi.offset, (double)roi.d, (double)isSaveToExcel});
            }
            qDebug() << "information.size:" << information.size();
            QString fileName =
                strPathList[i].mid(strPathList[i].lastIndexOf(QLatin1String("/")) + 1);
            QVector<int> errRoiId(callPython(img, information, std::string("result.xlsx"),
                                             (std::string)fileName.toLocal8Bit(), pixelSize));

            QHash<int, int> qHash;
            QVector<roiRect> errROI;
            QVector<roiRect> trueROI;

            QVector<QRectF> errRectf;
            QVector<QRectF> trueRectf;

            // ????????????
            if (!errRoiId.isEmpty()) {
                for (const auto& i : errRoiId) {
                    qHash[i]++;
                    errROI.push_back(roiRects[i]);
                    errRectf.push_back(roiRects[i].rect);
                }

                for (int i = 0; i < roiRects.size(); i++) {
                    if (!qHash[i]) {
                        trueROI.push_back(roiRects[i]);
                        trueRectf.push_back(roiRects[i].rect);
                    }
                }

                int result(processCode());
                //??????????????????????????????????????????
                switch (result) {
                case QMessageBox::Yes:
                {
                    //                    roiSelectionWindow = new
                    //                    ROISelectionWindow(strPathList.at(i), false, ROIW,
                    //                                                                ROIH,
                    //                                                                errRectf,
                    //                                                                trueRectf,
                    //                                                                nullptr);
                    //                    roiSelectionWindow->show();
                    //                    result = processCode();
                    //                    roiSelectionWindow->close();
                    //?????????????????????????????????
                    while (!errROI.isEmpty() && result != QMessageBox::Cancel) {
                        // ????????????????????????????????????????????????
                        roiSelectionWindow =
                            new ROISelectionWindow(strPathList.at(i), errRectf.front(), true);
                        connect(roiSelectionWindow, &ROISelectionWindow::sendConfirmImgs, this,
                                &DanLeiMTFCalc::getNewImgs);
                        roiSelectionWindow->exec();
                        QVector<roiRect> signleRoiRect;
                        signleRoiRect.push_back(newConfirmImgs.back());
                        if (!signleRoiRect.isEmpty()) {
                            if (calcMTF(signleRoiRect, strPathList.at(i), false)) {
                                errROI.pop_front();
                                errRectf.pop_front();
                                trueROI.push_front(
                                    signleRoiRect.front()); //?????????roi?????? ??????????????????
                            } else {
                                result = processCode();
                                if (result == QMessageBox::No) {
                                    trueROI.push_back(
                                        errROI.front()); // NO?????? ???????????????roi??????
                                                         // cancel?????????????????? ???????????? ????????????
                                    errROI.pop_front();
                                    errRectf.pop_front();
                                }
                            }
                        }
                    }
                    //????????????
                    calcMTF(trueROI, strPathList[i]);
                } break;
                case QMessageBox::No:
                    callPython(img, information, std::string("result.xlsx"),
                               (std::string)fileName.toLocal8Bit(), pixelSize);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void DanLeiMTFCalc::on_isManualRB_clicked() { ui->visualFieldAssistCB->setEnabled(true); }

void DanLeiMTFCalc::on_isAutoRB_clicked() { ui->visualFieldAssistCB->setEnabled(false); }

void DanLeiMTFCalc::on_CorPRB_clicked()
{
    ui->pixelSizeDB->setEnabled(false);
    ui->pixelSizeLB->setEnabled(false);
}

void DanLeiMTFCalc::on_LWOrPHRB_clicked()
{
    ui->pixelSizeDB->setEnabled(false);
    ui->pixelSizeLB->setEnabled(false);
}

void DanLeiMTFCalc::on_ipOrMmRB_clicked(bool checked)
{
    ui->LWOrPHRB->setEnabled(false);
    ui->CorPRB->setEnabled(false);
}

void DanLeiMTFCalc::getNewImgs(const QVector<roiRect>& imgs) { newConfirmImgs = imgs; }

int DanLeiMTFCalc::processCode()
{
    return QMessageBox::information(
        nullptr, tr("An abnormal result was detected"),
        tr("Detection of an exception in the result, do you want to manually "
           "process the exception data?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
}

// ??????python????????????roiRects?????????MTF???
bool DanLeiMTFCalc::calcMTF(const QVector<roiRect>& roiRects, const QString& strPath, bool isSave)
{
    std::vector<std::vector<std::vector<double>>> img;
    for (const auto& roi : qAsConst(roiRects)) {
        img.push_back(QimageToArray(roi.img));
    }
    std::vector<std::vector<double>> information;
    for (const auto& roi : qAsConst(roiRects)) {
        information.push_back({roi.rect.topLeft().x(), roi.rect.topLeft().y(),
                               roi.rect.bottomRight().x(), roi.rect.bottomRight().y(), roi.offset,
                               (double)roi.d, (double)isSave});
    }

    QString fileName = strPath.mid(strPath.lastIndexOf(QLatin1String("/")) + 1);
    QVector<int> errRoiId(callPython(img, information, std::string("result.xlsx"),
                                     (std::string)fileName.toLocal8Bit(),
                                     ui->pixelSizeDB->value()));
    return errRoiId.isEmpty();
}

std::vector<std::vector<double>> QimageToArray(const QImage& image)
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
