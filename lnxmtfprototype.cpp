#include "lnxmtfprototype.h"
#include "ui_lnxmtfprototype.h"

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
