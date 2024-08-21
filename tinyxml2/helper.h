
#pragma once

#include "stringex.h"

namespace su
{
namespace tinyxml2
{

inline std::string getAttributeString(const ::tinyxml2::XMLElement* element, const std::string& name, const std::string& def, bool isLower)
{
    std::string out = (element->Attribute(name.c_str())) ? element->Attribute(name.c_str()) : def;
    return isLower ? su::String_tolower(out) : out;
}

inline bool getAttributeBool(const ::tinyxml2::XMLElement* element, const std::string& name, bool def)
{
    std::string out = su::String_tolower((element->Attribute(name.c_str())) ? element->Attribute(name.c_str()) : (def ? "true" : "false"));

    if (out == "true" || out == "1")
    {
        return true;
    }
    else if (out == "false" || out == "0")
    {
        return false;
    }

    return !!atoi(out.c_str());
}


}
}
