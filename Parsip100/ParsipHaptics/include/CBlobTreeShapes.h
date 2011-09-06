#ifndef CBLOBTREESHAPES_H
#define CBLOBTREESHAPES_H

#include "PS_BlobTree/include/BlobTreeLibraryAll.h"
#include "_GlobalFunctions.h"


using namespace BLOBTREE;

class CBlobTreeShapes
{
private:


public:
	static CBlobTree * createSphere()
	{
		CSkeletonPoint * point = new CSkeletonPoint(vec3(0.0, 0.0, 0.0));
		CSkeletonPrimitive * primitive1 = new CSkeletonPrimitive(point, fftWyvill, 1.0f);		
		primitive1->setMaterial(CMaterial::mtrlRedPlastic());
		return primitive1;
	}

	static CBlobTree * createPeanut()
	{			
		CSkeletonPoint * point1 = new CSkeletonPoint(vec3(0.0f, 0.0f, 0.0f));
		CSkeletonPoint * point2 = new CSkeletonPoint(vec3(1.0f, 0.0f, 0.0f));

		CSkeletonPrimitive * primitive1 = new CSkeletonPrimitive(point1, fftWyvill);
		CSkeletonPrimitive * primitive2 = new CSkeletonPrimitive(point2, fftWyvill);

		primitive1->setMaterial(CMaterial::mtrlRedPlastic());
		primitive2->setMaterial(CMaterial::mtrlRedPlastic());

		CBlend * root = new CBlend(primitive1, primitive2);			
		return root;
	}
	
	static CBlobTree * createCrossCylinders()
	{	
		/*
		CSkeletonCylinder * cyl1 = new CSkeletonCylinder(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.01f, 6.0f);
		CSkeletonCylinder * cyl2 = new CSkeletonCylinder(vec3(-3.0f, 3.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 0.01f, 4.0f);
		CSkeletonPrimitive * primitive1 = new CSkeletonPrimitive(cyl1, fftWyvill);
		CSkeletonPrimitive * primitive2 = new CSkeletonPrimitive(cyl2, fftWyvill);

		primitive1->setMaterial(CMaterial::mtrlBlue());
		primitive2->setMaterial(CMaterial::mtrlRedPlastic());

		CWarpTaper * taper1 = new CWarpTaper(primitive1, 0.4f, xAxis, yAxis);
		CWarpTaper * taper2 = new CWarpTaper(primitive2, 0.2f, zAxis, xAxis);
		CBlend * blend =  new CBlend(taper1, taper2);
		return blend;
		*/
		
		CSkeletonCylinder * cyl1 = new CSkeletonCylinder(vec3(-0.5f, -0.5f, 0.0f), vec3(+1.0f, 1.0f, 0.0f), 0.01f, 6.0f);
		CSkeletonCylinder * cyl2 = new CSkeletonCylinder(vec3(+0.5f, -0.5f, 0.0f), vec3(-1.0f, 1.0f, 0.0f), 0.01f, 4.0f);
		CSkeletonPrimitive * primitive1 = new CSkeletonPrimitive(cyl1, fftWyvill);
		CSkeletonPrimitive * primitive2 = new CSkeletonPrimitive(cyl2, fftWyvill);

		primitive1->setMaterial(CMaterial::mtrlBlue());
		primitive2->setMaterial(CMaterial::mtrlRedPlastic());

		CWarpTaper * taper1 = new CWarpTaper(primitive1, 0.4f, zAxis, yAxis);
		CWarpTaper * taper2 = new CWarpTaper(primitive2, 0.4f, xAxis, yAxis);
		CUnion * root =  new CUnion(taper1, taper2);
		return root;
		
	}

	static CBlobTree * createLogo()
	{
		CSkeletonPoint * sphere1 = new CSkeletonPoint(vec3(0.0f, 0.0f, 0.0f));
		CSkeletonPoint * sphere2 = new CSkeletonPoint(vec3(0.0f, 1.0f, 0.0f));
		CSkeletonPoint * sphere3 = new CSkeletonPoint(vec3(1.0f, 1.0f, 0.0f));
		CSkeletonPoint * sphere4 = new CSkeletonPoint(vec3(1.0f, 0.0f, 0.0f));

		CSkeletonPrimitive * primitive1 = new CSkeletonPrimitive(sphere1);
		primitive1->setMaterial(CMaterial::mtrlBlue());
		CSkeletonPrimitive * primitive2 = new CSkeletonPrimitive(sphere2);
		primitive2->setMaterial(CMaterial::mtrlRedPlastic());
		CSkeletonPrimitive * primitive3 = new CSkeletonPrimitive(sphere3);
		primitive3->setMaterial(CMaterial::mtrlGreen());
		CSkeletonPrimitive * primitive4 = new CSkeletonPrimitive(sphere4);
		primitive4->setMaterial(CMaterial::mtrlRedPlastic());

		CBlend * blend = new CBlend(primitive1, primitive2, primitive3);
		blend->addChild(primitive4);

		//CSkeletonCylinder *cyl = new CSkeletonCylinder(vec3(0.5f, 0.5f, 0.0f), vec3(0.0f, 0.0f, 1.0f), 0.08f, 1.0f);
		//CSkeletonPrimitive * center = new CSkeletonPrimitive(cyl);

		//CDifference * dif = new CDifference(uni, center);
		return blend;

	}



	//Create an individual Gear
	static CBlobTree* createGear(int ctTeeth, CMaterial& outer, CMaterial& inner)
	{
		//Let's create one gear 
		vec3 center = vec3(0.0f, 2.0f, 0.0f);
		float radius = 3.0f;
		CSkeletonDisc * disk = new CSkeletonDisc(center, vec3(0.0f, 0.0f, 1.0f), radius);
		CSkeletonPrimitive * nodeDisk = new CSkeletonPrimitive(disk, fftWyvill);
		nodeDisk->setMaterial(outer);

		center.z -= 0.5f;
		CSkeletonCylinder* cyl = new CSkeletonCylinder(center, vec3(0.0f, 0.0f, 1.0f), 0.25f, 2.0f);
		CSkeletonPrimitive * nodeCyl = new CSkeletonPrimitive(cyl, fftWyvill);
		nodeCyl->setMaterial(inner);

		CDifference * dif = new CDifference(nodeDisk, nodeCyl);

		CSkeletonCylinder* tempCyl;
		CSkeletonPrimitive* tempPrim;
		vec3 pos = center;
		float ep = 0.20f;

		for(int i=0; i < ctTeeth; i++)
		{
			pos.x = center.x + (radius + ep) * cos(static_cast<float>(i*TwoPi)/(ctTeeth));
			pos.y = center.y + (radius + ep) * sin(static_cast<float>(i*TwoPi)/(ctTeeth));
			tempCyl = new CSkeletonCylinder(pos, vec3(0.0f, 0.0f, 1.0f), 0.25f, 2.0f);			
			tempPrim = new CSkeletonPrimitive(tempCyl, fftWyvill);
			tempPrim->setMaterial(CMaterial::mtrlBrass());
			dif->addChild(tempPrim);
		}
		
		return dif;
	}

	static CBlobTree* create3CylinderTaperBend(float xoffset, float zoffset, float RicciN = 2.0f)
	{
		CSkeletonCylinder * cyl1 = new CSkeletonCylinder(vec3(0.0f + xoffset, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.01f, 5.0f);
		CSkeletonCylinder * cyl2 = new CSkeletonCylinder(vec3(0.8f + xoffset, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.01f, 5.0f);
		//CSkeletonCylinder * cyl3 = new CSkeletonCylinder(vec3(-0.8f + xoffset, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.01f, 4.0f);
		CSkeletonPrimitive * primitive1 = new CSkeletonPrimitive(cyl1, fftWyvill);
		CSkeletonPrimitive * primitive2 = new CSkeletonPrimitive(cyl2, fftWyvill);
		//CSkeletonPrimitive * primitive3 = new CSkeletonPrimitive(cyl3, fftWyvill);

		primitive1->setMaterial(CMaterial::mtrlBrass());
		primitive2->setMaterial(CMaterial::mtrlBrass());
		//primitive3->setMaterial(CMaterial::mtrlGreen());

		CWarpTaper * taper1 = new CWarpTaper(primitive1, 0.3f, xAxis, yAxis);
		CWarpTaper * taper2 = new CWarpTaper(primitive2, 0.3f, xAxis, yAxis);
		//CWarpTaper * taper3 = new CWarpTaper(primitive3, 0.3f, xAxis, yAxis);

		CWarpBend * bend1 = new CWarpBend(taper1, 0.4f, 2.5f, 1.0f, 4.0f);
		CWarpBend * bend2 = new CWarpBend(taper2, 0.4f, 2.5f, 1.0f, 4.0f);

		CRicciBlend * blend1 = new CRicciBlend(bend1, bend2, RicciN);

		if((xoffset != 0.0f)||(zoffset != 0.0f))
		{
			CAffine * affine = new CAffine(blend1);
			affine->setTranslate(vec3(xoffset, 0.0f, zoffset));			
			return affine;
		}
		else
			return blend1;
	}
	
	static CBlobTree* create3CylinderTaperTwist(float xoffset, float zoffset, float RicciN = 2.0f)
	{
		//1.Twist
		CSkeletonCylinder * cyl1 = new CSkeletonCylinder(vec3(0.0f + xoffset, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.01f, 4.0f);
		CSkeletonCylinder * cyl2 = new CSkeletonCylinder(vec3(0.8f + xoffset, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.01f, 4.0f);
		CSkeletonCylinder * cyl3 = new CSkeletonCylinder(vec3(-0.8f + xoffset, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.01f, 4.0f);
		CSkeletonPrimitive * primitive1 = new CSkeletonPrimitive(cyl1, fftWyvill);
		CSkeletonPrimitive * primitive2 = new CSkeletonPrimitive(cyl2, fftWyvill);
		CSkeletonPrimitive * primitive3 = new CSkeletonPrimitive(cyl3, fftWyvill);

		primitive1->setMaterial(CMaterial::mtrlBlue());
		primitive2->setMaterial(CMaterial::mtrlRedPlastic());
		primitive3->setMaterial(CMaterial::mtrlGreen());
		
		CWarpTaper * taper1 = new CWarpTaper(primitive1, 0.3f, xAxis, yAxis);
		CWarpTaper * taper2 = new CWarpTaper(primitive2, 0.3f, xAxis, yAxis);
		CWarpTaper * taper3 = new CWarpTaper(primitive3, 0.3f, xAxis, yAxis);
		
		CWarpTwist * twist1 = new CWarpTwist(taper1, PiOver2, yAxis);
		CWarpTwist * twist2 = new CWarpTwist(taper2, PiOver2, yAxis);
		CWarpTwist * twist3 = new CWarpTwist(taper3, PiOver2, yAxis);

		CRicciBlend * blend1 = new CRicciBlend(twist1, twist2, twist3, RicciN);

		CFieldCache * cache  = new CFieldCache(blend1, AdaptiveUniformGrid);
		cache->setFieldValueSampleMode(TriLinear);
		cache->setGradientSampleMode(TriQuadratic);
		cache->setCacheResolution(64);

		CAffine * affine1 = new CAffine(cache);
		affine1->setTranslate(vec3(0.0f, -0.3f, 0.0f));		


		/*
		CFieldCacheManager* manager =  CFieldCacheManager::Get();
		manager->CreateCache(blend1, CacheType::ADAPTIVE_UNIFORM_GRID);
		manager->SetDefaultValueMode(CFieldCache::SampleMode::TriLinear, true);
		manager->SetDefaultGradientMode(CFieldCache::SampleMode::TriQuadratic, true);
		manager->SetDefaultCacheResolution(100)
		*/

		if((xoffset != 0.0f)||(zoffset != 0.0f))
		{
			CAffine * affine2 = new CAffine(cache);
			affine2->setTranslate(vec3(xoffset, 0.0f, zoffset));			
			return affine2;
		}
		else
			return cache;

		//lm->addLayer(twist1);
		//lm->addLayer(twist2);
		//lm->addLayer(twist3);

		/*

		CAffine * affine = new CAffine(createDVDDeck());
		affine->setTranslate(vec3(3.0f, 0.0f, -3.0f));
		affine->turnOn(false, true, false);
		lm->addLayer(affine);
		*/
	}

	static CBlobTree* createBlobSpheres()
	{
		CSkeletonPoint * point1 = new CSkeletonPoint(vec3(-2.5f, 0.0f, 0.0f));
		CSkeletonPoint * point2 = new CSkeletonPoint(vec3(2.5f, 0.0f, 0.0f));

		CSkeletonPrimitive * sphere1 = new CSkeletonPrimitive(point1, fftWyvill, 5.0f);
		CSkeletonPrimitive * sphere2 = new CSkeletonPrimitive(point2, fftWyvill, 5.0f);
		sphere1->setMaterial(CMaterial::mtrlBlue());
		sphere2->setMaterial(CMaterial::mtrlBlue());

		CBlend *blend = new CBlend(sphere1, sphere2);

		return blend;
	}

	static void createTwoSpheres(CLayerManager * lm = NULL)
	{
		CSkeletonPoint * point1 = new CSkeletonPoint(vec3(-3.0f, 0.0f, 0.0f));
		CSkeletonPoint * point2 = new CSkeletonPoint(vec3(5.0f, 0.0f, 0.0f));

		CSkeletonPrimitive * sphere1 = new CSkeletonPrimitive(point1, fftWyvill, 6.0f);
		CSkeletonPrimitive * sphere2 = new CSkeletonPrimitive(point2, fftWyvill, 6.0f);
		sphere1->setMaterial(CMaterial::mtrlRedPlastic());
		//sphere1->

		sphere2->setMaterial(CMaterial::mtrlBlue());

		lm->addLayer(sphere1, "Red");
		lm->addLayer(sphere2, "Blue");

		return;
	}

	static CBlobTree* createPiza()
	{
		CUnion * root = new CUnion();
/*	
		root->addChild(createPizaLevel(0, 20, 20.0f, CMaterial::mtrlCopper()));
		root->addChild(createPizaLevel(1, 20, 20.0f, CMaterial::mtrlObsidian()));
		root->addChild(createPizaLevel(2, 20, 20.0f, CMaterial::mtrlCopper()));
		root->addChild(createPizaLevel(3, 20, 20.0f, CMaterial::mtrlObsidian()));		

		root->addChild(createPizaLevel(4, 20, 20.0f, CMaterial::mtrlCopper()));
		root->addChild(createPizaLevel(5, 20, 20.0f, CMaterial::mtrlObsidian()));
		root->addChild(createPizaLevel(6, 20, 20.0f, CMaterial::mtrlCopper()));
		root->addChild(createPizaLevel(7, 20, 20.0f, CMaterial::mtrlObsidian()));		

		root->addChild(createPizaLevel(8, 20, 20.0f, CMaterial::mtrlCopper()));
		root->addChild(createPizaLevel(9, 20, 20.0f, CMaterial::mtrlObsidian()));
		root->addChild(createPizaLevel(10, 20, 20.0f, CMaterial::mtrlCopper()));
		root->addChild(createPizaLevel(11, 20, 20.0f, CMaterial::mtrlObsidian()));		
		*/
		
		for(int i=0; i < 7; i++)
		{
			root->addChild(createPizaLevel(i*2, 30, 30.0f, CMaterial::mtrlCopper()));
			root->addChild(createPizaLevel(i*2+1, 30, 30.0f, CMaterial::mtrlObsidian()));
		}


		CAffine * finalScale = new CAffine(root);		
		//finalScale->setTranslate(vec3(-0.3f, 0.3f, -0.3f));
		finalScale->setScale(vec3(0.2f, 0.2f, 0.2f));
		return finalScale;
	}

	static CBlobTree* createPizaLevel(int level, int nPillars, float radius, CMaterial mtrlLevel)
	{		
		float x, z;		

		vec3 c(0.0f, 2.0f + 11.0f * level, 0.0f);

		CAffine * aff = NULL;
		CUnion * root = new CUnion();
		for(int i=0; i < nPillars; i++)
		{
			x = c.x + radius * cosf(static_cast<float>(i * TwoPi)/nPillars);
			z = c.z + radius * sinf(static_cast<float>(i * TwoPi)/nPillars);

			aff = new CAffine(createPizaPillar(mtrlLevel));
			aff->setTranslate(vec3(x, c.y, z));			
			root->addChild(aff);
		}

		CSkeletonDisc * disc = new CSkeletonDisc(c + vec3(0.0f, 2.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f), radius + 4.0f);
		CSkeletonPrimitive * primDisc = new CSkeletonPrimitive(disc);
		primDisc->setMaterial(CMaterial::mtrlSilver());
		root->addChild(primDisc);
		return root;
	}

	static CBlobTree* createPizaPillar(CMaterial mtrl)
	{
		CSkeletonPoint * point1 = new CSkeletonPoint(vec3(0.0f, 0.0f, 0.0f));
		CSkeletonPrimitive * sphere1 = new CSkeletonPrimitive(point1, fftWyvill, 6.0f);
		sphere1->setMaterial(mtrl);

		CSkeletonCylinder* pillar = new CSkeletonCylinder(vec3(0.0, -8.0, 0.0), vec3(0.0f, 1.0f, 0.0f), 0.25f, 6.0f);
		CSkeletonPrimitive* pillarPrim = new CSkeletonPrimitive(pillar);
		pillarPrim->setMaterial(mtrl);

		CRicciBlend* blendUpper = new CRicciBlend(sphere1, pillarPrim, 32.0f);

		CSkeletonDisc * ground = new CSkeletonDisc(vec3(0.0, -8.0, 0.0), vec3(0.0f, 1.0f, 0.0f), 2.0f);
		CSkeletonPrimitive * groundPrim = new CSkeletonPrimitive(ground, fftWyvill);
		groundPrim->setMaterial(mtrl);

		CBlend* blend = new CBlend(blendUpper, groundPrim);				
		return blend;
	}


	static CBlobTree* createKitchen()
	{
		CBlobTree* chair1 = createChair();
		CAffine * affine2 = new CAffine(createChair());
		affine2->setTranslate(vec3(7.0f ,0.0f, 0.0f));		

		CAffine * affine3 = new CAffine(createChair());
		affine3->setTranslate(vec3(7.0f ,0.0f, 7.0f));
		
		CAffine * affine4 = new CAffine(createChair());
		affine4->setTranslate(vec3(0.0f ,0.0f, 7.0f));
		
		CUnion * uni = new CUnion(chair1, affine2, affine3);
		uni->addChild(affine4);

		return uni;
	}


	static CBlobTree* createChair()
	{
		CSkeletonPoint * point1 = new CSkeletonPoint(vec3(0.0f, 0.0f, 0.0f));
		CSkeletonPrimitive * sphere1 = new CSkeletonPrimitive(point1, fftWyvill, 6.0f);
		sphere1->setMaterial(CMaterial::mtrlEmerald());

		vec3 center(-4.0f, 2.0f, 0.0f);
		CSkeletonCylinder* cyl = new CSkeletonCylinder(center, vec3(1.0f, 0.0f, 0.0f), 1.8f, 8.0f);
		CSkeletonPrimitive* cylPrim = new CSkeletonPrimitive(cyl);
		cylPrim->setMaterial(CMaterial::mtrlEmerald());

		CSmoothDifference * sit = new CSmoothDifference(sphere1, cylPrim);

		CSkeletonCylinder* pillar = new CSkeletonCylinder(vec3(0.0, -8.0, 0.0), vec3(0.0f, 1.0f, 0.0f), 0.25f, 6.0f);
		CSkeletonPrimitive* pillarPrim = new CSkeletonPrimitive(pillar);
		pillarPrim->setMaterial(CMaterial::mtrlCopper());

		CRicciBlend* blendUpper = new CRicciBlend(sit, pillarPrim, 32.0f);


		CSkeletonDisc * ground = new CSkeletonDisc(vec3(0.0, -8.0, 0.0), vec3(0.0f, 1.0f, 0.0f), 2.0f);
		CSkeletonPrimitive * groundPrim = new CSkeletonPrimitive(ground, fftWyvill);
		groundPrim->setMaterial(CMaterial::mtrlCopper());


		CBlend* blend = new CBlend(blendUpper, groundPrim);				
		return blend;
	}

	//Create a complete gearbox
	static CBlobTree* createGearBox(CLayerManager * lm = NULL)
	{
		CBlobTree* gear1  = createGear(10, CMaterial::mtrlRedPlastic(), CMaterial::mtrlBrass());
		CBlobTree* gear2  = createGear(8, CMaterial::mtrlGreen(), CMaterial::mtrlBrass());
		CBlobTree* gear3  = createGear(10, CMaterial::mtrlBlue(), CMaterial::mtrlBrass());		
		//********************************************
		CAffine* affine2 = new CAffine(gear2);
		affine2->setRotation(90, vec3(0.0f, 1.0f, 0.0f));
		affine2->setScale(vec3(0.8f, 0.8f, 0.8f));
		affine2->setTranslate(vec3(3.2f, 1.4f, 2.4f));
		

		CAffine* affine3 = new CAffine(gear3);
		affine3->setRotation(90, vec3(1.0f, 0.0f, 0.0f));
		affine3->setScale(vec3(1.5f, 1.5f, 1.0f));
		affine3->setTranslate(vec3(-7.0f, 2.8f, -3.0f));
		

		if(lm == NULL)
		{
			CUnion* uni = new CUnion(gear1, affine2, affine3);
			return uni;
		}
		else
		{
			lm->addLayer(gear1, "Red");
			lm->addLayer(affine2, "Green");
			lm->addLayer(affine3, "Blue");
			return NULL;
		}		
	}

	static CBlobTree* createDVDDeck()
	{
		vec3 origin(0.0f, 0.0f, 0.0f);
		vec3 dir(0.0f, 1.0f, 0.0f);
		CSkeletonDisc* disk = new CSkeletonDisc(origin, dir, 2.0f);
		CSkeletonPrimitive * prim1 = new CSkeletonPrimitive(disk);
		prim1->setRange(0.01f);
		prim1->setMaterial(CMaterial::mtrlRedPlastic());

		CSkeletonCylinder* cyl = new CSkeletonCylinder(origin, dir, 0.1f, 4.0f);
		CSkeletonPrimitive * prim2 = new CSkeletonPrimitive(cyl);
		prim2->setMaterial(CMaterial::mtrlBrass());
		
		CWarpTaper * taper = new CWarpTaper(prim2, 0.6f, yAxis, xAxis); 

		/*
		origin.y += 1.0f;	
		CSkeletonRing * ring = new CSkeletonRing(origin,  dir, 1.4f);
		CSkeletonPrimitive * prim3 = new CSkeletonPrimitive(ring);
		//prim3->setRange(0.01f);
		prim3->setScale(0.7f);
		prim3->setMaterial(CMaterial::mtrlGreen());
		*/

		CBlend * blend = new CBlend(prim1, taper);
		return blend;
	}

	static CBlobTree * createDonut()
	{
		CSkeletonRing * ring = new CSkeletonRing();
		ring->setRadius(3.0f);

		CSkeletonPrimitive *primitive1 = new CSkeletonPrimitive(ring);
		primitive1->setMaterial(CMaterial::mtrlGold());
		return primitive1;
	}

/*
	static CBlobTree* createSpring()
	{
		CSkeletonCylinder* cyl = new CSkeleton
	}
*/			

/*
	static CBlobTree * createHollowSphere()
	{
		CSkeletonCylinder * cyl1 = new CSkeletonCylinder(vec3(-2.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.005f, 4.0f);
		CSkeletonCylinder * cyl2 = new CSkeletonCylinder(vec3(-1.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.005f, 4.0f);
		CSkeletonCylinder * cyl3 = new CSkeletonCylinder(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.005f, 4.0f);
		CSkeletonCylinder * cyl4 = new CSkeletonCylinder(vec3(+1.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.005f, 4.0f);
		CSkeletonCylinder * cyl5 = new CSkeletonCylinder(vec3(+2.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.005f, 4.0f);
		//CSkeletonCylinder * cyl6 = new CSkeletonCylinder(vec3(0.0f, +1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.01f, 4.0f);


		//CSkeletonCylinder * cyl2 = new CSkeletonCylinder(vec3(+1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f), 0.01f, 4.0f);

//		CSkeletonPoint * point = new CSkeletonPoint(vec3(0.7f, 0.0f, 0.0f));
//		CSkeletonCylinder * cyl = new CSkeletonCylinder(vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.01f, 3.0f);

		CSkeletonPrimitive *primitive1 = new CSkeletonPrimitive(cyl1, fftWyvill, 1.0f, 0.6f);
		CSkeletonPrimitive *primitive2 = new CSkeletonPrimitive(cyl2, fftWyvill, 1.0f, 0.6f);
		CSkeletonPrimitive *primitive3 = new CSkeletonPrimitive(cyl3, fftWyvill, 1.0f, 0.6f);
		CSkeletonPrimitive *primitive4 = new CSkeletonPrimitive(cyl4, fftWyvill, 1.0f, 0.6f);
		CSkeletonPrimitive *primitive5 = new CSkeletonPrimitive(cyl5, fftWyvill, 1.0f, 0.6f);
//		CSkeletonPrimitive *primitive2 = new CSkeletonPrimitive(cyl2, fftWyvill, 1.0f, 0.6f);

		CBlend * rootold =  new CBlend(primitive1, primitive2, primitive3);
		CUnion * uni =  new CUnion(primitive5, primitive4);

		CBlend * rootNew = new CBlend(rootold, uni);
		//CSmoothDifference * root = new CSmoothDifference(primitive1, primitive2, primitive3);
//		CSmoothDifference * root = new CSmoothDifference(primitive1, primitive2, primitive3);
		return rootNew;
	}

	static CBlobTree * createDisk()
	{
		CSkeletonDisc * disk = new CSkeletonDisc(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), 1.0f);
		CSkeletonPoint * point = new CSkeletonPoint(vec3(0.0, 0.0, 2.0f));

		CSkeletonPrimitive *primitive1 = new CSkeletonPrimitive(disk, fftWyvill, 1.0f, 0.6f);
		CSkeletonPrimitive *primitive2 = new CSkeletonPrimitive(point, fftWyvill, 1.0f, 0.6f);

		CUnion * root = new CUnion(primitive1, primitive2);
		return root;
	}


	static CBlobTree * createBLOB()
	{
		CSkeletonRing * ring1 = new CSkeletonRing(vec3(0.0f, 1.8f, 0.0f), vec3(0.0, 0.0, 1.0), 1.0f);		
		CSkeletonRing * ring2 = new CSkeletonRing(vec3(0.0f, 0.0f, 0.0f), vec3(0.0, 0.0, 1.0), 1.0f);		
		
		CSkeletonPrimitive *primitive1 = new CSkeletonPrimitive(ring1, fftWyvill, 0.75f, 0.5f, 30.0f);
		primitive1->setMaterial(CMaterial::mtrlGreen());
		CSkeletonPrimitive *primitive2 = new CSkeletonPrimitive(ring2, fftWyvill, 0.75f, 0.5f, 30.0f);
		primitive2->setMaterial(CMaterial::mtrlGreen());

		CUnion * firstUnion = new CUnion(primitive1, primitive2);

		CSkeletonCylinder * cyl1 = new CSkeletonCylinder(vec3(3.0f, 2.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.005f, 2.0f);
		CSkeletonCylinder * cyl2 = new CSkeletonCylinder(vec3(3.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 0.005f, 1.0f);
		CSkeletonPrimitive *primitive3 = new CSkeletonPrimitive(cyl1, fftWyvill, 0.75f, 0.5f, 30.0f);
		primitive1->setMaterial(CMaterial::mtrlGreen());
		CSkeletonPrimitive *primitive4 = new CSkeletonPrimitive(cyl2, fftWyvill, 0.75f, 0.5f, 30.0f);
		primitive2->setMaterial(CMaterial::mtrlGreen());

		CBlend * blend = new CBlend(primitive3, primitive4);

		CUnion * root = new CUnion(blend, firstUnion);
		
		return root;
	}

	static CBlobTree * createTaperTwistedPlus()
	{
		CSkeletonCylinder * cyl1 = new CSkeletonCylinder(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 0.05f, 3.0f);
		CWarpTaper * taper1 = new CWarpTaper(CSkeletonPrimitive::getBlobTreeNode(cyl1), 1.0f, xAxis);

		CSkeletonCylinder * cyl2 = new CSkeletonCylinder(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.05f, 3.0f);
		CWarpTaper * taper2 = new CWarpTaper(CSkeletonPrimitive::getBlobTreeNode(cyl2), 1.0f, yAxis);

		CBlend * blend = new CBlend(CSkeletonPrimitive::getBlobTreeNode(cyl1), CSkeletonPrimitive::getBlobTreeNode(cyl2));
		//CBlend * blend = new CBlend(taper1, taper2);
		
		return blend;
	}
	*/

	static int createStringBlob(CLayerManager & lm)
	{
		vec3 xAxis(1.0f,0.0f,0.0f);
		vec3 yAxis(0.0f,1.0f,0.0f);
		vec3 zAxis(0.0f,0.0f,1.0f);

		CSkeletonRing * ringUP = new CSkeletonRing(vec3(4.0f, 2.5f, 0.0f), zAxis, 1.0f);
		CSkeletonRing * ringDN = new CSkeletonRing(vec3(4.0f, 0.5f, 0.0f), zAxis, 1.0f);
		CSkeletonCylinder * cyl1 = new CSkeletonCylinder(vec3(5.25f, -0.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.03f, 4.0f);		
		CSkeletonPrimitive * primUp = new CSkeletonPrimitive(ringUP);
		CSkeletonPrimitive * primDn = new CSkeletonPrimitive(ringDN);		
		CSkeletonPrimitive * primLeft = new CSkeletonPrimitive(cyl1);		

		primUp->setMaterial(CMaterial::mtrlBlue());
		primUp->setRange(0.6f);
		primDn->setMaterial(CMaterial::mtrlBlue());
		primDn->setRange(0.6f);
		primLeft->setMaterial(CMaterial::mtrlBlue());
		primLeft->setRange(0.6f);
		CBlend * blend1 = new CBlend(primUp, primDn, primLeft);				
		lm.addLayer(blend1);
		lm.getLast()->setPolySeedPoint(vec3(5.0f, 0.0f, 0.0f));
		lm.getLast()->setGroupName("BLOB B");

		//****************
		CSkeletonCylinder * cyl5 = new CSkeletonCylinder(vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), 0.05f, 3.0f);		
		CSkeletonCylinder * cyl6 = new CSkeletonCylinder(vec3(1.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), 0.05f, 2.5f);
		CSkeletonPrimitive * prim5 = new CSkeletonPrimitive(cyl5);
		CSkeletonPrimitive * prim6 = new CSkeletonPrimitive(cyl6);
		prim5->setMaterial(CMaterial::mtrlRedPlastic());
		prim6->setMaterial(CMaterial::mtrlRedPlastic());
		CBlend * blend2 = new CBlend(prim5, prim6);
		lm.addLayer(blend2);
		lm.getLast()->setPolySeedPoint(vec3(1.0f, 0.0f, 0.0f));
		lm.getLast()->setGroupName("BLOB L");
		//****************
		CSkeletonRing * ring = new CSkeletonRing(vec3(-5.3f, 1.3f, 0.0f), zAxis, 2.0f);		
		CSkeletonPrimitive * prim1 = new CSkeletonPrimitive(ring);
		prim1->setRange(0.6f);
		prim1->setMaterial(CMaterial::mtrlGreen());
		lm.addLayer(prim1);
		lm.getLast()->setPolySeedPoint(vec3(-6.0f, 1.0f, 0.0f));
		lm.getLast()->setGroupName("BLOB O");
		//***************			
		CBlend* blend4 = dynamic_cast<CBlend*>(cloneBlobTree(blend1));		
		CAffine * affine = new CAffine(blend4);		
		affine->setTranslate(vec3(-14.0f, 0.0f, 0.0f));		
	
		lm.addLayer(affine);
		lm.getLast()->setPolySeedPoint(vec3(-9.0f, 0.0f, 0.0f));
		lm.getLast()->setGroupName("BLOB LastB");
		
		
		lm.setCellSize(0.15f);		
		lm.setPolyBounds(500);
		return 1;
	}

	 

/*

	static CBlobTree * createSimpleBlob()
	{
		CSkeletonPoint * left = new CSkeletonPoint(vec3(-0.8f, 0.0f, 0.0f));		
		CSkeletonPoint * right = new CSkeletonPoint(vec3(0.8f, 0.0f, 0.0f));
		CSkeletonPrimitive * prim1 = CSkeletonPrimitive::getBlobTreeNode(left);
		prim1->setMaterial(CMaterial::mtrlBlue());

		CSkeletonPrimitive * prim2 = CSkeletonPrimitive::getBlobTreeNode(right);
		prim2->setMaterial(CMaterial::mtrlRedPlastic());
		CBlend * blend = new CBlend(prim1, prim2);



		return blend;
	}

*/

};


#endif