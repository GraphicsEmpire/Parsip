#include <stack>

#include "_GlobalFunctions.h"
#include "PS_BlobTree/include/BlobTreeLibraryAll.h"

bool FindSeedPoint(CBlobTree* lpNode, bool bFindHot, float iso_value, int ctTries, vec3f& p, float& fp, size_t& fieldEvaluations)
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


bool FindSeedPoint(CBlobTree* lpNode, bool bFindHot, float iso_value, int ctTries, float search_step, vec3f search_dir, vec3f& p, float& fp, size_t& fieldEvaluations)
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


CBlobTree* cloneNode(CBlobTree* node, int id)
{
	CBlobTree* output = NULL;

	BlobNodeType nodeType = node->getNodeType();

	bool bOperator = node->isOperator();
	if(bOperator)
	{
		switch(nodeType)
		{
		case(bntOpUnion):
			output = new CUnion();
			break;
		case(bntOpBlend):
			output = new CGradientBlend();
			break;		
		case(bntOpRicciBlend):
			output = new CRicciBlend(reinterpret_cast<CRicciBlend*>(node)->getN());
			break;
		case(bntOpIntersect):
			output = new CIntersection();
			break;
		case(bntOpDif):
			output = new CDifference();
			break;
		case(bntOpSmoothDif):
			output = new CSmoothDifference();
			break;
		case(bntOpAffine):
			{
				CAffine* aff = new CAffine();
				aff->setParamsFrom(reinterpret_cast<CAffine*>(node));
				output = aff;
			}
			break;
		case(bntOpWarpBend):
			{		
				CWarpBend* wb = new CWarpBend();
				wb->setParamFrom(reinterpret_cast<CWarpBend*>(node));
				output = wb;
			}
			break;
		case(bntOpWarpShear):
			{
				CWarpShear* ws = new CWarpShear();
				ws->setParamFrom(reinterpret_cast<CWarpShear*>(node));
				output = ws;
			}
			break;
		case(bntOpWarpTaper):
			{
				CWarpTaper* wt = new CWarpTaper();
				wt->setParamFrom(reinterpret_cast<CWarpTaper*>(node));
				output = wt;
			}
			break;
		case(bntOpWarpTwist):
			{
				CWarpTwist* wtwist = new CWarpTwist();
				wtwist->setParamFrom(reinterpret_cast<CWarpTwist*>(node));
				output = wtwist;
			}
			break;	
		default:
			throw "Unable to Clone!";
		}
	}
	else
	{
		CSkeletonPrimitive* sprim = reinterpret_cast<CSkeletonPrimitive*>(node);
		output = new CSkeletonPrimitive(sprim);
	}

	output->getTransform().set(node->getTransform());
	output->setID(id);
	return output;
}

//Clones a complete BlobTree adds each parent node paired with a child to be cloned in a stack
CBlobTree* cloneBlobTree(CBlobTree* input, int rootID, int* lpCtClonned )
{	
	if(input == NULL) return NULL;
	int ctClonned = 1;
	CBlobTree* inputRoot  = input;
	CBlobTree* outputRoot = cloneNode(input, rootID++);

	//Pair First is Parent, Second in Similar Child to be cloned
        std::stack< pair<CBlobTree *, CBlobTree *> > stkChildren;
	for(size_t i=0; i < inputRoot->countChildren(); i++)
	{
		pair<CBlobTree*, CBlobTree*> MyPair(outputRoot, inputRoot->getChild(i));
		stkChildren.push(MyPair);
	}

	CBlobTree* curParent = NULL;
	CBlobTree* curChild = NULL;
	CBlobTree* simChild = NULL;
	pair<CBlobTree*, CBlobTree*> curPair;
	while(!stkChildren.empty())
	{
		curPair = stkChildren.top();
		stkChildren.pop();

		curParent = curPair.first;
		simChild = curPair.second;
		//Child will be cloned first
		ctClonned++;
		curChild  = cloneNode(simChild, rootID++);
		curParent->addChild(curChild);
		for(size_t i=0; i< curChild->countChildren(); i++)
		{
			curPair.first = curChild;
			curPair.second = curChild->getChild(i);
			stkChildren.push(curPair);
		}
	}

	if(lpCtClonned)
		*lpCtClonned = ctClonned;
	return outputRoot;
}

/*
void *aligned_malloc_ps(size_t size, size_t align_size)
{
	char *ptr,*ptr2,*aligned_ptr;
	int align_mask = align_size - 1;

	ptr=(char *)malloc(size + align_size + sizeof(int));
	if(ptr==NULL) return(NULL);

	ptr2 = ptr + sizeof(int);
	aligned_ptr = ptr2 + (align_size - ((size_t)ptr2 & align_mask));


	ptr2 = aligned_ptr - sizeof(int);
	*((int *)ptr2)=(int)(aligned_ptr - ptr);

	return(aligned_ptr);
}

void aligned_free_ps(void *ptr) 
{
	int ptr2 = (static_cast<int *>(ptr))[-1];

	int* iptr = (int *) ptr;
	iptr = (int*)((int)iptr - ptr2);
	ptr = (void*)iptr;
	free(ptr);
}
*/

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

CBlobTree* CompactBlobTree( CBlobTree* input )
{
	if(input == NULL) return NULL;

	CBlobTree* clonnedRoot   = cloneNode(input);
	CBlobTree* parentClonned = clonnedRoot;
	CBlobTree* parentActual = input;
	CBlobTree* kid = NULL;
	CBlobTree* clonnedKid = NULL;

	//Stack of clonned and actual trees
	typedef pair<CBlobTree*, CBlobTree*> ClonnedActualPair;
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

bool isEquivalentOp( CBlobTree* a, CBlobTree* b )
{
	if(a->isOperator())
	{
		if((a->getNodeType() == bntOpRicciBlend)&&(b->getNodeType() == bntOpRicciBlend))
		{
			CRicciBlend* ricciA = reinterpret_cast<CRicciBlend*>(a);
			CRicciBlend* ricciB = reinterpret_cast<CRicciBlend*>(b);
			return (ricciA->getN() == ricciB->getN());
		}
		else
			return (a->getNodeType() == b->getNodeType());
	}
	return false;
}

bool isCompactableOp( BlobNodeType bnt )
{
	if((bnt == bntOpUnion)||(bnt == bntOpIntersect)||(bnt == bntOpDif)||(bnt == bntOpSmoothDif)||(bnt == bntOpBlend)||(bnt == bntOpRicciBlend))
		return true;
	else 
		return false;
}


