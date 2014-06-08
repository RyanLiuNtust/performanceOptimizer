#include "synthesis_creator.h"
namespace synthesizer {
	namespace {
		struct MatchPathSeparator {
			bool operator()( char ch ) const {
				return ch == '\\' || ch == '/';
			}
		};

		std::string basename( std::string const& pathname ) {
			return std::string( 
				std::find_if( pathname.rbegin(), pathname.rend(),
				MatchPathSeparator() ).base(),
				pathname.end() );
		}

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
		//m :affine_matrix
		cv::Point affine_point(int x, int y, cv::Mat m) {
			cv::Point p;
			p.x = x * m.ptr<double>(0)[0] + y * m.ptr<double>(0)[1] + m.ptr<double>(0)[2];
			p.y = x * m.ptr<double>(1)[0] + y * m.ptr<double>(1)[1] + m.ptr<double>(1)[2];
			return p;
		}
	}
	
	cv::Mat rotate(cv::Mat src, double angle, double scale, bool isWidthMean) {
		double radius = angle * CV_PI/180;
		double alpha = cos(radius) * scale;
		double beta = sin(radius) * scale;
		int nCols = src.cols;
		int nRows = src.rows;
		cv::Point2f center(src.cols/2., src.rows/2.);
		cv::Mat m = cv::getRotationMatrix2D(center, angle, 1.0);
		cv::Scalar bg = (isWidthMean)? cv::mean(src):cv::Scalar(0,0,0);

		nCols = static_cast<int>(src.cols * fabs(alpha) + src.rows * fabs(beta));
		nRows = static_cast<int>(src.cols * fabs(beta) + src.rows * fabs(alpha));
		m.ptr<double>(0)[2] +=  static_cast<int>((nCols - src.cols)/2);
		m.ptr<double>(1)[2] +=	static_cast<int>((nRows - src.rows)/2);
		cv::Mat dst;
		warpAffine(src, dst, m, cv::Size(nCols, nRows), 1, 0, bg);
		return dst;
	}

	void synthesisData(std::vector<std::string>posImgs, std::string synDir, int &syn_w, int &syn_h, double syn_angle, double scale) {
		FileSystem filesystem;
		filesystem.createFolder(synDir);
		for(std::vector<std::string>::iterator it = posImgs.begin(); it != posImgs.end(); it++) {
			
			std::string filename = basename(*it);
			std::string savename = synDir + filename;
			cv::Mat src = cv::imread(*it);
			cv::Mat rot = rotate(src, syn_angle, scale, true);
			syn_w = rot.cols;
			syn_h = rot.rows;
			cv::imwrite(savename, rot);
		}
	}

	void synthesis_ground_truth(std::string ground_truth_file_name,
								std::string synthesis_file_name,
								double angle) {
		std::map<std::string, std::vector<cv::Rect>> ground_truth = load_ground_truth(ground_truth_file_name);
		double radius = angle * CV_PI/180;
		double alpha = cos(radius);
		double beta = sin(radius);

		std::ofstream fs(synthesis_file_name, std::ios::out);
		for(std::map<std::string, std::vector<cv::Rect>>::iterator it = ground_truth.begin(); it != ground_truth.end(); it++) {
			cv::Mat src = cv::imread(it->first);
			int nCols = static_cast<int>(src.cols * fabs(alpha) + src.rows * fabs(beta));
			int nRows = static_cast<int>(src.cols * fabs(beta) + src.rows * fabs(alpha));
			cv::Point2f center(src.cols/2., src.rows/2.);
			cv::Mat m = cv::getRotationMatrix2D(center, angle, 1.0);
			m.ptr<double>(0)[2] +=  static_cast<int>((nCols - src.cols)/2);
			m.ptr<double>(1)[2] +=	static_cast<int>((nRows - src.rows)/2);

			//cv::Mat dst;
			//warpAffine(src, dst, m, cv::Size(nCols, nRows), 1, 0);
			
			std::vector<cv::Rect> rects = it->second;
			fs << it->first << " " << rects.size() << " ";
			for(std::vector<cv::Rect>::iterator rect = rects.begin(); rect != rects.end(); rect++) {
				cv::Point upper_left_p = affine_point(rect->x, rect->y, m);
				cv::Point upper_right_p = affine_point(rect->x + rect->width, rect->y, m);
				cv::Point lower_left_p = affine_point(rect->x, rect->y + rect->height, m);
				cv::Point lower_right_p = affine_point(rect->x + rect->width, rect->y + rect->height, m);
				cv::Point syn_upper_left_p(std::min(upper_left_p.x, lower_left_p.x), std::min(upper_left_p.y, upper_right_p.y));
				cv::Point syn_lower_right_p(std::max(upper_right_p.x, lower_right_p.x), std::max(lower_left_p.y, lower_right_p.y));
				int syn_width = syn_lower_right_p.x - syn_upper_left_p.x;
				int syn_height = syn_lower_right_p.y - syn_upper_left_p.y;
				cv::Rect syn_rect(syn_upper_left_p.x, syn_upper_left_p.y, syn_width, syn_height);
				//cv::rectangle(dst, syn_rect, cv::Scalar(255, 0, 0), 2);
				fs << syn_upper_left_p.x << " " << syn_upper_left_p.y << " " << syn_width << " " << syn_height << std::endl;
			}
			//cv::imshow("test", dst);
			//cv::waitKey(0);
		}
		fs.close();
	}
};