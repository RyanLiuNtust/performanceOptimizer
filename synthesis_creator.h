#ifndef Synthesis_Creator
#define Synthesis_Creator

#include <opencv.hpp>
#include <fstream>
#include <math.h>
#include "FileSystem.h"

namespace synthesizer {
	void synthesisData(std::vector<std::string>posImgs, std::string synDir, int &syn_w, int &syn_h, double syn_angle = 0, double scale = 1);
	void synthesis_ground_truth(std::string ground_truth_file_name, std::string synthesis_file_name, double angle);
	cv::Mat rotate(cv::Mat src, double angle, double scale = 1, bool isWidthMean = false);
};
#endif