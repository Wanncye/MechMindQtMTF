#pragma once
#include <unordered_map>
#include <QtCharts/QChartView>

class ProfilerChartView : public QtCharts::QChartView
{
    Q_OBJECT
public:
    explicit ProfilerChartView(QWidget* parent = nullptr);

    void setChartTitle(const QString& title);
    void setChartTheme(QtCharts::QChart::ChartTheme theme);

    void resetChartSeries();
    void resetChartSeries(const QColor& color);
    void resetChartSeries(const QStringList& names, bool colorRandom = true);
    void resetChartSeries(const QStringList& names, const std::vector<QColor>& colors);

    void setAxisTitle(const QString& titleX, const QString& titleY);

    void setAxisLabelFormatX(const QString& format);
    void setAxisLabelFormatY(const QString& format);
    void setAxisLabelsFormat(const QString& formatX, const QString& formatY);
    void setAxisLabelsFormat(const QString& format);

    void setTickCountX(int count);
    void setTickCountY(int count);
    void setTickCount(int countX, int countY);
    void setTickCount(int count);

    void setLegendVisible(bool visibility);

    void setRangeX(double minVal, double maxVal);
    void setRangeY(double minVal, double maxVal);
    void scaleAxisX();
    void scaleAxisY();
    void scaleAxes();
    void scaleAxesBalanced();

    QString setValues(const std::vector<double>& vals);
    QString setValues(const QVector<QPointF>& qpts);

    QString setValues(const QString& seriesName, const std::vector<double>& vals);
    QString setValues(const QString& seriesName, const QVector<QPointF>& qpts);

protected:
    virtual QtCharts::QAbstractSeries* genChartSeries() const = 0;

    QtCharts::QChart* _chart;
    std::unordered_map<QString, QtCharts::QAbstractSeries*> _seriesMap;
};

class ProfilerScatterChart : public ProfilerChartView
{
public:
    explicit ProfilerScatterChart(QWidget* parent = nullptr);

    void setMarkerSize(qreal size);

private:
    QtCharts::QAbstractSeries* genChartSeries() const override;
};

class MTFProfilerLineChart : public ProfilerChartView
{
public:
    explicit MTFProfilerLineChart(QWidget* parent = nullptr);

private:
    QtCharts::QAbstractSeries* genChartSeries() const override;
};

class ProfilerSplineChart : public ProfilerChartView
{
public:
    explicit ProfilerSplineChart(QWidget* parent = nullptr);

private:
    QtCharts::QAbstractSeries* genChartSeries() const override;
};
