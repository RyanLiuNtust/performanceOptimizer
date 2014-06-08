#ifndef Performace_Optimize
#define Performace_Optimize

#include <opencv.hpp>
#include <string>
#include <vector>
#include <map>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <time.h>
#include "FileSystem.h"
#include "synthesis_creator.h"

namespace optimizer {
	struct score_table {
		score_table():tp(0), tn(0), fp(0), fn(0), precision(0.), recall(0.), f_measure(0.)
		{ }

		void print() {
			calc_accuracy();
			std::cout << std::fixed << std::setprecision(3);
			std::cout << "\rf_measure = " << f_measure << " precision = " << precision << " recall = " << recall 
				<< " tp = " << tp << " tn = " << tn << " fp = " << fp << " fn = " << fn;
		}
		void calc_accuracy() {
			double denominator = tp + fp;
			if(denominator != 0)
				precision = tp / denominator;

			denominator = tp + fn;
			if(denominator != 0)
				recall = tp / denominator;

			denominator = precision + recall;
			if(denominator != 0)
				f_measure = 2 * precision * recall / denominator; 
		}
		score_table operator+(const score_table& obj) {
			score_table sum_obj;
			sum_obj.tp = this->tp + obj.tp;
			sum_obj.fp = this->fp + obj.fp;
			sum_obj.tn = this->tn + obj.tn;
			sum_obj.fn = this->fn + obj.fn;
			return sum_obj;
		}
		int tp;
		int tn;
		int fp;
		int fn;
		double precision;
		double recall;
		double f_measure;
	};

	struct performance_result {
			performance_result():scale(-1), min_neighbors(-1) {
			}
			double scale;
			double fps;
			int min_neighbors;
			score_table score;
	};
	
	std::vector<performance_result> performance(std::string cascade_dir_name,
					 std::string testing_image_dir_name,
					 std::string ground_truth_file_name,
					 bool is_show = false);
};
#endif