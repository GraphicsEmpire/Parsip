#ifndef CUIWidgets
#define CUIWidgets

#include <QGLWidget>
#include <PS_Vector.h>
#include <PS_Quaternion.h>
#include <loki/Singleton.h>
#include "PS_Octree.h"

using namespace Loki;
using namespace PS;
using namespace PS::MATH;

#define AXIS_SELECTION_RADIUS 0.2f
typedef enum UITRANSFORMTYPE {uitTranslate, uitRotate, uitScale};
typedef enum UITRANSFORMAXIS {uiaX, uiaY, uiaZ, uiaFree};

struct UITRANSFORM{
    UITRANSFORMTYPE type;
    UITRANSFORMAXIS axis;
    vec3f           translate;
    vec3f           scale;
    CQuaternion     rotate;
    size_t          nStep;
    vec3f mouseDown;
    vec3f mouseMove;
};

typedef SingletonHolder<UITRANSFORM, CreateUsingNew, PhoenixSingleton> TheUITransform;

class CUIWidget{
public:    
    CUIWidget() {}
    virtual ~CUIWidget() {}

    virtual void draw() = 0;
    virtual void createWidget() = 0;
    virtual UITRANSFORMAXIS selectAxis(const CRay& ray, float zNear, float zFar)
    {
        return uiaFree;
    }

    void  drawCubePolygon(int a, int b, int c, int d);
    vec3f maskDisplacement(vec3f v, UITRANSFORMAXIS axis);
    vec4f maskColor( UITRANSFORMAXIS axis );
    void maskColorSetGLFront( UITRANSFORMAXIS axis );


    void setPos(const vec3f& pos) {m_pos = pos;}
    vec3f getPos() const {return m_pos;}

    void setLength(const vec3f& len) {m_length = len;}
    vec3f getLength() const {return m_length;}
protected:
    vec3f m_pos;
    vec3f m_length;
};


/*!
  * Creates a rotation Widget
  */
class CMapRotate : public CUIWidget
{
public:
    CMapRotate()
    {
        m_pos = vec3f(0,0,0);
        m_length = vec3f(0.5f, 0.5f, 0.5f);
        this->createWidget();
    }

    ~CMapRotate()
    {
        if(glIsList(m_glList))
            glDeleteLists(m_glList, 1);
    }


    void draw();
    void createWidget();

private:
    GLuint m_glList;    
};

/*!
 * Create scale Graph
 */
class CMapScale : public CUIWidget
{
public:
    CMapScale()
    {
        m_pos = vec3f(0,0,0);
        m_length = vec3f(1,1,1);
        this->createWidget();
    }

    ~CMapScale()
    {
        if(glIsList(m_glList))
            glDeleteLists(m_glList, 1);
    }


    void draw();
    void createWidget();
    UITRANSFORMAXIS selectAxis(const CRay& ray, float zNear, float zFar);
private:
    GLuint m_glList;    
    vec3f m_axisBoxesLo[3];
    vec3f m_axisBoxesHi[3];
};



/*!
  * CMapTranslate Shows a translation map on GUI
  */
class CMapTranslate : public CUIWidget
{
public:
    CMapTranslate()
    {
        m_pos = vec3f(0,0,0);
        m_length = vec3f(1,1,1);
        this->createWidget();
    }

    ~CMapTranslate()
    {
        if(glIsList(m_glList))
            glDeleteLists(m_glList, 1);
    }


    void draw();
    void createWidget();
    UITRANSFORMAXIS selectAxis(const CRay& ray, float zNear, float zFar);

private:
    GLuint m_glList;    
    vec3f m_axisBoxesLo[3];
    vec3f m_axisBoxesHi[3];
};

#endif
