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
    ui->imgView->addRangle(0, 0, 50, 50);
    ui->imgView->addRangle(0, 0, 40, 60);
}

void LNXMTFPrototype::on_calcMTF_clicked(){
    ui->imgView->addRangle(0, 0, 90, 90);
    ui->imgView->update();
}
