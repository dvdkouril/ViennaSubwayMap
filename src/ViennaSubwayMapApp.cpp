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
    void adjustScale();
    
private:
    CameraPersp                     mCamera;
    CameraUi                        mCameraController;
    //std::vector<UbahnLine*>         lines;
    //std::map<size_t, UbahnLine*>    uBahnLines;
    //std::vector<UbahnStation*>      stations;
    
    //~ Data from Yun
    //std::vector<vec3>                                       yunStations;
    //std::map<std::string, UbahnStation*>                    uBahnStations;
    //std::map<std::string, std::vector<UbahnStation*>>       uBahnLines;
    std::vector<UbahnLine*>                 lines;
    //gl::VboMeshRef                                          linesVbos[5];
    
    
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
        for (auto station : line->stations)
        {
            auto coord = station->position;
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

void ViennaSubwayMapApp::adjustScale()
{
    for (auto line : lines)
    {
        for (auto & station : line->stations)
        {
            station->position -= this->dataMiddlePoint;
            station->position *= 1000.0;
        }
    }
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
    
    this->lines = UbahnDataLoader::loadDataFromYun("data/vienna-ubahn.txt");
    
    this->dataMiddlePoint = computeDataMiddlePoint();
    this->adjustScale();
    
    for (auto line : lines)
    {
        std::vector<vec3> points;
        for (auto station : line->stations)
        {
            vec2 offsetted = station->position;
            vec3 pointPosition = vec3(offsetted.x, station->height, offsetted.y);
            points.push_back(pointPosition);
        }
        
        //~ Generate rectangular strip around the line
        std::vector<vec3> stripData;
        for (int i = 0; i < line->stations.size() - 1; i++)
        {
            vec3 thisPoint;
            thisPoint.x = line->stations[i]->position.x;
            thisPoint.y = line->stations[i]->height;
            thisPoint.z = line->stations[i]->position.y;
            vec3 nextPoint;
            nextPoint.x = line->stations[i + 1]->position.x;
            nextPoint.y = line->stations[i + 1]->height;
            nextPoint.z = line->stations[i + 1]->position.y;
            auto direction = nextPoint - thisPoint;
            /*auto thisPoint = line->stations[i]->position;
            auto nextPoint = line->stations[i + 1]->position;
            
            vec2 direction2d = nextPoint - thisPoint;
            vec3 direction = vec3(direction2d.x, 0, direction2d.y);*/
            vec3 up = vec3(0, 1, 0);
            
            vec3 offsetVector = cross(normalize(direction), normalize(up));
            vec3 offsetVectorInv = -offsetVector;
            
            /*vec3 A = vec3(thisPoint.x, 0, thisPoint.y) + 0.5f * offsetVectorInv;
            vec3 B = vec3(thisPoint.x, 0, thisPoint.y) + 0.5f * offsetVector;
            vec3 C = vec3(nextPoint.x, 0, nextPoint.y) + 0.5f * offsetVectorInv;
            vec3 D = vec3(nextPoint.x, 0, nextPoint.y) + 0.5f * offsetVector;*/
            
            vec3 A = thisPoint + 0.5f * offsetVectorInv;
            vec3 B = thisPoint + 0.5f * offsetVector;
            vec3 C = nextPoint + 0.5f * offsetVectorInv;
            vec3 D = nextPoint + 0.5f * offsetVector;
            
            stripData.push_back(A);
            stripData.push_back(B);
            stripData.push_back(C);
            stripData.push_back(D);
        }
        
        gl::VboMesh::Layout layout;
        layout.usage(GL_STATIC_DRAW).attrib(geom::POSITION, 3);
        
        line->lineVertexData = gl::VboMesh::create(points.size(), GL_LINE_STRIP, {layout});
        line->lineVertexData->bufferAttrib(geom::POSITION, points.size() * sizeof(vec3), points.data());
        
        line->stripVertexData = gl::VboMesh::create(stripData.size(), GL_TRIANGLE_STRIP, {layout});
        line->stripVertexData->bufferAttrib(geom::POSITION, stripData.size() * sizeof(vec3), stripData.data());
        
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
        gl::ScopedGlslProg progStrip(simpleShaderStrip);
        gl::setDefaultShaderVars();
        for (auto line : lines)
        {
            //vec4 colVec4 = vec4(line->lineColor.r, line->lineColor.g, line->lineColor.b, 1.0);
            vec4 colVec = vec4(line->color.r, line->color.g, line->color.b, 1.0);
            simpleShaderStrip->uniform("lineColor", colVec);
            gl::draw(line->lineVertexData);
            gl::draw(line->stripVertexData);
            
            for (auto station : line->stations)
            {
                //auto pos = station->position - this->dataMiddlePoint;
                auto pos = station->position;
                auto height = station->height;
                
                gl::drawCube(vec3(pos.x, height, pos.y), vec3(1.0));
            }
            //gl::drawCube(vec3(0,0,0), vec3(1.0));
        }
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

CINDER_APP( ViennaSubwayMapApp, RendererGl( RendererGl::Options().msaa(16)), [](App::Settings* settings) {
    settings->setWindowSize(1280, 720);
})
