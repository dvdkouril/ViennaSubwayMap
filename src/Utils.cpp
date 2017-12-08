//
//  Utils.cpp
//  ViennaSubwayMap
//
//  Created by David Kouril on 08/12/2017.
//
//

#include "Utils.hpp"

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
