#pragma once
//#include "../stdafx.h"
#include <opencv2/imgproc/imgproc.hpp>

namespace Core
{
	class HoughTransform
	{
	protected:
		HoughTransform();
		~HoughTransform();

	public:
		static HoughTransform& GetInstance();
		void ComputeHough(cv::Mat ipImg);
	};
}
