
#include <iostream>
#include "ImageClass.h"
#include <opencv2\imgcodecs.hpp>
#include <opencv2\imgproc.hpp>

using namespace Core;

ImageClass::ImageClass()
{
	sourceFileName = "";
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
		cv::resize(sourceImg, processdImg, cv::Size(1000, 800));

		// resize if needed using cv::resize
		//cv::imwrite("resizedImage.jpg", sourceImg);
	}

	/// Reduce the noise so we avoid false circle detection
	processdImg.copySize(sourceImg);
	cv::GaussianBlur(processdImg, sourceImg, sourceImg.size(), 2, 2);

	std::vector<cv::Vec3f> circles;

	/// Apply the Hough Transform to find the circles
	cv::HoughCircles(sourceImg, circles, CV_HOUGH_GRADIENT, 1, sourceImg.rows / 8, 
				200, 100, cvRound(MIN(sourceImg.size().height, sourceImg.size().width)*0.1), 
				cvRound(MAX(sourceImg.size().height, sourceImg.size().width)*0.5));

	int largestCircle = -1;
	cv::Mat imgCircles;
	sourceImg.copyTo(imgCircles);
	cv::Point clockCenter;

	// Draw the circles detected
	for (size_t i = 0; i < circles.size(); i++)
	{
		cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		if (circles[largestCircle][2] < radius)
		{
			largestCircle = i;
			clockCenter = center;
		}
		
		cv::circle(imgCircles, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);// circle center     
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

	// do houghline transform

	std::vector<cv::Vec4i> handLines;
	float lineAngles[handLines.size];
	float diffX, diffY;

	for (int i = 0; i < handLines.size; i++)
	{
		//find the longest line segment from the center and discard the other one.
		if (euclideanDist(cv::Point(handLines[i][0], handLines[i][1]), clockCenter) < euclideanDist(cv::Point(handLines[i][2], handLines[i][3]), clockCenter))
		{
			handLines[i][0] = handLines[i][2];
			handLines[i][1] = handLines[i][3];
		}

		//handLines[i][0] -= clockCenter.x;
		//handLines[i][1] -= clockCenter.y;
		handLines[i][2] = clockCenter.x;
		handLines[i][3] = clockCenter.y;

		if(handLines[i][0] == clockCenter.x) 
			handLines[i][0] += 0.001;
		if (handLines[i][1] == clockCenter.y)
			handLines[i][1] += 0.001;

		// find angle and convert into radians
		lineAngles[i] = atan2(handLines[i][1] - handLines[i][3], handLines[i][0] - handLines[i][2]);
		lineAngles[i] = lineAngles[i] * 180 / CV_PI;

		diffX = handLines[i][0] - clockCenter.x;
		diffY = handLines[i][1] - clockCenter.y;

		// consider 4 quadrents now
		if (diffX >= 0 && diffY >= 0)
		{
			lineAngles[i] = 90 - lineAngles[i];
		}
		else if (diffX < 0 && diffY >= 0)
		{
			lineAngles[i] = 450 - lineAngles[i]; // check this again
		}
		else if (diffX >= 0 && diffY < 0)
		{
			lineAngles[i] = abs(lineAngles[i]) + 90;
		}
		else
		{
			lineAngles[i] = abs(lineAngles[i]) + 90;
		}
	}

	
	// all lines with a degree offset less than 5 should be merged.
	int degOffset = 5;
	cv::Vec4i tmpVec;

	for (int i = 0; i < handLines.size(); i++)
	{
		// do sorting here
		int j = i;
		while (j > 0 && lineAngles[j - 1] > lineAngles[j])
		{
			// swap
			tmpVec = handLines[j];
			handLines[j] = handLines[j-1];
			handLines[j-1] = tmpVec;
			diffX = lineAngles[j];
			lineAngles[j] = lineAngles[j-1];
			lineAngles[j - 1] = diffX;
			
			j--;
		}
	}

	std::vector<cv::Vec4i> selectedLines;
	std::vector<float> selectedAngles;

	for (int i = 0; i < lineAngles.size()-1; i++)
	{
		// need to check here. I am discarding lines which has angle offset 
		// less than the threshold value
		if (lineAngles[i + 1] - lineAngles[i] > degOffset)
		{
			selectedLines.push_back(handLines[i]);
			selectedAngles.push_back(lineAngles[i]);
		}
	}
	if(lineAngles[lineAngles.size() - 1] - lineAngles[lineAngles.size() - 2] > degOffset)
	{
		selectedLines.push_back(handLines[lineAngles.size() - 1]);
		selectedAngles.push_back(lineAngles[lineAngles.size() - 1]);
	}

	// sort lines based on length
	for (int i = 0; i < selectedLines.size(); i++)
	{
		// do sorting here
		int j = i;
		
		while (j > 0 && euclideanDist(cv::Point(handLines[j - 1][0], handLines[j - 1][1]), cv::Point(handLines[j - 1][2], handLines[j - 1][3]))
		> euclideanDist(cv::Point(handLines[j][0], handLines[j][1]), cv::Point(handLines[j][2], handLines[j][3])))
		{
			// swap
			tmpVec = selectedLines[j];
			selectedLines[j] = selectedLines[j - 1];
			selectedLines[j - 1] = tmpVec;
			diffX = selectedAngles[j];
			selectedAngles[j] = selectedAngles[j - 1];
			selectedAngles[j - 1] = diffX;

			j--;
		}
	}

	for (size_t i = 0; i < selectedLines.size(); i++)
	{
		cv::Point pt1(selectedLines[i][0], selectedLines[i][1]), pt2(selectedLines[i][2], selectedLines[i][3]);
		cv::line(sourceImg, pt1, pt2, cv::Scalar(i == 0 ? 255 : 0, i == 1 ? 255 : 0, i > 1 ? 255 : 0), 3, CV_AA);
	}

	cv::imwrite("HandLines.jpg", sourceImg);

	// if one line, assume second, hour and minute are same
	// if 2 lines, then 2 of them are same
	// if 3 lines easy case
}

float ImageClass::euclideanDist(cv::Point p, cv::Point q)
{
	cv::Point diff = p - q;
	return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
}

