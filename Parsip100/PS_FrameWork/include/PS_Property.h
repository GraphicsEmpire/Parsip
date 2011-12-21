#ifndef PS_PROPERTY_H
#define PS_PROPERTY_H
#include <vector>
#include "PS_String.h"
#include "PS_Vector.h"
#include <assert.h>
#include <exception>

using namespace std;
using namespace PS::MATH;

namespace PS{

class TypeException : public std::exception
{
 public:
    const char* what() const throw() { return "Unknown Type"; }

    static void OnUnknownType()
    {
        throw TypeException();
    }
};

typedef enum PROPERTYTYPE{ttUnknown, ttInt, ttFLT, ttVec2, ttVec3, ttVec4, ttStr};

class Property{
public:
    //enum PROPERTYTYPE{ttUnknown, ttInt, ttFLT, ttVec2, ttVec3, ttVec4, ttStr};

    Property()
    {
        this->name = "empty";
        this->dtype = ttUnknown;
    }

    Property(const char* chrName, int type, const char* fromString)
    {
        this->name = DAnsiStr(chrName);
        this->dtype = (PROPERTYTYPE)type;
    }

    ~Property() {}

    int valInt() const
    {
        assert(dtype == ttInt);
        return value.valInt;
    }

    float valFloat() const {
        assert(dtype == ttFLT);
        return value.valFloat;
    }

    vec2f valVec2() const {
        assert(dtype == ttVec2);
        return value.valVec2;
    }

    vec3f valVec3() const {
        assert(dtype == ttVec3);
        return value.valVec3;
    }

    vec4f valVec4() const {
        assert(dtype == ttVec4);
        return value.valVec4;

    }
    DAnsiStr valString() const {
        assert(dtype == ttStr);
        return value.valStr;
    }

    DAnsiStr asString(int floatPrecision = 2) const;

    operator const int () const {return this->valInt();}
    operator const float () const {return this->valFloat();}
    operator const vec2f () const {return this->valVec2();}
    operator const vec3f () const {return this->valVec3();}
    operator const vec4f () const {return this->valVec4();}
    operator const char* () const {return this->valString().cptr();}
public:
   DAnsiStr name;
   PROPERTYTYPE dtype;
   struct PROPERTYVALUE{
       int valInt;
       float valFloat;
       vec2f valVec2;
       vec3f valVec3;
       vec4f valVec4;
       DAnsiStr valStr;
   }value;
};

/*!
  *
  *
  */
class PropertyList : public vector<Property>
{
public:
    PropertyList() {}
    ~PropertyList() {this->resize(0);}

    void add(int val, const char* lpName = "int");
    void add(float val, const char* lpName = "float");
    void add(const vec2f& val, const char* lpName = "vec2");
    void add(const vec3f& val, const char* lpName = "vec3");
    void add(const vec4f& val, const char* lpName = "vec4");
    void add(const DAnsiStr& val, const char* lpName = "string");

    static int FindProperty(const PropertyList& propList,
                            const char* lpName,
                            PROPERTYTYPE expectedType = ttInt,
                            int fromIdx = 0);


};
}

#endif // PS_PROPERTY_H
