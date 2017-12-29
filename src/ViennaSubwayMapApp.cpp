#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"
#include "cinder/Log.h"
#include "cinder/Easing.h"

#include "CinderImGui.h"

#include <fstream>

#include "UbahnDataLoader.hpp"
#include "Utils.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class ViennaSubwayMapApp : public App {
  public:
	void setup() override;
	//void mouseDown( MouseEvent event ) override;
    //void mouseMove( MouseEvent event ) override;
    void mouseUp( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void mouseWheel( MouseEvent event ) override;
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
    //std::map<size_t, UbahnLine*>    uBahnLines;
    std::vector<UbahnStation*>      stations;
    
    //~ Data from Yun
    std::vector<vec3>                                       yunStations;
    std::map<std::string, UbahnStation*>                    uBahnStations;
    std::map<std::string, std::vector<UbahnStation*>>       uBahnLines;
    gl::VboMeshRef                                          linesVbos[5];
    
    
    //bool                    mouseIsDown;
    bool                    draggingNow;
    vec2                    lastMousePosition;
    
    float                   lastFrameTime;
    // zooming:
    vec3                    cameraTargetPosition;
    vec3                    cameraStartPosition;
    float                   cameraAnimDurationMs;
    float                   cameraAnimT;
    // camera rotation:
    quat                    cameraTargetRotation;
    quat                    cameraStartRotation;
    float                   cameraRotDurationMs;
    float                   cameraRotT;
    
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

void ViennaSubwayMapApp::setup()
{
    ImGui::initialize();
    mCamera.lookAt(vec3(10, 10, 0), vec3(0,0,0));
    mCameraController = CameraUi(&mCamera, getWindow(), -1);
    
    draggingNow = false;
    cameraAnimDurationMs = 2000;
    cameraRotDurationMs = 1000;
    lastFrameTime = 0.0f;
    cameraStartPosition = mCamera.getEyePoint();
    cameraTargetPosition = cameraStartPosition;
    cameraStartRotation = mCamera.getOrientation();
    cameraTargetRotation = cameraStartRotation;
    
    this->lines = UbahnDataLoader::loadLineDataFromFile("data/UBAHNOGD.csv");
    this->dataMiddlePoint = computeDataMiddlePoint();
    this->stations = UbahnDataLoader::loadStationsDataFromFile("data/UBAHNHALTOGD.csv");
    
    UbahnDataLoader::loadDataFromYun("data/vienna-ubahn.txt", uBahnStations, uBahnLines);
    
    int i = 0;
    for (auto pair : uBahnLines)
    {
        auto line = pair.second;
        std::vector<vec3> points;
        for (auto station : line)
        {
            vec2 offsetted = station->position - this->dataMiddlePoint;
            vec3 pointPosition = vec3(offsetted.x, station->height, offsetted.y);
            points.push_back(pointPosition);
        }
        
        gl::VboMesh::Layout layout;
        layout.usage(GL_STATIC_DRAW).attrib(geom::POSITION, 3);
        linesVbos[i] = gl::VboMesh::create(points.size(), GL_LINE_STRIP, {layout});
        linesVbos[i]->bufferAttrib(geom::POSITION, points.size() * sizeof(vec3), points.data());
        i++;
    }
    
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
            
            vec3 A = vec3(thisPoint.x, 0, thisPoint.y) + 0.0005f * offsetVectorInv;
            vec3 B = vec3(thisPoint.x, 0, thisPoint.y) + 0.0005f * offsetVector;
            vec3 C = vec3(nextPoint.x, 0, nextPoint.y) + 0.0005f * offsetVectorInv;
            vec3 D = vec3(nextPoint.x, 0, nextPoint.y) + 0.0005f * offsetVector;
            
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

void ViennaSubwayMapApp::mouseUp( MouseEvent event )
{
    //mouseIsDown = false;
    draggingNow = false;
    cout << "mouseUp" << endl;
}

void ViennaSubwayMapApp::mouseWheel(MouseEvent event)
{
    float incr = event.getWheelIncrement();
    
    auto v = mCamera.getViewDirection();
    auto currentPos = mCamera.getEyePoint();
    cameraStartPosition = currentPos;
    cameraTargetPosition = currentPos + 10.0f * incr * v;
    cameraAnimT = 0.0f;
}

void ViennaSubwayMapApp::mouseDrag(MouseEvent event)
{
    ivec2 posNow;
    ivec2 posBefore;
    if (draggingNow)
    {
        posNow = event.getPos();
        posBefore = lastMousePosition;
        lastMousePosition = posNow;
        auto diff = posNow - posBefore;
        
        quat camRotation = mCamera.getOrientation();
        quat yaw = angleAxis((float)(diff.x * toRadians(1.0f)), vec3(0, 1, 0) * camRotation);
        //quat pitch = angleAxis((float)(diff.y * toRadians(1.0f)), vec3(1, 0, 0) * camRotation);
        quat pitch = angleAxis((float)(diff.y * toRadians(1.0f)), vec3(0, 0, 1) * camRotation);
        cameraStartRotation = mCamera.getOrientation();
        cameraTargetRotation = mCamera.getOrientation() * yaw * pitch;
        cameraRotT = 0.0f;
        //mCamera.setOrientation(camRotation * yaw * pitch);
    }
    else {
        draggingNow = true;
        lastMousePosition = event.getPos();
    }
}

void ViennaSubwayMapApp::update()
{
//    double elapsedMs = getElapsedSeconds() * 1000;
//    float frameTime = elapsedMs - lastFrameTime;
//    lastFrameTime = elapsedMs;
//    
//    //~ zoom update
//    cameraAnimT += frameTime;
//    float t = cameraAnimT / cameraAnimDurationMs;
//    t = clamp(t, 0.0f, 1.0f);
//    vec3 newPos = lerp(cameraStartPosition, cameraTargetPosition, easeOutExpo(t));
//    mCamera.setEyePoint(newPos);
//    
//    //~ camera rotation update
//    cameraRotT += frameTime;
//    t = cameraRotT / cameraRotDurationMs;
//    t = clamp(t, 0.0f, 1.0f);
//    quat newRot = glm::slerp(cameraStartRotation, cameraTargetRotation, easeOutExpo(t));
//    mCamera.setOrientation(newRot);
}

void ViennaSubwayMapApp::draw()
{
    //gl::clear( Color( 1, 1, 1 ) );
    gl::clear( Color( 0.1, 0.1, 0.15 ) );
    
    gl::setMatrices(mCamera);
    {
        //~ Simple Line rendering
//        gl::ScopedGlslProg prog(simpleShader);
//        gl::setDefaultShaderVars();
//        for (auto line : lines)
//        {
//            vec4 colVec4 = vec4(line->lineColor.r, line->lineColor.g, line->lineColor.b, 1.0);
//            simpleShader->uniform("lineColor", colVec4);
//            gl::draw(line->linePositionData);
//        }
        //~ Thick Line rendering
//        gl::ScopedGlslProg progStrip(simpleShaderStrip);
//        gl::setDefaultShaderVars();
//        for (auto line : lines)
//        {
//            vec4 colVec4 = vec4(line->lineColor.r, line->lineColor.g, line->lineColor.b, 1.0);
//            simpleShaderStrip->uniform("lineColor", colVec4);
//            gl::draw(line->lineStripData);
//        }
//        gl::ScopedGlslProg prog2(simpleShader);
//        gl::setDefaultShaderVars();
//        for (auto line : lines)
//        {
//            vec4 colVec4 = vec4(line->lineColor.r, line->lineColor.g, line->lineColor.b, 1.0);
//            simpleShader->uniform("lineColor", colVec4);
//            gl::draw(line->linePositionData);
//        }
        
        gl::ScopedGlslProg progStrip(simpleShaderStrip);
        gl::setDefaultShaderVars();
        for (int i = 0; i < 5; i++)
        {
            auto line = linesVbos[i];
            //vec4 colVec4 = vec4(line->lineColor.r, line->lineColor.g, line->lineColor.b, 1.0);
            simpleShaderStrip->uniform("lineColor", vec4(1.0, 0.0, 0.0, 1.0));
            gl::draw(line);
        }
    }
//    for (auto station : stations)
//    {
//        auto coord = station->position - this->dataMiddlePoint;
//        gl::drawCube(vec3(coord.x * 1000.0, 0, coord.y * 1000.0), vec3(0.5));
//    }
    
    for (auto station : uBahnStations)
    {
        auto st = station.second;
        auto pos = st->position - this->dataMiddlePoint;
        auto height = st->height;
        
        gl::drawCube(vec3(pos.x * 1000.0, height * 1000.0, pos.y * 1000.0), vec3(1.0));
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
