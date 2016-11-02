#include "../stdafx.h"
#include "HoughTransform.h"

using namespace Core;

HoughTransform::HoughTransform()
{
}


HoughTransform::~HoughTransform()
{
}

HoughTransform HoughTransform::GetInstance()
{
	if (m_inst == nullptr)
		m_inst = new HoughTransform();

	return *m_inst;
}

void Core::HoughTransform::ComputeHough(cv::Mat ipImg)
{

}
