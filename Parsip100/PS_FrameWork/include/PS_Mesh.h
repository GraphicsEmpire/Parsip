/*
 * PS_Mesh.h
 *
 *  Created on: Jul 4, 2011
 *      Author: pourya
 */

#ifndef PS_MESH_H_
#define PS_MESH_H_

#include <vector>
#include "_dataTypes.h"
#include "PS_Vector.h"
#include "PS_String.h"
#include "PS_Octree.h"
#include "PS_ResourceManager.h"

using namespace std;
using namespace PS;
using namespace PS::MATH;
using namespace PS::CONTAINERS;



namespace PS{
namespace MESH{


/*!
 * structure holding a mesh material object
 */
class CMeshMaterial
{
public:
	CMeshMaterial() {}
	~CMeshMaterial() {}
public:
	DAnsiStr strMTLName;
	DAnsiStr strTexName;
	vec3f ambient;
	vec3f diffused;
	vec3f specular;
	float reflect;
	float refract;
	float trans;
	float shininess;
	float glossy;
	float refractIndex;

	int illuminationModel;
};

/*!
 * CMeshNode represents a single node in our mesh file.
 * It is complete mesh by itself.
 *
 */


class CMeshNode{
public:
	CMeshNode();
	~CMeshNode();

	enum ArrayType {atVertex, atNormal, atTexture, atFaceIndex, atNone};
	enum FaceType {ftTriangle = 3, ftQuad = 4};

	/*!
	 * Adds a vec3 v to the destination buffer dest.
	 * @return designated index of added vertex. This index is the actual vertex number
	 * not the position in the float array. The position in the float array can be computed as
	 * index x szUnitVertex
	 */
	int add(const vec3f& v, ArrayType dest = atVertex, int count = 3);
	int addVertex(const vec3f& v);
	int addNormal(const vec3f& n);
	int addTexCoord2(const vec2f& t);
	int addTexCoord3(const vec3f& t);
	void addFaceIndex(U32 index);


	/*!
	 * Checks all face indices for invalid values
	 * @return number of indices found out of range
	 */
	int countFaceIndexErrors() const;


	//Statistics
	size_t countVertices() const {return arrVertices.size() / (size_t)szUnitVertex;}
	size_t countNormals() const {return arrNormals.size() / 3;}
	size_t countFaces() const {return arrIndices.size() / (size_t)szUnitFace;}
	size_t countTexCoords() const {return arrTexCoords.size() / (size_t)szUnitTexCoord;}

	/*!
	 * returns unit vertex memory stride in bytes
	 */
	size_t getVertexStride() const {return szUnitVertex * sizeof(float);}
	size_t getNormalStride() const {return 3 * sizeof(float);}
	size_t getFaceStride() const {return szUnitFace * sizeof(U32);}


	//Access Vertex, Normal
	vec3f getAsVec3(int index, ArrayType dest = atVertex) const;
	vec3f getVertex(int idxVertex) const;
	vec3f getNormal(int idxVertex) const;
	vec2f getTexCoord2(int idxVertex) const;
	vec3f getTexCoord3(int idxVertex) const;

	//Access Material
	CMeshMaterial* getMaterial() const {return lpMaterial;}
	void setMaterial(CMeshMaterial* aMaterial) {lpMaterial = aMaterial;}

	COctree computeBoundingBox() const;
public:
	DAnsiStr 	strNodeName;

	std::vector<float> arrVertices;
	std::vector<float> arrNormals;
	std::vector<float> arrTexCoords;
	std::vector<U32>	arrIndices;


	U8			szUnitVertex;
	U8   		szUnitTexCoord;
	U8			szUnitFace;
	CMeshMaterial* lpMaterial;
};


/*
 * structure holding a complete mesh
 */
class CMesh{
public:
	//typedef CMeshNode<FaceIndexType> MESHNODE;

	CMesh() {;}
	CMesh(const char* chrFileName);
	~CMesh();

	bool load(const char* chrFileName);

	//Mesh Nodes
	void addNode(CMeshNode* lpMeshNode);
	CMeshNode*	getNode(int idx) const;
	int getNodeCount() const;

	//Mesh Materials
	void addMeshMaterial(CMeshMaterial* lpMaterial);
	CMeshMaterial* getMaterial(int idx) const;
	CMeshMaterial* getMaterial(const DAnsiStr& strName) const;

	COctree computeBoundingBox() const;
private:
	bool loadObj(const char* chrFileName);
	bool loadObjGlobalVertexNormal(const char* chrFileName);
	bool loadMTL(const char* chrFileName);

	bool readFileContent(const char* chrFileName, std::vector<DAnsiStr>& content);


private:
	std::vector<CMeshNode*> m_nodes;
	std::vector<CMeshMaterial*> m_materials;
	DAnsiStr m_strFilePath;
};


class CMeshManager : public CResourceManager<CMesh>
{
public:
	CMeshManager();
	~CMeshManager();

	void cleanup();

	CMesh* loadResource(const DAnsiStr& inStrFilePath, DAnsiStr& inoutStrName);
};

}
}
#endif /* PS_MESH_H_ */
