//
//  UbahnDataLoader.hpp
//  ViennaSubwayMap
//
//  Created by David Kouril on 08/12/2017.
//
//

#ifndef UbahnDataLoader_hpp
#define UbahnDataLoader_hpp

#include <stdio.h>
#include <string>
//#include "cinder/gl/gl.h"

using namespace std;
using namespace ci::app;
using namespace ci;

class UbahnDataLoader
{
public:
    static string getOneLineDataInString(string line);
    static string getLineNumber(string line);
    static Color getLineColor(int number);
    static vector<vec2> getCoordinatesFromString(string dataString);
    static vec2 getPositionFromString(string point);
};

#endif /* UbahnDataLoader_hpp */
