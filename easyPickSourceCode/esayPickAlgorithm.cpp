//#include "easyPick.h"
#include "DataJson.h"
#include "helper.h"
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>
#include <cstdio>
#include <string>
using namespace rapidjson;
using namespace EasyPickLibrary;

std::string dir = "C:\\projects\\EasyPickFL_ImgProcessing\\easyPick Images\\";
//std::string dir = "C:\\projects\\EasyPickFL_ImgProcessing\\trouble images\\";    // trouble shooting image directory

int main(int, char** argv)
{
	// list files in folder
	std::vector<string> imgFiles;
	getFileList(dir, imgFiles);

	// loop through each image
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

		// test edge detector function
		//grad = imgProcFunctions::sobel_edge_detector(src, 31);
		//imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\sobeledge.jpg", grad);
	
		// test morphorlogy edge detection
		//Mat morph_edge;
		//morph_edge = EasyPickLibrary::imgProcFunctions::morphology_edge_detector(src, 35, 15);
		//imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\morphedge.jpg", morph_edge);


		// test boundary detection
		Circle well_circle;
		well_circle = EasyPickLibrary::imgProcFunctions::well_boundary_detection(src, 35, 17, 3);  // original: 35, 17, 3
		circle(src_color, Point(well_circle.a, well_circle.b), well_circle.r, Scalar(0, 255, 0), 3);
		//imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\boundary.jpg", src_color);


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
		std::vector<EasyPickLibrary::Halo> halos = imgProcFunctions::halos_detection(src, well_circle, 39, 17, 31);
		std::vector<std::vector<Point>> contours;
		for(int n=0; n<halos.size(); ++n) {
			contours.push_back(halos[n].contour);
		}
		circle(src_color_copy, Point(well_circle.a, well_circle.b), well_circle.r, Scalar(0, 255, 0), 3);
		//drawContours(src_color_copy, conts, -1, Scalar(0,0,255),2, 8);
		//drawContours(src_color_copy, contours, -1, Scalar(0,150,205),2, 8);
		//imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\halo\\halos.jpg", src_color_copy);


		// test index linking function
		std::vector<std::vector<int>> links = imgProcFunctions::link_halo_spot(spots, halos, 10);
		// spots
		//std::vector<std::vector<Point>> spotConts;
		//for(int s=0; s<spots.size(); ++s) {
		//	spotConts.push_back(spots[s].contour);
		//}
		//for(int l=0; l<links.size(); ++l) {
		//	// draw the contours and see what's happening
		//	std::vector<std::vector<Point>> contours;
		//	std::vector<int> tempinds = links[l];
		//	std::string postfix = std::to_string(static_cast<unsigned long long>(l));
		//	if(tempinds[0]!=-1){
		//		// find halo contours
		//		for(int h=0; h<tempinds.size(); ++h) {
		//			contours.push_back(halos[tempinds[h]].contour);
		//		}
		//		drawContours(src_color_copy, contours, -1, Scalar(0,0,255),2, 8);
		//		putText(src_color_copy, postfix, spots[l].centrePoint, 2, 2.2, FONT_HERSHEY_SIMPLEX);
		//	}
		//	else {
		//		putText(src_color_copy, "-1", spots[l].centrePoint, 2, 2.2, FONT_HERSHEY_SIMPLEX);	
		//	}
		//	drawContours(src_color_copy, spotConts, l, Scalar(0,160,255),2, 8);
		//	std::string filedirc = "c:\\projects\\easypickfl_imgprocessing\\inter_results\\test\\" + postfix + "_"  + imgFiles[i];
		//	imwrite(filedirc, src_color_copy);
		//}


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
