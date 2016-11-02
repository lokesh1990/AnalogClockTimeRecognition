#pragma once
//#include "../stdafx.h"
#include <opencv2/imgproc/imgproc.hpp>

namespace Core
{
	class HoughTransform
	{
	private:
		HoughTransform *m_inst = nullptr;

	protected:
		HoughTransform();
		~HoughTransform();

	public:
		static HoughTransform GetInstance();
		void ComputeHough(cv::Mat ipImg);
	};
}
