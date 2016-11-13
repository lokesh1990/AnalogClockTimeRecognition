
#include <iostream>
#include "ImageClass.h"
#include <opencv2\imgcodecs.hpp>
#include <opencv2\imgproc.hpp>

using namespace Core;

ImageClass::ImageClass()
{
	sourceFileName = "clock.jpg";
	readImage();
}

ImageClass::~ImageClass()
{
}

ImageClass& ImageClass::GetInstance()
{
	static ImageClass inst;

	return inst;
}

void ImageClass::readImage()
{
	sourceImg = cv::imread(sourceFileName);
	cv::cvtColor(sourceImg, sourceImg, CV_BGR2GRAY);
	
	cv::imwrite("grayscale.jpg", sourceImg);
}

void ImageClass::extractClock()
{
	// resize image if it is large
	int height = sourceImg.size().height;
	int width = sourceImg.size().width;
	cv::Mat processdImg;

	if (height > 800 || width > 1000) 
	{
		cv::resize(sourceImg, sourceImg, cv::Size(1000, 800));
		cv::cvtColor(sourceImg, sourceImg, CV_BGR2GRAY);

		// resize if needed using cv::resize
		//cv::imwrite("resizedImage.jpg", sourceImg);
	}
	std::cout << "1";
	/// Reduce the noise so we avoid false circle detection
	//the third parameter of GaussianBlur is the window size of a Gaussian filter
	//both height and width must be positive and odd.
	cv::GaussianBlur(sourceImg, sourceImg, cv::Size(9, 9), 2, 2);
	std::cout << "2";
	
	std::vector<cv::Vec3f> circles;
	
	/// Apply the Hough Transform to find the circles
	cv::HoughCircles(sourceImg, circles, CV_HOUGH_GRADIENT, 1, sourceImg.rows / 8, 
				200, 100, cvRound(MIN(sourceImg.size().height, sourceImg.size().width)*0.1), 
				cvRound(MAX(sourceImg.size().height, sourceImg.size().width)*0.5));
	


	int largestCircle = -1;
	cv::Mat imgCircles;
	sourceImg.copyTo(imgCircles);
	// Draw the circles detected
	for (size_t i = 0; i < circles.size(); i++)
	{
		std::cout <<"6";
		cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);

		std::cout << circles[largestCircle].cols;
		std::cout << circles[largestCircle].rows;
		if (circles[largestCircle][2] < radius)
			largestCircle = i;

		std::cout << "5";
		cv::circle(imgCircles, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);// circle center     

		std::cout << "6";
		cv::circle(imgCircles, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);// circle outline
		std::cout << "center : " << center << "\nradius : " << radius << std::endl;
	}
	// Show your results
	cv::imwrite("allCirclesImage.jpg", imgCircles);

	if (largestCircle == -1)
		return;	//	no circle found

	cv::Mat clockImg;
	cv::Mat mask = cv::Mat::zeros(sourceImg.rows, sourceImg.cols, CV_8UC1);
	cv::circle(mask, cv::Point(cvRound(circles[largestCircle][0]), cvRound(circles[largestCircle][1])),
							circles[largestCircle][2], cv::Scalar(255, 255, 255), -1, 8, 0); //-1 means filled
	sourceImg.copyTo(clockImg, mask); // copy values of img to dst if mask is > 0.

	cv::imwrite("clockImage.jpg", clockImg);

	cv::cvtColor(clockImg, sourceImg, CV_BGR2GRAY);

	//sourceImg = clockImg.clone();
	cv::Canny(sourceImg, sourceImg, 50, 150);
	cv::imwrite("cannyImage.jpg", sourceImg);
	std::cout << "7";
	// do houghline transform

	std::vector<cv::Vec4i> handLines;

	for (int i = 0; i < handLines.size(); i++)
	{

	}
}

