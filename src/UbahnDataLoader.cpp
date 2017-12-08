//
//  UbahnDataLoader.cpp
//  ViennaSubwayMap
//
//  Created by David Kouril on 08/12/2017.
//
//

#include "UbahnDataLoader.hpp"
#include "Utils.hpp"

string UbahnDataLoader::getOneLineDataInString(string line)
{
    size_t first = 0;
    size_t second = 0;
    
    first = line.find("\"");
    second = line.find("\"", first+1);
    
    if (first == string::npos || second == string::npos) return "";
    
    size_t prestringLength = 13;
    size_t dataStarts = first + prestringLength;
    size_t dataEnds = second - 1;
    return line.substr(dataStarts, dataEnds-dataStarts);
}

string UbahnDataLoader::getLineNumber(string line)
{
    size_t firstApost = line.find("\"");
    size_t secondApost = line.find("\"", firstApost + 1);
    size_t lineNumberPos = secondApost + 1 + 1;
    return line.substr(lineNumberPos, 1);
}

Color UbahnDataLoader::getLineColor(int number)
{
    auto one =      Color(255.0 / 255.0, 0.0    / 255.0, 0.0    / 255.0); // 255.0 0.0 0.0
    auto two =      Color(153.0 / 255.0, 0.0    / 255.0, 255.0  / 255.0); // 153.0 0.0 255.0
    auto three =    Color(255.0 / 255.0, 153.0  / 255.0, 0.0    / 255.0); // 255.0 153.0 0.0
    auto four =     Color(0.0   / 255.0, 102.0  / 255.0, 0      / 255.0); // 0.0 102.0 0.0
    auto six =      Color(153.0 / 255.0, 102.0  / 255.0, 51.0   / 255.0); // 153.0 102.0 51.0
    
    Color returnColor;
    if (number == 1)
    {
        returnColor = one;
    }
    if (number == 2)
    {
        returnColor = two;
    }
    if (number == 3)
    {
        returnColor = three;
    }
    if (number == 4)
    {
        returnColor = four;
    }
    if (number == 6)
    {
        returnColor = six;
    }
    
    return returnColor;
}

vector<vec2> UbahnDataLoader::getCoordinatesFromString(string dataString)
{
    std::vector<vec2> result;
    
    size_t last = 0;
    size_t commaPos = 0;
    while ((commaPos = dataString.find(",", last)) != string::npos)
    {
        string coordsString = dataString.substr(last, commaPos - last);
        
        size_t spacePos = coordsString.find(" ");
        string firstCoordStr = coordsString.substr(0, spacePos);
        string secondCoordStr = coordsString.substr(spacePos+1, coordsString.length() - (spacePos+1));
        
        double firstCoord = stod(firstCoordStr);
        double secondCoord = stod(secondCoordStr);
        
        //result.push_back(vec2(firstCoord, secondCoord));
        result.push_back(vec2(secondCoord, firstCoord)); //~ the coordinates are actually inverted in the data file
        
        last = commaPos + 2; // +1 for the comma, +1 for the space after comma
    }
    
    return result;
}

vec2 UbahnDataLoader::getPositionFromString(string point)
{
    size_t firstBracket = point.find("(");
    size_t secondBracket = point.find(")");
    string coordsStr = point.substr(firstBracket + 1, (secondBracket - 1) - (firstBracket + 1));
    console() << "hey";
    
    auto tokens = Utils::tokenize(coordsStr, " ");
    auto xCoord = stod(tokens[0]);
    auto yCoord = stod(tokens[1]);
    return vec2(yCoord, xCoord);
    //return vec2(xCoord, yCoord);
}
