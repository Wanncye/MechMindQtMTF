#include "mtfcalc.h"
#include "ui_mtfcalc.h"

MTFCalc::MTFCalc(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MTFCalc)
{
    ui->setupUi(this);
}

MTFCalc::~MTFCalc()
{
    delete ui;
}

/***********************************************
 存在问题：
 1. 对焦工具使用的镜头是LNX的，得到的图片是深度激光线，可是MTF计算是需要图片？这个问题怎么操作？
 2. 已有现成的MTF计算界面，是不是将这些已有的代码功能完善移植到profiler的对角工具中？
 3. 按照设置的尺寸划取ROI是不是要支持手动和自动？
************************************************/
