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

def main(imgTuple: tuple, positions: tuple, fileNameTuple: tuple, pixelSize: float) -> int:
    '''
        imgTuple: it stores the 2d array of the image
        position: other import information
        fileNameTuple: first one stores the excel file name, second one stores name of the image
    '''
    # try:
    imgLen = len(imgTuple)
    print(imgLen)
    cnt = imgLen
    saveFileName = str(fileNameTuple[0], 'gbk')
    imgName = str(fileNameTuple[1], 'gbk')
    pixel_width = pixelSize[0]

    #针对横坐标，划分出一个中间区，对于中间区之外的两头部分（各取10%），允许其略微偏离0-1范围,但是中间区的value值必须在0-1范围内
    x_num = int(500/pixel_width)
    part_range = int(500/pixel_width*0.1)
    begin_index = part_range
    end_index = x_num - part_range -1


    data = list()
    for i in range(imgLen):
        imgArr = np.asarray(imgTuple[i]) / 255
        res = mtf.MTF.CalculateMtf(imgArr, pixel_width, verbose=mtf.Verbosity.DETAIL)
        position = positions[i]
        if (len(position) != 6):
            raise ValueError('Positions Error!')

        #四类错误
        over_exp_err = 0 #过曝
        edge_estim_err = 0  #刃边估计错误
        mtf_err = 0        #mtf计算值不在0-1范围内
        deg_err = 0       #刃边角度不对，此算法要求应在2到10度

        findErr = 0 #发现错误标志位

        #对于每个ROI框，检查是否有计算错误
        #检查角度错误
        #if res.mtfAtNyquist<2 or res.mtfAtNyquist>10:
         #   deg_err = 1
           # findErr = 1 #发现错误

        #检查mtf计算值是否在范围内
        #for x1 in range(begin_index):
         #   if(res.y[x1]>1.02):
          #      mtf_err = 1
           #     findErr = 1
            #    break
        #for x2 in range(begin_index,x_num):
         #   if(res.y[x2]>1 or res.y[x2]<0):
          #      mtf_err = 1
           #     findErr = 1
            #    break

        #检查刃边估计错误



        #检查是否过曝




        tmpData = [res.mtfAtNyquist, position[0], position[1], position[2], position[3], over_exp_err, edge_estim_err, mtf_err, deg_err, position[4], position[5]]
        tmpData.extend(res.y)
        data.append(tmpData)

        if findErr==1:
            break
        cnt -= 1

    if cnt == 0:
        isSaved =  writeToExcelFile(saveFileName, data, imgName=imgName)
        if not isSaved:
            print('Cannot save!')

    return cnt
    #  except Exception as e:
    #      print(e)
    #      print('Cannot save!')
    #      return cnt

def test_main(imgTuple: tuple, positions: tuple, fileNameTuple: tuple, pixelSize: float):
#    stdout = open("mtflog.txt", "w")
    print("start test_main", flush = True)
    try:
        imgLen = len(imgTuple)
        print("imgLen", imgLen)
        cnt = imgLen
        saveFileName = str(fileNameTuple[0], 'gbk')
        imgName = str(fileNameTuple[1], 'gbk')
        pixel_width = pixelSize[0]

        data = list()
        errRoiId = list()
        for i in range(imgLen):
            over_exp_err = 0 #过曝
            edge_estim_err = 0  #刃边估计错误
            mtf_err = 0        #mtf计算值不在0-1范围内
            deg_err = 0       #刃边角度不对，此算法要求应在2到10度
            find_err = 0
            imgArr = np.asarray(imgTuple[i],dtype=np.double) / 255
            imgArr = mtf.Helper.CorrectImageOrientation(imgArr) #保证图片暗面朝上 与原模块处理一样
            res = mtf.MTF.CalculateMtf(imgArr, pixel_width)
            for index in range(len(res.x)-1):
                if res.y[index] > 1.0000000000000002 or res.y[index] <= 0 or res.y[index]<res.y[index+1]:
                    mtf_err = 1 #MTF计算错误
            position = positions[i]
            if (len(position) != 7):
                raise ValueError('Positions Error!')
            for row in range(len(imgTuple[i])):
                for column in range(len(imgTuple[i][row])):
                    if imgTuple[i][row][column] >= 250 or imgTuple[i][row][column] < 0:
                        over_exp_err = 1 #过曝错误
            if res.mtfAtNyquist<2 or res.mtfAtNyquist>10:
                deg_err = 1 #刃边角度错误
            esf = mtf.MTF.GetEdgeSpreadFunctionCrop(imgArr,mtf.Verbosity.NONE)
            top = np.max(esf.rawESF.y) - esf.threshold
            bot = np.min(esf.rawESF.y) + esf.threshold
            for index in range(len(esf.interpESF.y)-1):
                if esf.interpESF.y[index] <= top and esf.interpESF.y[index] >=bot and esf.interpESF.y[index] > esf.interpESF.y[index+1]:
                   edge_estim_err = 1 #直线拟合异常

            diredict ={0:"ne",1:"nw",2:"sw",3:"se"}
            dire = diredict[position[5]]
            if deg_err or mtf_err or over_exp_err or edge_estim_err:
                errRoiId.append(i)
                find_err = 1
#            isSaveFlag = int(position[6])
#            if not isSaveFlag:
#               continue #错误暂时不保存 该算法模块需添加一个是否需要保存数据的参数 来应对错误数据的是否保存
            tmpData = [res.mtfAtNyquist, position[0], position[1], position[2], position[3], over_exp_err, edge_estim_err, mtf_err, deg_err, position[4], position[5]]
            tmpData.extend(res.y)
            print(len(tmpData))
            data.append(tmpData)
        print(len(data))
        return errRoiId, data
#        return errRoiId

    except Exception as e:
        print("----------error in test_main---------", flush = True)
        print(e, flush = True)
        return cnt

if __name__=='__main__':
    readExcelFile("../result.xlsx", "04.bmp")
