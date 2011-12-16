#include <stack>
#include "_GlobalFunctions.h"
#include "PS_String.h"

using namespace PS;

bool FindSeedPoint(CBlobNode* lpNode, bool bFindHot, float iso_value, int ctTries, vec3f& p, float& fp, size_t& fieldEvaluations)
{
    if(bFindHot == (fp > iso_value)) return true;
    if(fp == 0.0f) return false;


    vec3f c = p;
    float fc = fp;
    vec3f grad;
    float travel;
    float search_step = 0.001f;
    int iStep = 0;

    grad = lpNode->gradient(c, NORMAL_DELTA, fc);
    if(bFindHot)
    {
        while(fc < iso_value)
        {
            c  += search_step*grad;

            fieldEvaluations += lpNode->fieldValueAndGradient(c, NORMAL_DELTA, grad, fc);
            if(fp != fc)
            {
                travel = (c - p).length();
                search_step = fabsf((iso_value - fc)*travel / (fp - fc));
            }

            p = c;
            fp = fc;

            iStep++;

            if(iStep >= ctTries)
                return false;
        }
    }
    else
    {
        //Find cold
        while(fc > iso_value)
        {
            c  += search_step*(-1.0f)*grad;

            fieldEvaluations += lpNode->fieldValueAndGradient(c, NORMAL_DELTA, grad, fc);
            if(fp != fc)
            {
                travel = (c - p).length();
                search_step = fabsf((iso_value - fc)*travel / (fp - fc));
            }

            p = c;
            fp = fc;

            iStep++;

            if(iStep >= ctTries)
                return false;
        }
    }

    return true;
}


bool FindSeedPoint(CBlobNode* lpNode, bool bFindHot, float iso_value, int ctTries, float search_step, vec3f search_dir, vec3f& p, float& fp, size_t& fieldEvaluations)
{
    if(bFindHot == (fp > iso_value)) return true;
    if(fp == 0.0f) return false;

    int iStep = 1;
    while(bFindHot != (fp > iso_value))
    {
        p  += static_cast<float>(iStep)*search_step*search_dir;
        fp = lpNode->fieldValue(p);
        fieldEvaluations++;
        search_step *= 1.3f;
        iStep++;

        if((iStep - 1) >= ctTries)
            return false;
    }

    return true;
}

QString printToQStr( const char* pFmt, ... )
{
    va_list	vl;
    va_start( vl, pFmt );

    char	buff[1024];
    vsnprintf( buff, 1024, pFmt, vl );

    va_end( vl );

    return QString(buff);
}

DAnsiStr AxisToString(MajorAxices axis)
{
    DAnsiStr strAxis;
    if(axis == xAxis) strAxis = "x";
    else if(axis == yAxis)	strAxis = "y";
    else strAxis = "z";
    return strAxis;
}

MajorAxices StringToAxis(DAnsiStr strAxis)
{
    strAxis.toUpper();
    if(strAxis == "X")
        return xAxis;
    else if(strAxis == "Y")
        return yAxis;
    else
        return zAxis;
}

float Brightness(vec4f c)
{
    float brightness = sqrt(c.x * c.x * 0.241f + c.y * c.y * 0.691f + c.z * c.z * 0.068f);
    return brightness;
}

vec4f TextColor(vec4f bgColor)
{
    float threshold = 130.0f / 255.0f;
    if(Brightness(bgColor) < threshold)
        return vec4f(1.0f, 1.0f, 1.0f, 1.0f);
    else
        return vec4f(0.0f, 0.0f, 0.0f, 1.0f);
}

CMaterial ColorToMaterial(const vec4f& color)
{
    CMaterial m;
    m.diffused = color;
    m.ambient = color * 0.5f;
    m.specular = vec4f(0.8f, 0.8f, 0.8f, 0.8f) + color * 0.2f;
    m.shininess = 32.0f;
    return m;
}

PS::MATH::vec4f StringToVec4( DAnsiStr strVal )
{
    float f[4];
    size_t pos;
    int iComp = 0;
    DAnsiStr strTemp;

    while(strVal.lfind(' ', pos))
    {
        strTemp = strVal.substr(0, pos);
        strVal = strVal.substr(pos + 1);
        strVal.removeStartEndSpaces();
        f[iComp] = static_cast<float>(atof(strTemp.ptr()));
        iComp++;
    }

    if(strVal.length() >= 1 && iComp < 4)
    {
        strVal.removeStartEndSpaces();
        f[iComp] = static_cast<float>(atof(strVal.ptr()));
    }

    return vec4f(f);
}

PS::MATH::vec3f StringToVec3( DAnsiStr strVal )
{
    float f[4];
    size_t pos;
    int iComp = 0;
    DAnsiStr strTemp;

    while(strVal.lfind(' ', pos))
    {
        strTemp = strVal.substr(0, pos);
        strVal = strVal.substr(pos + 1);
        strVal.removeStartEndSpaces();
        f[iComp] = static_cast<float>(atof(strTemp.ptr()));
        iComp++;
    }

    if(strVal.length() >= 1 && iComp < 3)
    {
        strVal.removeStartEndSpaces();
        f[iComp] = static_cast<float>(atof(strVal.ptr()));
    }

    return vec3f(f);
}

DAnsiStr Vec3ToString( vec3f v)
{
    DAnsiStr res = printToAStr("%.2f %.2f %.2f", v.x, v.y, v.z);
    return res;
}

DAnsiStr Vec4ToString( vec4f v)
{
    DAnsiStr res = printToAStr("%.2f %.2f %.2f %.2f", v.x, v.y, v.z, v.w);
    return res;
}
/*
CBlobNode* CompactBlobTree( CBlobNode* input )
{
    if(input == NULL) return NULL;

    CBlobNode* clonnedRoot   = cloneNode(input);
    CBlobNode* parentClonned = clonnedRoot;
    CBlobNode* parentActual = input;
    CBlobNode* kid = NULL;
    CBlobNode* clonnedKid = NULL;

    //Stack of clonned and actual trees
    typedef pair<CBlobNode*, CBlobNode*> ClonnedActualPair;
    stack<ClonnedActualPair> stkProcess;
    stkProcess.push(ClonnedActualPair(parentClonned, parentActual));

    //Look for Union, Intersect, Dif, Blend and Ricci Blend with same N value
    ClonnedActualPair cap;
    while(stkProcess.size() > 0)
    {
        cap = stkProcess.top();
        stkProcess.pop();

        parentClonned = cap.first;
        parentActual = cap.second;
        if(parentActual->isOperator())
        {
            if(isCompactableOp(parentActual->getNodeType()))
            {
                for(size_t i=0;i<parentActual->countChildren(); i++)
                {
                    kid = parentActual->getChild(i);
                    if(isEquivalentOp(parentClonned, kid))
                    {
                        for(size_t j=0;j<kid->countChildren();j++)
                        {
                            clonnedKid = cloneNode(kid->getChild(j));
                            parentClonned->addChild(clonnedKid);
                            if(clonnedKid->isOperator())
                                stkProcess.push(ClonnedActualPair(clonnedKid, kid->getChild(j)));
                        }
                    }
                    else
                    {
                        clonnedKid = cloneNode(kid);
                        parentClonned->addChild(clonnedKid);
                        if(clonnedKid->isOperator())
                            stkProcess.push(ClonnedActualPair(clonnedKid, kid));
                    }
                }
            }
            else
            {
                for(size_t i=0;i<parentActual->countChildren(); i++)
                {
                    clonnedKid = cloneNode(parentActual->getChild(i));
                    parentClonned->addChild(clonnedKid);
                    if(clonnedKid->isOperator())
                        stkProcess.push(ClonnedActualPair(clonnedKid, parentActual->getChild(i)));
                }
            }
        }
    }

    return clonnedRoot;
}
*/
