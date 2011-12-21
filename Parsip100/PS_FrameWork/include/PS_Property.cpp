#include "PS_Property.h"
#include "PS_ErrorManager.h"

namespace PS{

DAnsiStr Property::asString(int floatPrecision) const
{
    DAnsiStr strValue;

    switch(dtype)
    {
    case(ttInt):
        strValue = printToAStr("%d", this->value.valInt);
        break;
    case(ttFLT):
        strValue = printToAStr("%.2f", this->value.valFloat);
        break;
    case(ttVec2):
    {
        vec2f v = this->value.valVec2;
        strValue = printToAStr("%.2f %.2f", v.x, v.y);
    }
    break;
    case(ttVec3):
    {
        vec3f v = this->value.valVec3;
        strValue = printToAStr("%.2f %.2f %.2f", v.x, v.y, v.z);
    }
    break;
    case(ttVec4):
    {
        vec4f v = this->value.valVec4;
        strValue = printToAStr("%.2f %.2f %.2f %.2f", v.x, v.y, v.z, v.w);
    }
    break;
    default:
        strValue = "Unknown";
    }

    return strValue;
}

void PropertyList::add(int val, const char* lpName)
{
    Property p;
    p.dtype = ttInt;
    p.value.valInt = val;
    p.name = DAnsiStr(lpName);
    this->push_back(p);
}

void PropertyList::add(float val, const char* lpName)
{
    Property p;
    p.dtype = ttFLT;
    p.value.valFloat = val;
    p.name = DAnsiStr(lpName);
    this->push_back(p);
}

void PropertyList::add(const vec2f& val, const char* lpName)
{
    Property p;
    p.dtype = ttVec2;
    p.value.valVec2 = val;
    p.name = DAnsiStr(lpName);
    this->push_back(p);
}

void PropertyList::add(const vec3f& val, const char* lpName)
{
    Property p;
    p.dtype = ttVec3;
    p.value.valVec3 = val;
    p.name = DAnsiStr(lpName);
    this->push_back(p);
}

void PropertyList::add(const vec4f& val, const char* lpName)
{
    Property p;
    p.dtype = ttVec4;
    p.value.valVec4 = val;
    p.name = DAnsiStr(lpName);
    this->push_back(p);
}

void PropertyList::add(const DAnsiStr& val, const char* lpName)
{
    Property p;
    p.dtype = ttStr;
    p.value.valStr = val;
    p.name = DAnsiStr(lpName);
    this->push_back(p);
}

/*
Property& PropertyList::findProperty(const char* lpName,
                                     PROPERTYTYPE expectedType,
                                     int fromIdx)

{
    Property unknown;
    if(fromIdx < 0 || fromIdx >= this->size())
        fromIdx = 0;

    for(int i=fromIdx; i<(int)this->size(); i++)
    {
        if(this->at(i).dtype == expectedType)
        {
            if(this->at(i).name == DAnsiStr(lpName))
                return this->at(i);
        }
    }

    return unknown;
}
*/
int PropertyList::FindProperty(const PropertyList& propList,
                        const char* lpName,
                        PROPERTYTYPE expectedType,
                        int fromIdx)
{
    if(propList.size() == 0) return -1;
    if(fromIdx < 0 || fromIdx >= propList.size())
        fromIdx = 0;

    for(int i=fromIdx; i<(int)propList.size(); i++)
    {
        if(propList[i].dtype == expectedType)
        {
            if(propList[i].name == DAnsiStr(lpName))
                return i;
        }
    }

    return -1;
}



}
