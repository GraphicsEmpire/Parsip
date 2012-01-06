#ifndef SAMPLESHAPES_H
#define SAMPLESHAPES_H

#include "BlobTreeLibraryAll.h"

using namespace PS;
using namespace PS::BLOBTREE;

class Shapes{
public:
    static CBlobNode* CreateGear(int ctTeeth, const CMaterial& outer, const CMaterial& inner)
    {
        //Let's create one gear
        vec3f center = vec3f(0.0f, 2.0f, 0.0f);
        float radius = 3.0f;
        CSkeletonDisc * disk = new CSkeletonDisc(center, vec3f(0.0f, 0.0f, 1.0f), radius);
        CSkeletonPrimitive * nodeDisk = new CSkeletonPrimitive(disk, fftWyvill);
        nodeDisk->setMaterial(outer);

        center.z -= 0.5f;
        CSkeletonCylinder* cyl = new CSkeletonCylinder(center, vec3f(0.0f, 0.0f, 1.0f), 0.25f, 2.0f);
        CSkeletonPrimitive * nodeCyl = new CSkeletonPrimitive(cyl, fftWyvill);
        nodeCyl->setMaterial(inner);

        CDifference * dif = new CDifference(nodeDisk, nodeCyl);

        CSkeletonCylinder* tempCyl;
        CSkeletonPrimitive* tempPrim;
        vec3f pos = center;
        float ep = 0.20f;

        for(int i=0; i < ctTeeth; i++)
        {
            pos.x = center.x + (radius + ep) * cos(static_cast<float>(i*TwoPi)/(ctTeeth));
            pos.y = center.y + (radius + ep) * sin(static_cast<float>(i*TwoPi)/(ctTeeth));
            tempCyl = new CSkeletonCylinder(pos, vec3f(0.0f, 0.0f, 1.0f), 0.25f, 2.0f);
            tempPrim = new CSkeletonPrimitive(tempCyl, fftWyvill);
            tempPrim->setMaterial(CMaterial::mtrlBrass());
            dif->addChild(tempPrim);
        }

        return dif;
    }

    static CBlobNode* createPiza(int maxLevel, int ctPillars, float radius, float levelHeight)
    {
        CUnion * root = new CUnion();

        //Build Tower
        for(int i=0; i < maxLevel; i++)
        {
            CMaterial mtrl = CMaterial::getMaterialFromList(i);
            CBlobNode* pLevel = createPizaLevel(ctPillars, radius, levelHeight, mtrl);
            pLevel->getTransform().addTranslate(vec3f(0.0f, i * 1.1f * levelHeight, 0.0f));
            root->addChild(pLevel);
        }

        //root->getTransform().setScale(vec3f(0.2f, 0.2f, 0.2f));
        return root;
    }

    static CBlobNode* createPizaLevel(int nPillars, float radius, float height, const CMaterial& mtrlLevel)
    {
        CUnion * root = new CUnion();
        for(int i=0; i < nPillars; i++)
        {
            float x = radius * cosf(static_cast<float>(i * TwoPi)/nPillars);
            float z = radius * sinf(static_cast<float>(i * TwoPi)/nPillars);

            CBlobNode* pillar = createPizaPillar(height, mtrlLevel);
            pillar->getTransform().setTranslate(vec3f(x, 0.0f, z));
            root->addChild(pillar);
        }

        //Create Ceiling
        CSkeletonDisc * disc = new CSkeletonDisc(vec3f(0.0f, height, 0.0f), vec3f(0.0f, 1.0f, 0.0f), radius * 1.2f);
        CSkeletonPrimitive * primDisc = new CSkeletonPrimitive(disc);
        primDisc->setMaterial(CMaterial::mtrlSilver());
        //primDisc->getTransform().setScale(vec3f(radius, 1.0f, radius));
        root->addChild(primDisc);
        return root;
    }

    static CBlobNode* createPizaPillar(float height, const CMaterial& mtrl)
    {
        CSkeletonDisc * ground = new CSkeletonDisc(vec3f(0.0, 0.0, 0.0), vec3f(0.0f, 1.0f, 0.0f), 1.0f);
        CSkeletonPrimitive * groundPrim = new CSkeletonPrimitive(ground, fftWyvill);
        groundPrim->setMaterial(mtrl);

        CSkeletonCylinder* pillar = new CSkeletonCylinder(vec3f(0.0, 0.0, 0.0), vec3f(0.0f, 1.0f, 0.0f), 0.25f, 0.7f * height);
        CSkeletonPrimitive* pillarPrim = new CSkeletonPrimitive(pillar);
        pillarPrim->setMaterial(mtrl);

        //
        //Spherical Scale
        float s = 0.5f * height;
        CSkeletonPrimitive * cap = new CSkeletonPrimitive(new CSkeletonPoint(vec3f(0.0f, 0.25f * height, 0.0f)), fftWyvill);
        cap->getTransform().setScale(vec3f(s, s, s));
        cap->setMaterial(mtrl);

        CUnion* blend = new CUnion(new CUnion(groundPrim, pillarPrim), cap);
        return blend;
    }

};
#endif // SAMPLESHAPES_H
