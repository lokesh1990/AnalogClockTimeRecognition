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

	public:
		static ImageClass& GetInstance();
		
		void extractClock();
	};
}
