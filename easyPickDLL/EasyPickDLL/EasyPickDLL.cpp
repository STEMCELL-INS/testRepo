// EasyPickDLL.cpp : Defines the exported functions for the DLL application.


#include "stdafx.h"
#include "EasyPickDLL.h"
#include <fstream>
using namespace rapidjson;

std::string version = "1.0.0";

int process_day4(char* dir)
{
	// list files in folder
	std::vector<string> imgFiles;
	getFileList(dir, imgFiles);

	//// loop through each image
	Mat src, src_color, src_color_copy, grad;

	for(int i=0; i< imgFiles.size(); ++i) {
		// full directory
		std::string fullPath = dir + imgFiles[i];
		// read image
		src = imread(fullPath, CV_LOAD_IMAGE_GRAYSCALE);
		src_color = imread(fullPath);
		src_color_copy = src_color.clone();
		if(src.empty()){
			std::cout << "failed to read image file. check directory.";
			return -1;
		}

		std::cout << "Processing image: " << i << "\n";

		// test boundary detection
		Circle well_circle;
		well_circle = EasyPickLibrary::imgProcFunctions::well_boundary_detection(src, 35, 17, 3);  // original: 35, 17, 3
		circle(src_color, Point(well_circle.a, well_circle.b), well_circle.r, Scalar(0, 255, 0), 3);


		// test spot detection
		std::vector<EasyPickLibrary::Spot> spots = EasyPickLibrary::imgProcFunctions::spots_detection(src, well_circle, 31, 5, 31, 9, 5);
		// extract contours from spots
		std::vector<std::vector<Point>> conts;
		for(int i=0; i<spots.size(); ++i) {
			conts.push_back(spots[i].contour);
		}
		drawContours(src_color, conts, -1, Scalar(0,0,255),2, 8);
		imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\spot\\"+ imgFiles[i], src_color);


		// test halo detection
		std::vector<EasyPickLibrary::Halo> halos = EasyPickLibrary::imgProcFunctions::halos_detection(src, well_circle, 39, 17, 31);
		std::vector<std::vector<Point>> contours;
		for(int n=0; n<halos.size(); ++n) {
			contours.push_back(halos[n].contour);
		}
		circle(src_color_copy, Point(well_circle.a, well_circle.b), well_circle.r, Scalar(0, 255, 0), 3);


		// test index linking function
		std::vector<std::vector<int>> links = EasyPickLibrary::imgProcFunctions::link_halo_spot(spots, halos, 10);

		// combing all the data to an object
		std::string fname = imgFiles[i];
		std::vector<std::string> SpotFeatures;
		std::vector<std::string> HaloFeatures;
		SpotFeatures.push_back("centralPoint");
		SpotFeatures.push_back("contour");
		SpotFeatures.push_back("area");
		SpotFeatures.push_back("perimeter");
		SpotFeatures.push_back("roundness");
		SpotFeatures.push_back("id");

		HaloFeatures.push_back("centralpoint");
		HaloFeatures.push_back("contour");
		HaloFeatures.push_back("area");
		HaloFeatures.push_back("perimeter");
		HaloFeatures.push_back("roundness");
		HaloFeatures.push_back("intensity");
		HaloFeatures.push_back("id");

		ObjectsDataJson data(fname, SpotFeatures, HaloFeatures, spots, halos, links);

		// test json FileWriteStream
		ObjectsDataJson::cvtJson(data);
	}

	return 0;
}