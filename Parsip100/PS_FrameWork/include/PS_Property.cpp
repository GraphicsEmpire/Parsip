#include "PS_Property.h"
#include "PS_ErrorManager.h"

namespace PS{


bool Property::convertTo(PROPERTYTYPE destType)
{
    if(value.valStr.length() == 0)
        return false;
    if(destType == ttStr)
        return false;

    this->dtype = destType;
    switch(destType)
    {
    case(ttInt):
    {
        value.valInt  = atoi(value.valStr.cptr());
    }
    break;
    case(ttFLT):
    {
        value.valFloat = atof(value.valStr.cptr());
    }
    break;
    case(ttVec2):
    {
        sscanf(value.valStr.cptr(), "%f %f", &value.valVec2.x, &value.valVec2.y);
    }
    break;
    case(ttVec3):
    {
        sscanf(value.valStr.cptr(), "%f %f %f",
               &value.valVec3.x, &value.valVec3.y, &value.valVec3.z);
    }
    break;
    case(ttVec4):
    {
        sscanf(value.valStr.cptr(), "%f %f %f %f",
               &value.valVec4.x, &value.valVec4.y, &value.valVec4.z, &value.valVec4.w);
    }
    break;

    }

    return true;
}



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

int PropertyList::FindProperty(const PropertyList& propList,
                        const char* lpName,
                        PROPERTYTYPE expectedType,
                        int fromIdx)
{
    if(propList.size() == 0) return -1;
    if(fromIdx < 0 || fromIdx >= propList.size())
        fromIdx = 0;

    if(expectedType == ttUnknown)
    {
        for(int i=fromIdx; i<(int)propList.size(); i++)
        {
            if(propList[i].name == DAnsiStr(lpName))
                return i;
        }
    }
    else
    {
        for(int i=fromIdx; i<(int)propList.size(); i++)
        {
            if(propList[i].dtype == expectedType)
            {
                if(propList[i].name == DAnsiStr(lpName))
                    return i;
            }
        }
    }

    return -1;
}



}
