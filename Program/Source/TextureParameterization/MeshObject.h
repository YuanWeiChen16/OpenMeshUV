#pragma once

#include <string>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <Common.h>

typedef OpenMesh::TriMesh_ArrayKernelT<>  TriMesh;

class MyMesh : public TriMesh
{
public:
	MyMesh();
	~MyMesh();

	int FindVertex(MyMesh::Point pointToFind);
	void ClearMesh();
};

class GLMesh
{
public:
	GLMesh();
	~GLMesh();

	bool Init(std::string fileName);
	void Render();

	MyMesh mesh;
	GLuint vao;
	GLuint ebo;
	GLuint vboVertices, vboNormal;

	bool LoadModel(std::string fileName);
	void LoadToShader();
};

class MeshObject
{
public:
	MeshObject();
	~MeshObject();

	bool Init(std::string fileName);
	void Render();
	void RenderSelectedFace();
	bool AddSelectedFace(unsigned int faceID);
	void DeleteSelectedFace(unsigned int faceID);

	bool AddSelectedPoint(unsigned int PointID);
	void DeleteSelectedPoint(unsigned int PointID);

	bool FindClosestPoint(unsigned int faceID, glm::vec3 worldPos, glm::vec3& closestPos);

	bool FaceToPoint();


	GLMesh model;
	std::vector<unsigned int> selectedFace;

	std::vector<unsigned int> selectedPoint;

	std::vector<int> selectPoint_Seq;
	void MY_LoadToShader();

	std::vector<unsigned int*> fvIDsPtr;
	std::vector<int> elemCount;

};

