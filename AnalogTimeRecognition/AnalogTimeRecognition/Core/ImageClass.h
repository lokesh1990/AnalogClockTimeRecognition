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
		float euclideanDist(cv::Point p, cv::Point q);

	public:
		static ImageClass& GetInstance();
		
		void extractClock();
	};
}
