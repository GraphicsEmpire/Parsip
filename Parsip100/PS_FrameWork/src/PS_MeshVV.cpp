#include "GL/glew.h"
#include "PS_MeshVV.h"

#include "PS_FrameWork/include/PS_ErrorManager.h"
#include "PS_FrameWork/include/PS_FileDirectory.h"
#include "PS_FrameWork/include/_dataTypes.h"


#pragma warning( push )
#pragma warning( disable : 4995 )
#include <fstream>
#include <sstream>
#pragma warning( pop )


namespace PS{

	//Temp Face used for writing obj files
	struct TempFace
	{
		int ctSides;
		vec4ui face;
	};


/**
*/
CMeshVV::CMeshVV(POLYGON_MODE mode, 
				 int vertexSize,
				 int colorSize, 
				 int texChannels,
				 int texCoordSize)
{
	removeAll();
	this->m_faceMode = mode;
	this->setUnitVertexSize(vertexSize);
	this->setUnitColorSize(colorSize);
	this->setTexChannelSize(texChannels);
	this->setUnitTexCoordSize(texCoordSize);	
}

CMeshVV::CMeshVV(const CMeshVV& other)
{
	removeAll();
	this->copyFrom(other);
}

void CMeshVV::initMesh( POLYGON_MODE mode, 
					   int szUnitVertex, 
					   int szUnitColor, 
					   int texChannels, 
					   int szUnitTexCoord )
{
	removeAll();
	m_faceMode		= mode;
	m_szUnitVertex	= szUnitVertex;
	m_szUnitColor	= szUnitColor;
	m_szTexChannels = texChannels;
	setUnitTexCoordSize(szUnitTexCoord);
}

CMeshVV::~CMeshVV(void)
{
	this->removeAll();
}

bool CMeshVV::performCompleteTest()
{
	int ctFaceElements	  = getUnitFaceSize();
	int ctVertexElements  = getVertexSize();
	size_t ctVertices = countVertices();
	size_t ctFaces    = countFaces();

	size_t ctErrors   = 0;

	//1. Check if all vertices are used
	int* vertexUsed = new int[ctVertices];	
	memset(vertexUsed, 0, sizeof(int) * ctVertices);

	unsigned int* pFaceIndices = NULL;
	size_t idxVertex;
	for(size_t iFace=0; iFace < ctFaces; iFace++)
	{
		pFaceIndices = const_cast<unsigned int*>( getFace(iFace) );
		for(int i=0; i < ctFaceElements; i++)
		{
			idxVertex = pFaceIndices[i]; 

			if(idxVertex < 0 || idxVertex >= ctVertices)
			{
				idxVertex = idxVertex;
				ctErrors++;
			}
			else
				vertexUsed[idxVertex]++;
		}
	}

	for(size_t iVertex=0; iVertex < ctVertices; iVertex++)
	{
		if(vertexUsed[iVertex] == 0)
		{
			vertexUsed[iVertex] = vertexUsed[iVertex];
			vec3 v1 = getVertex3(iVertex);
			vec3 v2 = getVertex3(iVertex+1);
			ctErrors++;
		}
	}
	SAFE_DELETE_ARRAY(vertexUsed);
	return (ctErrors == 0);
}


void CMeshVV::setModeByFaceSides(int sides)
{
	switch(sides)
	{
	case 2:
		m_faceMode = LINES;
		break;
	case 3:
		m_faceMode = TRIANGLES;
		break;
	case 4:
		m_faceMode = QUADS;
		break;
	default:
		m_faceMode = TRIANGLES;
	}
}
/**
*/
void CMeshVV::setMode(POLYGON_MODE mode)
{
	m_faceMode = mode;
}

/**
*/
CMeshVV::POLYGON_MODE CMeshVV::getMode(void) const
{
	return CMeshVV::POLYGON_MODE(m_faceMode);
}

/**
*/
void CMeshVV::addLabel(int label)
{ 
	m_lstLabels.push_back(label);
}

/**
*/
void CMeshVV::setLabel(int n, int label)
{
	if (n < (int)m_lstLabels.size())
		m_lstLabels[n] = label;
	else 
		MarkError();
}

/**
*/
int CMeshVV::getLabel(int n) const
{
	if (n < (int)m_lstLabels.size())
		return m_lstLabels[n];

	MarkError();

	return 0;
}

/**
*/
const int *CMeshVV::getLabelArray(void) const
{
	if (m_lstLabels.empty())
		return 0;

	return &m_lstLabels[0];
}

/**
*/
int CMeshVV::countLabels(void) const
{
	return (int)m_lstLabels.size();
}

/**
*/
void CMeshVV::clearLabels(void)
{
	m_lstLabels.clear();
}

void CMeshVV::addColor(vec3f color)
{
	m_lstColors.push_back(color.x);
	m_lstColors.push_back(color.y);
	m_lstColors.push_back(color.z);
}

void CMeshVV::addColor(vec4f color)
{
	m_lstColors.push_back(color.x);
	m_lstColors.push_back(color.y);
	m_lstColors.push_back(color.z);
	m_lstColors.push_back(color.w);
}

bool CMeshVV::setColor(int n, vec3f color)
{
	if(m_szUnitColor != 3)
		return false;

	float *lpColor;
	n *= m_szUnitColor;

	if(n < (int)m_lstColors.size())
	{
		lpColor = &m_lstColors[0] + n;

		*lpColor++ = color.x;
		*lpColor++ = color.y;
		*lpColor   = color.z;
	}
	return true;
}

bool CMeshVV::setColor(int n, vec4f color)
{
	if(m_szUnitColor != 4)
		return false;

	float *lpColor;
	n *= m_szUnitColor;

	if(n < (int)m_lstColors.size())
	{
		lpColor = &m_lstColors[0] + n;

		*lpColor++ = color.x;
		*lpColor++ = color.y;
		*lpColor++ = color.z;
		*lpColor   = color.w;
	}
	return true;
}

void CMeshVV::setUnitColorSize(int size)
{
	if (size > 0)
		m_szUnitColor = size;
	else 
		MarkError();
}


/**
*/
const float *CMeshVV::getColorArray(void) const
{
	if (m_lstColors.empty())
		return 0;

	return &m_lstColors[0];
}

int CMeshVV::countColors(void) const
{
	if (!m_szUnitColor)
		return 0;

	return (int)m_lstColors.size()/m_szUnitColor;
}

void CMeshVV::clearColors(void)
{
	m_szUnitColor = 0;
	m_lstColors.clear();
}

bool CMeshVV::isValidTexChannel(int idxChannel) const
{
	return ((idxChannel >=0)&&(idxChannel < m_szTexChannels));
}

bool CMeshVV::isValidTexChannelAndCoord(int idxChannel, unsigned int idxCoord) const
{
	if((idxChannel >=0)&&(idxChannel < m_szTexChannels))
	{
		int ctUnit = m_szUnitTexCoord[idxChannel];
		if(ctUnit > 0)
		{
			unsigned int ctCoords  = m_lstTexChannels[idxChannel].size() / ctUnit;
			return ((idxCoord >= 0)&&(idxCoord < ctCoords));
		}
	}

	return false;		
}

void CMeshVV::addTexCoord(int idxChannel, float tex)
{
	if(isValidTexChannel(idxChannel))
		m_lstTexChannels[idxChannel].push_back(tex);
}

void CMeshVV::addTexCoord(int idxChannel, vec2f tex)
{
	if(isValidTexChannel(idxChannel))
	{
		m_lstTexChannels[idxChannel].push_back(tex.x);
		m_lstTexChannels[idxChannel].push_back(tex.y);
	}
}

void CMeshVV::addTexCoord(int idxChannel, vec3f tex)
{
	if(isValidTexChannel(idxChannel))
	{
		m_lstTexChannels[idxChannel].push_back(tex.x);
		m_lstTexChannels[idxChannel].push_back(tex.y);
		m_lstTexChannels[idxChannel].push_back(tex.z);
	}
}

void CMeshVV::addTexCoord(int idxChannel, vec4f tex)
{
	if(isValidTexChannel(idxChannel))
	{
		m_lstTexChannels[idxChannel].push_back(tex.x);
		m_lstTexChannels[idxChannel].push_back(tex.y);
		m_lstTexChannels[idxChannel].push_back(tex.z);
		m_lstTexChannels[idxChannel].push_back(tex.w);
	}
}
/*
void CMeshVV::setTexCoord(int idxChannel, UINT idxTexCoord, float tex)
{


}

void CMeshVV::setTexCoord(int idxChannel, UINT idxTexCoord, vec2f tex)
{

}

void CMeshVV::setTexCoord(int idxChannel, UINT idxTexCoord, vec3f tex)
{

}

void CMeshVV::setTexCoord(int idxChannel, UINT idxTexCoord, vec4f tex)
{

}

*/
vec4f CMeshVV::getTexCoord(int idxChannel, UINT idxTexCoord)
{
	return vec4f (&m_lstTexChannels[idxChannel][idxTexCoord]);
}


void CMeshVV::setUnitTexCoordSize(int size)
{
	for(int i=0; i<MAX_TEXTURE_CHANNELS; i++)
		m_szUnitTexCoord[i] = size;
}

void CMeshVV::setUnitTexCoordSize(int idxChannel, int size)
{
	if((idxChannel >= 0)&&(idxChannel < MAX_TEXTURE_CHANNELS))
		m_szUnitTexCoord[idxChannel] = size;
}

int	 CMeshVV::getUnitTexCoordSize(int idxChannel) const
{
	if((idxChannel >= 0)&&(idxChannel < MAX_TEXTURE_CHANNELS))
		return m_szUnitTexCoord[idxChannel];
	else
		return 0;
}

void CMeshVV::setTexChannelSize(int size)
{
	m_szTexChannels = size;
}

int CMeshVV::getTexChannelSize() const
{
	return m_szTexChannels;
}


unsigned int CMeshVV::countTexCoords(int idxChannel) const
{
	if(isValidTexChannel(idxChannel))
		if(m_szUnitTexCoord[idxChannel] > 0)
			return m_lstTexChannels[idxChannel].size() / m_szUnitTexCoord[idxChannel];
	return 0;
}

void CMeshVV::clearTextures()
{
	for(size_t iChannel = 0; iChannel < MAX_TEXTURE_CHANNELS; iChannel++)
	{
		m_lstTexChannels[iChannel].clear();		
	}
}


void CMeshVV::addNormal(vec3 n)
{
	addNormal(n.x, n.y, n.z);
}

void CMeshVV::addNormal(float x, float y, float z)
{
	m_lstNormals.push_back(x);
	m_lstNormals.push_back(y);
	m_lstNormals.push_back(z);
}

void CMeshVV::addNormal(const float *begin, const float *end)
{
	int size;

	size = (int)(end - begin);

	if (size == 3)
		m_lstNormals.insert(m_lstNormals.end(), begin, end);
	else
		MarkError();
}

void CMeshVV::setNormal(int n, float x, float y, float z)
{
	float *normal;

	n *= 3;

	if (n < (int)m_lstNormals.size())
	{
		normal = &m_lstNormals[0] + n;

		*normal++ = x;
		*normal++ = y;
		*normal = z;
	}
	else 
		MarkError();
}

void CMeshVV::setNormal(int n, const float *begin, const float *end)
{
	int size;
	float *normal;

	size = (int)(end - begin);

	n *= 3;

	if (n < (int)m_lstNormals.size())
	{
		normal = &m_lstNormals[0] + n;

		for ( ; size--; )
			*normal++ = *begin++;
	}
	else 
		MarkError();
}

const float *CMeshVV::getNormal(int n) const
{
	n *= 3;

	if (n < (int)m_lstNormals.size())
		return (&m_lstNormals[0] + n);

	MarkError();

	return 0;
}

vec3 CMeshVV::getNormal3(int n) const
{
	vec3f normal;
	normal.set(m_lstNormals[n*3+0], m_lstNormals[n*3+1], m_lstNormals[n*3+2]);
	return normal;
}

const float *CMeshVV::getNormalArray(void) const
{
	if (m_lstNormals.empty())
		return 0;

	return &m_lstNormals[0];
}

unsigned int CMeshVV::countNormals(void) const
{
	return (int)m_lstNormals.size()/3;
}

void CMeshVV::clearNormals(void)
{
	m_lstNormals.clear();
}


void CMeshVV::addVertex(vec2 v2)
{
	addVertex(v2.x, v2.y);
}

void CMeshVV::addVertex(vec3 v3)
{
	addVertex(v3.x, v3.y, v3.z);
}


void CMeshVV::addVertex(float x)
{ 
	if (!m_szUnitVertex)
		m_szUnitVertex = 1;

	if (m_szUnitVertex == 1)
		m_lstVertices.push_back(x);
	else
		MarkError();
}

void CMeshVV::addVertex(float x, float y)
{ 
	if (!m_szUnitVertex)
		m_szUnitVertex = 2;

	if (m_szUnitVertex == 2)
	{
		m_lstVertices.push_back(x);
		m_lstVertices.push_back(y);
	}
	else
		MarkError();
}

void CMeshVV::addVertex(float x, float y, float z)
{ 
	if (!m_szUnitVertex)
		m_szUnitVertex = 3;

	if (m_szUnitVertex == 3)
	{
		m_lstVertices.push_back(x);
		m_lstVertices.push_back(y);
		m_lstVertices.push_back(z);
	}
	else
		MarkError();
}

void CMeshVV::addVertex(float x, float y, float z, float w)
{ 
	if (!m_szUnitVertex)
		m_szUnitVertex = 4;

	if (m_szUnitVertex == 4)
	{
		m_lstVertices.push_back(x);
		m_lstVertices.push_back(y);
		m_lstVertices.push_back(z);
		m_lstVertices.push_back(w);
	}
	else
		MarkError();
}

void CMeshVV::addVertex(const float *begin, const float *end)
{
	int size;

	size = (int)(end - begin);

	if (!m_szUnitVertex)
		m_szUnitVertex = size;

	if (m_szUnitVertex == size)
		m_lstVertices.insert(m_lstVertices.end(), begin, end);
	else
		MarkError();
}

void CMeshVV::setVertex(int n, vec2 v2)
{
	setVertex(n, v2.x, v2.y);
}

void CMeshVV::setVertex(int n, vec3 v3)
{
	setVertex(n, v3.x, v3.y, v3.z);
}

void CMeshVV::setVertex(int n, float x)
{
	float *vertex;

	n *= m_szUnitVertex;

	if ((n < (int)m_lstVertices.size()) && (m_szUnitVertex == 1))
	{
		vertex = &m_lstVertices[0] + n;

		*vertex = x;
	}
	else 
		MarkError();
}

void CMeshVV::setVertex(int n, float x, float y)
{
	float *vertex;

	n *= m_szUnitVertex;

	if ((n < (int)m_lstVertices.size()) && (m_szUnitVertex == 2))
	{
		vertex = &m_lstVertices[0] + n;

		*vertex++ = x;
		*vertex = y;
	}
	else 
		MarkError();
}

void CMeshVV::setVertex(int n, float x, float y, float z)
{
	float *vertex;

	n *= m_szUnitVertex;

	if ((n < (int)m_lstVertices.size()) && (m_szUnitVertex == 3))
	{
		vertex = &m_lstVertices[0] + n;

		*vertex++ = x;
		*vertex++ = y;
		*vertex = z;
	}
	else 
		MarkError();
}

void CMeshVV::setVertex(int n, float x, float y, float z, float w)
{
	float *vertex;

	n *= m_szUnitVertex;

	if ((n < (int)m_lstVertices.size()) && (m_szUnitVertex == 4))
	{
		vertex = &m_lstVertices[0] + n;

		*vertex++ = x;
		*vertex++ = y;
		*vertex++ = z;
		*vertex = w;
	}
	else 
		MarkError();
}

void CMeshVV::setVertex(int n, const float *begin, const float *end)
{
	int size;
	float *vertex;

	size = (int)(end - begin);

	n *= m_szUnitVertex;

	if ((n < (int)m_lstVertices.size()) && (m_szUnitVertex == size))
	{
		vertex = &m_lstVertices[0] + n;

		for ( ; size--; )
			*vertex++ = *begin++;
	}
	else 
		MarkError();
}

void CMeshVV::setUnitVertexSize(int size)
{
	if (size > 0)
		m_szUnitVertex = size;
	else 
		MarkError();
}

const float *CMeshVV::getVertex(int n) const
{
	n *= m_szUnitVertex;

	if (n < (int)m_lstVertices.size())
		return (&m_lstVertices[0] + n);

	MarkError();

	return 0;
}

vec3  CMeshVV::getVertex3(int n) const
{
	return vec3(getVertex(n));
}

const float *CMeshVV::getVertexArray(void) const
{
	if (m_lstVertices.empty())
		return 0;

	return &m_lstVertices[0];
}

int CMeshVV::getVertexSize(void) const
{
	return m_szUnitVertex;
}

unsigned int CMeshVV::countVertices(void) const
{
	if (!m_szUnitVertex)
		return 0;

	return m_lstVertices.size()/m_szUnitVertex;
}

void CMeshVV::clearVertices(void)
{
	m_szUnitVertex = 0;

	m_lstVertices.clear();
}

void CMeshVV::addFace(unsigned int vertexId)
{
	m_lstFaces.push_back(vertexId);
}

void CMeshVV::addFaceArray(DVec<unsigned int>& input)
{
	for(size_t i=0; i < input.size(); i++)
		m_lstFaces.push_back(input[i]);
}

void CMeshVV::addFaceArray(const unsigned int *begin, const unsigned int *end)
{
	m_lstFaces.insert(m_lstFaces.end(), begin, end);
}

void CMeshVV::addLine(unsigned int firstId, unsigned int secondId)
{
	if (!m_faceMode)
		m_faceMode = LINES;

	if (m_faceMode == LINES)
	{
		m_lstFaces.push_back(firstId);
		m_lstFaces.push_back(secondId);
	}
	else
		MarkError();
}

void CMeshVV::addTriangle(unsigned int firstId, unsigned int secondId, unsigned int thirdId)
{
	if (!m_faceMode)
		m_faceMode = TRIANGLES;

	if (m_faceMode == TRIANGLES)
	{
		m_lstFaces.push_back(firstId);
		m_lstFaces.push_back(secondId);
		m_lstFaces.push_back(thirdId);
	}
	else
		MarkError();
}

void CMeshVV::addQuad(unsigned int* ids)
{
	addQuad(ids[0], ids[1], ids[2], ids[3]);
}

void CMeshVV::addQuad(unsigned int firstId, unsigned int secondId, unsigned int thirdId, unsigned int fourthId)
{
	if (!m_faceMode)
		m_faceMode = QUADS;

	if (m_faceMode == QUADS)
	{
		m_lstFaces.push_back(firstId);
		m_lstFaces.push_back(secondId);
		m_lstFaces.push_back(thirdId);
		m_lstFaces.push_back(fourthId);
	}
	else
		MarkError();
}

const unsigned int *CMeshVV::getElementArray(void) const
{
	if (m_lstFaces.empty())
		return 0;

	return &m_lstFaces[0];
}

int CMeshVV::getFaceArraySize(void) const
{
	return (int)m_lstFaces.size();
}

void CMeshVV::clearFaces(void)
{
	m_faceMode = CMeshVV::POINTS;

	m_lstFaces.clear();
}

unsigned int CMeshVV::getFaceMode(void) const
{
	return m_faceMode;
}


int CMeshVV::getFace(size_t idxFace, vec3* arrVertices, size_t szVerticesBuffer)
{
	size_t ctFaceSides = getUnitFaceSize();
	if(szVerticesBuffer < ctFaceSides)
		return -1;
	if(arrVertices == NULL)
		return -2;
	
	const size_t* arrPrimitive = getFace(idxFace);
	for(size_t i=0; i < ctFaceSides; i++)
	{			
		arrVertices[i] = getVertex3(arrPrimitive[i]);
	}

	return ctFaceSides;
}

const unsigned *CMeshVV::getFace(unsigned int primitiveId) const
{
	int size;

	if (m_lstFaces.empty())
		return 0;

	switch (m_faceMode)
	{
	case POINTS:
		size = 1;
		break;
	case LINES:
		size = 2;
		break;
	case TRIANGLES:
		size = 3;
		break;
	case QUADS:
		size = 4;
		break;
	default:
		return 0;
	}

	return &m_lstFaces[0] + primitiveId*size;
}

/**
*/
int CMeshVV::getUnitFaceSize(void) const
{
	int size;

	switch (m_faceMode)
	{
	case POINTS:
		size = 1;
		break;
	case LINES:
		size = 2;
		break;
	case TRIANGLES:
		size = 3;
		break;
	case QUADS:
		size = 4;
		break;
	default:
		return -1;
	}

	return size;
}

/**
*/
unsigned int CMeshVV::countFaces(void) const
{
	int size;

	switch (m_faceMode)
	{
	case POINTS:
		size = 1;
		break;
	case LINES:
		size = 2;
		break;
	case TRIANGLES:
		size = 3;
		break;
	case QUADS:
		size = 4;
		break;
	default:
		return -1;
	}

	return m_lstFaces.size()/size;
}

void CMeshVV::removeAll(void)
{	
	m_lstLabels.clear();

	m_szUnitVertex = 0;
	m_lstVertices.clear();

	m_szUnitColor = 0;
	m_lstColors.clear();

	m_lstNormals.clear();	
	
	m_faceMode = POINTS;
	m_lstFaces.clear();
	clearTextures();
}

bool CMeshVV::open(const DAnsiStr& strFileName)
{
	if(!PS::FILESTRINGUTILS::fileExists(strFileName.c_str()))
		return false;
	
	DAnsiStr strExt = PS::FILESTRINGUTILS::extractFileExt(strFileName);
	strExt.toUpper();

	if(strcmp(strExt.c_str(), "OFF") == 0)
		return openOFF(strFileName);
	else if(strcmp(strExt.c_str(), "OBJ") == 0)
		return openOBJ(strFileName);
	else if(strcmp(strExt.c_str(), "msh") == 0)
		return openPSBinary(strFileName);
	else
		return false;
}

//type and file formats to open
bool CMeshVV::open(const DAnsiStr& strFileName, STREAM_FORMAT type)
{
	switch (type) 
	{
	case OFF:
		if (!openOFF(strFileName))
		{
			MarkError();

			return false;
		}
		break;
	case STEVEN_OFF:
		if (!openStevenOFF(strFileName))
		{
			MarkError();

			return false;
		}
		break;
	case OBJ:
		if (!openOBJ(strFileName))
		{
			MarkError();

			return false;
		}
		break;
	case PS_BINARY:
		if(!openPSBinary(strFileName))
		{
			MarkError();

			return false;
		}
	case PLY:
	case ASCII_STL:
	case SURFEL:
		MarkError();

		return false;
	case CRYSTAL_STRUCTURE_DISLOCATIONS:
		if (!openCrystalStructureDislocation(strFileName))
		{
			MarkError();

			return false;
		}
		break;
	}

	return true;
}

/**
*/
bool CMeshVV::save(const DAnsiStr& strFileName, STREAM_FORMAT type) const
{
	ofstream modelFile;
	const float *vertex,
		*color;
	const unsigned int *element;
	int vertexCount,
		elementCount,
		primitiveSize,
		primitiveCount,
		i;

	if (m_lstVertices.empty())
	{
		MarkError();

		return false;
	}

	vertex = &m_lstVertices[0];
	vertexCount = (int)m_lstVertices.size()/m_szUnitVertex;

	if (m_lstFaces.empty())
	{
		MarkError();

		return false;
	}

	element = &m_lstFaces[0];
	elementCount = (int)m_lstFaces.size();

	if (!m_lstColors.empty())
		color = &m_lstColors[0];
	else
		color = 0;

	primitiveSize = 1;

	switch (m_faceMode) {
  case GL_POINTS:
	  primitiveSize = 1;
	  break;
  case GL_LINES:
	  primitiveSize = 2;
	  break;
  case GL_TRIANGLES:
	  primitiveSize = 3;
	  break;
  case GL_QUADS:
	  primitiveSize = 4;
	  break;
  case GL_LINE_LOOP:
  case GL_LINE_STRIP:
  case GL_TRIANGLE_STRIP:
  case GL_TRIANGLE_FAN:
  case GL_QUAD_STRIP:
  case GL_POLYGON:
	  MarkError();

	  return false;
	};

	primitiveCount = elementCount/primitiveSize;

	switch (type) {
  case OFF:
	  modelFile.open(strFileName.c_str());

	  modelFile << "OFF" << endl;
	  modelFile << vertexCount << " " << primitiveCount << " " << primitiveCount*primitiveSize << endl;

	  for ( ; vertexCount--; )
	  {
		  for (i=0; i!=m_szUnitVertex; i++)
			  modelFile << *vertex++ << " ";

		  modelFile << endl;
	  }

	  for ( ; primitiveCount--; )
	  {
		  modelFile << primitiveSize << " ";

		  for (i=primitiveSize; i--; )
			  modelFile << *element++ << " ";

		  modelFile << endl;
	  }
	  break;
  case STEVEN_OFF:
	  if (!color)
	  {
		  MarkError();

		  return false;
	  }

	  modelFile.open(strFileName.c_str());

	  modelFile << vertexCount << " " << primitiveCount << endl;

	  for ( ; vertexCount--; )
	  {
		  for (i=0; i!=m_szUnitVertex; i++)
			  modelFile << *vertex++ << " ";

		  modelFile << (*color) << " ";

		  color += m_szUnitColor;

		  modelFile << endl;
	  }

	  for ( ; primitiveCount--; )
	  {
		  for (i=primitiveSize; i--; )
			  modelFile << *element++ << " ";

		  modelFile << endl;
	  }
	  break;
  case PLY:
	  modelFile.open(strFileName.c_str());

	  modelFile << "ply" << endl;
	  modelFile << "format ascii 1.0" << endl;

	  modelFile << "element vertex " << vertexCount << endl;
	  if (m_szUnitVertex >= 1)
		  modelFile << "property float32 x" << endl;
	  if (m_szUnitVertex >= 2)
		  modelFile << "property float32 y" << endl;
	  if (m_szUnitVertex >= 3)
		  modelFile << "property float32 z" << endl;
	  if (m_szUnitVertex >= 4)
		  modelFile << "property float32 w" << endl;

	  modelFile << "element face " << primitiveCount << endl;
	  modelFile << "property list uint8 int32 vertex_indices" << endl;

	  modelFile << "end_header" << endl;

	  for ( ; vertexCount--; )
	  {
		  for (i=0; i!=m_szUnitVertex; i++)
			  modelFile << *vertex++ << " ";

		  modelFile << endl;
	  }

	  for ( ; primitiveCount--; )
	  {
		  modelFile << primitiveSize << " ";

		  for (i=primitiveSize; i--; )
			  modelFile << *element++ << " ";

		  modelFile << endl;
	  }
	  break;
  case ASCII_STL:
	  if (!saveToASCIISTLFile(strFileName))
	  {
		  MarkError();

		  return false;
	  }
	  break;
  case SURFEL:
	  if (!saveToSURFELFile(strFileName))
	  {
		  MarkError();

		  return false;
	  }
	  break;
  case PS_BINARY:
	  if(!savePSBinary(strFileName))
	  {
		  MarkError();

		  return false;
	  }
	  break;
	};

	return true;
}

bool CMeshVV::openPSBinary(const DAnsiStr& strFileName)
{
	ifstream fpRead;
	fpRead.open(strFileName.ptr(), ios::in|ios::binary|ios::ate);
	if(fpRead.is_open())
	{
		return openPSBinary(fpRead);
	}
	else
		return false;	
}

bool CMeshVV::openPSBinary(ifstream& fpRead)
{
	//Polygon Mode + Color + Texture + Normal + Vertex + Element
	fpRead >> m_faceMode;	

	//Read Colors
	size_t ctColors;
	fpRead >> m_szUnitColor;	
	fpRead >> ctColors;
	float val;
	for(size_t i=0; i<ctColors*m_szUnitColor; i++)
	{	
		fpRead >> val;		
		m_lstColors.push_back(val);
	}

	//Normal
	size_t ctNormals;
	fpRead >> ctNormals;
	for (size_t i=0; i<3*ctNormals; i++)
	{
		fpRead >> val;	
		m_lstNormals.push_back(val);
	}

	//Vertex
	size_t ctVertices;
	fpRead >> m_szUnitVertex;
	fpRead >> ctVertices;
	for (size_t i=0; i<ctVertices*m_szUnitVertex; i++)
	{
		fpRead >> val;	
		m_lstVertices.push_back(val);
	}

	//Faces
	size_t ctFaces;
	unsigned int element;
	fpRead >> m_faceMode;
	fpRead >> ctFaces;	
	size_t ctFaceSize = getUnitFaceSize();		
	for (size_t i=0; i<ctFaces*ctFaceSize; i++)
	{
		fpRead >> element;	
		m_lstFaces.push_back(element);
	}		

	//Texture	
	size_t ctTexCoords[MAX_TEXTURE_CHANNELS];
	fpRead >> m_szTexChannels;
	for(int i=0; i < m_szTexChannels; i++)
	{
		fpRead >> m_szUnitTexCoord[i];	
		fpRead >> ctTexCoords[i];
	}
	
	for(int iChannel=0; iChannel < m_szTexChannels; iChannel++)
		for(size_t iCoord=0; iCoord < ctTexCoords[iChannel]*m_szUnitTexCoord[iChannel]; iCoord++)
		{	
			fpRead >> val;	
			m_lstTexChannels[iChannel].push_back(val);
		}

	fpRead.close();
	
	return true;
}

bool CMeshVV::savePSBinary(const DAnsiStr& strFileName) const
{
	ofstream fpWrite;
	
	fpWrite.open(strFileName.ptr(), ios::out | ios::binary | ios::ate);
	if(fpWrite.is_open())
		return savePSBinary(fpWrite);
	else
		return false;		
}


bool CMeshVV::savePSBinary(ofstream& fpWrite) const
{
	fpWrite << m_faceMode;	

	//Write Colors	
	fpWrite << m_szUnitColor;	
	fpWrite << countColors();
	for(size_t i=0; i<m_lstColors.size(); i++)
	{	
		fpWrite << m_lstColors[i];
	}

	//Normal		
	fpWrite << countNormals();
	for (size_t i=0; i<m_lstNormals.size(); i++)
	{		
		fpWrite << m_lstNormals[i];
	}

	//Vertex	
	fpWrite << m_szUnitVertex;
	fpWrite << countVertices();
	for (size_t i=0; i<m_lstVertices.size(); i++)
	{		
		fpWrite << m_lstVertices[i];
	}

	//Faces
	fpWrite << m_faceMode;
	fpWrite << countFaces();
	for (size_t i=0; i<m_lstFaces.size(); i++)
	{		
		fpWrite << m_lstFaces[i];
	}		

	//Texture		
	fpWrite << m_szTexChannels;
	for(int i=0; i < m_szTexChannels; i++)
	{
		fpWrite << m_szUnitTexCoord[i];	
		fpWrite << countTexCoords(i);
	}

	for(int iChannel=0; iChannel < m_szTexChannels; iChannel++)
		for(size_t iCoord=0; iCoord < m_lstTexChannels[iChannel].size(); iCoord++)
		{				
			fpWrite << m_lstTexChannels[iChannel][iCoord];
		}

	fpWrite.flush();
	fpWrite.close();

	return true;
}

/**
*/
bool CMeshVV::isValid(void) const
{
	size_t colorCount, normalCount,	vertexCount, elementCount;
	const unsigned int *element;

	colorCount = (int)m_lstColors.size();

	if (colorCount)
	{
		if (!m_szUnitColor)
			return false;

		colorCount /= m_szUnitColor;
	}

	size_t texCoordCount[MAX_TEXTURE_CHANNELS];
	for (int i=0; i < m_szTexChannels; i++)
	{
		texCoordCount[i] = m_lstTexChannels[i].size();

		if(m_szUnitTexCoord[i] > 0)
			texCoordCount[i] /= m_szUnitTexCoord[i];
		else
			return false;
	}

	normalCount = (int)m_lstNormals.size()/3;

	vertexCount = (int)m_lstVertices.size();

	if (vertexCount)
	{
		if (!m_szUnitVertex)
			return false;

		vertexCount /= m_szUnitVertex;

		if (colorCount)
			if (colorCount != vertexCount)
				return false;

		if (normalCount)
			if (normalCount != vertexCount)
				return false;

		/*
		for (int i=0; i < m_szTexChannels; i++)					
			if(texCoordCount[i] != vertexCount)
				return false;		
				*/
	}

	elementCount = (int)m_lstFaces.size();

	if (elementCount)
	{
		element = &m_lstFaces[0];

		for ( ; elementCount--; element++)
			if (*element > (unsigned int)vertexCount)
				return false;
	}

	return true;
}

/**
*/
bool CMeshVV::saveToASCIISTLFile(const DAnsiStr& fileName) const
{
	ofstream modelFileStream;
	// Pointers to triangle vertices.
	const float *firstVertex,
		*secondVertex,
		*thirdVertex;
	const unsigned int *element;
	int elementCount,
		primitiveCount,
		i;
	// Auxiliary vectors used to calculate the triangle normal.
	vec3 u,v, normal;

	if (m_lstVertices.empty())
	{
		MarkError();

		return false;
	}

	if (m_szUnitVertex != 3)
	{
		MarkError();

		return false;
	}

	if (m_lstFaces.empty())
	{
		MarkError();

		return false;
	}

	element = &m_lstFaces[0];
	elementCount = (int)m_lstFaces.size();

	if (m_faceMode != GL_TRIANGLES)
	{
		MarkError();

		return false;
	}

	primitiveCount = elementCount/3;

	modelFileStream.open(fileName.c_str());

	if (!modelFileStream.is_open())
	{
		MarkError();

		return false;
	}

	DAnsiStr temp = fileName.substr(0, fileName.length() - 4);
	modelFileStream << "solid " << temp << endl;

	for ( ; primitiveCount--; )
	{
		firstVertex = getVertex(element[0]);
		secondVertex = getVertex(element[1]);
		thirdVertex = getVertex(element[2]);

		for (i=0; i<3; i++)
		{
			u[i] = secondVertex[i] - firstVertex[i];
			v[i] = thirdVertex[i] - firstVertex[i];
		}

		normal.cross(u, v);
		normal.normalize();

		modelFileStream << "facet normal " << normal[0] << " " << normal[1] << " " << normal[2] << endl;

		modelFileStream << "  outer loop" << endl;

		modelFileStream << "    vertex   " << firstVertex[0] << " " << firstVertex[1] << " " << firstVertex[2] << endl;
		modelFileStream << "    vertex   " << secondVertex[0] << " " << secondVertex[1] << " " << secondVertex[2] << endl;
		modelFileStream << "    vertex   " << thirdVertex[0] << " " << thirdVertex[1] << " " << thirdVertex[2] << endl;

		modelFileStream << "  endloop" << endl;

		modelFileStream << "endfacet" << endl;

		element += 3;
	}

	modelFileStream << "endsolid" << endl;

	return true;
}

/**
*/
bool CMeshVV::saveToSURFELFile(const DAnsiStr& fileName) const
{
	ofstream modelFileStream;
	const float *vertexCoord,
		*normalCoord,
		*colorCoord;
	int primitiveCount,
		i;

	if (m_lstVertices.empty())
	{
		MarkError();

		return false;
	}

	if (m_szUnitVertex != 3)
	{
		MarkError();

		return false;
	}

	vertexCoord = &m_lstVertices[0];

	if (m_lstNormals.empty())
	{
		MarkError();

		return false;
	}

	normalCoord = &m_lstNormals[0];

	if (m_lstColors.empty())
		colorCoord = 0;
	else
		colorCoord = &m_lstColors[0];

	primitiveCount = countVertices();

	modelFileStream.open(fileName.c_str());

	if (!modelFileStream.is_open())
	{
		MarkError();

		return false;
	}

	modelFileStream << primitiveCount << endl;

	for (i=0; i<primitiveCount; i++)
	{
		modelFileStream << vertexCoord[0] << " " << vertexCoord[1] << " " << vertexCoord[2];

		modelFileStream << " ";

		if (colorCoord != 0)
			modelFileStream << colorCoord[0] << " " << colorCoord[1] << " " << colorCoord[2];
		else
			modelFileStream << "1.0 1.0 1.0";

		modelFileStream << " ";

		modelFileStream << normalCoord[0] << " " << normalCoord[1] << " " << normalCoord[2];

		modelFileStream << " ";

		modelFileStream << "1.0" << endl;

		vertexCoord += m_szUnitVertex;

		if (colorCoord != 0)
			colorCoord += m_szUnitColor;

		normalCoord += 3;
	}

	return true;
}


bool CMeshVV::openOBJ(const DAnsiStr& name)
{	
	initMesh(TRIANGLES, 3, 3, 1, 2);

    // find the file   
	std::wifstream ifs( name.c_str() );
    WCHAR line[256] = {0};    

    // Parse the .obj file. Both triangle faces and quad faces are supported.
    // Only v and f tags are processed, other tags like vn, vt etc are ignored.	
	vec3 pos;
	vec3 normal;
	vec2 texCoord;

	unsigned int idx[4];
	unsigned int i = 0;

	while ( ifs >> line )
	{
		if ( 0 == wcscmp( line, L"#" ) ) 
			ifs.getline( line, 255 );
		else
		{
			if ( wcscmp( line, L"v" ) == 0 )
			{				
				ifs >> pos.x >> pos.y >> pos.z;			
				addVertex(pos);
			}
			else if( wcscmp( line, L"vn" ) == 0 )
			{
				ifs >> normal.x >> normal.y >> normal.z;
				addNormal(normal);
			}
			else if( wcscmp( line, L"vt" ) == 0)
			{
				ifs >> texCoord.x >> texCoord.y;
				addTexCoord(0, texCoord);
			}
		}
	}

	DVec<TempFace> lstFaces;
	TempFace	tmpFace;	
	size_t ctTriangles = 0;
	size_t ctTotal = 0;
	

	//Count number of vertices per each face		
	ifs.clear( 0 );
	ifs.seekg( 0 );
	while ( ifs >> line )
	{
		if ( 0 == wcscmp( line, L"#" ) ) 
			ifs.getline( line, 255 );
		else
		{
			if ( 0 == wcscmp( line, L"f" ) )
			{
				ifs.getline( line, 255 );
				std::wstringstream ss(line);

				//Each line produces a face with associated vertices
				//Each vertex is VTN format: Vertex, Normal, Texture
				i = 0;
				while ( ss >> line )
				{
					std::wstringstream ss(line);
					ss >> idx[i++];
				}

				ctTotal++;
				if(i == 3)
					ctTriangles++;

				//Save for later decision
				tmpFace.face    = vec4ui(idx[0]-1, idx[1]-1, idx[2]-1, idx[3]-1);				
				tmpFace.ctSides = i;

				lstFaces.push_back(tmpFace);
			}
		}
	}        


	//Process
	if(ctTriangles > 0)
	{
		this->setMode(TRIANGLES);
		for (size_t iFace=0; iFace < lstFaces.size(); iFace++)
		{
			tmpFace = lstFaces[iFace];
			addTriangle( tmpFace.face.x, tmpFace.face.y, tmpFace.face.z);
			if(tmpFace.ctSides > 3)
				addTriangle( tmpFace.face.z, tmpFace.face.w, tmpFace.face.x);
		}
	}
	else
	{
		this->setMode(QUADS);
		for (size_t iFace=0; iFace < lstFaces.size(); iFace++)
		{
			tmpFace = lstFaces[iFace];
			addQuad(tmpFace.face.x, tmpFace.face.y, tmpFace.face.z, tmpFace.face.w);
		}
	}

	//Clear temp list
	lstFaces.clear();
	
	return (countFaces() > 0);
}

/**
*/
bool CMeshVV::openOFF(const DAnsiStr& fileName)
{
	// Declare temporary variables to read data into.
	// If the read goes well, we'll copy these into
	// our class variables, overwriting what used to 
	// be there. If it doesn't, we won't have messed up
	// our previous data structures.
	int tempNumPoints   = 0;	// Number of x,y,z coordinate triples
	int tempNumFaces    = 0;	// Number of polygon sets
	int tempNumEdges    = 0;	// Unused, except for reading.
	double** tempPoints = NULL;	// An array of x,y,z coordinates.

	// An array of arrays of point
	// pointers. Each entry in this
	// is an array of integers. Each
	// integer in that array is the
	// index of the x, y, and z
	// coordinates in the corresponding
	// arrays.
	int** tempFaces = NULL;		
	int*  tempFaceSizes = NULL;	// An array of polygon point counts.
	// Each of the arrays in the tempFaces
	// array may be of different lengths.
	// This array corresponds to that
	// array, and gives their lengths.
	int i;				// Generic loop variable.
	bool bres = true;		// Set to false if the file appears
	// not to be a valid OFF file.
	char tempBuf[128];		// A buffer for reading strings
	// from the file.

	ifstream ifs(fileName.c_str(), ios::in);

	// Grab the first string. If it's "OFF", we think this
	// is an OFF file and continue. Otherwise we give up.
	ifs >> tempBuf;
	if (strcmp(tempBuf, "OFF") != 0) 
	{
		bres = false;
	}

	// Read the sizes for our two arrays, and the third
	// int on the line. If the important two are zero
	// sized, this is a messed up OFF file. Otherwise,
	// we setup our temporary arrays.
	if (bres) 
	{
		ifs >> tempNumPoints >> tempNumFaces >> tempNumEdges;
		if (tempNumPoints < 1 ||
			tempNumFaces < 1) {
				// If either of these were negative, we make
				// sure that both are set to zero. This is
				// important for later deleting our temporary
				// storage.
				bres      = false;
				tempNumPoints = 0;
				tempNumFaces  = 0;
		} else {
			tempPoints = new double*[tempNumPoints];
			tempFaces = new int*[tempNumFaces];
			tempFaceSizes = new int[tempNumFaces];
		}
	}

	if (bres) {
		// Load all of the points.
		for (i = 0; i < tempNumPoints; i++) {
			tempPoints[i] = new double[3];
			ifs >> tempPoints[i][0] >> tempPoints[i][1] >> tempPoints[i][2];
		}

		// Load all of the faces.
		for (i = 0; i < tempNumFaces; i++) {
			// This tells us how many points make up
			// this face.
			ifs >> tempFaceSizes[i];
			// So we declare a new array of that size
			tempFaces[i] = new int[tempFaceSizes[i]];
			// And load its elements with the vertex indices.
			for (int j = 0; j < tempFaceSizes[i]; j++) {
				ifs >> tempFaces[i][j];
			}
			// Clear out any face color data by reading up to
			// the newline. 128 is probably considerably more
			// space than necessary, but better safe than
			// sorry.
			ifs.getline(tempBuf, 128);
		}
	}

	// Here is where we copy the data from the temp
	// structures into our permanent structures. We
	// probably will do some more processing on the
	// data at the same time. This code you must fill
	// in on your own.
	if (bres) 
	{
		this->removeAll();

		this->setUnitVertexSize(3);

		size_t ctFaceSidesQuad = 0;
		size_t ctFaceSidesTriangle = 0;

		for(i=0; i < tempNumFaces; i++)
		{
			if(tempFaceSizes[i] == 4)
				ctFaceSidesQuad++;
			else
				ctFaceSidesTriangle++;
		}

		if((ctFaceSidesTriangle == 0)&&(ctFaceSidesQuad > 0))
			this->setMode(QUADS);
		else
			this->setMode(TRIANGLES);
			

		//Add all points
		for(i=0; i < tempNumPoints; i++)
			this->addVertex(vec3(tempPoints[i]));

		//Add all faces
		for(i=0; i < tempNumFaces; i++)
		{
			if(m_faceMode == TRIANGLES)
			{
				if(tempFaceSizes[i] == 3)
					this->addTriangle(tempFaces[i][0], tempFaces[i][1], tempFaces[i][2]);
				else if(tempFaceSizes[i] == 4)
				{
					this->addTriangle(tempFaces[i][0], tempFaces[i][1], tempFaces[i][2]);
					this->addTriangle(tempFaces[i][2], tempFaces[i][3], tempFaces[i][0]);
				}
			}
			else if(m_faceMode == QUADS)
			{
				if(tempFaceSizes[i] == 4)
					this->addQuad(tempFaces[i][0], tempFaces[i][1], tempFaces[i][2], tempFaces[i][3]);
				else if(tempFaceSizes[i] == 3)
				{
					this->addQuad(tempFaces[i][0], tempFaces[i][1], tempFaces[i][2], tempFaces[i][2]);
				}
			}
		}
	}

	// Now that we're done, we have to make sure we
	// free our dynamic memory.
	for (i = 0; i < tempNumPoints; i++) {
		delete []tempPoints[i];
	}
	SAFE_DELETE_ARRAY(tempPoints);
	

	for (i = 0; i < tempNumFaces; i++) {
		delete tempFaces[i];
	}
	SAFE_DELETE_ARRAY(tempFaces);
	SAFE_DELETE_ARRAY(tempFaceSizes);

	// Clean up our ifstream. The MFC framework will
	// take care of the CArchive.
	ifs.close();

	if(!bres) 
		MarkError();

	return bres;

	/*
	FILE *fileStream;
	char token[128];
	int fileStatus,
	vertexCount,
	cellCount,
		dummyInt[3],
		i,
		j;
	float dummyFloat[3];

	if ((fileStream = fopen(fileName.c_str(), "r")) == NULL)
	{
		MarkError();

		return false;
	}

	fileStatus = fscanf_s(fileStream, "%s", token);

	if ((fileStatus == EOF) || (fileStatus == 0))
	{
		MarkError();

		return false;
	}

	if (strcmp(token, "OFF") != 0)
	{
		MarkError();

		return false;
	}

	fileStatus = fscanf_s(fileStream, "%i", &vertexCount);

	if ((fileStatus == EOF) || (fileStatus == 0))
	{
		MarkError();

		return false;
	}

	fileStatus = fscanf_s(fileStream, "%i", &cellCount);

	if ((fileStatus == EOF) || (fileStatus == 0))
	{
		MarkError();

		return false;
	}

	fileStatus = fscanf_s(fileStream, "%i", dummyInt);

	if ((fileStatus == EOF) || (fileStatus == 0))
	{
		MarkError();

		return false;
	}

	Clear();

	for (i=0; i!=vertexCount; i++)
	{
		for (j=0; j!=3; j++)
		{
			fileStatus = fscanf_s(fileStream, "%f", dummyFloat + j);

			if ((fileStatus == EOF) || (fileStatus == 0))
			{
				MarkError();

				return false;
			}
		}

		AddVertex(dummyFloat, dummyFloat + 3);
	}

	for (i=0; i!=cellCount; i++)
	{
		fileStatus = fscanf_s(fileStream, "%i", dummyInt);

		if ((fileStatus == EOF) || (fileStatus == 0))
		{
			MarkError();

			return false;
		}

		if (dummyInt[0] != 3)
		{
			MarkError();

			return false;
		}

		for (j=0; j!=3; j++)
		{
			fileStatus = fscanf_s(fileStream, "%i", dummyInt + j);

			if ((fileStatus == EOF) || (fileStatus == 0))
			{
				MarkError();

				return false;
			}
		}

		AddTriangle(dummyInt[0], dummyInt[1], dummyInt[2]);
	}

	fclose(fileStream);

	return true;
	*/
}

/**
*/
bool CMeshVV::openStevenOFF(const DAnsiStr& fileName)
{
	FILE *fileStream;
	int fileStatus,
		vertexCount,
		cellCount,
		i,
		j;
	unsigned int dummyInt[4];		
	float dummyFloat[4];

	errno_t res = fopen_s(&fileStream, fileName.c_str(), "r");
	if (res != 0)
	{
		MarkError();

		return false;
	}

	fileStatus = fscanf_s(fileStream, "%i", &vertexCount);

	if ((fileStatus == EOF) || (fileStatus == 0))
	{
		MarkError();

		return false;
	}

	fileStatus = fscanf_s(fileStream, "%i", &cellCount);

	if ((fileStatus == EOF) || (fileStatus == 0))
	{
		MarkError();

		return false;
	}

	removeAll();

	for (i=0; i<vertexCount; ++i)
	{
		for (j=0; j<4; ++j)
		{
			fileStatus = fscanf_s(fileStream, "%f", dummyFloat + j);

			if ((fileStatus == EOF) || (fileStatus == 0))
			{
				MarkError();

				return false;
			}
		}

		addVertex(dummyFloat, dummyFloat + 3);

		addColor(vec4f(*(dummyFloat + 3), *(dummyFloat + 3), *(dummyFloat + 3), *(dummyFloat + 3)));
	}

	for (i=0; i<cellCount; ++i)
	{
		for (j=0; j<3; ++j)
		{
			fileStatus = fscanf_s(fileStream, "%i", dummyInt + j);

			if ((fileStatus == EOF) || (fileStatus == 0))
			{
				MarkError();

				return false;
			}
		}

		addFaceArray(dummyInt, dummyInt + 4);
	}

	fclose(fileStream);

	return true;
}

/**
*/
bool CMeshVV::openCrystalStructureDislocation(const DAnsiStr& fileName)
{
	FILE *fileStream;
	char token[128];
	int fileStatus,
		lineSizeCount,
		dummyInt,
		j;
	float dummyFloat[3],
		texCoord[3],
		vertexCoord[3];

	errno_t res = fopen_s(&fileStream, fileName.c_str(), "r");
	if (res != 0)
	{
		MarkError();

		return false;
	}

	// Ignores the header of the file.
	do 
	{
		fileStatus = fscanf_s(fileStream, "%s", token);

		if ((fileStatus == EOF) || (fileStatus == 0))
		{
			MarkError();

			return false;
		}

		// Searches for the end of the line.
		if (token[0] == '#')
		{
			lineSizeCount = 0;

			while (fgetc(fileStream) != '\n')
				if (++lineSizeCount > 2048)
				{
					MarkError();

					return false;
				}
		}
		else
			break;
	} while ((fileStatus != EOF) && (fileStatus != 0));

	dummyFloat[0] = float(atof(token));

	initMesh(POINTS, 3, 3, 1, 3);

	while ((fileStatus != EOF) && (fileStatus != 0))
	{
		// number (readed from file in the previous iteration), type and mass.
		for (j=1; j!=3; ++j)
		{
			fileStatus = fscanf_s(fileStream, "%f", texCoord + j);

			if ((fileStatus == EOF) || (fileStatus == 0))
			{
				MarkError();

				return false;
			}
		}

		// x, y and z.
		for (j=0; j!=3; ++j)
		{
			fileStatus = fscanf_s(fileStream, "%f", vertexCoord + j);

			if ((fileStatus == EOF) || (fileStatus == 0))
			{
				MarkError();

				return false;
			}
		}

		// vx, vy and vz.
		for (j=0; j!=3; ++j)
		{
			fileStatus = fscanf_s(fileStream, "%f", dummyFloat + j);

			if ((fileStatus == EOF) || (fileStatus == 0))
			{
				MarkError();

				return false;
			}
		}

		// Crist.
		fileStatus = fscanf_s(fileStream, "%i", &dummyInt);

		if ((fileStatus == EOF) || (fileStatus == 0))
		{
			MarkError();

			return false;
		}

		// Discards red points (points along planes).
		if (dummyInt != 1)
		{
			addLabel(dummyInt);

			addTexCoord(0, vec3f(texCoord));

			addVertex(vertexCoord, vertexCoord + 3);

			// Color computed from crist.
			addColor(vec3f(float(dummyInt&1), float((dummyInt&2)>>1), float((dummyInt&4)>>2)));

			addFace(getFaceArraySize());
		}

		fileStatus = fscanf_s(fileStream, "%f", dummyFloat);
	}

	fclose(fileStream);

	return true;
}

bool CMeshVV::fitTo(vec3f minCorner, vec3f maxCorner)
{
	return fitTo(minCorner.x, minCorner.y, minCorner.z, maxCorner.x, maxCorner.y, maxCorner.z);
}

bool CMeshVV::fitTo(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax)
{
	int vertexCount, i;
	const float *vertexCoord;
	float scale, vertexCoordFitted[3];

	if (!this->isValid())
	{
		MarkError();

		return false;
	}

	if (this->getVertexSize() != 3)
	{
		MarkError();

		return false;
	}

	vec3 lo, hi;
	if (!this->getExtremes(lo, hi))
	{
		MarkError();

		return false;
	}

	scale = (xMax - xMin)/(hi.x - lo.x);
	scale = min(scale, (yMax - yMin)/(hi.y - lo.y));
	scale = min(scale, (zMax - zMin)/(hi.z - lo.z));

	vertexCount = this->countVertices();

	for (i=0; i!=vertexCount; i++)
	{
		vertexCoord = this->getVertex(i);

		vertexCoordFitted[0] = scale*(vertexCoord[0] - lo.x) + xMin;
		vertexCoordFitted[1] = scale*(vertexCoord[1] - lo.y) + yMin;
		vertexCoordFitted[2] = scale*(vertexCoord[2] - lo.z) + zMin;

		this->setVertex(i, vertexCoordFitted[0], vertexCoordFitted[1], vertexCoordFitted[2]);
	}

	return true;
}

/**
*/
bool CMeshVV::generateNormals(void)
{
	int vertexCount,
		triangleCount,
		i,
		j,
		k;
	const unsigned int *triangle;
	const float *firstTriangleVertexCoord,
		*secondTriangleVertexCoord,
		*thirdTriangleVertexCoord,
		*vertexNormal;

	float triangleNormal[3],
		temporaryVertexNormal[3],
		temporaryVertexNormalMagnitude;

	if (this->getMode() != CMeshVV::TRIANGLES)
	{
		MarkError();

		return false;
	}

	vertexCount = this->countVertices();

	this->m_lstNormals.assign(3*vertexCount, 0.0f);

	triangleCount = this->countFaces();

	triangle = this->getElementArray();

	for (k=0 ; k!=triangleCount; k++)
	{
		if (!triangle)
		{
			MarkError();

			return false;
		}

		firstTriangleVertexCoord = this->getVertex(triangle[0]);

		if (!firstTriangleVertexCoord)
		{
			MarkError();

			return false;
		}

		secondTriangleVertexCoord = this->getVertex(triangle[2]);

		if (!secondTriangleVertexCoord)
		{
			MarkError();

			return false;
		}

		thirdTriangleVertexCoord = this->getVertex(triangle[1]);

		if (!thirdTriangleVertexCoord)
		{
			MarkError();

			return false;
		}

		triangleNormal[0] = (secondTriangleVertexCoord[1] - firstTriangleVertexCoord[1])*(thirdTriangleVertexCoord[2] - firstTriangleVertexCoord[2]) - (secondTriangleVertexCoord[2] - firstTriangleVertexCoord[2])*(thirdTriangleVertexCoord[1] - firstTriangleVertexCoord[1]);
		triangleNormal[1] = (secondTriangleVertexCoord[2] - firstTriangleVertexCoord[2])*(thirdTriangleVertexCoord[0] - firstTriangleVertexCoord[0]) - (secondTriangleVertexCoord[0] - firstTriangleVertexCoord[0])*(thirdTriangleVertexCoord[2] - firstTriangleVertexCoord[2]);
		triangleNormal[2] = (secondTriangleVertexCoord[0] - firstTriangleVertexCoord[0])*(thirdTriangleVertexCoord[1] - firstTriangleVertexCoord[1]) - (secondTriangleVertexCoord[1] - firstTriangleVertexCoord[1])*(thirdTriangleVertexCoord[0] - firstTriangleVertexCoord[0]);

		vec3 n(triangleNormal);
		n.normalize();		
		n.get(triangleNormal);

		for (j=0; j!=3; j++)
		{
			vertexNormal = this->getNormal(triangle[j]);

			if (!vertexNormal)
			{
				MarkError();

				return false;
			}

			for (i=0; i!=3; i++)
				temporaryVertexNormal[i] = vertexNormal[i] + triangleNormal[i];

			this->setNormal(triangle[j], temporaryVertexNormal[0], temporaryVertexNormal[1], temporaryVertexNormal[2]);
		}

		triangle += 3;
	}

	for (j=0 ; j!=vertexCount; j++)
	{
		vertexNormal = this->getNormal(j);

		if (!vertexNormal)
		{
			MarkError();

			return false;
		}

		temporaryVertexNormalMagnitude = 0.0f;

		for (i=0; i!=3; i++)
			temporaryVertexNormalMagnitude += vertexNormal[i]*vertexNormal[i];

		temporaryVertexNormalMagnitude = sqrtf(temporaryVertexNormalMagnitude);

		if (temporaryVertexNormalMagnitude > 1.0e-3)
			for (i=0; i!=3; i++)
				temporaryVertexNormal[i] = vertexNormal[i]/temporaryVertexNormalMagnitude;

		this->setNormal(j, temporaryVertexNormal[0], temporaryVertexNormal[1], temporaryVertexNormal[2]);
	}

	return true;
}

/**
*/
bool CMeshVV::smoothNormals(void)
{
	int vertexCount,
		triangleCount,
		j,
		k;
	const unsigned int *triangle;
	const float *firstTriangleNormalCoord,
		*secondTriangleNormalCoord,
		*thirdTriangleNormalCoord;
	float normalAverage[3],
		*vertexNormal;
	vector<float> normalArray;
	vec3 temp;

	if (this->getMode() != CMeshVV::TRIANGLES)
	{
		MarkError();

		return false;
	}

	vertexCount = this->countVertices();

	normalArray.assign(3*vertexCount, 0.0f);

	triangleCount = this->countFaces();

	triangle = this->getElementArray();

	for (k=0 ; k!=triangleCount; k++)
	{
		if (!triangle)
		{
			MarkError();

			return false;
		}

		firstTriangleNormalCoord = this->getNormal(triangle[0]);

		if (!firstTriangleNormalCoord)
		{
			MarkError();

			return false;
		}

		secondTriangleNormalCoord = this->getNormal(triangle[2]);

		if (!secondTriangleNormalCoord)
		{
			MarkError();

			return false;
		}

		thirdTriangleNormalCoord = this->getNormal(triangle[1]);

		if (!thirdTriangleNormalCoord)
		{
			MarkError();

			return false;
		}

		for (j=0; j<3; ++j)
			normalAverage[j] = 0.5f*(secondTriangleNormalCoord[j] + thirdTriangleNormalCoord[j]);

		temp.set(normalAverage);
		temp.normalize();
		temp.get(normalAverage);		

		for (j=0; j<3; ++j)
			normalArray[3*triangle[0] + j] += normalAverage[j];

		for (j=0; j<3; ++j)
			normalAverage[j] = 0.5f*(firstTriangleNormalCoord[j] + thirdTriangleNormalCoord[j]);

		temp.set(normalAverage);
		temp.normalize();
		temp.get(normalAverage);


		for (j=0; j<3; ++j)
			normalArray[3*triangle[1] + j] += normalAverage[j];

		for (j=0; j<3; ++j)
			normalAverage[j] = 0.5f*(firstTriangleNormalCoord[j] + secondTriangleNormalCoord[j]);

		temp.set(normalAverage);
		temp.normalize();
		temp.get(normalAverage);

		for (j=0; j<3; ++j)
			normalArray[3*triangle[2] + j] += normalAverage[j];

		triangle += 3;
	}

	vertexNormal = &normalArray[0];

	for (j=0 ; j!=vertexCount; j++)
	{
		if (!vertexNormal)
		{
			MarkError();

			return false;
		}

		temp.set(vertexNormal);
		temp.normalize();
		temp.get(vertexNormal);	

		this->setNormal(j, vertexNormal[0], vertexNormal[1], vertexNormal[2]);

		vertexNormal += 3;
	};

	return true;
}

/*
void CVVMesh::SetMaterial(CMaterial& mtrl)
{

	float color[4];
	mtrl.getColorComponent(ctAmbient, color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, color);

	mtrl.getColorComponent(ctDiffused, color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, color);

	mtrl.getColorComponent(ctSpecular, color);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &mtrl.shininess);

}
*/

bool CMeshVV::getExtremes(vec3 &lo, vec3 &hi)
{
	int vertexCount = this->countVertices();

	float xMin = getMaxLimit<float>();
	float xMax = -getMaxLimit<float>();
	float yMin = getMaxLimit<float>();
	float yMax = -getMaxLimit<float>();
	float zMin = getMaxLimit<float>();
	float zMax = -getMaxLimit<float>();

	const float *vertex = this->getVertexArray();

	for (int i=0; i!=vertexCount; i++)
	{
		if (xMin > *vertex)
			xMin = *vertex;
		if (xMax < *vertex)
			xMax = *vertex;

		vertex++;

		if (yMin > *vertex)
			yMin = *vertex;
		if (yMax < *vertex)
			yMax = *vertex;

		vertex++;

		if (zMin > *vertex)
			zMin = *vertex;
		if (zMax < *vertex)
			zMax = *vertex;

		vertex++;
	}

	lo = vec3(xMin, yMin, zMin);
	hi = vec3(xMax, yMax, zMax);

	return true;

}

bool CMeshVV::appendFrom(const CMeshVV& rhs)
{
	if(m_faceMode != rhs.m_faceMode)
		return false;
	if(m_szUnitVertex != rhs.m_szUnitVertex)
		return false;
	if(m_szUnitColor != rhs.m_szUnitColor)
		return false;

	size_t offsetFace = countVertices();
	
	if (!rhs.m_lstLabels.empty())
		m_lstLabels.appendFrom(rhs.m_lstLabels);

	if (!rhs.m_lstColors.empty())
		m_lstColors.appendFrom(rhs.m_lstColors);

	if (!rhs.m_lstNormals.empty())
		m_lstNormals.appendFrom(rhs.m_lstNormals);
	
	if (!rhs.m_lstVertices.empty())
		m_lstVertices.appendFrom(rhs.m_lstVertices);

	if (!rhs.m_lstFaces.empty())
	{
		size_t idxContinue = m_lstFaces.size();
		m_lstFaces.grow(rhs.m_lstFaces.size());
		
		for(size_t i = 0; i<rhs.m_lstFaces.size(); i++)
		{
			m_lstFaces[idxContinue + i] = rhs.m_lstFaces[i] + offsetFace;
		}		
	}

	//Copy textures
	//m_szTexChannels  = rhs.m_szTexChannels;	
	for(int iChannel=0; iChannel < m_szTexChannels; iChannel++)
	{
		m_szUnitTexCoord[iChannel] = rhs.m_szUnitTexCoord[iChannel];
		if (!rhs.m_lstTexChannels[iChannel].empty())
			m_lstTexChannels[iChannel].appendFrom(rhs.m_lstTexChannels[iChannel]);
	}

	if (!isValid())
	{
		MarkError();

		return false;
	}

	return true;
}


bool CMeshVV::copyFrom( const CMeshVV& rhs )
{
	this->removeAll();

	m_faceMode = rhs.m_faceMode;

	if (!rhs.m_lstLabels.empty())
		m_lstLabels.copyFrom(rhs.m_lstLabels);		

	m_szUnitColor = rhs.m_szUnitColor;
	if (!rhs.m_lstColors.empty())
		m_lstColors.copyFrom(rhs.m_lstColors);
		
	if (!rhs.m_lstNormals.empty())
		m_lstNormals.copyFrom(rhs.m_lstNormals);

	m_szUnitVertex = rhs.m_szUnitVertex;	
	if (!rhs.m_lstVertices.empty())
		m_lstVertices.appendFrom(rhs.m_lstVertices);

	if (!rhs.m_lstFaces.empty())
		m_lstFaces.copyFrom(rhs.m_lstFaces);

	//Copy textures
	m_szTexChannels  = rhs.m_szTexChannels;	
	for(int iChannel=0; iChannel < m_szTexChannels; iChannel++)
	{
		m_szUnitTexCoord[iChannel] = rhs.m_szUnitTexCoord[iChannel];
		if (!rhs.m_lstTexChannels[iChannel].empty())
			m_lstTexChannels[iChannel].copyFrom(rhs.m_lstTexChannels[iChannel]);
	}

	if (!isValid())
	{
		MarkError();

		return false;
	}

	return true;
}

void CMeshVV::setOglMaterial(const CMaterial& mtrl)
{
	vec4f component;
	component = mtrl.getAmbient();	
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, component.ptr());

	component = mtrl.getDiffused();	
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, component.ptr());

	component = mtrl.getSpecular();
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, component.ptr());

	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &mtrl.shininess);
}


void CMeshVV::drawDirect() const
{	
	int ctFaceVertices = getUnitFaceSize();
	size_t ctFaceArrayElements = m_lstFaces.size();
	size_t idVertex, idTexture = 0;
	bool bHasNormals = (m_lstNormals.size() > 0);
	bool bHasVertices = (m_lstVertices.size() > 0);
	bool bHasMtrl = ((m_szTexChannels >= 4)&&
					 (m_lstTexChannels[0].size() > 0)&&
					 (m_lstTexChannels[1].size() > 0)&&
					 (m_lstTexChannels[2].size() > 0)&&
					 (m_lstTexChannels[3].size() > 0));
	CMaterial mtrl;

	glBegin(m_faceMode);
		for(size_t iface=0; iface < ctFaceArrayElements; iface ++)
		{
			idVertex = (m_lstFaces[iface] * m_szUnitVertex);
			
			if(bHasMtrl)
			{
				idTexture = m_lstFaces[iface]*4;
				
				//mtrl.ambient = vec4f()
				//setOglMaterial(CMaterial::mtrlCopper());				
				glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &m_lstTexChannels[0][idTexture]);

				glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &m_lstTexChannels[1][idTexture]);
			
				glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &m_lstTexChannels[2][idTexture]);

				glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &m_lstTexChannels[3][idTexture]);
				
			}
					

			if(bHasNormals)
			{
				vec3 n = vec3(&m_lstNormals[0] + idVertex);
				glNormal3fv(n.ptr());
			}

			if(bHasVertices)
			{
				vec3 v = vec3(&m_lstVertices[0] + idVertex);
				glVertex3fv(v.ptr());
			}			
		}
	glEnd();

}

void CMeshVV::drawBuffered() const
{
	bool bHasFaces = (m_lstFaces.size() > 0);
	bool bHasColor = (m_lstColors.size() > 0);
	bool bHasNormals = (m_lstNormals.size() > 0);
	bool bHasVertices = (m_lstVertices.size() > 0);

	if(bHasColor)
	{
		glColorPointer(m_szUnitColor, GL_FLOAT, 0, &m_lstColors[0]);
		glEnableClientState(GL_COLOR_ARRAY);
	}

	if(bHasNormals)
	{
		glNormalPointer(GL_FLOAT, 0, &m_lstNormals[0]);
		glEnableClientState(GL_NORMAL_ARRAY);
	}

	if(bHasVertices)
	{
		glVertexPointer(m_szUnitVertex, GL_FLOAT, 0, &m_lstVertices[0]);
		glEnableClientState(GL_VERTEX_ARRAY);
	}

	if(bHasFaces)
	{		
		glDrawElements(m_faceMode, (GLsizei)m_lstFaces.size(), GL_UNSIGNED_INT, &m_lstFaces[0]);
	}

	//Cleanup
	if (bHasColor)
		glDisableClientState(GL_COLOR_ARRAY);

	if (bHasNormals)
		glDisableClientState(GL_NORMAL_ARRAY);

	if (bHasVertices)
		glDisableClientState(GL_VERTEX_ARRAY);
}

//Draw Normals
void CMeshVV::drawNormals(int len) const
{	
	size_t ctNormals = countNormals();
	size_t ctVertices = countVertices();
	if(ctNormals != ctVertices) return;

	glColor3f(0.0f, 0.0f, 1.0f);

	vec3f start, end;	
	float flen = static_cast<float>(len) / 100.0f;

	glBegin(GL_LINES);	
	for(size_t i=0; i < ctNormals; i++)
	{			
		start = getVertex3(i);		
		glVertex3fv(start.ptr());

		end = start + flen * getNormal3(i);
		glVertex3fv(end.ptr());
	}
	glEnd();
}


void CMeshVV::drawUncolored() const
{
	size_t ctNormals  = countNormals();
	size_t ctVertices = countVertices();
	size_t ctFaces    = countFaces();
	size_t ctFaceSides = getUnitFaceSize();
	if(ctNormals != ctVertices) return;

	vec3f v, n;
	
	glBegin(m_faceMode);	
	for(size_t idxFace=0; idxFace < ctFaces; idxFace++)
	{			
		const size_t* arrPrimitive = getFace(idxFace);
		for(size_t i=0; i < ctFaceSides; i++)
		{			
			v = getVertex3(arrPrimitive[i]);
			n = getNormal3(arrPrimitive[i]);
		}
	}
	
	glEnd();

}

const float * CMeshVV::getColor( int n ) const
{
	n *= m_szUnitColor;

	if (n < (int)m_lstColors.size())
		return (&m_lstColors[0] + n);

	MarkError();

	return 0;

}

bool CMeshVV::saveOBJ( const DAnsiStr& name ) const
{
	// find the file   
	std::ofstream ofs( name.c_str() );
	WCHAR line[256] = {0};    

	// Parse the .obj file. Both triangle faces and quad faces are supported.
	// Only v and f tags are processed, other tags like vn, vt etc are ignored.	
	vec3 pos;
	vec3 normal;
	vec2 texCoord;
	
	
	DAnsiStr strOut = printToAStr("#Exported from Parsip\n");
	ofs << strOut;
	strOut = printToAStr("o cube1\n");
	ofs << strOut;

	strOut = printToAStr("#%d vertices, %d faces\n", this->countVertices(), this->countFaces());
	ofs << strOut;

	vec3f v;
	size_t ctVertices = countVertices();
	for(size_t i=0; i<ctVertices; i++)
	{
		v = getVertex3(i);
		strOut = printToAStr("v %f %f %f\n",  v.x, v.y, v.z);
		ofs << strOut;
	}

	vec3f n;
	size_t ctNormals = countNormals();
	for(size_t i=0; i<ctNormals; i++)
	{
		n = getNormal3(i);
		strOut = printToAStr("vn %f %f %f\n",  n.x, n.y, n.z);
		ofs << strOut;
	}


	strOut = printToAStr("g cube1_default\n");
	ofs << strOut;

	strOut = printToAStr("usemtl default\n");
	ofs << strOut;
		
	int f[4];
	size_t ctWritten = 0;
	if(m_faceMode == TRIANGLES)
	{				
		for(size_t i=0; i<m_lstFaces.size(); i+=3)
		{
			f[0] = m_lstFaces[i + 0] + 1;
			f[1] = m_lstFaces[i + 1] + 1;
			f[2] = m_lstFaces[i + 2] + 1;
			strOut = printToAStr("f %d//%d %d//%d %d//%d\n",  f[0], f[0], f[1], f[1], f[2], f[2]);
			ofs << strOut;
			ctWritten++;
		}
	}
	else if(m_faceMode == QUADS)
	{
		for(size_t i=0; i<m_lstFaces.size(); i+=4)
		{
			f[0] = m_lstFaces[i + 0] + 1;
			f[1] = m_lstFaces[i + 1] + 1;
			f[2] = m_lstFaces[i + 2] + 1;
			f[3] = m_lstFaces[i + 3] + 1;
			strOut = printToAStr("f %d//%d %d//%d %d//%d %d//%d\n",  f[0], f[0], f[1], f[1], f[2], f[2], f[3], f[3]);
			ofs << strOut;
		}
	}

	return (countFaces() > 0);
}


}