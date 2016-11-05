#include "../stdafx.h"
#include "HoughTransform.h"
#include <opencv2/highgui.hpp>

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

void Core::HoughTransform::ComputeHough(cv::Mat ipImg)
{
	//if (ipImg.channels() == 3)
	//	cv::cvtColor(ipImg, ipImg, CV_BGR2GRAY);

	std::vector<cv::Vec2f> lines;
	// detect lines
	//cv::HoughLines(ipImg, lines, 1, CV_PI / 180, 100); // 150);
	int minLineLength = 30;
	int maxLineGap = 10;
	cv::HoughLinesP(ipImg, lines, 1, CV_PI / 180, 80, minLineLength, maxLineGap); // 100, 100, 10

	int xc, yc;

	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		cv::Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		cv::line(ipImg, pt1, pt2, cv::Scalar(0, 0, 255), 3, CV_AA);
	}

	cv::imshow("detected lines", ipImg);
}
