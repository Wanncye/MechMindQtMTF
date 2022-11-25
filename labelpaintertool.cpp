#include "labelpaintertool.h"

LabelPainterTool::LabelPainterTool(QWidget *parent) : QLabel(parent){}
LabelPainterTool::~LabelPainterTool(){}

void LabelPainterTool::paintEvent(QPaintEvent *event){
    QLabel::paintEvent(event);
    QPainter painter(this);
    painter.setPen(QPen(Qt::red,10));
    for(const auto& rect : mRects)
        painter.drawRect(rect);
}
void LabelPainterTool::mousePressEvent(QMouseEvent *e){}   //鼠标按下
void LabelPainterTool::mouseMoveEvent(QMouseEvent *e){}    //鼠标移动
void LabelPainterTool::mouseReleaseEvent(QMouseEvent *e){} //鼠标抬起
void LabelPainterTool::addRangle(QRectF rect){
    mRects.push_back(rect);
}

void LabelPainterTool::addRangle(int x, int y, int h, int w){
    addRangle(QRectF(x,y,w,h));
}


