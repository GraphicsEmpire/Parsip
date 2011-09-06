#pragma once
#ifndef PS_MESHVV_H
#define PS_MESHVV_H

#include <iostream>
#include <fstream>

#include "PS_Vector.h"
#include "PS_Material.h"
#include "PS_String.h"
#include "DSystem/include/DContainers.h"

#define MAX_TEXTURE_CHANNELS 4


using namespace std;

namespace PS{

using namespace MATH;
//Mesh with Vertex, Index, Element Buffer for fast OpenGL rendering
class CMeshVV 
{
public:
	typedef DVec<unsigned int> element_array_type;
	typedef enum STREAM_FORMAT {
		OFF,
		STEVEN_OFF,
		OBJ,
		PLY,
		ASCII_STL,
		SURFEL,
		CRYSTAL_STRUCTURE_DISLOCATIONS,
		PS_BINARY
	};
	typedef enum POLYGON_MODE {
		POINTS = 0x0000,
		LINES = 0x0001,
		LINE_LOOP = 0x0002,
		LINE_STRIP = 0x0003,
		TRIANGLES = 0x0004,
		TRIANGLE_STRIP = 0x0005,
		TRIANGLE_FAN = 0x0006,
		QUADS = 0x0007,
		QUAD_STRIP = 0x0008,
		POLYGON = 0x0009
	};
	
	CMeshVV(POLYGON_MODE mode = TRIANGLES, 
				int vertexSize = 3,
				int colorSize = 3, 
				int texChannels = 1,
				int texCoordSize = 2);	

	CMeshVV(const CMeshVV& other);

	virtual ~CMeshVV(void);

	void initMesh(POLYGON_MODE mode, int szUnitVertex, int szUnitColor, int texChannels, int szUnitTexCoord);

	//Perform test on mesh
	bool performCompleteTest();
	void setModeByFaceSides(int sides);
	void setMode(POLYGON_MODE mode);
	POLYGON_MODE getMode(void) const;
	//===================================================
	void addLabel(int label);
	void setLabel(int n, int label);
	int getLabel(int n) const;
	const int *getLabelArray(void) const;
	int countLabels(void) const;
	void clearLabels(void);
	//===================================================
	void addColor(vec3f color);
	void addColor(vec4f color);	
	bool setColor(int n, vec3f color);
	bool setColor(int n, vec4f color);
	
	void setUnitColorSize(int size);
	const float *getColor(int n) const;
	const float *getColorArray(void) const;	
	int countColors(void) const;
	void clearColors(void);
	//===================================================
	//Add Texture Coordinates
	bool isValidTexChannel(int idxChannel) const;
	bool isValidTexChannelAndCoord(int idxChannel, unsigned int idxCoord) const;
	void addTexCoord(int idxChannel, float tex);
	void addTexCoord(int idxChannel, vec2f tex);
	void addTexCoord(int idxChannel, vec3f tex);
	void addTexCoord(int idxChannel, vec4f tex);
/*
	void setTexCoord(int idxChannel, UINT idxTexCoord, float tex);
	void setTexCoord(int idxChannel, UINT idxTexCoord, vec2f tex);
	void setTexCoord(int idxChannel, UINT idxTexCoord, vec3f tex);
	void setTexCoord(int idxChannel, UINT idxTexCoord, vec4f tex);
	*/

	vec4f getTexCoord(int idxChannel, UINT idxTexCoord);

	void setUnitTexCoordSize(int size);
	void setUnitTexCoordSize(int idxChannel, int size);
	int	 getUnitTexCoordSize(int idxChannel) const;	
	void setTexChannelSize(int size);
	int getTexChannelSize() const;
	unsigned int countTexCoords(int idxChannel) const;
	void clearTextures();
	//===================================================
	void addNormal(vec3 n);
	void addNormal(float x, float y, float z);
	void addNormal(const float *begin, const float *end);
	void setNormal(int n, float x, float y, float z);
	void setNormal(int n, const float *begin, const float *end);
	const float *getNormal(int n) const;
	vec3 getNormal3(int n) const;
	const float *getNormalArray(void) const;
	unsigned int countNormals(void) const;
	void clearNormals(void);
	//===================================================
	void addVertex(vec2 v2);
	void addVertex(vec3 v3);	
	void addVertex(float x);
	void addVertex(float x, float y);
	void addVertex(float x, float y, float z);
	void addVertex(float x, float y, float z, float w);
	void addVertex(const float *begin, const float *end);

	void setVertex(int n, vec2 v2);
	void setVertex(int n, vec3 v3);
	void setVertex(int n, float x);
	void setVertex(int n, float x, float y);
	void setVertex(int n, float x, float y, float z);
	void setVertex(int n, float x, float y, float z, float w);
	void setVertex(int n, const float *begin, const float *end);
	void setUnitVertexSize(int size);

	const float *getVertex(int n) const;
	vec3   getVertex3(int n) const;

	const float *getVertexArray(void) const;

	int getVertexSize(void) const;
	unsigned int countVertices(void) const;
	void clearVertices(void);
	//===================================================
	void addFace(unsigned int vertexId);
	void addFaceArray(DVec<unsigned int>& input);
	void addFaceArray(const unsigned int *begin, const unsigned int *end);
	void addLine(unsigned int firstId, unsigned int secondId);
	void addTriangle(unsigned int firstId, unsigned int secondId, unsigned int thirdId);
	void addQuad(unsigned int* ids);
	void addQuad(unsigned int firstId, unsigned int secondId, unsigned int thirdId, unsigned int fourthId);
	
	const unsigned *getElementArray(void) const;
	int getFaceArraySize(void) const;
	void clearFaces(void);

	//Get Primitives	
	unsigned int getFaceMode(void) const;	
	int getFace(size_t idxFace, vec3* arrVertices, size_t szVerticesBuffer);
	const unsigned *getFace(unsigned int primitiveId) const;
	int getUnitFaceSize(void) const;
	unsigned int countFaces(void) const;
	//===================================================
	virtual void removeAll(void);	
	//Open and Save File Formats
	bool open(const DAnsiStr& strFileName);
	bool open(const DAnsiStr& strFileName, STREAM_FORMAT type);
	bool save(const DAnsiStr& strFileName, STREAM_FORMAT type) const;
	bool saveOBJ(const DAnsiStr& name) const;
	bool isValid(void) const;

	//MeshUtils
	bool fitTo(vec3f minCorner, vec3f maxCorner);
	bool fitTo(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax);
	bool generateNormals(void);
	bool smoothNormals(void);
	bool getExtremes(vec3 &lo, vec3 &hi);

	//Append from other mesh
	bool appendFrom(const CMeshVV& rhs);
	bool copyFrom(const CMeshVV& rhs);

	//Draw functions
	void drawBuffered() const;
	void drawNormals(int len) const;
	void drawUncolored() const;

	void drawDirect() const;

	//Draw the mesh
	static void setOglMaterial(const CMaterial& mtrl);
protected:
	//Serialization
	bool saveToASCIISTLFile(const DAnsiStr& name) const;
	bool saveToSURFELFile(const DAnsiStr& name) const;
	bool savePSBinary(const DAnsiStr& strFileName) const;
	bool savePSBinary(ofstream& fpWrite) const;
	
	bool openOBJ(const DAnsiStr& name);
	bool openOFF(const DAnsiStr& name);
	bool openStevenOFF(const DAnsiStr& name);
	bool openCrystalStructureDislocation(const DAnsiStr& name);
	bool openPSBinary(const DAnsiStr& strFileName);
	bool openPSBinary(ifstream& fpRead);	
public:
	//Number of texture channel that are used in this mesh from 1-MAX_TEXTURE_CHANNELS
	//Typically we use texture units to transfer materials to GLSL
	int m_szTexChannels;
	//Size of an individual texture coordinate from 1-4
	int m_szUnitTexCoord[MAX_TEXTURE_CHANNELS];
	DVec<float> m_lstTexChannels[MAX_TEXTURE_CHANNELS];
	
	DVec<int> m_lstLabels;

	int m_szUnitColor;
	DVec<float> m_lstColors;

	int m_szUnitVertex;
	DVec<float> m_lstVertices;
	DVec<float> m_lstNormals;

	unsigned int m_faceMode;
	DVec<unsigned int> m_lstFaces;	
};

}

#endif 
