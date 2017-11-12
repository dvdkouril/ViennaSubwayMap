#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Log.h"

#include <fstream>

using namespace ci;
using namespace ci::app;
using namespace std;

struct UbahnLine {
    std::vector<vec2>   points;
    Color               lineColor;
    string              lineName;
    int                 lineNumber;
    gl::VboMeshRef      linePositionData;
};

class ViennaSubwayMapApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    gl::GlslProgRef loadShaders(std::string vsFilename, std::string fsFilename);
    
    vec2 computeDataMiddlePoint();
    
private:
    CameraPersp             mCamera;
    CameraUi                mCameraController;
    std::vector<UbahnLine*>  lines;
    
    vec2                    dataMiddlePoint;
    gl::GlslProgRef         simpleShader;
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

Color getLineColor(int number)
{
    auto one = Color(1, 0, 0);
    auto two = Color(0, 1, 0);
    auto three = Color(0, 0, 1);
    auto four = Color(1, 1, 0);
    auto six = Color(0, 1, 1);
    
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
        
        //result.push_back(vec2(firstCoord, secondCoord));
        result.push_back(vec2(secondCoord, firstCoord)); //~ the coordinates are actually inverted in the data file
        
        last = commaPos + 2; // +1 for the comma, +1 for the space after comma
    }
    
    return result;
}

vec2 ViennaSubwayMapApp::computeDataMiddlePoint()
{
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
    return midPoint;
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
            Color col = getLineColor(number);
            
            UbahnLine * subwayLine = new UbahnLine;
            subwayLine->points = coordinates;
            subwayLine->lineName = lineNumber;
            subwayLine->lineNumber = number;
            subwayLine->lineColor = col;
            lines.push_back(subwayLine);
            
            numOfLines++;
        }
        
        this->dataMiddlePoint = computeDataMiddlePoint();
    }
    
    for (auto& line : this->lines)
    {
        gl::VboMesh::Layout layout;
        layout.usage(GL_STATIC_DRAW).attrib(geom::POSITION, 2);
        
        for (auto &coord : line->points)
        {
            coord -= this->dataMiddlePoint;
        }
        
        line->linePositionData = gl::VboMesh::create(line->points.size(), GL_LINE_STRIP, {layout});
        line->linePositionData->bufferAttrib(geom::POSITION, line->points.size() * sizeof(vec2), line->points.data());
    }
    
    simpleShader = loadShaders("simple.vs", "simple.fs");
}

void ViennaSubwayMapApp::mouseDown( MouseEvent event )
{
}

void ViennaSubwayMapApp::update()
{
}

void ViennaSubwayMapApp::draw()
{
    //gl::clear( Color( 1, 1, 1 ) );
    gl::clear( Color( 0, 0, 0 ) );
    
    gl::setMatrices(mCamera);
    gl::ScopedGlslProg prog(simpleShader);
    gl::setDefaultShaderVars();
    for (auto line : lines)
    {
        vec4 colVec4 = vec4(line->lineColor.r, line->lineColor.g, line->lineColor.b, 1.0);
        simpleShader->uniform("lineColor", colVec4);
        gl::draw(line->linePositionData);
    }
}

gl::GlslProgRef ViennaSubwayMapApp::loadShaders(std::string vsFilename, std::string fsFilename) {
    // Shortcut for shader loading and error handling
    auto loadGlslProg = [ & ]( const gl::GlslProg::Format& format ) -> gl::GlslProgRef
    {
        string names = format.getVertexPath().string() + " + " +
        format.getFragmentPath().string();
        gl::GlslProgRef glslProg;
        try {
            glslProg = gl::GlslProg::create( format );
        } catch ( const Exception& ex ) {
            CI_LOG_EXCEPTION( names, ex );
            quit();
        }
        return glslProg;
    };
    
    DataSourceRef vertShader = loadAsset(vsFilename);
    DataSourceRef fragShader = loadAsset(fsFilename);
    
    gl::GlslProgRef shaderProgram = loadGlslProg(gl::GlslProg::Format().version(330)
                                                 .vertex(vertShader).fragment(fragShader));
    
    CI_LOG_D("All shaders loaded and compiled.");
    
    return shaderProgram;
    
}

CINDER_APP( ViennaSubwayMapApp, RendererGl, [](App::Settings* settings) {
    settings->setWindowSize(1280, 720);
})
