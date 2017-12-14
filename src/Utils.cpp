//
//  Utils.cpp
//  ViennaSubwayMap
//
//  Created by David Kouril on 08/12/2017.
//
//

#include "Utils.hpp"
#include "cinder/Log.h"
#include "cinder/app/App.h"

std::vector<string> Utils::tokenize(string line, string breakAt)
{
    size_t last = 0;
    size_t found = 0;
    std::vector<string> tokens;
    while ((found = line.find(breakAt, last)) != string::npos )
    {
        auto token = line.substr(last, found - last);
        tokens.push_back(token);
        last = found + 1;
    }
    
    auto token = line.substr(last, line.length() - last);
    tokens.push_back(token);
    return tokens;
}

//gl::GlslProgRef Utils::loadShaders(std::string vsFilename, std::string fsFilename) {
//    // Shortcut for shader loading and error handling
//    auto loadGlslProg = [ & ]( const gl::GlslProg::Format& format ) -> gl::GlslProgRef
//    {
//        string names = format.getVertexPath().string() + " + " +
//        format.getFragmentPath().string();
//        gl::GlslProgRef glslProg;
//        try {
//            glslProg = gl::GlslProg::create( format );
//        } catch ( const Exception& ex ) {
//            CI_LOG_EXCEPTION( names, ex );
//            quit();
//        }
//        return glslProg;
//    };
//    
//    DataSourceRef vertShader = loadAsset(vsFilename);
//    DataSourceRef fragShader = loadAsset(fsFilename);
//    
//    gl::GlslProgRef shaderProgram = loadGlslProg(gl::GlslProg::Format().version(330)
//                                                 .vertex(vertShader).fragment(fragShader));
//    
//    CI_LOG_D("All shaders loaded and compiled.");
//    
//    return shaderProgram;
//    
//}
