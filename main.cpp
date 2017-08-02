#include "stdafx.h"

#include "opencv2/opencv.hpp"  
using namespace cv;
#include <iostream>  
using namespace std;
//运动物体检测函数声明  
Mat MoveDetect(Mat temp, Mat frame);
Point getCenterPoint(Rect rect);
int GetAvg(vector<Point> point);

vector<vector<Point>>center_point;

int main(int argc,char* argv[])
{
	int frame_count = 0;
	center_point.resize(10);
	VideoCapture video(argv[1]);//定义VideoCapture类video  
	if (!video.isOpened())  //对video进行异常检测  
	{
		cout << "video open error!" << endl;
		return 0;
	}
	int frameCount = video.get(CV_CAP_PROP_FRAME_COUNT);//获取帧数  
	double FPS = video.get(CV_CAP_PROP_FPS);//获取FPS  
	Mat frame;//存储帧  
	Mat temp;//存储前一帧图像  
	Mat result;//存储结果图像  
	for (int i = 0; i < frameCount; i++)
	{
		double t = (double)cv::getTickCount();
		video >> frame;//读帧进frame  
		//imshow("frame", frame);
		if (frame.empty())//对帧进行异常检测  
		{
			cout << "frame is empty!" << endl;
			break;
		}
		pyrDown(frame, frame, Size(frame.cols / 2, frame.rows / 2));
		if (i == 0)//如果为第一帧（temp还为空）  
		{
			result = MoveDetect(frame, frame);//调用MoveDetect()进行运动物体检测，返回值存入result  
		}
		else//若不是第一帧（temp有值了）  
		{
			result = MoveDetect(temp, frame);//调用MoveDetect()进行运动物体检测，返回值存入result  
			frame_count++;
		}
	/*	for (int i = 0; i < center_point.size(); i++)
		{
			if (center_point[i].size() > 1){
				int avg_x = GetAvg(center_point[i]);
				//vector<Point>::iterator iter = center_point[i].begin();
				for (vector<Point>::iterator iter = center_point[i].begin(); iter!=center_point[i].end(); ++iter)
				{
					if (abs(avg_x - (*iter).x) > 30){
						--(iter = center_point[i].erase(iter));
					}

				}
			}
		}*/
		for (int i = 0; i < center_point.size(); i++)
		{
			if (center_point[i].size() > 1){
				int x_avg = GetAvg(center_point[i]);
				for (int j = 0; j < center_point[i].size() - 1; j++){
					if (abs(x_avg - center_point[i][j].x) < 100){
						circle(result, center_point[i][j], 5, Scalar(255, 255, 0), -1);
					}
				}
			}
		}
		char string[10];
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
		double fps = 1.0 / t;
		sprintf(string, "%.2f", fps);      // 帧率保留两位小数
		std::string fpsString("FPS:");
		fpsString += string;                    // 在"FPS:"后加入帧率数值字符串
		// 将帧率信息写在输出帧上
		putText(result,                  // 图像矩阵
			fpsString,                  // string型文字内容
			cv::Point(5, 20),           // 文字坐标，以左下角为原点
			cv::FONT_HERSHEY_SIMPLEX,   // 字体类型
			1,                    // 字体大小
			cv::Scalar(0, 0, 255),2);
		imshow("result", result);
		if (frame_count % 200==0){
			for (int  i = 0; i <center_point.size(); i++)
			{
				center_point[i].clear();
			}
		}
		if (waitKey(10) == 27)//按原FPS显示  
		{
			cout << "ESC退出!" << endl;
			break;
		}
		temp = frame.clone();

	}
	return 0;


}
Mat MoveDetect(Mat temp, Mat frame)
{
	Mat result = frame.clone();
	//1.将background和frame转为灰度图  
	Mat gray1, gray2;
	cvtColor(temp, gray1, CV_BGR2GRAY);
	cvtColor(frame, gray2, CV_BGR2GRAY);
	//2.将background和frame做差  
	Mat diff;
	absdiff(gray1, gray2, diff);
//	imshow("diff", diff);
	//3.对差值图diff_thresh进行阈值化处理  
	Mat diff_thresh;
	threshold(diff, diff_thresh, 50, 255, CV_THRESH_BINARY);
//	imshow("diff_thresh", diff_thresh);
	//4.腐蚀  
	Mat kernel_erode = getStructuringElement(MORPH_RECT, Size(2, 2));
	Mat kernel_dilate = getStructuringElement(MORPH_RECT, Size(18, 18));
	erode(diff_thresh, diff_thresh, kernel_erode);
	imshow("erode", diff_thresh);
	//5.膨胀  
	dilate(diff_thresh, diff_thresh, kernel_dilate);
	imshow("dilate", diff_thresh);
	//6.查找轮廓并绘制轮廓  
	vector<vector<Point>> contours;
	findContours(diff_thresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	drawContours(result, contours, -1, Scalar(0, 0, 255), 2);//在result上绘制轮廓  


	//7.查找正外接矩形  
	vector<Rect> boundRect(contours.size());
	cout << "---------------------------------------------------" << endl;
	for (int i = 0; i < contours.size(); i++)
	{
		if (i > 9) break;
		boundRect[i] = boundingRect(contours[i]);
		Point center = getCenterPoint(boundRect[i]);
		cout << center.x << " "<< center.y << endl;
		center_point[i].push_back(getCenterPoint(boundRect[i]));
		rectangle(result, boundRect[i], Scalar(0, 255, 0), 2);//在result上绘制正外接矩形  
	}
	cout << "---------------------------------------------------" << endl;
	return result;//返回result  
}

Point getCenterPoint(Rect rect)
{
	Point cpt;
	cpt.x = rect.x + cvRound(rect.width / 2.0);
	cpt.y = rect.y + cvRound(rect.height / 2.0);
	return cpt;
}

int GetAvg(vector<Point> point){
	int sum = 0;
	for (int i = 0; i < point.size(); i++)
	{
		sum += point[i].x;
	}
	return sum / point.size();
}
