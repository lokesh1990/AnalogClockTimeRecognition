#include <opencv2/ts/ts.hpp>
#include <opencv2/core/core.hpp>

class MyTestClass : public cvtest::BaseTest
{
public:
	MyTestClass() : mat(cv::Scalar::all(1)) {}

protected:
	cv::Mat mat;
	void run(int) {
		ts->set_failed_test_info(cvtest::TS::OK);

		//check that all values are between 1 and 1 (and not Nan)
		if (0 != cvtest::check(mat, 1, 1, 0))
			ts->set_failed_test_info(cvtest::TS::FAIL_INVALID_TEST_DATA);

		mylib::myfunction(mat);

		//check that all values are between 0 and 0 (and not Nan)
		if (0 != cvtest::check(mat, 0, 0, 0))
			ts->set_failed_test_info(cvtest::TS::FAIL_INVALID_OUTPUT);
	}
};

TEST(MyTestSuite, ATestThatPasses) {
	MyTestClass myTestClass;
	myTestClass.safe_run();
}

TEST(MyTestSuite, ATestThatFails) {
	bool mybool = false;
	ASSERT_TRUE(mybool);
}