#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void on_mtfCalc_clicked();
    void on_danLeiMTFCalc_clicked();
    void on_lnxPrototype_clicked();
    void on_imgView_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
