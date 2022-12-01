#include <stdexcept>
#include <QtCharts/QChart>
#include <QDebug>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include "ProfilerChartView.h"

using namespace std;
using namespace QtCharts;

constexpr char kDefaultSeries[]{"Default"};
constexpr char kDefaultColorName[]{"red"};
constexpr int kDefaultTickCount = 10;
constexpr qreal defaultMarkerSize = 1;

#define print(val) qDebug() << #val << val

template <class LessCompare>
pair<QPointF, QPointF> findMinMaxPt(
    const unordered_map<QString, QtCharts::QAbstractSeries*>& seriesMap, LessCompare lessComp)
{
    vector<pair<QPointF, QPointF>> minMaxElems;
    transform(seriesMap.begin(), seriesMap.end(), back_inserter(minMaxElems),
              [=](const pair<QString, QtCharts::QAbstractSeries*>& mapElem) {
                  const auto pts = static_cast<QXYSeries*>(mapElem.second)->pointsVector();
                  const auto iterElems = minmax_element(pts.begin(), pts.end(), lessComp);
                  return pair<QPointF, QPointF>(*iterElems.first, *iterElems.second);
              });
    auto minPt = minMaxElems.front().first;
    auto maxPt = minMaxElems.front().second;
    for (int i = 1; i < minMaxElems.size(); ++i) {
        const auto& pair = minMaxElems.at(i);
        if (lessComp(pair.first, minPt))
            minPt = pair.first;
        if (lessComp(maxPt, pair.second))
            maxPt = pair.second;
    }
    return {minPt, maxPt};
}

inline pair<qreal, qreal> findMinMaxX(
    const unordered_map<QString, QtCharts::QAbstractSeries*>& seriesMap)
{
    const auto minMaxPt =
        findMinMaxPt(seriesMap, [](const QPointF& a, const QPointF& b) { return a.x() < b.x(); });
    return {minMaxPt.first.x(), minMaxPt.second.x()};
}

inline pair<qreal, qreal> findMinMaxY(
    const unordered_map<QString, QtCharts::QAbstractSeries*>& seriesMap)
{
    const auto minMaxPt =
        findMinMaxPt(seriesMap, [](const QPointF& a, const QPointF& b) { return a.y() < b.y(); });
    print(minMaxPt.first.y());
    print(minMaxPt.second.y());
    return {minMaxPt.first.y(), minMaxPt.second.y()};
}

vector<QColor> genMultiColors(int count)
{
    const int len = 360 / count;
    const int h = rand() % len;
    vector<QColor> rgbColors;
    for (int i = 0; i < count; ++i)
        rgbColors.emplace_back(QColor::fromHsv(h + len * i, 255, 255));
    return rgbColors;
}

ProfilerChartView::ProfilerChartView(QWidget* parent) : QChartView(parent), _chart(new QChart)
{
    setChart(_chart);
}

void ProfilerChartView::resetChartSeries() { resetChartSeries(QColor(kDefaultColorName)); }

void ProfilerChartView::resetChartSeries(const QColor& color)
{
    resetChartSeries({QString::fromLatin1(kDefaultSeries)}, {color});
}

void ProfilerChartView::resetChartSeries(const QStringList& names)
{
    if (names.isEmpty()) {
        resetChartSeries();
        return;
    }
    const auto& colors = genMultiColors(names.size());
    resetChartSeries(names, colors);
}

void ProfilerChartView::resetChartSeries(const QStringList& names,
                                         const std::vector<QColor>& colors)
{
    _chart->removeAllSeries();
    _seriesMap.clear();
    if (names.empty())
        return;

    for (int i = 0; i < names.size(); ++i) {
        auto* series = genChartSeries();
        static_cast<QXYSeries*>(series)->setColor(colors.at(i));
        const auto& seriesName = names.at(i);
        series->setName(seriesName);
        series->setUseOpenGL(true);
        _chart->addSeries(series);
        _seriesMap.emplace(seriesName, series);
    }
    _chart->createDefaultAxes();
    //    _chart->legend()->setVisible(names.size() > 1);
    setTickCountX(kDefaultTickCount);
    setTickCountY(kDefaultTickCount);
}

void ProfilerChartView::setChartTitle(const QString& title) { _chart->setTitle(title); }

void ProfilerChartView::setChartTheme(QtCharts::QChart::ChartTheme theme)
{
    _chart->setTheme(theme);
}

void ProfilerChartView::setAxisTitle(const QString& titleX, const QString& titleY)
{
    _chart->axisX()->setTitleText(titleX);
    _chart->axisY()->setTitleText(titleY);
}

void ProfilerChartView::setAxisLabelFormatX(const QString& format)
{
    static_cast<QValueAxis*>(_chart->axisX())->setLabelFormat(format);
}

void ProfilerChartView::setAxisLabelFormatY(const QString& format)
{
    static_cast<QValueAxis*>(_chart->axisY())->setLabelFormat(format);
}

void ProfilerChartView::setAxisLabelsFormat(const QString& formatX, const QString& formatY)
{
    setAxisLabelFormatX(formatX);
    setAxisLabelFormatY(formatY);
}

void ProfilerChartView::setAxisLabelsFormat(const QString& format)
{
    setAxisLabelsFormat(format, format);
}

void ProfilerChartView::setTickCountX(int count)
{
    static_cast<QValueAxis*>(_chart->axisX())->setTickCount(count);
}

void ProfilerChartView::setTickCountY(int count)
{
    static_cast<QValueAxis*>(_chart->axisY())->setTickCount(count);
}

void ProfilerChartView::setTickCount(int countX, int countY)
{
    setTickCountX(countX);
    setTickCountY(countY);
}

void ProfilerChartView::setTickCount(int count) { setTickCount(count, count); }

void ProfilerChartView::setLegendVisible(bool visibility)
{
    _chart->legend()->setVisible(visibility);
}

void ProfilerChartView::setRangeX(double minVal, double maxVal)
{
    auto* axis = _chart->axisX();
    axis->setMin(minVal);
    axis->setMax(maxVal);
}

void ProfilerChartView::setRangeY(double minVal, double maxVal)
{
    auto* axis = _chart->axisY();
    axis->setMin(minVal);
    axis->setMax(maxVal);
}

void ProfilerChartView::scaleAxisX()
{
    const auto minMaxVal = findMinMaxX(_seriesMap);
    setRangeX(minMaxVal.first, minMaxVal.second);
}

void ProfilerChartView::scaleAxisY()
{
    const auto minMaxVal = findMinMaxY(_seriesMap);
    setRangeY(minMaxVal.first, minMaxVal.second);
}

void ProfilerChartView::scaleAxes()
{
    scaleAxisX();
    scaleAxisY();
}

void ProfilerChartView::scaleAxesBalanced()
{
    const auto minMaxX = findMinMaxX(_seriesMap);
    const auto minMaxY = findMinMaxY(_seriesMap);
    const auto minVal = qMin(minMaxX.first, minMaxY.first);
    const auto maxVal = qMax(minMaxX.second, minMaxY.second);
    _chart->axisX()->setMin(minVal);
    _chart->axisX()->setMax(maxVal);
    _chart->axisY()->setMin(minVal);
    _chart->axisY()->setMax(maxVal);
}

QString ProfilerChartView::setValues(const std::vector<double>& vals)
{
    return setValues(kDefaultSeries, vals);
}

QString ProfilerChartView::setValues(const QVector<QPointF>& qpts)
{
    return setValues(kDefaultSeries, qpts);
}

QString ProfilerChartView::setValues(const QString& seriesName, const std::vector<double>& vals)
{
    QVector<QPointF> qpts;
    for (decltype(vals.size()) i = 0; i < vals.size(); ++i)
        qpts.push_back(QPointF(i, vals.at(i)));
    return setValues(seriesName, qpts);
}

QString ProfilerChartView::setValues(const QString& seriesName, const QVector<QPointF>& qpts)
{
    auto iter = _seriesMap.find(seriesName);
    if (iter == _seriesMap.end())
        return QStringLiteral("Invalid series name %1.").arg(seriesName);
    auto* series = static_cast<QXYSeries*>(iter->second);
    series->replace(qpts);
    return {};
}

ProfilerScatterChart::ProfilerScatterChart(QWidget* parent) : ProfilerChartView(parent)
{
    resetChartSeries();
}

void ProfilerScatterChart::setMarkerSize(qreal size)
{
    for (auto& pair : _seriesMap) {
        static_cast<QScatterSeries*>(pair.second)->setMarkerSize(size);
    }
}

QAbstractSeries* ProfilerScatterChart::genChartSeries() const
{
    auto* series = new QScatterSeries(_chart);
    series->setPen(QPen(Qt::NoPen));
    series->setMarkerSize(defaultMarkerSize);
    return series;
}

MTFProfilerLineChart::MTFProfilerLineChart(QWidget* parent) : ProfilerChartView(parent)
{
    resetChartSeries();
}

QAbstractSeries* MTFProfilerLineChart::genChartSeries() const { return new QLineSeries(_chart); }

ProfilerSplineChart::ProfilerSplineChart(QWidget* parent) : ProfilerChartView(parent)
{
    resetChartSeries();
}

QAbstractSeries* ProfilerSplineChart::genChartSeries() const { return new QSplineSeries(_chart); }
