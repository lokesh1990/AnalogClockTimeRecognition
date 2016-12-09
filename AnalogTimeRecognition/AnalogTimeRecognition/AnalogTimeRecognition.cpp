//Hough Transformation for a single frame
#include "stdafx.h"
#include <cstdio>
#include <iostream>

#include <opencv2\imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


#include "Core/HoughTransform.h"
#include "Core/ImageClass.h"

int main(int argc, char** argv)
{
	/**
	Guys this is the main class.
	Do not write the function code here.
	Create a new class in the core or helper class and write the functions there
	We will merge then later.
	*/

	cv::Mat src, src_gray;
	Core::ImageClass &im = Core::ImageClass::GetInstance();
	std::string line;
	std::cout << "Input file: ";
	std::cin >> line;
	im.setFileName(line);
	im.extractClock();
	getchar();
	getchar();
	cv::waitKey(0);
	return 0;
}