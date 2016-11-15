#pragma once

#include "DataJson.h"
#include "helper.h"
#include "opencv2/imgproc/imgproc.hpp"



#ifdef EASYPICK_EXPORTS
#define EASYPICK_API __declspec(dllexport) 
#else
#define EASYPICK_API __declspec(dllimport) 
#endif

extern "C" EASYPICK_API int process_day4();

