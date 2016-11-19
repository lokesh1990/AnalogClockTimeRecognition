#pragma once

#include <opencv2/imgproc/imgproc.hpp>

namespace Core
{
	class ImageClass
	{
	protected:
		ImageClass();
		~ImageClass();

		cv::Mat sourceImg;
		std::string sourceFileName;

		void readImage();
		double euclideanDist(cv::Point2d p, cv::Point2d q);

	public:
		static ImageClass& GetInstance();
		
		void extractClock();
	};
}
