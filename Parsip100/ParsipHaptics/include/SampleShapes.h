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

    static CBlobNode* createPiza(int maxLevel, int ctPillars = 30)
    {
        CUnion * root = new CUnion();
        for(int i=0; i < maxLevel; i++)
        {
            root->addChild(createPizaLevel(i*2, ctPillars, 30.0f, CMaterial::mtrlCopper()));
            root->addChild(createPizaLevel(i*2+1, ctPillars, 30.0f, CMaterial::mtrlObsidian()));
        }

        root->getTransform().setScale(vec3f(0.2f, 0.2f, 0.2f));
        return root;
    }

    static CBlobNode* createPizaLevel(int level, int nPillars, float radius, CMaterial mtrlLevel)
    {
        float x, z;

        vec3f c(0.0f, 2.0f + 11.0f * level, 0.0f);
        CUnion * root = new CUnion();
        for(int i=0; i < nPillars; i++)
        {
            x = c.x + radius * cosf(static_cast<float>(i * TwoPi)/nPillars);
            z = c.z + radius * sinf(static_cast<float>(i * TwoPi)/nPillars);

            CBlobNode* pillar = createPizaPillar(mtrlLevel);
            pillar->getTransform().setTranslate(vec3f(x, c.y, z));
            root->addChild(pillar);
        }

        CSkeletonDisc * disc = new CSkeletonDisc(c + vec3f(0.0f, 2.5f, 0.0f), vec3f(0.0f, 1.0f, 0.0f), 1.5f);
        CSkeletonPrimitive * primDisc = new CSkeletonPrimitive(disc);
        primDisc->setMaterial(CMaterial::mtrlSilver());
        primDisc->getTransform().setScale(vec3f(radius, 1.0f, radius));
        root->addChild(primDisc);
        return root;
    }

    static CBlobNode* createPizaPillar(CMaterial mtrl)
    {
        CSkeletonPrimitive * sphere = new CSkeletonPrimitive(new CSkeletonPoint(vec3f(0.0f, 0.0f, 0.0f)), fftWyvill);
        sphere->getTransform().setScale(vec3f(6.0f, 6.0f, 6.0f));
        sphere->setMaterial(mtrl);

        CSkeletonCylinder* pillar = new CSkeletonCylinder(vec3f(0.0, -8.0, 0.0), vec3f(0.0f, 1.0f, 0.0f), 0.25f, 6.0f);
        CSkeletonPrimitive* pillarPrim = new CSkeletonPrimitive(pillar);
        pillarPrim->setMaterial(mtrl);

        CRicciBlend* blendUpper = new CRicciBlend(sphere, pillarPrim, 32.0f);

        CSkeletonDisc * ground = new CSkeletonDisc(vec3f(0.0, -8.0, 0.0), vec3f(0.0f, 1.0f, 0.0f), 2.0f);
        CSkeletonPrimitive * groundPrim = new CSkeletonPrimitive(ground, fftWyvill);
        groundPrim->setMaterial(mtrl);

        CBlend* blend = new CBlend(blendUpper, groundPrim);
        return blend;
    }

};
#endif // SAMPLESHAPES_H
