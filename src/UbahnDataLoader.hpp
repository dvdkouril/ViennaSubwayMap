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

struct UbahnLine {
    std::vector<vec2>   points;
    Color               lineColor;
    string              lineName;
    int                 lineNumber;
    gl::VboMeshRef      linePositionData;
    gl::VboMeshRef      lineStripData;
};

struct UbahnStation {
    vec2                position;
    float               height;
    string              name;
};

class UbahnDataLoader
{
public:
    static string getOneLineDataInString(string line);
    static string getLineNumber(string line);
    static Color getLineColor(int number);
    static vector<vec2> getCoordinatesFromString(string dataString);
    static vec2 getPositionFromString(string point);
    
    static std::vector<UbahnLine*> loadLineDataFromFile(string filePath);
    static std::vector<UbahnStation*> loadStationsDataFromFile(string filePath);
    
    static bool loadDataFromYun(string filePath,
                                std::map<std::string, UbahnStation*> & allStations,
                                std::map<std::string, std::vector<UbahnStation*>> & lines);
};

#endif /* UbahnDataLoader_hpp */
