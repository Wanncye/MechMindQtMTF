'''需要安装pandas和xlsxwriter、openpyxl'''
import numpy as np
import mtf as mtf
import pandas as pd
import os
import matplotlib.pyplot as plt
import pylab

def readExcelFile(fileName: str, imgName) -> list:
    df = pd.read_excel(fileName, sheet_name=imgName)
    # MTF数值，获取的是整个表的数据，因为在计算界面还有MTF错误的显示
    colum = df.columns.size
    roiMTFValueList = []
    for col in range(1, colum):
        roiMTFValueList.append(list(df.iloc[:,col]))
    return roiMTFValueList

def writeToExcelFile(fileName: str, data: tuple, imgName: str='') -> bool:
    # pandas的版本得是1.2.5
#    absPath = "/Users/wanncye/Desktop/MTF"
    absPath = "D://projects_build/MTF_build"
    fileName = os.path.join(absPath, fileName)
    if len(data) == 0:
        return False
    if len(imgName) == 0:
        imgName = 'Sheet1'

    try:
        Attributes = ['deg', 'x1', 'y1', 'x2', 'y2', 'over exposure error', 'edge estimation error', 'MTF error', 'deg error', 'visual field', 'direction']
        Attributes.extend([str(i) for i in range(len(data[0]) - len(Attributes))]) #data[0]含义

        columns = ['ROI '] * len(data)
        formatData = dict()
        for index in range(len(data)):
            columns[index] += str(index)
            formatData[columns[index]] = data[index]

        df = pd.DataFrame(formatData, index=Attributes)
        print("current path:", os.getcwd())
        if os.path.exists(fileName):
            writer = pd.ExcelWriter(fileName, mode='a', engine='openpyxl')
            wb = writer.book
            if imgName in wb:
                print("remove sheet:", imgName)
                wb.remove(wb[imgName])
                print("wb.sheetnames:", wb.sheetnames)
        else:
            writer = pd.ExcelWriter(fileName, mode='w', engine='openpyxl')

        df.to_excel(writer, sheet_name=imgName)
        writer.save()
        print("in writeToExcelFile fileName",fileName)
        writer.close()
        return True

    except Exception as e:
        print("error! in writeToExcelFile")
        print(e)
        return False

def myFun():
    print("in python myFun", flush = True)

def test_main(imgTuple: tuple, positions: tuple, pixelSize: float):
    print("start test_main", flush = True)
    try:
        imgLen = len(imgTuple)
        cnt = imgLen
        pixel_width = pixelSize[0]

        data = list()
        for i in range(imgLen):
            over_exp_err = 0 #过曝
            edge_estim_err = 0  #刃边估计错误
            mtf_err = 0        #mtf计算值不在0-1范围内
            deg_err = 0       #刃边角度不对，此算法要求应在3到11度
            imgArr = np.asarray(imgTuple[i],dtype=np.double) / 255
            imgArr = mtf.Helper.CorrectImageOrientation(imgArr) #保证图片暗面朝上 与原模块处理一样
            res = list()
            position = positions[i]
            try:
                res = mtf.MTF.CalculateMtf(imgArr, pixel_width)
            except Exception as e:
                print(e, flush = True)
                tmpData = [0, position[0], position[1], position[2], position[3], 1, 1, 1, 1, position[4], position[5]]
                tmpData.extend([0]*100)
                data.append(tmpData)
                continue
            # 这里做了一点修改，容忍一定的误差
            mtfErrCount = 0
            for index in range(len(res.x)-1):
                if res.y[index] > 1.01 or res.y[index] <= 0:
                    mtf_err = 1 #MTF计算错误
                    break
                if res.y[index]<res.y[index+1]:
                    mtfErrCount += 1
            if(mtfErrCount > 5):
                mtf_err = 1
            if (len(position) != 6):
                raise ValueError('Positions Error!')
            for row in range(len(imgTuple[i])):
                for column in range(len(imgTuple[i][row])):
                    if imgTuple[i][row][column] >= 250 or imgTuple[i][row][column] < 0:
                        over_exp_err = 1 #过曝错误
            if res.mtfAtNyquist<3 or res.mtfAtNyquist>11:
                deg_err = 1 #刃边角度错误
            esf = mtf.MTF.GetEdgeSpreadFunctionCrop(imgArr,mtf.Verbosity.NONE)
            top = np.max(esf.rawESF.y) - esf.threshold
            bot = np.min(esf.rawESF.y) + esf.threshold
            for index in range(len(esf.interpESF.y)-1):
                if esf.interpESF.y[index] <= top and esf.interpESF.y[index] >=bot and esf.interpESF.y[index] > esf.interpESF.y[index+1]:
                   edge_estim_err = 1 #直线拟合异常
            tmpData = [res.mtfAtNyquist, position[0], position[1], position[2], position[3], over_exp_err, edge_estim_err, mtf_err, deg_err, position[4], position[5]]
            tmpData.extend(res.y)
            print(len(tmpData))
            data.append(tmpData)
        print(len(data))
        return data

    except Exception as e:
        print("----------error in test_main---------", flush = True)
        print(e, flush = True)
        return cnt

if __name__=='__main__':
    readExcelFile("../result.xlsx", "04.bmp")
