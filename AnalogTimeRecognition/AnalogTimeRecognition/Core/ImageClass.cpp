
#include <iostream>
#include "ImageClass.h"
#include "HoughTransform.h"
#include <opencv2\imgcodecs.hpp>
#include <opencv2\imgproc.hpp>

using namespace Core;

using namespace std;
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
	// read the image file

	sourceImg = cv::imread(sourceFileName,0);
	//cv::cvtColor(sourceImg, sourceImg, CV_BGR2GRAY);
	std::cout << "row:"<< sourceImg.rows << std::endl;
	std::cout << "col:"<<sourceImg.cols << std::endl;

	cv::imwrite("grayscale.jpg", sourceImg);
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
		cv::cvtColor(sourceImg, sourceImg, CV_BGR2GRAY);

		// resize if needed using cv::resize
		//cv::imwrite("resizedImage.jpg", sourceImg);
	}

	/// Reduce the noise so we avoid false circle detection

	// perform circle detection

	//processdImg.copySize(sourceImg);
	//cv::GaussianBlur(processdImg, sourceImg, sourceImg.size(), 2, 2);

	//the third parameter of GaussianBlur is the window size of a Gaussian filter
	//both height and width must be positive and odd.
	cv::GaussianBlur(sourceImg, sourceImg, cv::Size(9, 9), 2, 2);
	
	std::vector<cv::Vec3f> circles;
	
	/// Apply the Hough Transform to find the circles
	cv::HoughCircles(sourceImg, circles, CV_HOUGH_GRADIENT, 1, sourceImg.rows / 8,
		200, 100, 0, 0);
		//cvRound(MIN(sourceImg.size().height, sourceImg.size().width)*0.1), 
			//	cvRound(MAX(sourceImg.size().height, sourceImg.size().width)*0.5));
	

	//index can't be -1
	int largestCircle = 0;
	cv::Mat imgCircles;
	sourceImg.copyTo(imgCircles);
	//x and y of point are int so to make it more accurate with point2d
	cv::Point2d clockCenter;

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
	
	//cvRound on (circles[largestCircle][2]) because of the conversion warning.
	cv::circle(mask, cv::Point(cvRound(circles[largestCircle][0]), cvRound(circles[largestCircle][1])),
		cvRound(circles[largestCircle][2]), cv::Scalar(255, 255, 255), -1, 8, 0); //-1 means filled
	sourceImg.copyTo(clockImg, mask); // copy values of img to dst if mask is > 0.

	cv::imwrite("clockImage.jpg", clockImg);

	// perfrom canny edge detection
	// for finding the lines in the circle

	//comment the error because your clockimage should have 3 or 4 channels before applying the function cv2.cvtColor
	//actually you don't need to convert because it is already grayscale
	//cv::cvtColor(clockImg, sourceImg, CV_BGR2GRAY);

	//sourceImg = clockImg.clone();
	cv::Canny(sourceImg, sourceImg, 50, 30);
	cv::imwrite("cannyImage.jpg", sourceImg);

	// do houghline transform

	// get the lines from the houghtransform class
	// TODO call the HoughTransform ComputeHough method to find the lines
	// detected inside the circle
	//add call houghtransform
	Core::HoughTransform &ht = Core::HoughTransform::GetInstance();
	//---change Vec4i to Vec4d--- to use 0.001 on line 155,157
	std::vector<cv::Vec4d> handLines = ht.ComputeHough(sourceImg, clockCenter);
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
	std::vector<double> selectedAngles;

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

	for (size_t i = 0; i < selectedLines.size(); i++)
	{
		cv::Point pt1(selectedLines[i][0], selectedLines[i][1]), pt2(selectedLines[i][2], selectedLines[i][3]);
		cv::line(sourceImg, pt1, pt2, cv::Scalar(i == 0 ? 255 : 0, i == 1 ? 255 : 0, i > 1 ? 255 : 0), 3, CV_AA);
	}

	cv::imwrite("HandLines.jpg", sourceImg);

	// if one line, assume second, hour and minute are same
	// if 2 lines, then 2 of them are same
	// if 3 lines easy case
	if (selectedLines.size() > 2)
	{

	}

}

double ImageClass::euclideanDist(cv::Point2d p, cv::Point2d q)
{
	// computes the euclidean distance between 2 points
	cv::Point2d diff = p - q;
	return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
}

