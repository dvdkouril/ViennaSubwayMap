#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include <fstream>

using namespace ci;
using namespace ci::app;
using namespace std;

struct UbahnLine {
    std::vector<vec2>   points;
    //vec2                dataMiddle;
    string              lineName;
    int                 lineNumber;
};

class ViennaSubwayMapApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
private:
    CameraPersp             mCamera;
    CameraUi                mCameraController;
    std::vector<UbahnLine*>  lines;
    
    vec2                    dataMiddlePoint;
};

string getOneLineDataInString(string line)
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

string getLineNumber(string line)
{
    size_t firstApost = line.find("\"");
    size_t secondApost = line.find("\"", firstApost + 1);
    size_t lineNumberPos = secondApost + 1 + 1;
    return line.substr(lineNumberPos, 1);
}

vector<vec2> getCoordinatesFromString(string dataString)
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
        
        result.push_back(vec2(firstCoord, secondCoord));
        
        last = commaPos + 2; // +1 for the comma, +1 for the space after comma
    }
    
    return result;
}

void ViennaSubwayMapApp::setup()
{
    mCameraController = CameraUi(&mCamera, getWindow(), -1);
    
    ifstream textFile(getAssetPath("data/UBAHNOGD.csv").c_str());
    string wholeFile;
    if (textFile.is_open())
    {
        string line;
        size_t numOfLines = 0;
        while (getline(textFile, line))
        {
            string lineData = getOneLineDataInString(line);
            
            if (lineData.length() == 0) continue;
            
            vector<vec2> coordinates = getCoordinatesFromString(lineData);
            string lineNumber = getLineNumber(line);
            int number = stoi(lineNumber);
            
            UbahnLine * subwayLine = new UbahnLine;
            subwayLine->points = coordinates;
            subwayLine->lineName = lineNumber;
            subwayLine->lineNumber = number;
            lines.push_back(subwayLine);
            
            numOfLines++;
        }
        
        vec2 minCoord(500,500);
        vec2 maxCoord(0,0);
        for (auto line : lines)
        {
            for (auto coord : line->points)
            {
                if (coord.x < minCoord.x) {
                    minCoord.x = coord.x;
                }
                if (coord.y < minCoord.y) {
                    minCoord.y = coord.y;
                }
                if (coord.x > maxCoord.x) {
                    maxCoord.x = coord.x;
                }
                if (coord.y > maxCoord.y) {
                    maxCoord.y = coord.y;
                }
            }
        }
        vec2 difference = maxCoord - minCoord;
        vec2 midPoint = minCoord + (difference / 2.0f);
        this->dataMiddlePoint = midPoint;
    }
}

void ViennaSubwayMapApp::mouseDown( MouseEvent event )
{
}

void ViennaSubwayMapApp::update()
{
}

void ViennaSubwayMapApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    gl::setMatrices(mCamera);
    
    auto one = Color(1, 0, 0);
    auto two = Color(0, 1, 0);
    auto three = Color(0, 0, 1);
    auto four = Color(1, 1, 0);
    auto six = Color(0, 1, 1);
    
    for (auto line : lines)
    {
        if (line->lineNumber == 1)
        {
            gl::color(one);
        }
        if (line->lineNumber == 2)
        {
            gl::color(two);
        }
        if (line->lineNumber == 3)
        {
            gl::color(three);
        }
        if (line->lineNumber == 4)
        {
            gl::color(four);
        }
        if (line->lineNumber == 6)
        {
            gl::color(six);
        }
        for (auto coord : line->points)
        {
            coord -= this->dataMiddlePoint;
            gl::drawCube(vec3(coord.x * 1000.0, 0, coord.y * 1000.0), vec3(0.1));
        }
    }
    //gl::drawCube(vec3(0), vec3(1));
}

CINDER_APP( ViennaSubwayMapApp, RendererGl, [](App::Settings* settings) {
    settings->setWindowSize(1280, 720);
})
