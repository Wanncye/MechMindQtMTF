#include<opencv2/opencv.hpp>
#include<iostream>
#include<math.h>

using namespace cv;
using namespace std;

#define print(x) cout << #x << " " << x << endl;
#define printWithUnit(x, unit) cout << #x << " " << x << #unit << endl;

void sharpImg(Mat& img){
    //定义锐化算子
	Mat sharp = (Mat_<int>(3, 3) << 0, -1, 0,
									-1, 5, -1,
									0, -1, 0);
	filter2D(img, img, -1, sharp, Point(-1, -1), 0, BORDER_DEFAULT);
	convertScaleAbs(img, img);
}

void  CoaxialObjectFieldOfView(Mat& src){
    //转换灰度图
    Mat dst;
    cvtColor(src,dst,COLOR_BGR2GRAY);
    imshow("gray",dst);

    // sharpImg(dst);
    // imshow("sharp",dst);

    //边缘检测
    /*
    第一个参数：InputArray类型的image，输入图像，Mat对象节课，需为单通道8位图像。
    第二个参数：OutputArray类型的edges，输出的边缘图，需要和输入图像有相同的尺寸和类型。
    第三个参数：double类型的threshold1，第一个滞后性阈值。
    第四个参数：double类型的threshold2，第二个滞后性阈值。
    第五个参数：int类型的apertureSize，表示算子的孔径的大小，默认值时3.
    第六个参数：bool类型的L2gradient，一个计算图像梯度复制的标识，默认false。
    阈值1和阈值2两者中比较小的值用于边缘连接，较大的值用来控制边缘的初始段，推荐高低阈值比在2:1和3:1之间。
    */
    Canny(dst,dst,150,200,3,true);
    imshow("canny",dst);

    //霍夫直线检测
    vector<Vec4f> plines;
    /*
    第一个参数，InputArray类型的image，输入图像，即源图像，需为8位的单通道二进制图像，可以将任意的源图载入进来后由函数修改成此格式后，再填在这里。
    第二个参数，InputArray类型的lines，经过调用HoughLinesP函数后后存储了检测到的线条的输出矢量，每一条线由具有四个元素的矢量(x_1,y_1, x_2, y_2） 表示，其中，(x_1, y_1)和(x_2, y_2) 是是每个检测到的线段的结束点。
    第三个参数，double类型的rho， 以像素为单位的距离精度。 另一种形容方式是直线搜索时的进步尺寸的单位半径。
    第四个参数，double类型的theta，以弧度为单位的角度精度。另一种形容方式是直线搜索时的进步尺寸的单位角度。
    第五个参数，int类型的threshold，累加平面的阈值参数，即识别某部分为图中的一条直线时它在累加平面中必须达到的值。 大于阈值 threshold 的线段才可以被检测通过并返回到结果中。
    第五个参数可以让用户来调节
    第六个参数，double类型的minLineLength，有默认值0，表示最低线段的长度，比这个设定参数短的线段就不能被显现出来。
    第七个参数，double类型的maxLineGap，有默认值0，允许将同一行点与点之间连接起来的最大的距离。
    */
    HoughLinesP(dst, plines, 1, CV_PI/180, 15, 20, 100);

    // TODO:可以对检测的线去重，但是线去不去重，好像不是很影响结果

    Mat matLine = Mat::zeros(src.size(),src.type());
    int lineWidth = 5;
    for(int i=0;i<plines.size();i++){
        Vec4f h = plines[i];
        line(matLine,Point(h[0],h[1]),Point(h[2],h[3]),Scalar(0,255,0),lineWidth,LINE_AA);
        // circle(matLine, Point(h[0],h[1]), 3, Scalar(255, 0, 0), 30);
        // circle(matLine, Point(h[2],h[3]), 3, Scalar(0, 0, 255), 30);
    }
    imshow("lines",matLine);

    // 计算从中心点到两边通过多少条线
    Point midPoint(src.rows / 2, src.cols / 2);
    int verticalNum = 0, horizontalNum = 0;
    int verticalFirstIndex = -1, verticalLastIndex = -1;
    int horizontalFirstIndex = -1 , horizontalLastIndex = -1;

    // 中心点上下
    for(int i = 0; i < src.rows; i++){
        if((int)matLine.at<Vec3b>(i, midPoint.y)[1] == 255){
            // 需要后面几个都等于255才行
            while((int)matLine.at<Vec3b>(++i, midPoint.y)[1] == 255);
            verticalNum++;
            if(verticalNum == 1)
                verticalFirstIndex = i;
            verticalLastIndex = i;
        }
    }

    // 计算X
    const double pixelPerGridVertical = (verticalLastIndex - verticalFirstIndex) / (verticalNum - 1);
    const double upGridPercent = (verticalFirstIndex + 1) / pixelPerGridVertical;
    const double downGridPercent = (src.rows - (verticalLastIndex + 1)) / pixelPerGridVertical;
    print((verticalNum - 1) + upGridPercent + downGridPercent);
    const double X = ((verticalNum - 1) + upGridPercent + downGridPercent) * 3;
    print(X);

    // 中心点左右
    for(int i = 0; i < src.cols; i++){
        if((int)matLine.at<Vec3b>(midPoint.x, i)[1] == 255){
            // 需要后面几个都等于255才行
            while((int)matLine.at<Vec3b>(midPoint.x, ++i)[1] == 255);
            horizontalNum++;
            if(horizontalNum == 1)
                horizontalFirstIndex = i;
            horizontalLastIndex = i;
        }
    }

    // 计算Y
    const double pixelPerGridHorizontal = (horizontalLastIndex - horizontalFirstIndex) / (horizontalNum - 1);
    const double leftGridPercent = (horizontalFirstIndex + 1) / pixelPerGridHorizontal;
    const double rightGridPercent = (src.cols - (horizontalLastIndex + 1)) / pixelPerGridHorizontal;
    print((horizontalNum - 1) + leftGridPercent + rightGridPercent);
    const double Y = ((horizontalNum - 1) + leftGridPercent + rightGridPercent) * 3;
    print(Y);
}

void relativeIllumination(Mat& src){
    // 获取灰度图
    Mat grayImg;
    cvtColor(src,grayImg,COLOR_BGR2GRAY);
    imshow("gray",grayImg);

    // 得到灰度图中最大最小值
    int maxGrayLevel = -1, minGrayLevel = 256;
    for(int row = 0; row < grayImg.rows; ++row){
        for(int col = 0; col < grayImg.cols; ++ col){
            maxGrayLevel = max(maxGrayLevel, (int)grayImg.at<uchar>(row, col));
            minGrayLevel = min(minGrayLevel, (int)grayImg.at<uchar>(row, col));
        }
    }
    
    // 计算相对照度
    const double P = minGrayLevel / maxGrayLevel * 100;
    printWithUnit(P, %);
    print(maxGrayLevel);
    print(minGrayLevel);
}

int main(int argc, char** argv)
{
    //显示原图
    Mat src = imread("/Users/wanncye/Downloads/MechMind/验收软件支持/同轴畸变，物方视场/02.bmp");
    if(src.empty()){
        return 0 ;
    }
    imshow("src",src);

    // 同轴物方视场计算
    CoaxialObjectFieldOfView(src);

    cout << endl;

    // 相对照度计算
    relativeIllumination(src);
    
    waitKey(0);
	return 0;
}