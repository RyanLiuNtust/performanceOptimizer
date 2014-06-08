#include "performance_optimize.h"
#include "synthesis_creator.h"
void save(std::vector<optimizer::performance_result> results, std::string cascade_dir_name) {
	if(results.empty()) return;
	std::fstream fs("performance.txt", std::ios::out | std::ios::app);
	fs << cascade_dir_name << std::endl;
	for(std::vector<optimizer::performance_result>::iterator it = results.begin();
		it != results.end();
		it++) {
			fs << "F measure = "	 << it->score.f_measure <<
				  " tp = "			 << it->score.tp		<<  
				  " pre = "			 << it->score.precision <<
				  " recall = "		 << it->score.recall	<<
				  " scale = "		 << it->scale			<< 
				  " minNeigh = "	 << it->min_neighbors   <<
				  " fps = "			 << it->fps   << std::endl;
	}
	fs.close();
}

void main() {
	//FileSystem filesystem;
	//std::vector<std::string> posImgs = filesystem.getFileList("D:/Program/db/car/plate_test", ALL);
	//std::string synDir = ".\\syn\\-15\\";
	//
	//int syn_w, syn_h;
	//synthesizer::synthesisData(posImgs, synDir, syn_w, syn_h, -15);
	/*synthesizer::synthesis_ground_truth("../../../db/car/plate_test/plate.txt",
										"./syn_plate_-15.txt",
										-15);*/
	std::string cascade_dir_name[4] = {"../../../include/cascade/plate/808_2721_-15.xml",
									   "../../../include/cascade/plate/808_1721_-15.xml",
									   "../../../include/cascade/plate/700_2700_-15.xml",
									   "../../../include/cascade/plate/950_2721_-15.xml"};
	for(int i = 0; i < 4; i++) {
		std::vector<optimizer::performance_result> results = optimizer::performance(cascade_dir_name[i],
															  "D:/Program/db/car/plate_test_-15",
															  "./syn_plate_-15.txt",
															  false);
		save(results, cascade_dir_name[i]);
	}
	/*"../../../db/car/plate_test/plate.txt"*/
	system("pause");
}