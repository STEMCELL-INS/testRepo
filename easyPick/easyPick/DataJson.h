//  a data object to be parsed to json 

#pragma once

#include "easyPick.h"
#include "circle.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/writer.h>
#include <rapidjson/ostreamwrapper.h>
#include <fstream>
#include <sstream>

using namespace cv;
using namespace rapidjson;

class ObjectsDataJson {
public:
	std::string filename;
	std::vector<std::string> SpotFeatureNames;
	std::vector<std::string> HaloFeatureNames;
	std::vector<EasyPickLibrary::Spot> spotList;
	std::vector<EasyPickLibrary::Halo> haloList;
	std::vector<std::vector<int>> linkage;

	// constructor
	ObjectsDataJson(std::string f, std::vector<std::string> sfns, std::vector<std::string> hfns, 
		std::vector<EasyPickLibrary::Spot> sLst, std::vector<EasyPickLibrary::Halo> hLst, std::vector<std::vector<int>> linkg);       
	 
	static void cvtJson(ObjectsDataJson d);
	
};


class centralPoint {
public:
	int x;
	int y;

	// constructor
	centralPoint(int xCord, int yCord);
	static centralPoint cvtPoint(Point pt);
};

