
#ifndef FUNC_H
#define FUNC_H

#include <string>
#include <sstream>
#include <map>
#include <iostream>

#include <osgAnimation/Sampler>

enum TagName { NotDefined,
                    Body,
                    Seq,
                    Par,
                    Img,
                    Model,
                    Text,
                    Query,
                    Digital_Image}; 

inline float convertToInt(const char* s)
    {
    if(s) {
       std::istringstream i(s);
       int x;
       if ((i >> x))
            return x;
        else
            return 0;
         //throw BadConversion("convertToDouble(\"" + s + "\")");
    } else 
        return 0;
}

inline float convertToInt(const std::string &s)
    {
    if(!s.empty()) {
       std::istringstream i(s);
       int x;
       if ((i >> x))
            return x;
        else
         throw 1;
    } else 
        throw 2;
}


inline std::string convertToString(const float s)
{

    std::ostringstream i;
    if( i << s)
        return i.str();
    else
        return "";        
}


inline std::string convertToString(const char* s)
{

    if(s) {
        return std::string(s);
    } else {
        return std::string();
    }
        
}


inline float convertToFloat(const char* s)
 {
    if(s) {
        std::istringstream i(s);
        float x;
        if ((i >> x))
            return x;
        else
            return 0.0f;
    } else
        return 0.0f;
 }


inline float convertToFloat(std::string&  s) 
{
    std::istringstream i(s);
    float x;
    if ((i >> x))
        return x;
    else
        return 0.0f;
}


inline std::vector<std::string> split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}



inline std::vector<std::string> split2(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}


inline osg::Vec2f convertToVec2f(const char* s) 
{

    if(s) {
        char delimiter = ',';    
        std::vector<std::string> list = split2(s,delimiter);
        return osg::Vec2f(convertToFloat(list[0]), convertToFloat(list[1]));
    } else {
        
        return osg::Vec2f(0.0f, 0.0f);
    }
}

inline bool sortKeyframes(const osgAnimation::FloatKeyframe& k1, const osgAnimation::FloatKeyframe& k2) {

    return k1.getTime() < k2.getTime();
}

#endif
