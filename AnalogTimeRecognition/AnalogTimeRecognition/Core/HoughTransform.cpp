#include "../stdafx.h"
#include "HoughTransform.h"
#include <opencv2/highgui.hpp>
#include <iostream>
using namespace Core;

HoughTransform::HoughTransform()
{
}


HoughTransform::~HoughTransform()
{
}

HoughTransform& HoughTransform::GetInstance()
{
	static HoughTransform m_inst;

	return m_inst;
}
//change the return value from vector<Vec4i> to vector<Vec4f> to use 0.001 on imageclass
std::vector<cv::Vec4d> Core::HoughTransform::ComputeHough(cv::Mat ipImg, cv::Point center)
{
	// find the hand lines 

	//if (ipImg.channels() == 3)
	//	cv::cvtColor(ipImg, ipImg, CV_BGR2GRAY);

	std::vector<cv::Vec4d> lines;
	// detect lines
	//cv::HoughLines(ipImg, lines, 1, CV_PI / 180, 100); // 150);
	int minLineLength = 30;
	int maxLineGap = 10;

	//there is the problem in some cases with clocks because the line size = 0.
	cv::HoughLinesP(ipImg, lines, 1, CV_PI / 180, 80, minLineLength, maxLineGap); // 100, 100, 10

	//create new mat with the same size as ipImg and black background
	cv::Mat allLineImg(ipImg.rows, ipImg.cols, CV_8UC3, cv::Scalar(0, 0, 0));

	//unused
	//int xc, yc;
	std::cout <<"s:"<< lines.size() << std::endl;
	for (size_t i = 0; i < lines.size(); i++)
	{
		//float rho = lines[i][0], theta = lines[i][1];
		//change to point2d
		cv::Point2d pt1(lines[i][0], lines[i][1]), pt2(lines[i][2], lines[i][3]);
		//double a = cos(theta), b = sin(theta);
		//double x0 = a*rho, y0 = b*rho;
		//pt1.x = cvRound(x0 + 1000 * (-b));
		//pt1.y = cvRound(y0 + 1000 * (a));
		//pt2.x = cvRound(x0 - 1000 * (-b));
		//pt2.y = cvRound(y0 - 1000 * (a));
		//white foreground
		cv::line(allLineImg, pt1, pt2, cv::Scalar(255, 255, 255), 1, CV_AA);
	}

	cv::imwrite("allLines.jpg", allLineImg);
	std::vector<cv::Vec4d> handLines;

	for (size_t i = 0; i < lines.size(); i++)
	{
		if (CheckCenterOnLine(lines[i], center))
			handLines.push_back(lines[i]);
	}

	return lines;
}

bool Core::HoughTransform::CheckCenterOnLine(cv::Vec4i line, cv::Point center)
{
	double slope;
	slope = (1.0 * (line[1] - line[3])) / (line[0] - line[2]);

	if (slope == 0 || isnan(slope))
		return false;
	if (abs(line[1] - center.y) - (slope * (line[0] - center.x)) < 1)
		return true;

	return false;
}
