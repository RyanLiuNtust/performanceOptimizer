#include "performance_optimize.h"

namespace optimizer {
	namespace {
		std::map<std::string, std::vector<cv::Rect>> load_ground_truth(std::string roi_file) {
			std::map<std::string, std::vector<cv::Rect>> name_roi_map;
			std::string img_dir;
			int numRect = 0;

			std::fstream fopener(roi_file);
			if(!fopener.is_open()) {
				std::cout << "Note can not open the " << roi_file << std::endl;
				return name_roi_map;
			}

			while(fopener >> img_dir >> numRect) {
				std::vector<cv::Rect> rect(numRect, cv::Rect(0,0,0,0));
				for(std::vector<cv::Rect>::iterator it = rect.begin(); it != rect.end(); it++) {
					fopener >> it->x >> it->y >> it->width >> it->height;
				}
				name_roi_map.insert(std::make_pair(img_dir, rect));
			}
			fopener.close();
			std::cout << "number of recorded eyes: "  << name_roi_map.size()  << std::endl;
			return name_roi_map;
		}
		// r1 ground_truth,
		// r2 predict_rect
		double inter_union(const cv::Rect r1, const cv::Rect r2) {
			if(r2.height > 1.2 * r1.height || r2.width > 1.2 * r1.width) {
				return 0;
			}

			int bi[4];
			bi[0] = std::max(r1.x, r2.x);
			bi[1] = std::max(r1.y, r2.y);
			bi[2] = std::min(r1.x + r1.width, r2.x + r2.width);
			bi[3] = std::min(r1.y + r1.height, r2.y + r2.height);
			double iw = bi[2] - bi[0] + 1;
			double ih = bi[3] - bi[1] + 1;
			double iarea = iw * ih;
			double ov = 0;
			if (iw > 0 && ih > 0) {
				double ua = r1.area() + r2.area() - iarea;
				ov = iarea/ua;
			}
			return ov;
		}

		score_table grading(std::vector<cv::Rect> ground_truth_rects, std::vector<cv::Rect> predict_rects, double ratio_overlap = 0.9) {
			score_table scores;
			for(std::vector<cv::Rect>::iterator predict = predict_rects.begin(); predict != predict_rects.end(); predict++) {
				bool is_correct = false;
				for(std::vector<cv::Rect>::iterator truth = ground_truth_rects.begin(); truth != ground_truth_rects.end(); truth++) {
					double ratio = inter_union(*truth, *predict);
					if(ratio > ratio_overlap) {
						is_correct = true;
						ground_truth_rects.erase(truth);
						break;
					}
				}
				if(is_correct) 
					scores.tp++;
				else
					scores.fp++;
			}
			// false negative is the remaining unrecognized
			scores.fn = ground_truth_rects.size();
			return scores;
		}

		void draw_rects(cv::Mat &src, std::vector<cv::Rect> rois, cv::Scalar color = cv::Scalar(255, 0, 0)) {
			for(std::vector<cv::Rect>::iterator rect_it = rois.begin(); rect_it != rois.end(); rect_it++) {
				cv::rectangle(src, *rect_it, color, 2);
			}
		}
	}
	
	//width:original image width
	//height:original image height
	void extension(int width, int height, cv::Rect &rect) {
		std::cout << rect.x << " " << rect.y << " " << rect.width << " " << rect.height << std::endl;
		if(rect.x > 10)
			rect.x -= 10;
		if(rect.y > 10)
			rect.y -= 10;
		if((rect.x + rect.width + 20) <= width)
			rect.width += 20;
		else 
			rect.width = (width - rect.x - 1);
		if((rect.y + rect.height + 20)<= height)
			rect.height += 20;
		else
			rect.height = (height - rect.y - 1);
	}
	
	cv::Point2f compute_intersect(cv::Vec4i a, cv::Vec4i b) {
		int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3];
		int x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];

		if (float d = ((float)(x1-x2) * (y3-y4)) - ((y1-y2) * (x3-x4))) {
			cv::Point2f pt;
			pt.x = ((x1*y2 - y1*x2) * (x3-x4) - (x1-x2) * (x3*y4 - y3*x4)) / d;
			pt.y = ((x1*y2 - y1*x2) * (y3-y4) - (y1-y2) * (x3*y4 - y3*x4)) / d;
			return pt;
		}
		else
			return cv::Point2f(-1, -1);
	}

	std::vector<cv::Point2f> find_intersects(std::vector<cv::Vec2f> lines) {
	/*	for (int i = 0; i < lines.size(); i++) {
			cv::Vec4i v = lines[i];
			lines[i][0] = 0;
			lines[i][1] = ((float)v[1] - v[3]) / (v[0] - v[2]) * -v[0] + v[1]; 
			lines[i][2] = src.cols; 
			lines[i][3] = ((float)v[1] - v[3]) / (v[0] - v[2]) * (src.cols - v[2]) + v[3];
		}*/

		std::vector<cv::Point2f> corners;
		for (int i = 0; i < lines.size(); i++) {
			for (int j = i+1; j < lines.size(); j++) {
		/*		cv::Point2f pt = compute_intersect(lines[i], lines[j]);
				if (pt.x >= 0 && pt.y >= 0)
					corners.push_back(pt);*/
			}
		}
	}

	void lp_detection(cv::Mat src) {
		std::string cascade_dir_name[1] = {
			"../../../include/cascade/plate/700_2700_-15.xml"
		};
		cv::CascadeClassifier classifier(cascade_dir_name[0]);
		std::vector<cv::Rect> predict_rects;
		double scale = 1.01;
		int min_neighbors = 2;
		cv::Size minSize(100, 100);
		cv::Size maxSize(250, 250);
		classifier.detectMultiScale( src, predict_rects, scale, min_neighbors, 0, minSize, maxSize);

		for(std::vector<cv::Rect>::iterator it = predict_rects.begin(); it != predict_rects.end(); it++) {
			cv::imshow("roi", src(*it));

			extension(src.cols, src.rows, *it);
			cv::Mat rot = synthesizer::rotate(src(*it), 15);

			double ratio_1 = 0.1, ratio_2 = 0.9;
			cv::Rect roi(rot.cols * ratio_1, rot.rows * ratio_1, rot.cols * ratio_2, rot.rows * ratio_2);
			cv::Mat dst = rot(roi).clone();
			cv::Mat canny;
			cv::Mat gray;
			cv::cvtColor(dst, gray, CV_BGR2GRAY);
			cv::Canny(gray, canny, 150, 200, 3);
			cv::imshow("canny", canny);

			std::vector<cv::Vec4i> lines;
			cv::HoughLinesP(canny, lines, 1, CV_PI/180, 70, 30, 10);
			for( size_t i = 0; i < lines.size(); i++ ) {
				float rho = lines[i][0], theta = lines[i][1];
				cv::Point pt1, pt2;
				
				std::cout << "theta" << theta << std::endl;
				//100(100-90) < theta < 110
				//if(!(theta < 1.92 && theta > 1.744)) continue;
				double a = cos(theta), b = sin(theta);
				double x0 = a*rho, y0 = b*rho;
				pt1.x = cvRound(x0 + 1000*(-b));
				pt1.y = cvRound(y0 + 1000*(a));
				pt2.x = cvRound(x0 - 1000*(-b));
				pt2.y = cvRound(y0 - 1000*(a));
				line( dst, pt1, pt2, cv::Scalar(0,0,255), 3, CV_AA);
			}
			cv::imshow("normalize_1", synthesizer::rotate(src(*it), 15));
			cv::imshow("draw line", dst);
			cv::waitKey();
		}
	}

	std::vector<performance_result> performance(std::string cascade_dir_name,
												std::string testing_image_dir_name,
												std::string ground_truth_file_name,
												bool is_show) {
		std::vector<performance_result> results;
		cv::CascadeClassifier classifier(cascade_dir_name);
		if(classifier.empty()) {
			std::cout << "cascade could not be loaded, pls check the cascade directory \n";
			return results;
		}

		FileSystem filesystem;
		std::vector<std::string> testing_list= filesystem.getFileList(testing_image_dir_name, ALL, false);
		std::map<std::string, std::vector<cv::Rect>>ground_truth = load_ground_truth(ground_truth_file_name);
		if(ground_truth.empty()) {
			std::cout << "Error no ground truth provided\n";
			return results;
		}

		double scales_test[6] = {1.01, 1.02, 1.05, 1.1, 1.2, 1.3};
		int min_neighbors_test[4] = {2, 3, 4, 5};

		for(int s = 0; s < 6; s++) {
			for(int n = 0; n < 4; n++) {
				cv::Size minSize = cv::Size(100, 100);
				cv::Size maxSize = cv::Size(230, 230);
				double scale = scales_test[s];
				int min_neighbors = min_neighbors_test[n];

				performance_result performance;
				performance.scale = scale;
				performance.min_neighbors = min_neighbors;
				
				clock_t start_time = clock();
				for(std::vector<std::string>::iterator it = testing_list.begin(); it != testing_list.end(); it++) {
					cv::Mat test_img = cv::imread(*it);
					std::vector<cv::Rect> rois = ground_truth.find(*it)->second;
					std::vector<cv::Rect> predict_rects;
					classifier.detectMultiScale( test_img, predict_rects, scale, min_neighbors, 0, minSize, maxSize);
					score_table result = grading(rois, predict_rects, 0.8);
					performance.score = performance.score + result;
					performance.score.print();
					
					//lp_detection(test_img);
					if(is_show) {
						draw_rects(test_img, rois);
						draw_rects(test_img, predict_rects, cv::Scalar(0, 255, 0));
						cv::imshow("roi_test", test_img);
						cv::waitKey();
					}
				}
				clock_t total_time = (clock() - start_time) ;
				
				double fps = testing_list.size() / ((double)total_time/ (CLOCKS_PER_SEC));
				performance.fps = fps;
				results.push_back(performance);
			}
		}
		return results;	
	}
}