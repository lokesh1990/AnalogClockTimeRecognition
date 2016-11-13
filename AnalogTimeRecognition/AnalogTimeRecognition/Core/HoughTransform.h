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

		bool CheckCenterOnLine(cv::Vec4i line, cv::Point center);

	public:
		static HoughTransform& GetInstance();
		std::vector<cv::Vec4i> ComputeHough(cv::Mat ipImg, cv::Point center);
	};
}
