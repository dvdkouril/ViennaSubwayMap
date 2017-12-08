#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Log.h"

#include "CinderImGui.h"

#include <fstream>

#include "UbahnDataLoader.hpp"
#include "Utils.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

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
    string              name;
};

class ViennaSubwayMapApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    gl::GlslProgRef loadShaders(std::string vsFilename, std::string fsFilename);
    
    void loadLineDataFromFile(string filePath);
    void loadStationsDataFromFile(string filePath);
    
    vec2 computeDataMiddlePoint();
    
private:
    CameraPersp                     mCamera;
    CameraUi                        mCameraController;
    std::vector<UbahnLine*>         lines;
    std::map<size_t, UbahnLine*>    uBahnLines;
    std::vector<UbahnStation*>      stations;
    
    vec2                    dataMiddlePoint;
    gl::GlslProgRef         simpleShader;
    gl::GlslProgRef         simpleShaderStrip;
};

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

void ViennaSubwayMapApp::loadLineDataFromFile(string filePath)
{
    ifstream textFile(getAssetPath(filePath).c_str());
    if (textFile.is_open())
    {
        string line;
        size_t numOfLines = 0;
        while (getline(textFile, line))
        {
            string lineData = UbahnDataLoader::getOneLineDataInString(line);
            
            if (lineData.length() == 0) continue;
            
            vector<vec2> coordinates = UbahnDataLoader::getCoordinatesFromString(lineData);
            string lineNumber = UbahnDataLoader::getLineNumber(line);
            int number = stoi(lineNumber);
            Color col = UbahnDataLoader::getLineColor(number);
            
            UbahnLine * subwayLine = new UbahnLine;
            subwayLine->points = coordinates;
            subwayLine->lineName = lineNumber;
            subwayLine->lineNumber = number;
            subwayLine->lineColor = col;
            lines.push_back(subwayLine);
            
//            if (uBahnLines.count(number) == 0)
//            {
//                uBahnLines[number] = subwayLine;
//            } else {
//                // a.insert(a.end(), b.begin(), b.end());
//                //uBahnLines[number]->points.push_back(coordinates);
//                uBahnLines[number]->points.insert(uBahnLines[number]->points.end(), coordinates.begin(),
//                                                  coordinates.end());
//                                                  
//            }
            
            numOfLines++;
        }
        
        this->dataMiddlePoint = computeDataMiddlePoint();
    }
}

void ViennaSubwayMapApp::loadStationsDataFromFile(string filePath)
{
    ifstream textFile(getAssetPath(filePath).c_str());
    if (textFile.is_open())
    {
        string line;
        size_t numOfLines = 0;
        while (getline(textFile, line))
        {
            if (numOfLines == 0)
            {
                numOfLines++;
                continue; //~ skip the first line with headings
            }
            
            auto tokens = Utils::tokenize(line, ",");
            string point = tokens[2];
            string name = tokens[5];
            
            vec2 stationPosition = UbahnDataLoader::getPositionFromString(point);
            
            UbahnStation * station = new UbahnStation;
            station->name = name;
            station->position = stationPosition;
            
            stations.push_back(station);
            
            numOfLines++;
        }
    }
}


void ViennaSubwayMapApp::setup()
{
    ImGui::initialize();
    mCameraController = CameraUi(&mCamera, getWindow(), -1);
    
    loadLineDataFromFile("data/UBAHNOGD.csv");
    loadStationsDataFromFile("data/UBAHNHALTOGD.csv");
    
    //~ Load data to GPU buffer
    for (auto& line : this->lines)
    {
        for (auto &coord : line->points)
        {
            coord -= this->dataMiddlePoint;
        }
        
        gl::VboMesh::Layout layout;
        layout.usage(GL_STATIC_DRAW).attrib(geom::POSITION, 2);
        line->linePositionData = gl::VboMesh::create(line->points.size(), GL_LINE_STRIP, {layout});
        line->linePositionData->bufferAttrib(geom::POSITION, line->points.size() * sizeof(vec2), line->points.data());
        
        //~ Generate rectangular strip around the line
        std::vector<vec3> stripData;
        for (int i = 0; i < line->points.size() - 1; i++)
        {
            auto thisPoint = line->points[i];
            auto nextPoint = line->points[i + 1];
            
            vec2 direction2d = nextPoint - thisPoint;
            vec3 direction = vec3(direction2d.x, 0, direction2d.y);
            vec3 up = vec3(0, 1, 0);
            
            vec3 offsetVector = cross(normalize(direction), normalize(up));
            vec3 offsetVectorInv = -offsetVector;
            
            vec3 A = vec3(thisPoint.x, 0, thisPoint.y) + 0.001f * offsetVectorInv;
            vec3 B = vec3(thisPoint.x, 0, thisPoint.y) + 0.001f * offsetVector;
            vec3 C = vec3(nextPoint.x, 0, nextPoint.y) + 0.001f * offsetVectorInv;
            vec3 D = vec3(nextPoint.x, 0, nextPoint.y) + 0.001f * offsetVector;
            
            stripData.push_back(A);
            stripData.push_back(B);
            stripData.push_back(C);
            stripData.push_back(D);
        }
        
        gl::VboMesh::Layout layoutStrip;
        layoutStrip.usage(GL_STATIC_DRAW).attrib(geom::POSITION, 3);
        line->lineStripData = gl::VboMesh::create(stripData.size(), GL_TRIANGLE_STRIP, {layoutStrip});
        line->lineStripData->bufferAttrib(geom::POSITION, stripData.size() * sizeof(vec3), stripData.data());
        
    }
    
    //~ Shaders loading
    simpleShader = loadShaders("simple.vs", "simple.fs");
    simpleShaderStrip = loadShaders("simple-tristrip.vs", "simple-tristrip.fs");
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
    {
        gl::ScopedGlslProg prog(simpleShader);
        gl::setDefaultShaderVars();
        for (auto line : lines)
        {
            vec4 colVec4 = vec4(line->lineColor.r, line->lineColor.g, line->lineColor.b, 1.0);
            simpleShader->uniform("lineColor", colVec4);
            gl::draw(line->linePositionData);
        }
        gl::ScopedGlslProg progStrip(simpleShaderStrip);
        gl::setDefaultShaderVars();
        for (auto line : lines)
        {
            vec4 colVec4 = vec4(line->lineColor.r, line->lineColor.g, line->lineColor.b, 1.0);
            simpleShaderStrip->uniform("lineColor", colVec4);
            gl::draw(line->lineStripData);
        }
    }
    for (auto station : stations)
    {
        auto coord = station->position - this->dataMiddlePoint;
        gl::drawCube(vec3(coord.x * 1000.0, 0, coord.y * 1000.0), vec3(0.5));
    }
    
    //ImGui::Text("Hello, gabi!");
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

CINDER_APP( ViennaSubwayMapApp, RendererGl( RendererGl::Options().msaa(16)), [](App::Settings* settings) {
    settings->setWindowSize(1280, 720);
})
