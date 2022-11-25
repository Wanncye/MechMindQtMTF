#include "ui_mainwindow.h"
#include "danleimtfcalc.h"
#include "lnxmtfprototype.h"
#include "mainwindow.h"
#include "mtfcalc.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_mtfCalc_clicked(){
    auto mtf = new MTFCalc();
    mtf->show();
}

void MainWindow::on_danLeiMTFCalc_clicked(){
    auto danLeiMTF = new DanLeiMTFCalc();
    danLeiMTF->show();
}

void MainWindow::on_lnxPrototype_clicked(){
    auto lnxPrototype = new LNXMTFPrototype();
    lnxPrototype->show();
}
