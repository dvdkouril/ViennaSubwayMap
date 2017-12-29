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

//std::vector<UbahnLine*> UbahnDataLoader::loadLineDataFromFile(string filePath)
//{
//    std::vector<UbahnLine*> lines;
//    
//    ifstream textFile(getAssetPath(filePath).c_str());
//    if (textFile.is_open())
//    {
//        string line;
//        size_t numOfLines = 0;
//        while (getline(textFile, line))
//        {
//            string lineData = UbahnDataLoader::getOneLineDataInString(line);
//            
//            if (lineData.length() == 0) continue;
//            
//            vector<vec2> coordinates = UbahnDataLoader::getCoordinatesFromString(lineData);
//            string lineNumber = UbahnDataLoader::getLineNumber(line);
//            int number = stoi(lineNumber);
//            Color col = UbahnDataLoader::getLineColor(number);
//            
//            UbahnLine * subwayLine = new UbahnLine;
//            subwayLine->points = coordinates;
//            subwayLine->lineName = lineNumber;
//            subwayLine->lineNumber = number;
//            subwayLine->lineColor = col;
//            lines.push_back(subwayLine);
//            
//            numOfLines++;
//        }
//    }
//    
//    return lines;
//}

//std::vector<UbahnStation*> UbahnDataLoader::loadStationsDataFromFile(string filePath)
//{
//    std::vector<UbahnStation*> stations;
//    ifstream textFile(getAssetPath(filePath).c_str());
//    if (textFile.is_open())
//    {
//        string line;
//        size_t numOfLines = 0;
//        while (getline(textFile, line))
//        {
//            if (numOfLines == 0)
//            {
//                numOfLines++;
//                continue; //~ skip the first line with headings
//            }
//            
//            auto tokens = Utils::tokenize(line, ",");
//            string point = tokens[2];
//            string name = tokens[5];
//            
//            vec2 stationPosition = UbahnDataLoader::getPositionFromString(point);
//            
//            UbahnStation * station = new UbahnStation;
//            station->name = name;
//            station->position = stationPosition;
//            
//            stations.push_back(station);
//            
//            numOfLines++;
//        }
//    }
//    
//    return stations;
//}

std::vector<UbahnLine*> UbahnDataLoader::loadDataFromYun(string filePath)
{
    std::vector<UbahnLine*> lines;
    ifstream textFile(getAssetPath(filePath).c_str());
    if (textFile.is_open())
    {
        string line;
        size_t numOfLines = 0;
        std::string currentLineName;
        UbahnLine * currentLine;
        std::string currentColor;
        
        while (getline(textFile, line))
        {
            cout << line << endl;
            
            auto tokens = Utils::tokenize(line, ",");
            if (tokens[0] == "Line")
            {
                //stations.clear();
                currentLineName = tokens[1];

                UbahnLine * newLine = new UbahnLine();
                newLine->name = currentLineName;
                currentLine = newLine;
                lines.push_back(newLine);
                continue;
            }
            
            if (tokens[0] == "Color")
            {
                auto rStr = tokens[1];
                auto gStr = tokens[2];
                auto bStr = tokens[3];
                
                auto r = stod(rStr);
                auto g = stod(gStr);
                auto b = stod(bStr);
                Color col(r / 255.0, g / 255.0, b / 255.0);
                currentLine->color = col;
                continue;
            }
            
            if (tokens[0] == "#")
            { // stations data
                auto name = tokens[1];
                auto xStr = tokens[2];
                auto yStr = tokens[3];
                auto hStr = tokens[4];
                
                auto x = stod(xStr);
                auto y = stod(yStr);
                auto h = stod(hStr);
                
                UbahnStation * station = new UbahnStation();
                station->name = name;
                station->position = vec2(x, y);
                station->height = h * 2.0f;
                
                currentLine->stations.push_back(station);
            }
            
            if (tokens[0] == "~")
            { // lines data
                auto firstStation = tokens[1];
                auto secondStation = tokens[2];
                auto weight = tokens[3];
                
                //~ Actually, do nothing
            }
            
            numOfLines++;
        }
    }
    return lines;

}

//bool UbahnDataLoader::loadDataFromYun(string filePath,
//                                      std::map<std::string, UbahnStation*> & allStations,
//                                      std::map<std::string, std::vector<UbahnStation*>> & lines)
//{
//    //std::vector<vec3> stations;
//    ifstream textFile(getAssetPath(filePath).c_str());
//    if (textFile.is_open())
//    {
//        string line;
//        size_t numOfLines = 0;
//        std::string currentLine;
//        std::string currentColor;
//        
//        //std::vector<UbahnStation*> stations;
//        //std::map<std::string, UbahnStation*> allStations;
//        //std::map<std::string, std::vector<UbahnStation*>> lines;
//        allStations.clear();
//        lines.clear();
//        while (getline(textFile, line))
//        {
//            cout << line << endl;
//            
//            auto tokens = Utils::tokenize(line, ",");
//            if (tokens[0] == "Line")
//            {
//                //stations.clear();
//                currentLine = tokens[1];
//                continue;
//            }
//            
//            if (tokens[0] == "Color")
//            {
//                
//                continue;
//            }
//            
//            if (tokens[0] == "#")
//            { // stations data
//                auto name = tokens[1];
//                auto xStr = tokens[2];
//                auto yStr = tokens[3];
//                auto hStr = tokens[4];
//                
//                auto x = stod(xStr);
//                auto y = stod(yStr);
//                auto h = stod(hStr);
//                
//                
//                UbahnStation * station = new UbahnStation();
//                station->name = name;
//                station->position = vec2(x, y);
//                station->height = h * 0.01;
//                
//                //stations.push_back(station);
//                allStations[station->name] = station; // this is exactly what i don't want,
//                                                      // I need to store all the physical stations,
//                                                      // that are under one name
//                
//                lines[currentLine].push_back(station);
//            }
//            
//            if (tokens[0] == "~")
//            { // lines data
//                auto firstStation = tokens[1];
//                auto secondStation = tokens[2];
//                auto weight = tokens[3];
//                
//                //~ Actually, do nothing
//            }
//            
//            numOfLines++;
//        }
//    }
//    return true;
//}



