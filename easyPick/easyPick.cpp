
#include "easyPick.h"
#include "circleFit.h"
#include "data.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <array>
#include <cmath>

namespace EasyPickLibrary
{
	/************ define functions for algorithm - imgProcFunctions ******************/

	// sobel edge detection method
	Mat imgProcFunctions::sobel_edge_detector(Mat img, int kernelSize)
	{
		Mat grad_x, grad_y, temp, grad;
		Mat abs_grad_x, abs_grad_y;
		double min, max;

		Sobel(img, grad_x, CV_64F, 1, 0, kernelSize, 1, 0, BORDER_DEFAULT);
		Sobel(img, grad_y, CV_64F, 0, 1, kernelSize, 1, 0, BORDER_DEFAULT);
		
		grad_x = grad_x.mul(grad_x);   
		grad_y = grad_y.mul(grad_y);
		grad = grad_x + grad_y;
		sqrt(grad, grad);
		
		minMaxLoc(grad, &min, &max);
		grad *= 255.0/max;
		convertScaleAbs(grad, grad);

		return grad;
	}

	// morphology edge detection method
	Mat imgProcFunctions::morphology_edge_detector(Mat img, int blrKsize, int dilKsize)
	{
		Mat blurred, kernel, dilated, edge;
		// apply average blur
		blur(img, blurred, Size(blrKsize, blrKsize)); 
		// define kernel for dilation
		kernel = getStructuringElement(MORPH_RECT, Size(dilKsize,dilKsize));
		// apply dilation
		dilate(blurred, dilated, kernel);

		edge = dilated - blurred;
		return edge;
	}

	// sort a list of values, return index list
	std::array<int, 3> imgProcFunctions::top_three_ind(const std::vector<double> values)
	{
		// initialize array elements to return
		std::array<int, 3> inds;
		
		// combine values and their index to a vector
		std::vector<std::array<int, 2>> val_ind_list;
		for(int i=0; i<values.size(); ++i) {
			std::array<int, 2> temp = {values[i], i};
			val_ind_list.push_back(temp);
		}

		// sort the vector based on value
		std::sort(val_ind_list.begin(), val_ind_list.end(), sortVal);
		// push the 3 indices to array
		inds.at(0) = val_ind_list[0].at(1);
		inds.at(1) = val_ind_list[1].at(1);
		inds.at(2) = val_ind_list[2].at(1);
		
		return inds;
	}

	// well boundary detection
	Circle imgProcFunctions::well_boundary_detection(Mat img, int blurKerSize, int dilKerSize, int eroKerSize)
	{
		// find edges
		Mat edges;
		edges = imgProcFunctions::morphology_edge_detector(img, blurKerSize, dilKerSize);

		// threshold to binary image
		Mat og_bin;
		threshold(edges, og_bin, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

		// erode image with kernel
		Mat kernel, eroded;
		kernel = getStructuringElement(MORPH_ELLIPSE, Size(eroKerSize, eroKerSize));
		erode(og_bin, eroded, kernel);

		// detect all the contours
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		findContours(eroded, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
		// initialize a vector of perimeters to hold each contour length
		std::vector<double> perimeter_list;
		// iterate and find the perimeter of each contour
		for (int i = 0; i < contours.size(); ++i) {
			double perimeter = arcLength(contours[i], true);
			double epsilon = 0.1*perimeter;
			vector<Point> approx;
			approxPolyDP(Mat(contours[i]), approx, epsilon, true);
			perimeter = arcLength(approx, true);
			perimeter_list.push_back(perimeter);	
		}
		
		// find the top 3 contours
		std::array<int, 3> indices;
		indices = imgProcFunctions::top_three_ind(perimeter_list);
		// test only - print result
		//std::cout << "top three contours: "<< indices[0] << ", " << indices[1] << ", " << indices[2] << "\n";
		
		// initialize a list of circles
		std::vector<Circle> circle_list;
		// fit each contour into circle
		for (int k=0; k<3; ++k) {

			// points forming the contour
			vector<Point> temp_cont = contours[indices[k]];
			// number of points consisting of the contour
			int num_points = temp_cont.size();
			// x and y coordinates of points
			reals* xCoords = new reals[num_points];
			reals* yCoords = new reals[num_points];
			for(int j=0; j<num_points; ++j){
				xCoords[j] = temp_cont[j].x;
				yCoords[j] = temp_cont[j].y;
			}
			// create Data object
			Data temp_data(num_points, xCoords, yCoords);
			// calculate the circle
			Circle temp_circle;
			temp_circle = CircleFit (temp_data);

			// push into circle list
			circle_list.push_back(temp_circle);

			// release memory
			delete[] xCoords;
			delete[] yCoords;
		}
		// test only - print circles
		/*for(int n=0; n<3; ++n) {
			circle_list[n].print();
		}*/
	
		// calculate perimeters of the three circles
		Mat dummy;
		int max_ind = 0;
		double max_perim = 0;
		for(int m=0; m<3; ++m) {
			// get the circle
			Circle cir = circle_list[m];
			// draw circle on the dummy mat
			dummy = Mat::zeros(img.size(), CV_8UC1);
			circle(dummy, Point(cir.a, cir.b), cir.r, Scalar(255, 255, 255), 1);
			// contour of the circle - only one contour will be in the vector
			vector<vector<Point>> contour_t;
			vector<Vec4i> h_t;
			findContours(dummy, contour_t, h_t, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
			// calculate the perimeter
			int temp_perim = arcLength(contour_t[0], true);
			if (temp_perim > max_perim) {
				// update the max contour (inner circle)
				max_perim = temp_perim;
				max_ind = m;
			}
		}

		// test only
		//std::cout << "max perimeter: " << max_perim << "\n" << "max ind: " << max_ind << "\n";

		// construct the inner circle
		Circle fitCircle(circle_list[max_ind].a, circle_list[max_ind].b, circle_list[max_ind].r);
		//fitCircle.print(); // test only

		return fitCircle;
	}

	// fill holes 
	Mat imgProcFunctions::imBinFill(Mat in, int conn)
	{
		// image size
		int width = in.cols;
		int height = in.rows;

		const int background = 0;
		const int foreground = 255;

		// border points
		vector<Point> seedPoints;
		for (int i=0; i<width; i++){
			seedPoints.push_back(Point(i,0));
			seedPoints.push_back(Point(i,height-1));
		}
		for (int i=1; i<height-1; i++){
			seedPoints.push_back(Point(0,i));
			seedPoints.push_back(Point(width-1,i));
		}

		// initialize output 
		Mat out;
		out = in.clone();
		for (int i=0; i<seedPoints.size(); i++){
			if (out.data[seedPoints[i].x + seedPoints[i].y*out.step1()]==background){
				floodFill(out, seedPoints[i], foreground, 0, 0, 0, conn | FLOODFILL_FIXED_RANGE); // conn 4 or 8
			}
		}

		out = (~out) | in; // ~(out) is holes - "or" holes with input to fill the holes

		return out;
	}

	bool imgProcFunctions::is_inside_circle(std::vector<Point> contour, Circle bound)
	{
			for(int i=0; i<contour.size(); ++i) {
				if (pow(contour[i].x - bound.a, 2) + pow(contour[i].y - bound.b, 2) > pow(bound.r, 2)) {
					return true;
				}
			}
			return false;
	}


	Scalar imgProcFunctions::intensity(Mat grayImg, std::vector<Point> ctr)
	{
		Mat mask = Mat::zeros(grayImg.size(), CV_8UC1);

		std::vector<std::vector<Point>> dummy_conts;
		dummy_conts.push_back(ctr);

		drawContours(mask, dummy_conts, -1, 255, -1);

		Scalar intens = mean(grayImg, mask=mask);
		return intens;
	}

	// spot detection
	std::vector<Spot> imgProcFunctions::spots_detection(Mat img, Circle plate, int gaussianKsize, int blurKsize, int sobelKsize, int closeKsize, int openKsize) 
	{
		// blurring
		Mat blurred;
		GaussianBlur(img, blurred, Size(gaussianKsize, gaussianKsize), 0, 0);
		blur(blurred, blurred, Size(blurKsize, blurKsize), Point(-1,-1));
		//imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\spot\\gaus_average.jpg", blurred);

		// edge detection
		Mat edge;
		edge = sobel_edge_detector(blurred, sobelKsize);
		//imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\spot\\edge.jpg", edge);

		// binarize
		Mat bin;
		threshold(edge, bin, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		//imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\spot\\binary.jpg", bin);
		
		// fill the wholes in objects
		Mat bin_filled;
		bin_filled = imBinFill(bin, 8);
		// define kernel for close
		Mat kernel = getStructuringElement(MORPH_RECT, Size(closeKsize, closeKsize));
		morphologyEx(bin_filled, bin_filled, MORPH_CLOSE, kernel);
		//imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\spot\\binfilled.jpg", bin_filled);

		// image opening to remove small artifacts
		kernel = getStructuringElement(MORPH_RECT, Size(openKsize, openKsize));
		morphologyEx(bin_filled, bin_filled, MORPH_OPEN, kernel);
		//imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\spot\\openned.jpg", bin_filled);

		// declare result 
		std::vector<Spot> result;

		// detect contours of spots
		vector<vector<Point>> spot_conts;
		findContours(bin_filled, spot_conts, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
		// loop throught each contour, filtering out invalid ones
		for(int c=0; c<spot_conts.size(); ++c) {

			//  get the spot/contour
			std::vector<Point> cont_temp = spot_conts[c];
			// check if the spot touches boundary
			bool flag = is_inside_circle(cont_temp, plate);
			if(!flag) {
				// get the moment of spot
				Moments temp = moments(cont_temp, false);
				if(temp.m00 != 0) {
					// bounding box of the spot
					Rect box = boundingRect(cont_temp);
					// calculate aspect ratio
					if(box.width!=0 && box.height!=0 && box.width<0.1*plate.r && box.height<0.1*plate.r) {
						double aspect_ratio_w = (box.width)/(box.height);
						double aspect_ratio_h = (box.height)/(box.width);
						if(aspect_ratio_w<1.5 && aspect_ratio_h<1.5) {
							// calculate area
							double area = contourArea(cont_temp);
							// calculate perimeter
							double perim = arcLength(cont_temp, true);
							// calculate roundness
							double round = pow(perim, 2)/(2*Pi*area);
							if(round<4.1 && area > 50 && area<20000){
								// centroid
								int centroid_x = (temp.m10)/(temp.m00);
								int centroid_y = (temp.m01)/(temp.m00);
								Point centroid(centroid_x, centroid_y);
								// push back to vector 
								result.push_back(Spot(centroid, cont_temp, area, perim, round));
							}
						}
						
					}
					
				}
				
			}
		}
		// return spot list
		return result;
	}

	// halo detection
	std::vector<Halo> imgProcFunctions::halos_detection(Mat img, Circle boundary, int gaussianKsize, int blurKsize, int sobelKsize)
	{
		// blurring
		Mat blurred;
		GaussianBlur(img, blurred, Size(gaussianKsize, gaussianKsize), 0, 0);
		blur(blurred, blurred, Size(blurKsize, blurKsize), Point(-1,-1));

		// edge detection
		Mat edge;
		edge = sobel_edge_detector(blurred, sobelKsize);
		imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\halo\\edge.jpg", edge);

		// binary
		Mat bin;
		threshold(edge, bin, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		imwrite("c:\\projects\\easypickfl_imgprocessing\\inter_results\\halo\\binary.jpg", bin);

		// declare halos to be returned
		std::vector<Halo> result;

		// find contours
		vector<vector<Point>> halo_conts;
		vector<Vec4i> hierarchy;
		findContours(bin, halo_conts, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
		for(int i=0; i<hierarchy.size(); ++i) {
			// child flag
			int child = hierarchy[i][2];
			if(child != -1){
				// get the contour
				std::vector<Point> cont = halo_conts[i];
				// if the contour touch boundary
				bool flag = is_inside_circle(cont, boundary);
				if(!flag){
					// perimeter/length
					double l = arcLength(cont, true);
					// area
					double area = contourArea(cont);
					if(l<2000 && area<20000){
						// get the moment
						Moments m = moments(cont, false);
						if(m.m00 != 0){
							Rect box = boundingRect(cont);
							double aspect_ratio_w = (box.width)/(box.height);
							double aspect_ratio_h = (box.height)/(box.width);
							double roundness = pow(l, 2)/(2*Pi*area);
							if(aspect_ratio_w < 1.5 && aspect_ratio_h < 1.5 && roundness < 4.5){
								// coordinate of halo
								int centroid_x = (m.m10)/(m.m00);
								int centroid_y = (m.m01)/(m.m00);
								// intensity of halo
								//Mat gray_image;
								//cvtColor(img, gray_image, CV_BGR2GRAY );
								Scalar intens = intensity(img, cont);
								Point pt(centroid_x, centroid_y);
								Halo temp(pt, cont, area, l, roundness, intens.val[0]);

								// push to result list
								result.push_back(temp);
							}
						}
					}
				}
			}
		}

		return result;
	}

	// link halo to spot
	std::vector<std::vector<int>> imgProcFunctions::link_halo_spot(std::vector<Spot> spot_list, std::vector<Halo> halo_list, int dist_threshold)
	{
		// get all the central points from spots and halos
		std::vector<Point> spots_points;
		std::vector<Point> halos_points;
		for(int i=0; i<spot_list.size(); ++i) {
			spots_points.push_back(spot_list[i].centrePoint);
		}
		for(int j=0; j<halo_list.size(); ++j) {
			halos_points.push_back(halo_list[j].centr);
		}
		// declare the result
		std::vector<std::vector<int>> res;
		// loop through spots 
		for(int k=0; k<spots_points.size(); ++k){
			int num = 0;
			std::vector<int> inds;
			for(int r=0; r<halos_points.size(); ++r) {
				// calculate difference
				float diff_x = spots_points[k].x - halos_points[r].x;
				float diff_y = spots_points[k].y - halos_points[r].y;
				float diff = (diff_x)*(diff_x) + (diff_y)*(diff_y);
				diff = sqrt(diff);
				if (diff<dist_threshold){
					num += 1;
					inds.push_back(r);
				}
			}
			if(num==0) {
				inds.push_back(-1);
			}

			// push the list of index to result array
			res.push_back(inds);
		
		}
		return res;
	}

};



// define constructors for classes
EasyPickLibrary::Spot::Spot(Point cp, std::vector<Point> c, int a, int p, double r)
{
	centrePoint = cp;
	contour = c;
	area = a;
	perimeter = p;
	roundness = r;
}


EasyPickLibrary::Halo::Halo(Point central, std::vector<Point> contr, int s, int len, double rodgr, double intens)
{
	centr = central;
	contour = contr;
	size = s;
	length = len;
	roundDgree = rodgr;
	intensity = intens;
}