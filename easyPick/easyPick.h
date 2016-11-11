#pragma once


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp> 
#include <iostream>
#include <array>
#include "circle.h"

using namespace cv;
using namespace std;

namespace EasyPickLibrary
{

	// Spot class - a colony inside of a well
	class Spot
	{
	public:	
		Point centrePoint;  // coordinate of the centre point
		std::vector<Point> contour;    // contour representing the spot
		// more features to add ...
		int area;
		int perimeter;
		double roundness;

		Spot(Point cp, std::vector<Point> c, int a, int p, double r);  // constructor
	};

	// halo class - colonies with halo
	class Halo
	{
	public:
		Point centr;
		std::vector<Point> contour;
		// more features to add ...
		int size;
		int length;
		double roundDgree;
		double intensity;

		Halo(Point central, std::vector<Point> contr, int s, int len, double rodgr, double intens);  // constructor
	};

    // This class is exported from the EasyPickLibrary.dll
    class imgProcFunctions
    {
    public:
        // Returns sobel detection results - edges for colony detection
        static Mat sobel_edge_detector(Mat img, int kernelSize);

        // Returns morphology edge detection reseult - edges for boundary detection
        static Mat morphology_edge_detector(Mat img, int blurKsize, int dilKsize);

		// sort method - descending order by value
		static bool sortVal(std::array<int, 2> a, std::array<int, 2> b) {return a.at(0) > b.at(0);};

		// Sort elements in a vector, return 3 indices representing top ones
		static std::array<int, 3> top_three_ind(const std::vector<double> values);

        // Returns a circle representing the boundary
        static Circle well_boundary_detection(Mat img, int blurKerSize, int dilKerSize, int eroKerSize);

		// fill holes in a binary image
		static Mat imBinFill(Mat in, int conn);

		// decide if a contour inside a circle
		static bool is_inside_circle(std::vector<Point> contour, Circle bound);

		// Returns a vector of spots
		static std::vector<Spot> spots_detection(Mat img, Circle boundary, int gaussianKsize, int blurKsize, int sobelKsize, int closeKsize, int openKsize);

		// Returns a vector of halos
		static std::vector<Halo> halos_detection(Mat img, Circle boundary, int gaussianKsize, int blurKsize, int sobelKsize);

		// Link a list of halos to a list of spots on one image
		static std::vector<std::vector<int>> link_halo_spot(std::vector<Spot> spot_list, std::vector<Halo> halo_list, int dist_threshold);
			 
		// Calculate intensity
		static Scalar intensity(Mat grayImg, std::vector<Point>);

		// Draw a list of string on image based on coordinations
		static Mat draw_text(Mat img, std::vector<std::string> text_list, std::vector<CvPoint> coor_list);

    };
}

