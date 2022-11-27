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
    ui->imgView->setPixmap(QPixmap::fromImage(img));

    // 要在图上画好13个ROI框
    myRectProcessor rectProcessor;
    QVector<QVector<bool>> roiBool = {{true, true, false, false, true, false, false, false, true},
                                      {true, true, false, false, true, false, false, false, true},
                                      {true, true, false, false, true, false, false, false, true},
                                      {true, true, false, false, true, false, false, false, true}};
    qDebug() << "img.width():" << img.width();
    qDebug() << "img.height():" << img.height();
    QVector<roiRect> allROI = rectProcessor.getRoIRects(img, roiBool, img.width(), img.height(), ui->roiWidth->value(), ui->roiHeight->value());
    ui->imgView->addRectangle(allROI);
    ui->imgView->update();
}

void LNXMTFPrototype::on_calcMTF_clicked(){
    ui->imgView->addRectangle(0, 0, 90, 90);
    ui->imgView->update();
}
