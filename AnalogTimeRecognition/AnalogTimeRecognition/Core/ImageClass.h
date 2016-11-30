#pragma once

#include <opencv2/imgproc/imgproc.hpp>

namespace Core
{
	class ImageClass
	{
	private:
		int hour;
		int minute;
		int second;
	protected:
		ImageClass();
		~ImageClass();

		cv::Mat sourceImg;
		std::string sourceFileName;

		void readImage();
		double euclideanDist(cv::Point2d p, cv::Point2d q);
		int getHour(double);
		int getMinSec(double);
	public:
		static ImageClass& GetInstance();
		void setFileName(std::string);
		void extractClock();
		void printTime();
	};
}
