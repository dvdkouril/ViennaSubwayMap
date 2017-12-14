//
//  Utils.hpp
//  ViennaSubwayMap
//
//  Created by David Kouril on 08/12/2017.
//
//

#ifndef Utils_hpp
#define Utils_hpp

#include <stdio.h>
#include "cinder/gl/gl.h"

using namespace std;
using namespace ci::app;
using namespace ci;

class Utils
{
public:
    static std::vector<string> tokenize(string line, string breakAt);
    //static gl::GlslProgRef loadShaders(std::string vsFilename, std::string fsFilename);
};

#endif /* Utils_hpp */
