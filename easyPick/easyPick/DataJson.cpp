
#include "DataJson.h"
# include <boost\filesystem.hpp>

ObjectsDataJson::ObjectsDataJson(std::string f, std::vector<std::string> sfns, std::vector<std::string> hfns, 
		std::vector<EasyPickLibrary::Spot> sLst, std::vector<EasyPickLibrary::Halo> hLst, std::vector<std::vector<int>> linkg){
	// define constructor
	filename = f;
	SpotFeatureNames = sfns;
	HaloFeatureNames = hfns;
	spotList = sLst;
	haloList = hLst;
	linkage = linkg;
}

centralPoint::centralPoint(int xCord, int yCord)
{
	x = xCord;
	y = yCord;
}


// convert cvPoint to point object
centralPoint centralPoint::cvtPoint(Point pt)
{
	centralPoint ptObj(pt.x, pt.y);
	return ptObj;
}


// function to build json object from data
void ObjectsDataJson::cvtJson(ObjectsDataJson dataObject)
{
	// create root of the message
	Document doc;
	// define the document as an object rather than an array
	doc.SetObject();
	// pass an allocator when the object may need to allocate memory
	Document::AllocatorType& allocator = doc.GetAllocator();

	// 1. a string to store filename
	std::string fname = dataObject.filename;
	Value filename(fname.c_str(), allocator);
	doc.AddMember("filename", filename, allocator);


	// 2. rapidjson array - store name of spot&halo features
	Value SpotFeatureArray(kArrayType);
	Value HaloFeatureArray(kArrayType);
	std::vector<std::string> snames = dataObject.SpotFeatureNames;
	std::vector<std::string> hnames = dataObject.HaloFeatureNames;
	for(int i=0; i<snames.size(); ++i){
		Value sname(snames[i].c_str(), allocator);
		SpotFeatureArray.PushBack(sname, allocator);
	}
	for(int j=0; j<hnames.size(); ++j){
		Value hname(hnames[j].c_str(), allocator);
		HaloFeatureArray.PushBack(hname, allocator);
	}
	// add feature names to doc
	doc.AddMember("SpotFeatureArray", SpotFeatureArray, allocator);
	doc.AddMember("HaloFeatureArray", HaloFeatureArray, allocator);


	// 3. array to store spots as objects
	Value SpotObjectArray(kArrayType);
	int snum = dataObject.spotList.size();

	for(int m=0; m<snum; ++m){
		// loop through each spot
		EasyPickLibrary::Spot thisSpot = dataObject.spotList[m];
		// create json object
		Value SpotObject(kObjectType);

		// add items to spot object:
		//  (0) - "central point" - point object
		Value PointObject(kObjectType);
		centralPoint ctPoint = centralPoint::cvtPoint(thisSpot.centrePoint);
		PointObject.AddMember("x", Value(ctPoint.x), allocator);
		PointObject.AddMember("y", Value(ctPoint.y), allocator);
		SpotObject.AddMember(StringRef(snames[0].c_str()), PointObject, allocator);

		//  (1) - "contour" (vector of points)
		// get all points forming contour
		std::vector<Point> contour = thisSpot.contour;
		Value SpotContour(kArrayType);
		for(int p=0; p<contour.size(); ++p) {
			centralPoint cont_p = centralPoint::cvtPoint(contour[p]);
			Value ptObject(kObjectType);
			ptObject.AddMember("x", Value(cont_p.x), allocator);
			ptObject.AddMember("y", Value(cont_p.y), allocator);

			SpotContour.PushBack(ptObject, allocator);
		}
		SpotObject.AddMember(StringRef(snames[1].c_str()), SpotContour, allocator);

		//  (2) - "area"
		SpotObject.AddMember(StringRef(snames[2].c_str()), Value(thisSpot.area), allocator);
		//  (3) - "perimeter"
		SpotObject.AddMember(StringRef(snames[3].c_str()), Value(thisSpot.perimeter), allocator);
		//  (4) - "roundness"
		SpotObject.AddMember(StringRef(snames[4].c_str()), Value(thisSpot.roundness), allocator);

		//  (5) - 'id' is a vector of index
		Value inds(kArrayType);
		for(int id=0; id<dataObject.linkage[m].size(); ++id) {
			inds.PushBack(dataObject.linkage[m][id], allocator);
		}
		SpotObject.AddMember(StringRef(snames[5].c_str()), inds, allocator);

		// push spot object to spot array
		SpotObjectArray.PushBack(SpotObject, allocator);
	}
	doc.AddMember("SpotDataArray", SpotObjectArray, allocator);


	// 4. array to store halos as objects
	Value HaloObjectArray(kArrayType);
	int hnum = dataObject.haloList.size();

	for(int n=0; n<hnum; ++n){
		// loop through each spot
		EasyPickLibrary::Halo thisHalo = dataObject.haloList[n];
		// create json object
		Value HaloObject(kObjectType);
		// add items to halo object:
		//  (0) - "central point"
		Value PointObject(kObjectType);
		centralPoint ctPoint = centralPoint::cvtPoint(thisHalo.centr);
		PointObject.AddMember("x", Value(ctPoint.x), allocator);
		PointObject.AddMember("y", Value(ctPoint.y), allocator);
		HaloObject.AddMember(StringRef(snames[0].c_str()), PointObject, allocator);
		// get all points forming contour
		std::vector<Point> contour = thisHalo.contour;
		Value HaloContour(kArrayType);
		for(int p=0; p<contour.size(); ++p) {
			centralPoint cont_p = centralPoint::cvtPoint(contour[p]);
			Value ptObject(kObjectType);
			ptObject.AddMember("x", Value(cont_p.x), allocator);
			ptObject.AddMember("y", Value(cont_p.y), allocator);

			HaloContour.PushBack(ptObject, allocator);
		}
		//  (1) - "contour" (vector of points)
		HaloObject.AddMember(StringRef(hnames[1].c_str()), HaloContour, allocator);
		//  (2) - "size"
		HaloObject.AddMember(StringRef(hnames[2].c_str()), Value(thisHalo.size), allocator);
		//  (3) - "perimeter"
		HaloObject.AddMember(StringRef(hnames[3].c_str()), Value(thisHalo.length), allocator);
		//  (4) - "roundness"
		HaloObject.AddMember(StringRef(hnames[4].c_str()), Value(thisHalo.roundDgree), allocator);
		//  (5) - "intensity"
		HaloObject.AddMember(StringRef(hnames[5].c_str()), Value(thisHalo.intensity), allocator);
		// (6) - "id"
		HaloObject.AddMember(StringRef(hnames[6].c_str()), Value(1), allocator);
		
		// push to spot array
		HaloObjectArray.PushBack(HaloObject, allocator);
	}
	doc.AddMember("HaloDataArray", HaloObjectArray, allocator);

	/**** pass data to a file stream ****/
	boost::filesystem::path p(fname);
	std::string jsonName = p.stem().string() + ".json";
	ofstream ofs( jsonName);
	OStreamWrapper osw(ofs);

	Writer<OStreamWrapper> writer(osw);
	doc.Accept(writer);
}
