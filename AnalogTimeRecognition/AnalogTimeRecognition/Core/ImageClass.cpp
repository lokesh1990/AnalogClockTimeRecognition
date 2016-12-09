
#include <iostream>
#include <cmath>
#include "ImageClass.h"
#include "HoughTransform.h"
#include <opencv2\imgcodecs.hpp>
#include <opencv2\imgproc.hpp>

using namespace Core;

ImageClass::ImageClass()
{
}

ImageClass::~ImageClass()
{
}

void ImageClass::setFileName(std::string filename)
{
	sourceFileName = filename;
	readImage();
}

ImageClass& ImageClass::GetInstance()
{
	static ImageClass inst;

	return inst;
}

void ImageClass::readImage()
{
	// read the image file
	
	sourceImg = cv::imread(sourceFileName,0);
	//cv::cvtColor(sourceImg, sourceImg, CV_BGR2GRAY);
	std::cout << "row:"<< sourceImg.rows << std::endl;
	std::cout << "col:"<<sourceImg.cols << std::endl;
	if (!sourceImg.data)                              // Check for invalid input
	{
		std::cout << "file not found" << std::endl;
	}
	else
	{

		cv::imwrite(sourceFileName.substr(0, sourceFileName.size() - 4) + "_grayscale.jpg", sourceImg);
	}
}

void ImageClass::extractClock()
{
	// resize image if it is large
	int height = sourceImg.size().height;
	int width = sourceImg.size().width;
	//cv::Mat processdImg;

	if (height > 800 || width > 1000) 
	{
		cv::resize(sourceImg, sourceImg, cv::Size(1000, 800));
		//duplicate with the one above
		//cv::cvtColor(sourceImg, sourceImg, CV_BGR2GRAY);

		// resize if needed using cv::resize
		//cv::imwrite("resizedImage.jpg", sourceImg);
	}

	/// Reduce the noise so we avoid false circle detection

	// perform circle detection

	//processdImg.copySize(sourceImg);
	//cv::GaussianBlur(processdImg, sourceImg, sourceImg.size(), 2, 2);

	//the third parameter of GaussianBlur is the window size of a Gaussian filter
	//both height and width must be positive and odd.
	cv::GaussianBlur(this->sourceImg, this->sourceImg, cv::Size(9, 9), 2, 2);
	
	std::vector<cv::Vec3f> circles;
	
	/// Apply the Hough Transform to find the circles

	cv::HoughCircles(this->sourceImg, circles, CV_HOUGH_GRADIENT, 1, sourceImg.rows / 8,
		200, 100, //0, 0);
		cvRound(MIN(sourceImg.size().height, sourceImg.size().width)*0.1), 
				cvRound(MAX(sourceImg.size().height, sourceImg.size().width)*0.5));
	

	//index can't be -1
	int largestCircle = 0;
	cv::Mat imgCircles(sourceImg.rows, sourceImg.cols, CV_8UC3, cv::Scalar(0, 0, 0));

	//sourceImg.copyTo(imgCircles);

	//x and y of point are int so to make it more accurate with point2d
	cv::Point2d clockCenter;
	if (circles.size() > 0)
	{
		clockCenter.x = cvRound(circles[0][0]);
		clockCenter.y = cvRound(circles[0][1]);
	}
	// Draw the circles detected
	for (size_t i = 0; i < circles.size(); i++)
	{
		cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
	
		// find the largest circle
		// TODO remove this? probably select all circles
		if (circles[largestCircle][2] < radius)
		{
			largestCircle = static_cast<int>(i);
			clockCenter.x = center.x;
			clockCenter.y = center.y;
		}
		
		cv::circle(imgCircles, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);// circle center     

		cv::circle(imgCircles, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0);// circle outline
		std::cout << "center : " << center << "\nradius : " << radius << std::endl;
	}

	// Show your results
	cv::imwrite(sourceFileName.substr(0, sourceFileName.size() - 4) +"_allCirclesImage.jpg", imgCircles);

	if (largestCircle == -1)
		return;	//	no circle found

	cv::Mat clockImg;
	cv::Mat mask = cv::Mat::zeros(sourceImg.rows, sourceImg.cols, CV_8UC1);
	
	//cvRound on (circles[largestCircle][2]) because of the conversion warning.
	cv::circle(mask, cv::Point(cvRound(circles[largestCircle][0]), cvRound(circles[largestCircle][1])),
		cvRound(circles[largestCircle][2]), cv::Scalar(255, 255, 255), -1, 8, 0); //-1 means filled
	sourceImg.copyTo(clockImg, mask); // copy values of img to dst if mask is > 0.

	cv::imwrite(sourceFileName.substr(0, sourceFileName.size() - 4) +"_blurImage.jpg", clockImg);
	cv::imwrite(sourceFileName.substr(0, sourceFileName.size() - 4) +"_maskImage.jpg", mask);

	// perfrom canny edge detection
	// for finding the lines in the circle

	//comment the error because your clockimage should have 3 or 4 channels before applying the function cv2.cvtColor
	//actually you don't need to convert because it is already grayscale
	//cv::cvtColor(clockImg, sourceImg, CV_BGR2GRAY);
	//***check here
	clockImg.copyTo(sourceImg);

	cv::blur(sourceImg, sourceImg, cv::Size(5,5));

	//sourceImg = clockImg.clone();
	cv::Canny(sourceImg, sourceImg, 50, 30);
	cv::imwrite(sourceFileName.substr(0, sourceFileName.size() - 4) +"_cannyImage.jpg", sourceImg);
	
	// do houghline transform

	// get the lines from the houghtransform class
	// TODO call the HoughTransform ComputeHough method to find the lines
	// detected inside the circle
	//add call houghtransform
	Core::HoughTransform &ht = Core::HoughTransform::GetInstance();
	//---change Vec4i to Vec4d--- to use 0.001 on line 155,157
	std::vector<cv::Vec4d> handLines = ht.ComputeHough(sourceImg, clockCenter, sourceFileName);
	//end houghtransform

	
	//add the initial size of lineAngles = handLines.size()
	std::vector<double> lineAngles(handLines.size());
	double diffX, diffY;

	for (int i = 0; i < handLines.size(); i++)
	{
		//find the longest line segment from the center and discard the other one.
		//x,y in point is int so use point2d instead
		if (euclideanDist(cv::Point2d(handLines[i][0], handLines[i][1]), clockCenter) < euclideanDist(cv::Point2d(handLines[i][2], handLines[i][3]), clockCenter))
		{
			handLines[i][0] = handLines[i][2];
			handLines[i][1] = handLines[i][3];
		}
		//handLines[i][0] -= clockCenter.x;
		//handLines[i][1] -= clockCenter.y;
		handLines[i][2] = clockCenter.x;
		handLines[i][3] = clockCenter.y;

		if(handLines[i][0] == clockCenter.x) 
			handLines[i][0] += 0.001f;
		if (handLines[i][1] == clockCenter.y)
			handLines[i][1] += 0.001f;

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
	int degOffset = 10;
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
	std::vector<double> selectedAngles;
	
	//find 3 selectedLines
	for (int i = 0; i < lineAngles.size()-1; i++)
	{

		// need to check here. I am discarding lines which has angle offset 
		// less than the threshold value

		//the problem is when you got i+1 360 and i is 5.
		//index of the upper bound
		
		int value=0;
		int in;
		for (int j = i+1; j < lineAngles.size(); j++)
		{
			if (lineAngles[j] - lineAngles[i] > degOffset)
			{
				value = j-1;
				break;
			}
			else if (j==lineAngles.size()-1)
			{
				value = j;
				break;
			}
		}
		in = value;
		if(value==i)
		{
			selectedLines.push_back(handLines[value]);
			selectedAngles.push_back(lineAngles[value]);
		}
		else if(value!=0)
		{
			value = i+static_cast<int>(round((value - i)/2));
			selectedLines.push_back(handLines[value]);
			selectedAngles.push_back(lineAngles[value]);
		}
		i = in;
		
		/*
		if (lineAngles[i + 1] - lineAngles[i] > degOffset)
		{
			selectedLines.push_back(handLines[i]);
			selectedAngles.push_back(lineAngles[i]);
			
		}
		*/
		
	}
	
	if (lineAngles.size() < 2)
	{
		std::cout << "error with line Angles" << std::endl;
	}
	else if(lineAngles[lineAngles.size() - 1] - lineAngles[lineAngles.size() - 2] > degOffset)
	{
		selectedLines.push_back(handLines[lineAngles.size() - 1]);
		selectedAngles.push_back(lineAngles[lineAngles.size() - 1]);
	}

	// sort lines based on length
	for (int i = 0; i < selectedLines.size(); i++)
	{
		// do sorting here
		int j = i;
		//initial value of x,y on point is int. need point2d
		while (j > 0 && euclideanDist(cv::Point2d(handLines[j - 1][0], handLines[j - 1][1]), cv::Point2d(handLines[j - 1][2], handLines[j - 1][3]))
		> euclideanDist(cv::Point2d(handLines[j][0], handLines[j][1]), cv::Point2d(handLines[j][2], handLines[j][3])))
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
	//if more than 3 lines
	while (selectedLines.size() > 3)
	{
		selectedLines.pop_back();
	}

	cv::Mat handLinesImg(sourceImg.rows, sourceImg.cols, CV_8UC3, cv::Scalar(0, 0, 0));
	for (size_t i = 0; i < selectedLines.size(); i++)
	{
		cv::Point pt1(selectedLines[i][0], selectedLines[i][1]), pt2(selectedLines[i][2], selectedLines[i][3]);
		cv::line(handLinesImg, pt1, pt2, cv::Scalar(i == 0 ? 255 : 0, i == 1 ? 255 : 0, i > 1 ? 255 : 0), 3, CV_AA);
		//cv::line(sourceImg, pt1, pt2, cv::Scalar(i == 0 ? 255 : 0, i == 1 ? 255 : 0, i > 1 ? 255 : 0), 3, CV_AA);
	}
	cv::imwrite(sourceFileName.substr(0, sourceFileName.size() - 4)+"_handLines.jpg", handLinesImg);

	//cv::imwrite("HandLines.jpg", sourceImg);

	//longest one is second, middle one is min, shortest one is hour
	// if one line, assume second, hour and minute are same
	// if 2 lines, then 2 of them are same
	// if 3 lines easy case
	if (selectedLines.size() == 1)
	{
		int h = getHour(selectedAngles[0]);
		int m = getMinSec(selectedAngles[0]);	
	}
	else if (selectedLines.size() == 2)
	{
		cv::Point pt1(selectedLines[0][0], selectedLines[0][1]), pt2(selectedLines[0][2], selectedLines[0][3]);
		cv::Point pt3(selectedLines[1][0], selectedLines[1][1]), pt4(selectedLines[1][2], selectedLines[1][3]);
		double length1 = euclideanDist(pt1, pt2);
		double length2 = euclideanDist(pt3, pt4);
		/*
		std::cout << "blue length :" << length1 << std::endl;
		std::cout << "green length:" << length2 << std::endl;

		std::cout << "blue degree  :" << selectedAngles[0] << std::endl;
		std::cout << "green degree :" << selectedAngles[1] << std::endl;
		*/
		if (length1 > length2)
		{
			//0 is minutes,1 is hours
			this->hour = getHour(selectedAngles[1]);
			this->minute = getMinSec(selectedAngles[0]);
		}
		else if (length1 < length2)
		{
			//1 is minutes,0 is hours
			this->hour = getHour(selectedAngles[0]);
			this->minute = getMinSec(selectedAngles[1]);
		}
		else
		{
			//i don't know which one is hour or minutes
			//guess 0 is minutes,1 is hours
			this->hour = getHour(selectedAngles[1]);
			this->minute = getMinSec(selectedAngles[0]);
		}
	}
	else if (selectedLines.size() >= 3)
	{
		cv::Point pt1(selectedLines[0][0], selectedLines[0][1]), pt2(selectedLines[0][2], selectedLines[0][3]);
		cv::Point pt3(selectedLines[1][0], selectedLines[1][1]), pt4(selectedLines[1][2], selectedLines[1][3]);
		cv::Point pt5(selectedLines[2][0], selectedLines[2][1]), pt6(selectedLines[2][2], selectedLines[2][3]);
		double length_b = euclideanDist(pt1, pt2);
		double length_g = euclideanDist(pt3, pt4);
		double length_r = euclideanDist(pt5, pt6);
		
		//b:g:r
		if (length_b < length_g&&length_g < length_r)
		{
			this->hour = getHour(selectedAngles[0]);
			this->minute = getMinSec(selectedAngles[1]);
			this->second = getMinSec(selectedAngles[2]);
		}
		//r:g:b
		else if (length_r < length_g&&length_g < length_b)
		{
			this->hour = getHour(selectedAngles[2]);
			this->minute = getMinSec(selectedAngles[1]);
			this->second = getMinSec(selectedAngles[0]);
		}
		//g:b:r
		else if (length_g < length_b && length_b < length_r)
		{
			this->hour = getHour(selectedAngles[1]);
			this->minute = getMinSec(selectedAngles[0]);
			this->second = getMinSec(selectedAngles[2]);
		}
		//r:b:g
		else if (length_r < length_b && length_b < length_g )
		{
			this->hour = getHour(selectedAngles[2]);
			this->minute = getMinSec(selectedAngles[0]);
			this->second = getMinSec(selectedAngles[1]);
		}
		//b:r:g
		else if (length_b < length_r && length_r < length_g)
		{
			this->hour = getHour(selectedAngles[0]);
			this->minute = getMinSec(selectedAngles[2]);
			this->second = getMinSec(selectedAngles[1]);
		}
		//g:r:b
		else if (length_g < length_r && length_r < length_b)
		{
			this->hour = getHour(selectedAngles[1]);
			this->minute = getMinSec(selectedAngles[2]);
			this->second = getMinSec(selectedAngles[0]);
		}
		
	}
	printTime();
}

double ImageClass::euclideanDist(cv::Point2d p, cv::Point2d q)
{
	// computes the euclidean distance between 2 points
	cv::Point2d diff = p - q;
	double val = cv::sqrt(diff.x*diff.x + diff.y*diff.y);
	return val;
}

int ImageClass::getHour(double selectedAngle)
{
	int h1 = (static_cast<int>(selectedAngle + 180)) % 360;
	int h = 12 - static_cast<int>(ceil(static_cast<double>(h1) / 30));
	return h;
}

int ImageClass::getMinSec(double selectedAngle)
{
	int m1 = (static_cast<int>(selectedAngle + 180)) % 360;
	int m = (60 - static_cast<int>(ceil(static_cast<double>(m1) / 6)))%60;
	return m;
}

void ImageClass::printTime()
{
	std::cout <<"The image's time is (h:m:s) "<< this->hour << ":" << this->minute << ":" << this->second <<""<< std::endl;
}