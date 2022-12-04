#pragma once
#include <QDebug>

void PythonInit();

QVector<int> callPython(const std::vector<std::vector<std::vector<double>>>& img,
                        const std::vector<std::vector<double>>& information,
                        const std::string& saveFileName, const std::string& imgFileName,
                        const double& pixelSize);

void callPythonReturnMTFData(const std::vector<std::vector<std::vector<double>>>& img,
                             const std::vector<std::vector<double>>& information,
                             const double& pixelSize, std::vector<std::vector<double>>& mtfData,
                             std::vector<std::vector<double>>& mtfControlData);

QVector<int> PythonTest(const std::vector<std::vector<std::vector<double>>>& img,
                        const std::vector<std::vector<double>>& information,
                        const double& pixelSize, std::vector<std::vector<double>>& mtfData,
                        std::vector<std::vector<double>>& mtfControlData);
