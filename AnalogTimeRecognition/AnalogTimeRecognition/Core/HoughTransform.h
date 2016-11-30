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

		bool CheckCenterOnLine(cv::Vec4i line, cv::Point2d center);
		
	public:
		static HoughTransform& GetInstance();
		std::vector<cv::Vec4d> ComputeHough(cv::Mat ipImg, cv::Point2d &center, std::string);
	};
}
