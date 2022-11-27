#include "lnxmtfprototype.h"
#include "ui_lnxmtfprototype.h"

#include<QFileDialog>
#include<QDebug>
#include<QMessageBox>
#include <QPainter>

LNXMTFPrototype::LNXMTFPrototype(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LNXMTFPrototype)
{
    ui->setupUi(this);
    ui->roiWidth->setValue(50);
    ui->roiHeight->setValue(80);
}

LNXMTFPrototype::~LNXMTFPrototype()
{
    delete ui;
}

void LNXMTFPrototype::on_loadImg_clicked()
{
    // 选择图片界面
    QFileDialog fileDlg(this);
    fileDlg.setWindowTitle(QStringLiteral("Choose Pictures"));
    const QStringList qstrFilters(QStringLiteral("Image files(*.jpg *.png *.bmp)"));
    fileDlg.setNameFilters(qstrFilters);
    QImage img;
    if(fileDlg.exec() == QDialog::Rejected)
        return;
    QStringList imgPath = fileDlg.selectedFiles();
    if(imgPath.isEmpty())
        return;

    // 设置图片到label标签中
    if(!img.load(imgPath.front())){
        QMessageBox::information(this,tr("提示框"),tr("打开图像失败!"));
        return;
    }
    ui->imgView->setImg(img);

    // 要在图上画好13个ROI框
    ui->imgView->setRectProcessor(new myRectProcessor);
    QVector<QVector<bool>> roiBool = {{true, true, false, false, true, false, false, false, true},
                                      {true, true, false, false, true, false, false, false, true},
                                      {true, true, false, false, true, false, false, false, true},
                                      {true, true, false, false, true, false, false, false, true}};

    QVector<roiRect> allROI = ui->imgView->getRectProcessor()->getRoIRects(img, roiBool, img.width(), img.height(), ui->roiWidth->value(), ui->roiHeight->value());
    ui->imgView->addFieldRectangle(allROI);
}

void LNXMTFPrototype::on_calcMTF_clicked(){
}

void LNXMTFPrototype::on_zoomIn_clicked(){
    ui->imgView->onZoomInImage();
}
void LNXMTFPrototype::on_zoomOut_clicked(){
    ui->imgView->onZoomOutImage();
}
