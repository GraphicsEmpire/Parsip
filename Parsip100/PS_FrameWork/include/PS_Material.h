#pragma once
#ifndef CMATERIAL_H
#define CMATERIAL_H

#include<vector>
#include<string>
#include "PS_Vector.h"


#define DEFAULT_SHININESS 16.0f

enum ColorType {ctAmbient, ctDiffused, ctSpecular};

namespace PS{
using namespace MATH;
using namespace std;

class CMaterial
{
public:
    CMaterial()
    {
        ambient.zero();
        diffused.zero();
        specular.zero();
        shininess = 0.0f;
    }

    CMaterial(vec4f amb, vec4f dif, vec4f spec, double shine = DEFAULT_SHININESS)
    {
        ambient = amb;
        diffused = dif;
        specular = spec;
        shininess = static_cast<float>(shine);
    }


    CMaterial(vec4f amb, vec4f dif, vec4f spec, float shine = DEFAULT_SHININESS)
    {
        ambient = amb;
        diffused = dif;
        specular = spec;
        shininess = shine;
    }

    CMaterial(const vec3f &amb, const vec3f &dif, const vec3f &spec, float shine = DEFAULT_SHININESS)
    {
        ambient  = vec4f(amb.x, amb.y, amb.z, 1.0f);
        diffused = vec4f(dif.x, dif.y, dif.z, 1.0f);
        specular = vec4f(spec.x, spec.y, spec.z, 1.0f);
        shininess = shine;
    }

    CMaterial(const CMaterial & m)
    {
        ambient   = m.ambient;
        diffused  = m.diffused;
        specular  = m.specular;
        shininess = m.shininess;
    }

    CMaterial operator +(const CMaterial &m)
    {
        CMaterial result;
        result.ambient = ambient + m.ambient;
        result.diffused = diffused + m.diffused;
        result.specular = specular + m.specular;
        result.shininess = shininess + m.shininess;
        return result;
    }

    CMaterial operator -(const CMaterial &m)
    {
        CMaterial result;
        result.ambient = ambient - m.ambient;
        result.diffused = diffused - m.diffused;
        result.specular = specular - m.specular;
        result.shininess = shininess - m.shininess;
        return result;
    }

    CMaterial operator *(float x)
    {
        CMaterial result;
        result.ambient = ambient * x;
        result.diffused = diffused * x;
        result.specular = specular * x;
        result.shininess = shininess * x;
        return result;

    }

    CMaterial operator *(const CMaterial &m)
    {
        CMaterial result;
        result.ambient = ambient * m.ambient;
        result.diffused = diffused * m.diffused;
        result.specular = specular * m.specular;
        result.shininess = shininess * m.shininess;
        return result;
    }

    /*
 CMaterial operator /(const CMaterial &m)
 {
  CMaterial result;
  result.ambient = ambient / m.ambient;
  result.diffused = diffused / m.diffused;
  result.specular = specular / m.specular;
  result.shininess = shininess / m.shininess;
  return result;
 }
*/

    //+=
    void operator +=(const CMaterial &m)
    {
        ambient = ambient + m.ambient;
        diffused = diffused + m.diffused;
        specular = specular + m.specular;
        shininess = shininess + m.shininess;
    }

    void operator -=(const CMaterial &m)
    {
        ambient = ambient - m.ambient;
        diffused = diffused - m.diffused;
        specular = specular - m.specular;
        shininess = shininess - m.shininess;
    }

    /*
 void operator *=(const CMaterial &m)
 {
  ambient = ambient * m.ambient;
  diffused = diffused * m.diffused;
  specular = specular * m.specular;
  shininess = shininess * m.shininess;
 }

 void operator /=(const CMaterial &m)
 {
  ambient = ambient / m.ambient;
  diffused = diffused / m.diffused;
  specular = specular / m.specular;
  shininess = shininess / m.shininess;
 }
 */

    bool operator == (const CMaterial& m)
    {
        return ((ambient == m.ambient)&&(diffused == m.diffused)&&
                (specular == m.specular)&&(shininess == m.shininess));
    }

    CMaterial& operator = (const CMaterial& m)
    {
        ambient   = m.ambient;
        diffused  = m.diffused;
        specular  = m.specular;
        shininess = m.shininess;

        return(*this);
    }

    CMaterial interpolate(const CMaterial &m1, float f1, const CMaterial &m2, float f2)
    {
        CMaterial res;
        res.ambient  = m1.ambient * f1 + m2.ambient * f2;
        res.diffused = m1.diffused * f1 + m2.diffused * f2;
        res.specular = m1.specular * f1 + m2.specular * f2;
        res.shininess = m1.shininess * f1 + m2.shininess * f2;
        return res;
    }


    vec4f getColorAsVector(ColorType type) const
    {
        switch(type)
        {
        case(ctAmbient):
            return ambient;
        case(ctDiffused):
            return diffused;
        case(ctSpecular):
            return specular;
        default:
            return diffused;
        }
    }

    vec4f getAmbient() const
    {
        return ambient;
    }

    vec4 getDiffused() const
    {
        return diffused;
    }

    vec4f getSpecular() const
    {
        return specular;
    }

    float getShininess() const
    {
        return shininess;
    }


    void setColorComponent(ColorType type, float* color)
    {
        vec4f rhs(color);
        switch(type)
        {
        case(ctAmbient):
            ambient = rhs;
            break;
        case(ctDiffused):
            diffused = rhs;
            break;
        case(ctSpecular):
            specular = rhs;
            break;
        default:
            diffused = rhs;
        }
    }

    bool isBlank()
    {
        vec4f c(0.0f, 0.0f, 0.0f, 0.0f);

        return ((ambient == c)&&(diffused == c)&&(specular == c)&&(shininess == 0.0f));
    }

    void setAmbient(const vec4f& amb)
    {
        ambient = amb;
    }

    void setDiffuse(const vec4f& dif)
    {
        diffused = dif;
    }

    void setSpecular(const vec4f& spec)
    {
        specular = spec;
    }

    void setShininess(const float shine)
    {
        shininess = shine;
    }

    void colorToMaterial(const vec4f &color)
    {
        ambient = color;
        diffused = color;
        specular = color;
        shininess = DEFAULT_SHININESS;
    }

    static CMaterial getMaterialFromList(size_t index)
    {
        std::vector<CMaterial> lstMaterials;
        lstMaterials.push_back(mtrlGold());
        lstMaterials.push_back(mtrlBlack());
        lstMaterials.push_back(mtrlBrass());
        lstMaterials.push_back(mtrlBlue());
        lstMaterials.push_back(mtrlGray());
        lstMaterials.push_back(mtrlGreen());
        lstMaterials.push_back(mtrlSilver());
        lstMaterials.push_back(mtrlRedPlastic());
        lstMaterials.push_back(mtrlWhiteShiny());

        index = index % lstMaterials.size();
        return lstMaterials[index];
    }

    static size_t getMaterialNames(std::vector<string>& lstMtrlNames)
    {
        lstMtrlNames.push_back("Gold");
        lstMtrlNames.push_back("Black");
        lstMtrlNames.push_back("Brass");
        lstMtrlNames.push_back("Blue");
        lstMtrlNames.push_back("Gray");
        lstMtrlNames.push_back("Green");
        lstMtrlNames.push_back("Silver");
        lstMtrlNames.push_back("Red");
        lstMtrlNames.push_back("White");
        return lstMtrlNames.size();
    }

    static CMaterial mtrlGold()
    {
        vec4f amb(1.0f, 0.88f, 0.25f, 1.0f);
        vec4f dif(1.0f, 0.88f, 0.25f, 1.0f);
        vec4f spec(1.0f, 0.8f, 0.8f, 1.0f);
        CMaterial mtrl;
        mtrl.setAmbient(amb);
        mtrl.setDiffuse(dif);
        mtrl.setSpecular(spec);
        mtrl.setShininess(48.0f);
        return mtrl;
    }

    static CMaterial mtrlBrass()
    {
        vec4f amb(0.33f, 0.22f, 0.03f, 1.0f);
        vec4f dif(0.78f, 0.57f, 0.11f, 1.0f);
        vec4f spec(0.99f, 0.91f, 0.81f, 1.0f);
        CMaterial mtrl;
        mtrl.setAmbient(amb);
        mtrl.setDiffuse(dif);
        mtrl.setSpecular(spec);
        mtrl.setShininess(27.8f);
        return mtrl;
    }

    static CMaterial mtrlRedPlastic()
    {
        vec4f amb(0.3f, 0.0f, 0.0f, 1.0f);
        vec4f dif(0.6f, 0.0f, 0.0f, 1.0f);
        vec4f spec(0.8f, 0.6f, 0.6f, 1.0f);
        CMaterial mtrl;
        mtrl.setAmbient(amb);
        mtrl.setDiffuse(dif);
        mtrl.setSpecular(spec);
        mtrl.setShininess(32.0f);

        return mtrl;
    }

    static CMaterial mtrlGreen()
    {
        vec4f amb(0.0f, 0.3f, 0.0f, 1.0f);
        vec4f dif(0.0f, 0.6f, 0.0f, 1.0f);
        vec4f spec(0.6f, 0.8f, 0.6f, 1.0f);
        CMaterial mtrl;
        mtrl.setAmbient(amb);
        mtrl.setDiffuse(dif);
        mtrl.setSpecular(spec);
        mtrl.setShininess(32.0f);
        return mtrl;
    }
    /*
 static CMaterial mtrlSilver()
 {
  vec4f amb(0.70f, 0.70f, 0.70f, 1.0f);
  vec4f dif(0.85f, 0.85f, 0.85f, 1.0f);
  vec4f spec(0.70f, 0.70f, 0.70f, 1.0f);
  CMaterial mtrl;
  mtrl.setAmbient(amb);
  mtrl.setDiffuse(dif);
  mtrl.setSpecular(spec);
  mtrl.setShininess(32.0f);
  return mtrl;
 }
*/
    static CMaterial mtrlGray()
    {
        vec4f amb(0.5f, 0.5f, 0.5f, 1.0f);
        vec4f dif(0.5f, 0.5f, 0.5f, 1.0f);
        vec4f spec(0.5f, 0.5f, 0.5f, 1.0f);
        CMaterial mtrl;
        mtrl.setAmbient(amb);
        mtrl.setDiffuse(dif);
        mtrl.setSpecular(spec);
        mtrl.setShininess(32.0f);
        return mtrl;
    }

    static CMaterial mtrlBlack()
    {
        vec4f amb(0.0f, 0.0f, 0.0f, 1.0f);
        vec4f dif(0.0f, 0.0f, 0.0f, 1.0f);
        vec4f spec(0.0f, 0.0f, 0.0f, 1.0f);
        CMaterial mtrl;
        mtrl.setAmbient(amb);
        mtrl.setDiffuse(dif);
        mtrl.setSpecular(spec);
        mtrl.setShininess(32.0f);
        return mtrl;
    }

    static CMaterial mtrlBlue()
    {
        vec4f amb(0.0f, 0.0f, 0.3f, 1.0f);
        vec4f dif(0.0f, 0.0f, 0.6f, 1.0f);
        vec4f spec(0.6f, 0.6f, 0.8f, 1.0f);
        CMaterial mtrl;
        mtrl.setAmbient(amb);
        mtrl.setDiffuse(dif);
        mtrl.setSpecular(spec);
        mtrl.setShininess(32.0f);
        return mtrl;
    }

    static CMaterial mtrlWhiteShiny()
    {
        vec4f amb(1.0f, 1.0f, 1.0f, 1.0f);
        vec4f dif(1.0f, 1.0f, 1.0f, 1.0f);
        vec4f spec(1.0f, 1.0f, 1.0f, 1.0f);
        CMaterial mtrl;
        mtrl.setAmbient(amb);
        mtrl.setDiffuse(dif);
        mtrl.setSpecular(spec);
        mtrl.setShininess(100.0f);

        return mtrl;
    }


    //Some new Materials
    static CMaterial mtrlEmerald()
    {
        vec4f amb(0.0215f,0.1745f,0.0215f, 1.0f);
        vec4f dif(0.07568f,0.61424f,0.07568f, 1.0f);
        vec4f spec(0.633f, 0.727811f, 0.633f, 1.0f);
        float shine = 76.8f;
        CMaterial mtrl(amb, dif, spec, shine);
        return mtrl;
    }

    static CMaterial mtrlJade()
    {
        vec4f amb(0.135f, 0.2225f, 0.1575f);
        vec4f dif(0.54f, 0.89f, 0.63f);
        vec4f spec(0.316228f, 0.316228f, 0.316228f);
        float shine = 12.8f;
        CMaterial mtrl(amb, dif, spec, shine);
        return mtrl;
    }

    static CMaterial mtrlObsidian()
    {
        vec4f amb(0.05375f, 0.05f, 0.06625f);
        vec4f dif(0.18275f, 0.17f, 0.22525f);
        vec4f spec(0.332741f, 0.328634f, 0.346435f);
        float shine = 38.4f;
        CMaterial mtrl(amb, dif, spec, shine);
        return mtrl;
    }

    static CMaterial mtrlChrome()
    {
        vec4f amb(0.25f, 0.25f, 0.25f);
        vec4f dif(0.4f, 0.4f, 0.4f);
        vec4f spec(0.774597f, 0.774597f, 0.774597f);
        float shine = 76.8f;
        CMaterial mtrl(amb, dif, spec, shine);
        return mtrl;
    }

    static CMaterial mtrlCopper()
    {
        vec4f amb(0.19125f, 0.0735f, 0.0225f);
        vec4f dif(0.7038f, 0.27048f, 0.0828f );
        vec4f spec(0.256777f, 0.137622f, 0.086014f );
        float shine = 12.8f;
        CMaterial mtrl(amb, dif, spec, shine);
        return mtrl;
    }


    static CMaterial mtrlSilver()
    {
        vec4f amb(0.19225f, 0.19225f, 0.19225f );
        vec4f dif(0.50754f, 0.50754f, 0.50754f );
        vec4f spec(0.508273f, 0.508273f, 0.508273f );
        float shine = 51.2f;
        CMaterial mtrl(amb, dif, spec, shine);
        return mtrl;
    }

    static CMaterial mtrlRuby()
    {
        vec4f amb(0.1745f, 0.01175f, 0.01175f );
        vec4f dif(0.61424f, 0.04136f, 0.04136f );
        vec4f spec(0.727811f, 0.626959f, 0.626959f );
        float shine = 76.8f;
        CMaterial mtrl(amb, dif, spec, shine);
        return mtrl;
    }

    static CMaterial mtrlWhiteLight()
    {
        vec4f amb(0.1f, 0.1f, 0.1f, 1.0f);
        vec4f dif(1.0f, 1.0f, 1.0f, 1.0f);
        vec4f spec(1.0f, 1.0f, 1.0f, 1.0f);
        float shine = 1.0f;
        CMaterial mtrl(amb, dif, spec, shine);
        return mtrl;
    }
public:
    vec4f ambient;
    vec4f diffused;
    vec4f specular;
    float shininess;
};

}

#endif
