#pragma once
#include <QDebug>

void PythonInit();

QVector<int> callPython(const std::vector<std::vector<std::vector<double>>>& img,
                        const std::vector<std::vector<double>>& information,
                        const std::string& saveFileName, const std::string& imgFileName,
                        const double& pixelSize);

QVector<int> callPythonReturnMTFData(const std::vector<std::vector<std::vector<double>>>& img,
                                     const std::vector<std::vector<double>>& information,
                                     const std::string& saveFileName,
                                     const std::string& imgFileName, const double& pixelSize,
                                     std::vector<std::vector<double>>& mtfData);
