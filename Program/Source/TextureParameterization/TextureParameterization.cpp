#include <Common.h>
#include <ViewManager.h>
#include <AntTweakBar/AntTweakBar.h>
#include <ResourcePath.h>
#include <fstream>
#include<map>

#include <Eigen/Sparse>

#include "OpenMesh.h"
#include "MeshObject.h"
#include "DrawModelShader.h"
#include "PickingShader.h"
#include "PickingTexture.h"
#include "DrawPickingFaceShader.h"
#include "DrawTextureShader.h"
#include "DrawPointShader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../Include/STB/stb_image.h"


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../Include/STB/stb_image_write.h"





//ifdebug===============================================
//#define DEBUG 

using namespace glm;
using namespace std;
using namespace Eigen;

glm::vec3 worldPos;
bool updateFlag = false;
bool isRightButtonPress = false;
GLuint currentFaceID = 0;
int currentMouseX = 0;
int currentMouseY = 0;
int windowWidth = 600;
int windowHeight = 600;
const std::string ProjectName = "TextureParameterization";

GLuint			program;			// shader program
mat4			proj_matrix;		// projection matrix
float			aspect;
ViewManager		meshWindowCam;

MeshObject model;
MeshObject BeSelectModel;

std::vector<MeshObject> ALLModel;
std::vector<MeshObject> ClusterModel;

vector<unsigned int> OuterPoint;
vector<unsigned int> InnerPoint;

float OuterLengh = 0;


float TextureX = 0;
float TextureY = 0;
float TexRotate = 0;



// use by Roof thing============================================
vector<vector<int>> adjmatrix;
map<int, int > ClusterList;
int NowShow = 0;
int RealShow = 0;
vector <vector<int>> ffconnect;
#define COLORMAP_SIZE  60000
double** colormap = new double* [COLORMAP_SIZE];
int Showwwwwwwwww = 0;

int FriendCount = 0;
std::vector<std::vector<int>> FriendZone;
std::map<int, int> Idx;


std::vector<std::vector<MyMesh::Point>> BoundingPoint;
std::vector<std::vector<double>> BoundingPointheight;

double DeepY = 1000000000000000;
double HeightY = -10000000000000000;
bool ShowOModel = true;
// use by Roof thing============================================


unsigned int textureID;
std::vector<unsigned int> TextureIDs;
std::vector<unsigned int> NoramlIDs;

unsigned int NormalID;

// shaders
DrawModelShader drawModelShader;
DrawPickingFaceShader drawPickingFaceShader;
PickingShader pickingShader;
PickingTexture pickingTexture;
DrawPointShader drawPointShader;

// vbo for drawing point
GLuint vboPoint;

int mainWindow;
TwBar* bar;

enum SelectionMode
{
	ADD_FACE,
	DEL_FACE,
	SELECT_POINT,
	ADD_POINT,
	DEL_POINT,
	ONE_RING
};
SelectionMode selectionMode = ADD_FACE;
TwEnumVal SelectionModeEV[] = { {ADD_FACE, "Add face"}, {DEL_FACE, "Delete face"}, {SELECT_POINT, "Point"},{ADD_POINT,"Add point"},{DEL_POINT,"Delete point"},{ONE_RING,"One Ring"} };
TwType SelectionModeType;

enum choseModelID
{
	armadillo,
	bear,
	dancer,
	dancing_children,
	feline,
	fertilty,
	gargoyle,
	neptune,
	octopus,
	screwdriver,
	xyzrgb_dragon_100k
};
choseModelID chosemodel = bear;
choseModelID choseNOWmodel = bear;
TwEnumVal chosemodelEV[] = { {armadillo, "armadillo"}, {bear, "bear"}, {dancer, "dancer"},{dancing_children,"dancing_children"},{feline,"feline"},{fertilty,"fertilty"},{gargoyle,"gargoyle"},{neptune,"neptune"},{octopus,"octopus"},{screwdriver,"screwdriver"},{xyzrgb_dragon_100k,"xyzrgb_dragon_100k"} };
TwType chosemodelType;
std::vector<std::string> modelNames;

enum choseTexture
{
	checkboard,
	WoodWall,
	Brickwall,
	stonewall,
	gears,
	happyface,
	NTUST_Logo
};
choseTexture chosetexture = checkboard;
TwEnumVal chosetextureEV[] = { {checkboard, "checkboard"}, {WoodWall, "WoodWall"},{Brickwall,"Brickwall" },{stonewall,"stonewall" },{gears,"gears"},{happyface,"happyface"},{NTUST_Logo,"NTUST_Logo"} };
TwType chosetextureType;
std::vector<std::string> TextureNames;

enum choseNormalName
{
	No_normal,
	WoodWall_normal,
	Brickwall_normal,
	stonewall_normal,
	water_normal,
	some_normal
};
choseNormalName choseNormal = No_normal;
TwEnumVal chosenormalEV[] = { {No_normal, "No_normal"}, {WoodWall_normal, "WoodWall_normal"},{Brickwall_normal,"Brickwall_normal" },{stonewall_normal,"stonewall_normal" },{water_normal,"water_normal"},{some_normal,"some_normal"} };
TwType chosenormalType;
std::vector<std::string> NoramlNames;

//calu Angle 
double PointAngle(MyMesh::Point P1, MyMesh::Point P2, MyMesh::Point VPoint)
{
	double V1x = (P1[0] - VPoint[0]);
	double V1y = (P1[1] - VPoint[1]);
	double V1z = (P1[2] - VPoint[2]);

	double V2x = (P2[0] - VPoint[0]);
	double V2y = (P2[1] - VPoint[1]);
	double V2z = (P2[2] - VPoint[2]);

	double A_B = V1x * V2x + V1y * V2y + V1z * V2z;
	double lAl = sqrt(V1x * V1x + V1y * V1y + V1z * V1z);
	double lBl = sqrt(V2x * V2x + V2y * V2y + V2z * V2z);

	double angle = acos(A_B / (lAl * lBl));

	angle = (angle * 180.0) / 3.1415926;
	return angle;
}
void CoutMat4(glm::mat4 M)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			cout << M[i][j] << " ";
		}
		cout << endl;
	}
}
unsigned int loadTexture(std::string path, int imageType);
void magic();
void magic_delete();

void DFS(std::vector<std::vector<int>>& M, std::vector<bool>& visited, int i);
int FindFriend(std::vector<std::vector<int>>& M);
void ChangeCameraLook(glm::vec3 Face_Diraction, std::vector<glm::vec3> Face_Size);
void MergeBoundary();
void detectRoof();
void caluBoundary();
void NewDetectRoof();

void SetupGUI()
{
#ifdef _MSC_VER
	TwInit(TW_OPENGL, NULL);
#else
	TwInit(TW_OPENGL_CORE, NULL);
#endif
	TwGLUTModifiersFunc(glutGetModifiers);
	bar = TwNewBar("Texture Parameter Setting");
	TwDefine(" 'Texture Parameter Setting' size='220 90' ");
	TwDefine(" 'Texture Parameter Setting' fontsize='3' color='96 216 224'");

	// Defining season enum type
	SelectionModeType = TwDefineEnum("SelectionModeType", SelectionModeEV, 6);
	// Adding season to bar
	TwAddVarRW(bar, "SelectionMode", SelectionModeType, &selectionMode, NULL);



	modelNames.push_back("gta07_dt1_02_w01_high_0.obj");
	modelNames.push_back("Stairs.obj");
	modelNames.push_back("Ahole.obj");
	modelNames.push_back("gta02_dt1_03_build2_high.obj");
	modelNames.push_back("gta03_dt1_11_dt1_tower_high.obj");
	modelNames.push_back("gta01_gta_townobj_fillhole.obj");
	modelNames.push_back("Imposter01_Res_Building_4x8_012_003_root.obj");
	modelNames.push_back("Imposter01_Res_Building_4x8_012_003_root.obj");
	modelNames.push_back("WorldBuilding02_french_Arc_de_Triomphe.obj");



	modelNames.push_back("WorldBuilding01_EmpireState_lp.obj");
	modelNames.push_back("Res_Building_8x8_038_001_root.obj");


	modelNames.push_back("xyzrgb_dragon_100k.obj");

	TextureNames.push_back("checkerboard4.jpg");
	TextureNames.push_back("176.jpg");
	TextureNames.push_back("bricks2.jpg");
	TextureNames.push_back("TexturesCom_albedo.jpg");
	TextureNames.push_back("gears.jpg");
	TextureNames.push_back("happyface.jpg");
	TextureNames.push_back("NTUST.jpg");

	NoramlNames.push_back("none.jpg");
	NoramlNames.push_back("176_norm.jpg");
	NoramlNames.push_back("bricks2_normal.jpg");
	NoramlNames.push_back("TexturesCom_normal.jpg");
	NoramlNames.push_back("normal123.jpg");
	NoramlNames.push_back("normalmap.jpg");


	chosemodelType = TwDefineEnum("Model", chosemodelEV, 11);
	// Adding season to bar
	TwAddVarRW(bar, "Model", chosemodelType, &chosemodel, NULL);

	chosetextureType = TwDefineEnum("Texture", chosetextureEV, 7);
	// Adding season to bar
	TwAddVarRW(bar, "Texture", chosetextureType, &chosetexture, NULL);

	chosenormalType = TwDefineEnum("Normal Texture", chosenormalEV, 6);
	// Adding season to bar
	TwAddVarRW(bar, "Normal Texture", chosenormalType, &choseNormal, NULL);

}

void My_LoadModel()
{
	if (model.Init(ResourcePath::modelPath))
	{
		/*int id = 0;
		while (model.AddSelectedFace(id))
		{
			++id;
		}
		model.Parameterization();
		drawTexture = true;*/
		model.model.mesh.request_face_normals();
		model.model.mesh.update_normals();
		puts("Load Model");
	}
	else
	{
		puts("Load Model Failed");
	}
}

void InitOpenGL()
{
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_POINT_SMOOTH);
}

void InitData()
{

	for (int i = 0; i < COLORMAP_SIZE; i++)
	{
		colormap[i] = new double[3];
		for (int j = 0; j < 3; j++)
		{
			colormap[i][j] = (double)rand() / (RAND_MAX + 1.0);
		}
	}


	ResourcePath::shaderPath = "./Shader/" + ProjectName + "/";
	//ResourcePath::imagePath = "./Imgs/" + ProjectName + "/";
	ResourcePath::imagePath = "./Imgs/" + ProjectName + "/";

	ResourcePath::modelPath = "./Model/" + modelNames[chosemodel];

	//Initialize shaders
	///////////////////////////	
	drawModelShader.Init();
	pickingShader.Init();
	pickingTexture.Init(windowWidth, windowHeight);
	drawPickingFaceShader.Init();
	drawPointShader.Init();

	glGenBuffers(1, &vboPoint);

	//Load model to shader program
	My_LoadModel();
	textureID = loadTexture("Imgs\\TextureParameterization\\checkerboard4.jpg", GL_RGB);
	NormalID = loadTexture("Imgs\\TextureParameterization\\normalmap.jpg", GL_RGB);
	for (int i = 0; i < 7; i++)
	{
		TextureIDs.push_back(loadTexture("Imgs\\TextureParameterization\\" + TextureNames[i], GL_RGB));
	}
	for (int i = 0; i < 6; i++)
	{
		NoramlIDs.push_back(loadTexture("Imgs\\TextureParameterization\\" + NoramlNames[i], GL_RGB));
	}
}

void Reshape(int width, int height)
{
	windowWidth = width;
	windowHeight = height;

	TwWindowSize(width, height);
	glutReshapeWindow(windowWidth, windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);

	aspect = windowWidth * 1.0f / windowHeight;
	meshWindowCam.SetWindowSize(windowWidth, windowHeight);
	pickingTexture.Init(windowWidth, windowHeight);
}

// GLUT callback. Called to draw the scene.
void RenderMeshWindow()
{
	//Update shaders' input variable
	///////////////////////////	

	glm::mat4 mvMat = meshWindowCam.GetViewMatrix() * meshWindowCam.GetModelMatrix();
	glm::mat4 pMat = meshWindowCam.GetProjectionMatrix(aspect);

	// write faceID+1 to framebuffer
	pickingTexture.Enable();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pickingShader.Enable();
	pickingShader.SetMVMat(value_ptr(mvMat));
	pickingShader.SetPMat(value_ptr(pMat));
	model.Render();
	pickingShader.Disable();
	pickingTexture.Disable();


	// draw model
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawModelShader.Enable();
	//glm::mat3 normalMat; // glm::transpose(glm::inverse(glm::mat3(mvMat)));
	glm::mat3 normalMat = glm::mat3(mvMat);//glm::transpose(glm::inverse(glm::mat3(mvMat)));

	drawModelShader.SetWireColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	drawModelShader.SetFaceColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	drawModelShader.UseLighting(true);
	drawModelShader.DrawWireframe(true);
	drawModelShader.SetTexcoord(0, 0, 0);
	drawModelShader.SetNormalType(false);
	drawModelShader.SetNormalMat(normalMat);
	drawModelShader.SetMVMat(mvMat);
	drawModelShader.SetPMat(pMat);
	if (ShowOModel)
	{
		model.Render();
	}
#ifdef OneRing
	//if (model.selectedPoint.size() > 0)
	//{
	//	drawModelShader.SetWireColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	//	drawModelShader.SetFaceColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

	//	//drawModelShader.DrawTexCoord(true);
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, TextureIDs[chosetexture]);

	//	glActiveTexture(GL_TEXTURE1);
	//	glBindTexture(GL_TEXTURE_2D, NoramlIDs[choseNormal]);
	//	drawModelShader.UseLighting(false);
	//	drawModelShader.DrawTexture(false);
	//	drawModelShader.DrawWireframe(false);

	//	//BeSelectModel.Render();
	//	drawModelShader.SetTexcoord(TextureX, TextureY, TexRotate);
	//	if (chosemodel == No_normal)
	//	{
	//		drawModelShader.SetNormalType(false);
	//	}
	//	else
	//	{
	//		drawModelShader.SetNormalType(true);
	//	}
	//	//drawModelShader.DrawTexCoord(true);
	//	BeSelectModel.Render();
	//	drawModelShader.DrawTexture(false);
	//	drawModelShader.DrawWireframe(true);
	//	//drawModelShader.DrawTexCoord(false);
	//	drawModelShader.SetNormalType(false);
	//}
	//else
	//{

	//}
#endif // OneRing
	if (Showwwwwwwwww != 0)
	{
		drawModelShader.SetWireColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		drawModelShader.SetFaceColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

		//drawModelShader.DrawTexCoord(true);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TextureIDs[chosetexture]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, NoramlIDs[choseNormal]);
		drawModelShader.UseLighting(false);
		drawModelShader.DrawTexture(false);
		drawModelShader.DrawWireframe(false);

		//BeSelectModel.Render();
		drawModelShader.SetTexcoord(TextureX, TextureY, TexRotate);
		if (chosemodel == No_normal)
		{
			drawModelShader.SetNormalType(false);
		}
		else
		{
			drawModelShader.SetNormalType(true);
		}

		drawModelShader.DrawWireframe(false);
		BeSelectModel.Render();
		if (ShowOModel)
		{
			drawModelShader.DrawWireframe(true);
			BeSelectModel.Render();
		}
		drawModelShader.DrawTexture(false);
		drawModelShader.DrawWireframe(true);
		//drawModelShader.DrawTexCoord(false);
		drawModelShader.SetNormalType(false);

		double bsize = 0.05;
		int LineWeith = 5;
		//for (int i = 0; i < ALLModel.size(); i++)
		//{
		//	ALLModel[i].Render();

		//	glBindBuffer(GL_ARRAY_BUFFER, vboPoint);
		//	std::vector<MyMesh::Point> vertices;
		//	vertices.reserve(ALLModel[i].model.mesh.n_vertices());
		//	for (MyMesh::VertexIter v_it = ALLModel[i].model.mesh.vertices_begin(); v_it != ALLModel[i].model.mesh.vertices_end(); ++v_it)
		//	{
		//		MyMesh::Point P = ALLModel[i].model.mesh.point(*v_it);
		//		//P[1] -= bsize;
		//		vertices.push_back(P);
		//	}

		//	glBufferData(GL_ARRAY_BUFFER, sizeof(MyMesh::Point) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
		//	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), glm::value_ptr(worldPos), GL_STATIC_DRAW);
		//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//	glEnableVertexAttribArray(0);
		//	glm::vec4 pointColor(1.0, 0.0, 0.0, 1.0);
		//	drawPointShader.Enable();
		//	drawPointShader.SetMVMat(mvMat);
		//	drawPointShader.SetPMat(pMat);
		//	drawPointShader.SetPointColor(pointColor);
		//	//drawPointShader.SetPointSize(15.0);
		//	glLineWidth(LineWeith);
		//	//glDrawElements(GL_TRIANGLES, model.mesh.n_faces() * 3, GL_UNSIGNED_INT, 0);
		//	glDrawArrays(GL_LINE_LOOP, 0, vertices.size());
		//	drawPointShader.Disable();
		//	glBindBuffer(GL_ARRAY_BUFFER, 0);




		//	//glBindBuffer(GL_ARRAY_BUFFER, vboPoint);
		//	//std::vector<MyMesh::Point> vertices2;
		//	//vertices2.reserve(ALLModel[i].model.mesh.n_vertices());
		//	//for (MyMesh::VertexIter v_it = ALLModel[i].model.mesh.vertices_begin(); v_it != ALLModel[i].model.mesh.vertices_end(); ++v_it)
		//	//{
		//	//	MyMesh::Point P = ALLModel[i].model.mesh.point(*v_it);
		//	//	//P[1] += bsize;
		//	//	vertices2.push_back(P);
		//	//}

		//	//glBufferData(GL_ARRAY_BUFFER, sizeof(MyMesh::Point) * vertices2.size(), &vertices2[0], GL_STATIC_DRAW);
		//	////glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), glm::value_ptr(worldPos), GL_STATIC_DRAW);
		//	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//	//glEnableVertexAttribArray(0);
		//	//pointColor = vec4(1.0, 1.0, 0.0, 1.0);
		//	//drawPointShader.Enable();
		//	//drawPointShader.SetMVMat(mvMat);
		//	//drawPointShader.SetPMat(pMat);
		//	//drawPointShader.SetPointColor(pointColor);
		//	////drawPointShader.SetPointSize(15.0);
		//	//glLineWidth(LineWeith);
		//	////glDrawElements(GL_TRIANGLES, model.mesh.n_faces() * 3, GL_UNSIGNED_INT, 0);
		//	//glDrawArrays(GL_LINE_LOOP, 0, vertices2.size());
		//	//drawPointShader.Disable();
		//	//glBindBuffer(GL_ARRAY_BUFFER, 0);

		//	//Down Ray
		//	glBindBuffer(GL_ARRAY_BUFFER, vboPoint);
		//	std::vector<MyMesh::Point> verticesTD;
		//	verticesTD.reserve(ALLModel[i].model.mesh.n_vertices() * 2);
		//	int PCount = 0;
		//	for (MyMesh::VertexIter v_it = ALLModel[i].model.mesh.vertices_begin(); v_it != ALLModel[i].model.mesh.vertices_end(); ++v_it)
		//	{

		//		MyMesh::Point p = ALLModel[i].model.mesh.point(*v_it);

		//		//p[1] += bsize;

		//		verticesTD.push_back(p);


		//		//p[1] -= (bsize*2);


		//		if (BoundingPointheight[i][PCount] < 199)
		//		{
		//			p[1] = BoundingPointheight[i][PCount];
		//		}

		//		verticesTD.push_back(p);
		//		PCount++;
		//	}

		//	glBufferData(GL_ARRAY_BUFFER, sizeof(MyMesh::Point) * verticesTD.size(), &verticesTD[0], GL_STATIC_DRAW);
		//	//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), glm::value_ptr(worldPos), GL_STATIC_DRAW);
		//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//	glEnableVertexAttribArray(0);
		//	glm::vec4 lineColor(1.0, 1.0, 0.0, 1.0);
		//	drawPointShader.Enable();
		//	drawPointShader.SetMVMat(mvMat);
		//	drawPointShader.SetPMat(pMat);
		//	drawPointShader.SetPointColor(lineColor);
		//	//drawPointShader.SetPointSize(15.0);
		//	glLineWidth(LineWeith);
		//	//glDrawElements(GL_TRIANGLES, model.mesh.n_faces() * 3, GL_UNSIGNED_INT, 0);
		//	glDrawArrays(GL_LINES, 0, verticesTD.size());
		//	drawPointShader.Disable();
		//	glBindBuffer(GL_ARRAY_BUFFER, 0);


		//}



	}
	else
	{
		//model.Render();
	}


	drawModelShader.Disable();



	// render selected face
	if (selectionMode == SelectionMode::ADD_FACE || selectionMode == SelectionMode::DEL_FACE)
	{
		drawPickingFaceShader.Enable();
		drawPickingFaceShader.SetMVMat(value_ptr(mvMat));
		drawPickingFaceShader.SetPMat(value_ptr(pMat));
		if (model.selectedPoint.size() <= 0)
		{
			//model.RenderSelectedFace();
		}
		drawPickingFaceShader.Disable();
	}

	glUseProgram(0);

	// render closest point
	if ((selectionMode == SelectionMode::SELECT_POINT) || (selectionMode == SelectionMode::ADD_POINT) || (selectionMode == SelectionMode::DEL_POINT))
	{
		unsigned int Pointidx;
		MyMesh::VertexHandle VH;
		if (updateFlag)
		{
			float depthValue = 0;
			int windowX = currentMouseX;
			int windowY = windowHeight - currentMouseY;
			glReadPixels(windowX, windowY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthValue);

			GLint _viewport[4];
			glGetIntegerv(GL_VIEWPORT, _viewport);

			glm::vec4 viewport(_viewport[0], _viewport[1], _viewport[2], _viewport[3]);
			glm::vec3 windowPos(windowX, windowY, depthValue);
			glm::vec3 wp = glm::unProject(windowPos, mvMat, pMat, viewport);

			model.FindClosestPoint(currentFaceID - 1, wp, worldPos, VH);

			if (selectionMode == SelectionMode::SELECT_POINT)
			{
				magic_delete();
				model.selectedFace.clear();
			}
			//MyMesh::Vertex Vthis = model.model.mesh.vertex(VH);


			//找出每個屋頂的邊界並向下延伸
			// 
			//找面normal向上的面
			for (MyMesh::FIter F = model.model.mesh.faces_begin(); F != model.model.mesh.faces_end(); ++F)
			{
				MyMesh::FaceHandle Fh = model.model.mesh.face_handle(F->idx());
				MyMesh::Normal FN = model.model.mesh.normal(Fh);
				//避免normal誤差
				if (FN[1] > 0.5)
				{
					cout << "Find face" << F->idx() << endl;
					model.AddSelectedFace(F->idx());
				}
			}

			//#ifdef OneRing


						//One Ring分析 分析有幾個屋頂與
						//屋頂拆出來重建
						//依照面順序產生點的list
			model.FaceToPoint();
			//重新排序point index
			for (int i = 0; i < model.selectPoint_Seq.size(); i++)
			{
				for (int j = 0; j < model.selectedPoint.size(); j++)
				{
					if (model.selectedPoint[j] == model.selectPoint_Seq[i])
					{
						model.selectPoint_Seq[i] = j;
					}
				}
			}
			std::vector < MyMesh::VertexHandle> vhandle;
			//Add point to beselectModel===================================================================================
			for (int i = 0; i < model.selectedPoint.size(); i++)
			{
				MyMesh::VertexHandle TempPoint = model.model.mesh.vertex_handle(model.selectedPoint[i]);
				MyMesh::Point closestPoint = model.model.mesh.point(TempPoint);
				vhandle.push_back(BeSelectModel.model.mesh.add_vertex(closestPoint));
				//#ifdef DEBUG
								//cout << "X: " << closestPoint[0] << "Y: " << closestPoint[1] << "Z: " << closestPoint[2] << endl;
				//endif // DEBUG
			}
			//connect point to face to beselectModel===================================================================================
			std::vector<MyMesh::VertexHandle>  face_vhandles;
			for (int i = 0; i < model.selectPoint_Seq.size() / 3; i++)
			{
				face_vhandles.clear();
				face_vhandles.push_back(vhandle[model.selectPoint_Seq[i * 3]]);
				face_vhandles.push_back(vhandle[model.selectPoint_Seq[(i * 3) + 1]]);
				face_vhandles.push_back(vhandle[model.selectPoint_Seq[(i * 3 + 2)]]);
				BeSelectModel.model.mesh.add_face(face_vhandles);
				//#ifdef DEBUG
				cout << "FACE " << i << " Is be Rebuild" << endl;
				//#endif // DEBUG
			}


			//one ring 找屋頂
			bool Find_ALL_DONE = false;
			vector<bool> IsFound;
			vector<int>partition;
			//init 
			ClusterList.clear();

			//
			int faceSize = model.selectPoint_Seq.size() / 3;
			for (int i = 0; i < model.selectPoint_Seq.size() / 3; i++)
			{
				ClusterList[i] = i;
			}
			//共用對應matrix
			adjmatrix.clear();
			adjmatrix.resize(faceSize);


			//建立共用對應matrix (adjmatrix)
			for (MyMesh::FIter F1 = BeSelectModel.model.mesh.faces_begin(); F1 != BeSelectModel.model.mesh.faces_end(); ++F1)
			{
				for (MyMesh::FIter F2 = BeSelectModel.model.mesh.faces_begin(); F2 != BeSelectModel.model.mesh.faces_end(); ++F2)
				{
					if (F1->idx() != F2->idx()) //檢查是否有共用邊(用face 的vertx id 直接對照)
					{
						int count = 0;
						MyMesh::FHandle FH1 = BeSelectModel.model.mesh.face_handle(F1->idx());
						MyMesh::FHandle FH2 = BeSelectModel.model.mesh.face_handle(F2->idx());

						for (MyMesh::FVIter FV1 = BeSelectModel.model.mesh.fv_begin(FH1); FV1 != BeSelectModel.model.mesh.fv_end(FH1); ++FV1)
						{
							for (MyMesh::FVIter FV2 = BeSelectModel.model.mesh.fv_begin(FH2); FV2 != BeSelectModel.model.mesh.fv_end(FH2); ++FV2)
							{
								if (FV1->idx() == FV2->idx())
								{
									count++;
								}
							}
						}
						if (count == 2)
						{
							adjmatrix[F1->idx()].push_back(F2->idx());//如果是有共用邊
						}
					}
				}
				std::cout << "check materix" << F1->idx() << endl;
			}
			std::cout << "check materix" << endl;
			//建立(ClusterList)
			for (int i = 0; i < adjmatrix.size(); i++)
			{
				for (int j = 0; j < adjmatrix[i].size(); j++)
				{
					//Union(i, adjmatrix[i][j]);//根據adj圖合併
					int x = i;
					int y = adjmatrix[i][j];

					int op1 = ClusterList[x];
					int op2 = ClusterList[y];

					if (ClusterList[x] != ClusterList[y])
					{
						for (int i = 0; i < adjmatrix.size(); i++)
						{
							if (x > y && ClusterList[i] == op1)
							{
								ClusterList[i] = op2;
							}
							else if (x < y && ClusterList[i] == op2)
							{
								ClusterList[i] = op1;
							}
						}
					}
				}
				cout << "adjmatrix" << i << endl;
			}
			int count = 0;
			//建立與計算 獨立面list (partition)
			for (int i = 0; i < ClusterList.size(); i++)
			{
				std::vector<int>::iterator it = find(partition.begin(), partition.end(), ClusterList[i]);//找出 不連接的部分有多少個
				if (it == partition.end())
				{
					partition.push_back(ClusterList[i]);
					count++;
				}
			}

			//面與面連接關系 分類
			ffconnect.resize(count);
			for (int i = 0; i < ClusterList.size(); i++)
			{
				for (int j = 0; j < partition.size(); j++)
				{
					if (ClusterList[i] == partition[j])
					{
						ffconnect[j].push_back(i);
					}
				}
			}
			cout << "class" << ffconnect.size() << endl;
			cout << "Mywork Done" << endl;
			BeSelectModel.model.mesh.request_vertex_texcoords2D();

#ifdef DEBUG
			cout << "InnerUV " << UV[0] << " " << UV[1] << endl;
#endif // DEBUG

			int mycount = 0;
			//上顏色
			for (int i = 0; i < ffconnect.size(); i++)
			{
				for (int j = 0; j < ffconnect[i].size(); j++)
				{
					MyMesh::FHandle FH = BeSelectModel.model.mesh.face_handle(ffconnect[i][j]);
					for (MyMesh::FVIter FVI = BeSelectModel.model.mesh.fv_begin(FH); FVI != BeSelectModel.model.mesh.fv_end(FH); ++FVI)
					{
						MyMesh::VHandle VH = BeSelectModel.model.mesh.vertex_handle(FVI->idx());
						MyMesh::TexCoord2D TC;

						//float R = (float)((int)(i * 13 + 10) % 7) / 7.0;
						//float G = (float)((int)((i + 12)) * 19 % 5) / 5.0;
						//float B = (float)((int)((i + 5)) * 3 % 7) / 7.0;

						TC[0] = colormap[i][0];
						TC[1] = colormap[i][1];

						BeSelectModel.model.mesh.set_texcoord2D(VH, TC);

					}
					cout << "R G B " << mycount << " COLOR " << colormap[i][0] << "   " << colormap[i][1] << endl;
					mycount++;
				}
			}
			//find height roof  create bounding mesh




			//BeSelectModel.model.mesh.request_face_normals();
			//BeSelectModel.model.mesh.update_normals();
			//BeSelectModel.model.mesh.release_face_normals();

			BeSelectModel.MY_LoadToShader();

			//#endif // OneRing
						////Find First Boundary Point===================================================================================================================
						//MyMesh::HalfedgeIter HFI = BeSelectModel.model.mesh.halfedges_begin();
						//MyMesh::HalfedgeHandle HFh;
						//for (; HFI != BeSelectModel.model.mesh.halfedges_end(); ++HFI)
						//{
						//	HFh = BeSelectModel.model.mesh.halfedge_handle(HFI->idx());

						//	bool ISBOUND = BeSelectModel.model.mesh.is_boundary(HFh);
						//	if (ISBOUND)
						//	{
						//		MyMesh::VertexHandle outPoint = BeSelectModel.model.mesh.from_vertex_handle(HFh);
						//		OuterPoint.push_back(outPoint.idx());
						//		break;
						//	}
						//}
						////travel all bundary Point===================================================================================================================
						//HFh = BeSelectModel.model.mesh.next_halfedge_handle(HFh);
						//for (;;)
						//{
						//	MyMesh::VertexHandle outPoint = BeSelectModel.model.mesh.from_vertex_handle(HFh);
						//	if (outPoint.idx() == OuterPoint[0])
						//	{
						//		break;
						//	}
						//	OuterPoint.push_back(outPoint.idx());
						//	HFh = BeSelectModel.model.mesh.next_halfedge_handle(HFh);
						//}

#ifdef OneRingSelect


			for (MyMesh::VFIter VF = model.model.mesh.vf_begin(VH); VF != model.model.mesh.vf_end(VH); ++VF)
			{
				MyMesh::FaceHandle Fh = model.model.mesh.face_handle(VF->idx());
				if ((selectionMode == SelectionMode::ADD_POINT) || (selectionMode == SelectionMode::SELECT_POINT))
				{
					model.AddSelectedFace(Fh.idx());
				}
				else if (selectionMode == SelectionMode::DEL_POINT)
				{
					model.DeleteSelectedFace(Fh.idx());
				}
				for (MyMesh::FFIter FF1 = model.model.mesh.ff_begin(Fh); FF1 != model.model.mesh.ff_end(Fh); ++FF1)
				{
					MyMesh::FaceHandle Fh1 = model.model.mesh.face_handle(FF1->idx());
					if ((selectionMode == SelectionMode::ADD_POINT) || (selectionMode == SelectionMode::SELECT_POINT))
					{
						model.AddSelectedFace(Fh1.idx());
					}
					else if (selectionMode == SelectionMode::DEL_POINT)
					{
						model.DeleteSelectedFace(Fh1.idx());
					}
					for (MyMesh::FFIter FF2 = model.model.mesh.ff_begin(*FF1); FF2 != model.model.mesh.ff_end(*FF1); ++FF2)
					{
						MyMesh::FaceHandle Fh2 = model.model.mesh.face_handle(FF2->idx());
						if ((selectionMode == SelectionMode::ADD_POINT) || (selectionMode == SelectionMode::SELECT_POINT))
						{
							model.AddSelectedFace(Fh2.idx());
						}
						else if (selectionMode == SelectionMode::DEL_POINT)
						{
							model.DeleteSelectedFace(Fh2.idx());
						}
						for (MyMesh::FFIter FF3 = model.model.mesh.ff_begin(*FF2); FF3 != model.model.mesh.ff_end(*FF2); ++FF3)
						{
							MyMesh::FaceHandle Fh3 = model.model.mesh.face_handle(FF3->idx());
							if ((selectionMode == SelectionMode::ADD_POINT) || (selectionMode == SelectionMode::SELECT_POINT))
							{
								model.AddSelectedFace(Fh3.idx());
							}
							else if (selectionMode == SelectionMode::DEL_POINT)
							{
								model.DeleteSelectedFace(Fh3.idx());
							}
							for (MyMesh::FFIter FF4 = model.model.mesh.ff_begin(*FF3); FF4 != model.model.mesh.ff_end(*FF3); ++FF4)
							{
								MyMesh::FaceHandle Fh4 = model.model.mesh.face_handle(FF4->idx());
								if ((selectionMode == SelectionMode::ADD_POINT) || (selectionMode == SelectionMode::SELECT_POINT))
								{
									model.AddSelectedFace(Fh4.idx());
								}
								else if (selectionMode == SelectionMode::DEL_POINT)
								{
									model.DeleteSelectedFace(Fh4.idx());
								}
								/*for (MyMesh::FFIter FF5 = model.model.mesh.ff_begin(*FF4); FF3 != model.model.mesh.ff_end(*FF4); ++FF3)
								{
									MyMesh::FaceHandle Fh5 = model.model.mesh.face_handle(FF5->idx());
									model.AddSelectedFace(Fh5.idx());
									cout << "HA" << kkk << endl;;
									kkk ++ ;
								}*/
							}
						}
					}
				}
			}
			///this who magic work
			if (selectionMode == SelectionMode::SELECT_POINT)
			{
				magic();
			}
#endif // OneRingSelect
			updateFlag = false;
		}


		//if (RealShow != NowShow)
		//{
		//	if (NowShow >= ffconnect.size())
		//	{
		//		NowShow = 0;
		//	}
		//	RealShow = NowShow;
		//	cout << "RealShow" << RealShow << endl;
		//	for (int i = 0; i < ffconnect.size(); i++)
		//	{

		//		for (int j = 0; j < ffconnect[i].size(); j++)
		//		{
		//			MyMesh::FHandle FH = BeSelectModel.model.mesh.face_handle(j);
		//			for (MyMesh::FVIter FVI = BeSelectModel.model.mesh.fv_begin(FH); FVI != BeSelectModel.model.mesh.fv_end(FH); ++FVI)
		//			{
		//				MyMesh::VHandle VH = BeSelectModel.model.mesh.vertex_handle(FVI->idx());
		//				MyMesh::TexCoord2D TC;

		//				//float R = (float)((int)(i * 13 + 10) % 7) / 7.0;
		//				//float G = (float)((int)((i + 12)) * 19 % 5) / 5.0;
		//				//float B = (float)((int)((i + 5)) * 3 % 7) / 7.0;
		//				if (RealShow == i)
		//				{
		//					TC[0] = 1;
		//					TC[1] = 0;
		//				}
		//				else
		//				{
		//					TC[0] = 0.5;
		//					TC[1] = 0.5;
		//				}
		//				BeSelectModel.model.mesh.set_texcoord2D(VH, TC);
		//			}
		//		}

		//	}

		//}

		drawPickingFaceShader.Enable();
		drawPickingFaceShader.SetMVMat(value_ptr(mvMat));
		drawPickingFaceShader.SetPMat(value_ptr(pMat));
		if (model.selectedPoint.size() <= 0)
		{
			//model.RenderSelectedFace();
		}

		drawPickingFaceShader.Disable();

		//glBindBuffer(GL_ARRAY_BUFFER, vboPoint);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), glm::value_ptr(worldPos), GL_STATIC_DRAW);
		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//glEnableVertexAttribArray(0);
		//glm::vec4 pointColor(1.0, 0.0, 1.0, 1.0);
		//drawPointShader.Enable();
		//drawPointShader.SetMVMat(mvMat);
		//drawPointShader.SetPMat(pMat);
		//drawPointShader.SetPointColor(pointColor);
		//drawPointShader.SetPointSize(15.0);
		//glDrawArrays(GL_POINTS, 0, 1);
		//drawPointShader.Disable();
		//glBindBuffer(GL_ARRAY_BUFFER, 0);

	}


#ifdef givecolor

	if (ffconnect.size() > 0)
	{
		glColor3f(1, 1, 1);
		glBegin(GL_TRIANGLES);
		for (MyMesh::FIter FI = model.model.mesh.faces_begin(); FI != model.model.mesh.faces_end(); ++FI)
		{
			MyMesh::FHandle FH = model.model.mesh.face_handle(FI->idx());
			for (MyMesh::FVIter FVI = model.model.mesh.fv_begin(FH); FVI != model.model.mesh.fv_end(FH); ++FVI)
			{
				MyMesh::VHandle VH = model.model.mesh.vertex_handle(FVI->idx());
				MyMesh::Point P = model.model.mesh.point(VH);
				glm::vec4 point = vec4(P[0] * 0.1, P[1] * 0.1, P[2] * 0.1, 1.0);
				point = pMat * mvMat * point;
				glVertex2d(point[0], point[1]);
			}
		}
		glEnd();

		glColor3f(1, 1, 1);
		glBegin(GL_TRIANGLES);

		glVertex2d(0, -1);
		glVertex2d(1, -1);
		glVertex2d(0, 0);
		for (int i = 0; i < ffconnect.size(); i++)
		{
			glColor3f(colormap[i][0], colormap[i][1], colormap[i][2]);
			for (int j = 0; j < ffconnect[i].size(); j++)
			{
				MyMesh::FHandle FH = BeSelectModel.model.mesh.face_handle(j);
				for (MyMesh::FVIter FVI = BeSelectModel.model.mesh.fv_begin(FH); FVI != BeSelectModel.model.mesh.fv_end(FH); ++FVI)
				{
					MyMesh::VHandle VH = BeSelectModel.model.mesh.vertex_handle(FVI->idx());
					MyMesh::Point P = BeSelectModel.model.mesh.point(VH);
					glm::vec4 point = vec4(P[0] * 0.1, P[1] * 0.1, P[2] * 0.1, 1.0);

					point = pMat * mvMat * point;

					glVertex2d(point[0], point[1]);
				}
			}
		}
		glEnd();
	}

#endif // givecolor

	//貼圖參數化區域
#ifdef Parameter
//右邊貼圖
#pragma region RightSiedTexture
	glBindTexture(GL_TEXTURE_2D, TextureIDs[chosetexture]);
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	float tempx = 0 - 0.5;
	float tempy = 1 - 0.5;
	float Nx = cos(-TexRotate) * tempx - sin(-TexRotate) * tempy + 0.5 - TextureX;
	float Ny = sin(-TexRotate) * tempx + cos(-TexRotate) * tempy + 0.5 + TextureY;
	glTexCoord2d(Nx, Ny); glVertex2d(0, -1);
	tempx = 1 - 0.5;
	tempy = 1 - 0.5;
	Nx = cos(-TexRotate) * tempx - sin(-TexRotate) * tempy + 0.5 - TextureX;
	Ny = sin(-TexRotate) * tempx + cos(-TexRotate) * tempy + 0.5 + TextureY;
	glTexCoord2d(Nx, Ny); glVertex2d(1, -1);
	tempx = 1 - 0.5;
	tempy = 0 - 0.5;
	Nx = cos(-TexRotate) * tempx - sin(-TexRotate) * tempy + 0.5 - TextureX;
	Ny = sin(-TexRotate) * tempx + cos(-TexRotate) * tempy + 0.5 + TextureY;
	glTexCoord2d(Nx, Ny); glVertex2d(1, 1);
	tempx = 0 - 0.5;
	tempy = 0 - 0.5;
	Nx = cos(-TexRotate) * tempx - sin(-TexRotate) * tempy + 0.5 - TextureX;
	Ny = sin(-TexRotate) * tempx + cos(-TexRotate) * tempy + 0.5 + TextureY;
	glTexCoord2d(Nx, Ny); glVertex2d(0, 1);
	glEnd();


#pragma endregion
	//畫線部分
#pragma region Reight Side MeshLine
	if (model.selectedPoint.size() > 0)
	{

		/*glColor3f(0, 1, 1);
		glBegin(GL_TRIANGLES);
		for (MyMesh::FaceIter F = BeSelectModel.model.mesh.faces_begin(); F != BeSelectModel.model.mesh.faces_end(); ++F)
		{
			for (MyMesh::FVIter FV = BeSelectModel.model.mesh.fv_begin(*F); FV != BeSelectModel.model.mesh.fv_end(*F); ++FV)
			{
				MyMesh::VHandle DrawPointVH = BeSelectModel.model.mesh.vertex_handle(FV->idx());
				MyMesh::Point DrawPoint = BeSelectModel.model.mesh.point(DrawPointVH);
				MyMesh::TexCoord2D DrawPoint_TEXUV = BeSelectModel.model.mesh.texcoord2D(DrawPointVH);
				glVertex3f(DrawPoint[0], DrawPoint[1], DrawPoint[2]);

				cout << "DRAW　TRI"<< DrawPoint <<endl;
				glTexCoord2f(DrawPoint_TEXUV[0], DrawPoint_TEXUV[1]);
			}
		}
		glEnd();*/



		glColor3f(0, 1, 0);
		glBegin(GL_POINTS);
		glPointSize(50);

		for (MyMesh::VertexIter V = BeSelectModel.model.mesh.vertices_begin(); V != BeSelectModel.model.mesh.vertices_end(); ++V)
		{
			MyMesh::VHandle DrawPoint1 = BeSelectModel.model.mesh.vertex_handle(V->idx());
			MyMesh::TexCoord2D outPoint1_p = BeSelectModel.model.mesh.texcoord2D(DrawPoint1);
			glVertex3f(outPoint1_p[0], outPoint1_p[1] * 2 - 1, 0);
		}
		glEnd();

		glColor3f(1, 0, 0);
		glBegin(GL_LINES);
		glLineWidth(5);
		for (MyMesh::EdgeIter E = BeSelectModel.model.mesh.edges_begin(); E != BeSelectModel.model.mesh.edges_end(); ++E)
		{
			MyMesh::EdgeHandle EH = BeSelectModel.model.mesh.edge_handle(E->idx());
			MyMesh::HalfedgeHandle HFH = BeSelectModel.model.mesh.halfedge_handle(EH, 0);
			MyMesh::VHandle DrawPoint1 = BeSelectModel.model.mesh.from_vertex_handle(HFH);
			MyMesh::VHandle DrawPoint2 = BeSelectModel.model.mesh.to_vertex_handle(HFH);
			MyMesh::TexCoord2D outPoint1_p = BeSelectModel.model.mesh.texcoord2D(DrawPoint1);
			MyMesh::TexCoord2D outPoint2_p = BeSelectModel.model.mesh.texcoord2D(DrawPoint2);
			glVertex3f(outPoint1_p[0], outPoint1_p[1] * 2 - 1, 0);
			glVertex3f(outPoint2_p[0], outPoint2_p[1] * 2 - 1, 0);
		}
		glEnd();

	}
#pragma endregion

#endif // Parameter

	TwDraw();
	glutSwapBuffers();
}

void RenderAll()
{
	RenderMeshWindow();
}


//Timer event
void My_Timer(int val)
{
	glutPostRedisplay();
	glutTimerFunc(16, My_Timer, val);
}

void SelectionHandler(unsigned int x, unsigned int y)
{
	GLuint faceID = pickingTexture.ReadTexture(x, windowHeight - y - 1);
	if (faceID != 0)
	{
		currentFaceID = faceID;
	}

	if (selectionMode == ADD_FACE)
	{
		if (faceID != 0)
		{
			model.AddSelectedFace(faceID - 1);
		}
	}
	else if (selectionMode == DEL_FACE)
	{
		if (faceID != 0)
		{
			model.DeleteSelectedFace(faceID - 1);
		}
	}
	else if ((selectionMode == SELECT_POINT) || (selectionMode == ADD_POINT) || (selectionMode == DEL_POINT))
	{
		currentMouseX = x;
		currentMouseY = y;
		updateFlag = true;
	}
}

//Mouse event
void MyMouse(int button, int state, int x, int y)
{
	if (!TwEventMouseButtonGLUT(button, state, x, y))
	{
		meshWindowCam.mouseEvents(button, state, x, y);

		if (button == GLUT_RIGHT_BUTTON)
		{
			if (state == GLUT_DOWN)
			{
				isRightButtonPress = true;
				SelectionHandler(x, y);
			}
			else if (state == GLUT_UP)
			{
				isRightButtonPress = false;
			}
		}
	}
}

//Keyboard event
void MyKeyboard(unsigned char key, int x, int y)
{
	if (!TwEventKeyboardGLUT(key, x, y))
	{
		meshWindowCam.keyEvents(key);
	}

	std::cout << "Input : " << key << endl;

	if ((key == 'f') && (model.selectedFace.size() > 0))
	{
		magic();
	}
	else if (key == 'g')
	{
		magic_delete();
	}
	else if (key == 'u')
	{
		TextureY += 0.01;
	}
	else if (key == 'j')
	{
		TextureY -= 0.01;
	}
	else if (key == 'h')
	{
		TextureX -= 0.01;
	}
	else if (key == 'k')
	{
		TextureX += 0.01;
	}
	else if (key == 'i')
	{
		TexRotate += 0.01;
	}
	else if (key == 'y')
	{
		TexRotate -= 0.01;
	}
	else if (key == 'n')
	{
		NewDetectRoof();
		caluBoundary();
		MergeBoundary();
	}
	else if (key == 'o')
	{
		meshWindowCam.ToggleOrtho();
	}
	else if (key == 'm')
	{
		ShowOModel = !ShowOModel;
	}
	else if (key == 'l')
	{
		glm::vec3 Fdir(1, 0, 0);
		glm::vec3 FMid(1, 0, 0);
		std::vector<glm::vec3> F;
		for (int i = 0; i < 4; i++)
		{
			F.push_back(FMid);
		}
		ChangeCameraLook(Fdir, F);
	}
	else
	{

	}
}

void ReLoadModel()
{
	ResourcePath::modelPath = "./Model/" + modelNames[chosemodel];

	My_LoadModel();

}


void MyMouseMoving(int x, int y) {
	if (!TwEventMouseMotionGLUT(x, y))
	{
		meshWindowCam.mouseMoveEvent(x, y);

		if (isRightButtonPress)
		{
			SelectionHandler(x, y);
		}
		if (chosemodel != choseNOWmodel)
		{
			choseNOWmodel = chosemodel;
			ReLoadModel();
		}
	}
}

int main(int argc, char* argv[])
{
#ifdef __APPLE__
	//Change working directory to source code path
	chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(windowWidth, windowHeight);
	mainWindow = glutCreateWindow(ProjectName.c_str()); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif

	glutReshapeFunc(Reshape);
	glutIdleFunc(RenderAll);
	glutSetOption(GLUT_RENDERING_CONTEXT, GLUT_USE_CURRENT_CONTEXT);
	SetupGUI();

	glutMouseFunc(MyMouse);
	glutKeyboardFunc(MyKeyboard);
	glutMotionFunc(MyMouseMoving);
	glutDisplayFunc(RenderMeshWindow);
	InitOpenGL();
	InitData();

	//Print debug information 
	Common::DumpInfo();

	// Enter main event loop.
	glutMainLoop();

	return 0;
}

unsigned int loadTexture(std::string path, int imageType)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	int width, height, nrComponents;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, imageType, width, height, 0, imageType, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	return textureID;
}

void magic()
{
	model.FaceToPoint();

	for (int i = 0; i < model.selectPoint_Seq.size(); i++)
	{
		for (int j = 0; j < model.selectedPoint.size(); j++)
		{
			if (model.selectedPoint[j] == model.selectPoint_Seq[i])
			{
				model.selectPoint_Seq[i] = j;
			}
		}
	}

	std::vector < MyMesh::VertexHandle> vhandle;
	//std::vector < MyMesh::Normal> ThisNormal;
	//vhandle = new MyMesh::VertexHandle[model.selectedPoint.size()];

	//Add point to beselectModel===================================================================================
	for (int i = 0; i < model.selectedPoint.size(); i++)
	{
		MyMesh::VertexHandle TempPoint = model.model.mesh.vertex_handle(model.selectedPoint[i]);
		MyMesh::Point closestPoint = model.model.mesh.point(TempPoint);
		//MyMesh::Normal closestNor = model.model.mesh.normal(TempPoint);
		//ThisNormal.push_back(closestNor);

		vhandle.push_back(BeSelectModel.model.mesh.add_vertex(closestPoint));
		//BeSelectModel.model.mesh.normal(vhandle[vhandle.size()-1]) = closestNor;
#ifdef DEBUG
		cout << "X: " << closestPoint[0] << "Y: " << closestPoint[1] << "Z: " << closestPoint[2] << endl;
#endif // DEBUG
	}

	//connect point to face to beselectModel===================================================================================
	std::vector<MyMesh::VertexHandle>  face_vhandles;
	for (int i = 0; i < model.selectPoint_Seq.size() / 3; i++)
	{
		face_vhandles.clear();
		face_vhandles.push_back(vhandle[model.selectPoint_Seq[i * 3]]);
		face_vhandles.push_back(vhandle[model.selectPoint_Seq[(i * 3) + 1]]);
		face_vhandles.push_back(vhandle[model.selectPoint_Seq[(i * 3 + 2)]]);
		BeSelectModel.model.mesh.add_face(face_vhandles);
#ifdef DEBUG
		cout << "FACE " << i << " Is be Rebuild" << endl;
#endif // DEBUG

		/*for (int i=0;i< vhandle.size();i++)
		{
			MyMesh::Normal temp = model.model.mesh.vertex(v_it->idx());
			temp[0] = -temp[0];
			temp[1] = -temp[1];
			temp[2] = -temp[2];
			normals.push_back(temp);
		}
*/
	}

	//Find First Boundary Point===================================================================================================================
	MyMesh::HalfedgeIter HFI = BeSelectModel.model.mesh.halfedges_begin();
	MyMesh::HalfedgeHandle HFh;
	for (; HFI != BeSelectModel.model.mesh.halfedges_end(); ++HFI)
	{
		HFh = BeSelectModel.model.mesh.halfedge_handle(HFI->idx());

		bool ISBOUND = BeSelectModel.model.mesh.is_boundary(HFh);
		if (ISBOUND)
		{
			MyMesh::VertexHandle outPoint = BeSelectModel.model.mesh.from_vertex_handle(HFh);
			OuterPoint.push_back(outPoint.idx());
			break;
		}
	}
	//travel all bundary Point===================================================================================================================
	HFh = BeSelectModel.model.mesh.next_halfedge_handle(HFh);
	for (;;)
	{
		MyMesh::VertexHandle outPoint = BeSelectModel.model.mesh.from_vertex_handle(HFh);
		if (outPoint.idx() == OuterPoint[0])
		{
			break;
		}
		OuterPoint.push_back(outPoint.idx());
		HFh = BeSelectModel.model.mesh.next_halfedge_handle(HFh);
	}



	//caclu Outer lengh===================================================================================================================
	for (int i = 0; i < OuterPoint.size(); i++)
	{
		MyMesh::VertexHandle outPoint1 = BeSelectModel.model.mesh.vertex_handle(OuterPoint[(i) % OuterPoint.size()]);
		MyMesh::VertexHandle outPoint2 = BeSelectModel.model.mesh.vertex_handle(OuterPoint[(i + 1) % OuterPoint.size()]);
		MyMesh::Point outPoint1_p = BeSelectModel.model.mesh.point(outPoint1);
		MyMesh::Point outPoint2_p = BeSelectModel.model.mesh.point(outPoint2);
		MyMesh::Point disPoint = outPoint1_p - outPoint2_p;
		float dis = disPoint[0] * disPoint[0] + disPoint[1] * disPoint[1];
		dis = sqrt(dis);
		dis = sqrt(dis * dis + disPoint[2] * disPoint[2]);
		OuterLengh += dis;
	}
#ifdef DEBUG
	cout << "Outer Lengh is " << OuterLengh << endl;
#endif // DEBUG


	//Make OuterSide UV=====================================================================================================
	BeSelectModel.model.mesh.request_vertex_texcoords2D();// _vertex_texcoords2D();
	float nowDis = 0;
	MyMesh::VertexHandle tempoutPoint1 = BeSelectModel.model.mesh.vertex_handle(OuterPoint[(0) % OuterPoint.size()]);
	MyMesh::TexCoord2D tempUV;
	tempUV[0] = 0;
	tempUV[1] = 0;
#ifdef DEBUG
	cout << "Point " << 0 << " UV " << tempUV[0] << " " << tempUV[1] << endl;
#endif // DEBUG
	BeSelectModel.model.mesh.set_texcoord2D(tempoutPoint1, tempUV);

	//set Outter UV================================================================================================================
	for (int i = 0; i < OuterPoint.size(); i++)
	{
		MyMesh::VertexHandle outPoint1 = BeSelectModel.model.mesh.vertex_handle(OuterPoint[(i) % OuterPoint.size()]);
		MyMesh::VertexHandle outPoint2 = BeSelectModel.model.mesh.vertex_handle(OuterPoint[(i + 1) % OuterPoint.size()]);
		MyMesh::Point outPoint1_p = BeSelectModel.model.mesh.point(outPoint1);
		MyMesh::Point outPoint2_p = BeSelectModel.model.mesh.point(outPoint2);
		MyMesh::Point disPoint = outPoint1_p - outPoint2_p;
		float dis = disPoint[0] * disPoint[0] + disPoint[1] * disPoint[1];
		dis = sqrt(dis);
		dis = sqrt(dis * dis + disPoint[2] * disPoint[2]);
#ifdef DEBUG
		cout << "Dis " << dis;
#endif // DEBUG	
		nowDis = nowDis + dis;
		if (nowDis <= (OuterLengh / 4.0))
		{
			MyMesh::TexCoord2D UV;
			UV[0] = 0;
			UV[1] = nowDis / (OuterLengh / 4.0);
			BeSelectModel.model.mesh.set_texcoord2D(outPoint2, UV);
#ifdef DEBUG
			cout << "Point " << i + 1 << " UV " << UV[0] << " " << UV[1] << endl;
#endif // DEBUG
		}
		else if ((nowDis <= ((OuterLengh / 4.0) * 2.0)) && (nowDis > ((OuterLengh / 4.0) * 1.0)))
		{
			MyMesh::TexCoord2D UV;
			UV[0] = ((nowDis - (OuterLengh / 4.0) * 1.0) / (OuterLengh / 4.0));
			UV[1] = 1;
			BeSelectModel.model.mesh.set_texcoord2D(outPoint2, UV);
#ifdef DEBUG
			cout << "Point " << i + 1 << " UV " << UV[0] << " " << UV[1] << endl;
#endif // DEBUG
		}
		else if ((nowDis <= ((OuterLengh / 4.0) * 3.0)) && (nowDis > ((OuterLengh / 4.0) * 2.0)))
		{
			MyMesh::TexCoord2D UV;
			UV[0] = 1;
			UV[1] = 1 - ((nowDis - (OuterLengh / 4.0) * 2.0)) / (OuterLengh / 4.0);
			BeSelectModel.model.mesh.set_texcoord2D(outPoint2, UV);
#ifdef DEBUG
			cout << "Point " << i + 1 << " UV " << UV[0] << " " << UV[1] << endl;
#endif // DEBUG
		}
		else if ((nowDis <= (OuterLengh)) && (nowDis > ((OuterLengh / 4.0) * 3.0)))
		{
			MyMesh::TexCoord2D UV;
			UV[0] = 1 - ((nowDis - (OuterLengh / 4.0) * 3.0)) / (OuterLengh / 4.0);
			UV[1] = 0;
			BeSelectModel.model.mesh.set_texcoord2D(outPoint2, UV);
#ifdef DEBUG
			cout << "Point " << i + 1 << " UV " << UV[0] << " " << UV[1] << endl;
#endif // DEBUG
		}
	}
	//select inner point================================================================================================================
	MyMesh::VertexIter VI = BeSelectModel.model.mesh.vertices_begin();
	for (; VI != BeSelectModel.model.mesh.vertices_end(); ++VI)
	{
		MyMesh::VertexHandle SeachPoint = BeSelectModel.model.mesh.vertex_handle(VI->idx());// VI.handle();// BeSelectModel.model.mesh.vertex_handle(VI);
		bool ISOUT = false;
		for (int j = 0; j < OuterPoint.size(); j++)
		{
			if (OuterPoint[j] == SeachPoint.idx())
			{
				ISOUT = true;
			}
		}
		if (ISOUT == false)
		{
			InnerPoint.push_back(SeachPoint.idx());
#ifdef DEBUG
			cout << "InnerPoint " << InnerPoint.size() << " ID: " << SeachPoint.idx() << endl;
#endif // DEBUG
		}
	}


	SparseMatrix<double> A(InnerPoint.size(), InnerPoint.size());
	for (int i = 0; i < InnerPoint.size(); i++)
	{
		A.insert(i, i) = 1.0;
	}
	SparseQR<SparseMatrix<double>, COLAMDOrdering<int>> linearSolver;
	VectorXd Bx(InnerPoint.size());
	VectorXd By(InnerPoint.size());

	OpenMesh::EPropHandleT<MyMesh::Point> Wi;
	BeSelectModel.model.mesh.add_property(Wi);
	//Do wi==================================================================================================================================
	for (MyMesh::EdgeIter EI = BeSelectModel.model.mesh.edges_begin(); EI != BeSelectModel.model.mesh.edges_end(); ++EI)
	{
		MyMesh::Point tmepWi;

		MyMesh::EdgeHandle Eh = BeSelectModel.model.mesh.edge_handle(EI->idx());
		MyMesh::HalfedgeHandle HeH = BeSelectModel.model.mesh.halfedge_handle(Eh, 0);
		if (BeSelectModel.model.mesh.is_boundary(Eh) == false)
		{
			MyMesh::VertexHandle FromVertexH = BeSelectModel.model.mesh.from_vertex_handle(HeH);
			MyMesh::VertexHandle ToVertexH = BeSelectModel.model.mesh.to_vertex_handle(HeH);
			MyMesh::VertexHandle OppositeVertexH = BeSelectModel.model.mesh.opposite_vh(HeH);
			MyMesh::VertexHandle OppoOppoVertexH = BeSelectModel.model.mesh.opposite_he_opposite_vh(HeH);

			MyMesh::Point FromVertex = BeSelectModel.model.mesh.point(FromVertexH);
			MyMesh::Point ToVertex = BeSelectModel.model.mesh.point(ToVertexH);
			MyMesh::Point OppositeVertex = BeSelectModel.model.mesh.point(OppositeVertexH);
			MyMesh::Point OppoOppoVertex = BeSelectModel.model.mesh.point(OppoOppoVertexH);

			double Angle1, Angle2;

			Angle1 = PointAngle(FromVertex, ToVertex, OppositeVertex);
			Angle2 = PointAngle(FromVertex, ToVertex, OppoOppoVertex);
#ifdef DEBUG
			cout << "Tri1_1" << FromVertex[0] << " " << FromVertex[1] << " " << FromVertex[2] << endl;
			cout << "Tri1_2" << ToVertex[0] << " " << ToVertex[1] << " " << ToVertex[2] << endl;
			cout << "Tri1_3" << OppositeVertex[0] << " " << OppositeVertex[1] << " " << OppositeVertex[2] << endl;
			cout << "Tri2_3" << OppoOppoVertex[0] << " " << OppoOppoVertex[1] << " " << OppoOppoVertex[2] << endl;
			cout << "Angles 1 2 " << Angle1 << " " << Angle2 << endl;
#endif // DEBUG

			tmepWi[0] = ((1.0 / tan((Angle1 * 3.1415926) / 180.0)) + (1.0 / tan((Angle2 * 3.1415926) / 180.0)));
#ifdef DEBUG
			cout << tmepWi[0] << endl;
#endif // DEBUG
			tmepWi[1] = 123;
			tmepWi[2] = 456;
			BeSelectModel.model.mesh.property(Wi, *EI) = tmepWi;
		}
	}
#ifdef DEBUG
	cout << endl;
#endif // DEBUG
	for (int j = 0; j < InnerPoint.size(); j++)
	{
		Bx[j] = 0;
		By[j] = 0;
	}

	//make matrix================================================================================================================
	for (int i = 0; i < InnerPoint.size(); i++)
	{
		float ABigWi = 0;
		MyMesh::VertexHandle NowVertex = BeSelectModel.model.mesh.vertex_handle(InnerPoint[i]);
		vector<double> ThisA_Array;
		for (int j = 0; j < InnerPoint.size(); j++)
		{
			ThisA_Array.push_back(0.0);
		}
#ifdef DEBUG
		cout << "NOW POINT" << InnerPoint[i] << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
#endif // DEBUG

		//Done Matrix===========================================================================================================================
		for (MyMesh::VVIter VV = BeSelectModel.model.mesh.vv_begin(NowVertex); VV != BeSelectModel.model.mesh.vv_end(NowVertex); ++VV)
		{
			float ThisVertexWi = 0;
			MyMesh::VertexHandle TargetVet = BeSelectModel.model.mesh.vertex_handle(VV->idx());

			///Not Boundry
			for (MyMesh::VEIter VE = BeSelectModel.model.mesh.ve_begin(NowVertex); VE != BeSelectModel.model.mesh.ve_end(NowVertex); ++VE)
			{
				MyMesh::EdgeHandle Eh = BeSelectModel.model.mesh.edge_handle(VE->idx());
				MyMesh::HalfedgeHandle HeH1 = BeSelectModel.model.mesh.halfedge_handle(Eh, 0);
				MyMesh::HalfedgeHandle HeH2 = BeSelectModel.model.mesh.halfedge_handle(Eh, 1);//?
				MyMesh::VertexHandle HeH1_FromVetx = BeSelectModel.model.mesh.from_vertex_handle(HeH1);
				MyMesh::VertexHandle HeH1_ToVetx = BeSelectModel.model.mesh.to_vertex_handle(HeH1);
				if (BeSelectModel.model.mesh.is_boundary(TargetVet) == true)
				{
					//MyMesh::Point tempwi = BeSelectModel.model.mesh.property(Wi, Eh);
					if ((HeH1_FromVetx.idx() == TargetVet.idx()) && (HeH1_ToVetx.idx() == NowVertex.idx()))
					{
						MyMesh::Point EdgeWi = BeSelectModel.model.mesh.property(Wi, Eh);
						MyMesh::TexCoord2D tempT2D = BeSelectModel.model.mesh.texcoord2D(HeH1_FromVetx);
						Bx[i] += EdgeWi[0] * tempT2D[0];//x
						By[i] += EdgeWi[0] * tempT2D[1];//y
						ThisVertexWi = EdgeWi[0];
#ifdef DEBUG
						cout << "wi Right1 " << EdgeWi[0] << " " << tempT2D[0] << endl;
						cout << "wi Right2 " << EdgeWi[0] << " " << tempT2D[1] << endl;
						cout << "wi Right3 " << Bx[i] << " " << By[i] << endl << endl;
#endif // DEBUG
						break;
					}
					else if ((HeH1_FromVetx.idx() == NowVertex.idx()) && (HeH1_ToVetx.idx() == TargetVet.idx()))
					{
						MyMesh::Point EdgeWi = BeSelectModel.model.mesh.property(Wi, Eh);
						MyMesh::TexCoord2D tempT2D = BeSelectModel.model.mesh.texcoord2D(HeH1_ToVetx);
						Bx[i] += EdgeWi[0] * tempT2D[0];//x
						By[i] += EdgeWi[0] * tempT2D[1];//y
						ThisVertexWi = EdgeWi[0];
#ifdef DEBUG
						cout << "wi Right1 " << EdgeWi[0] << " " << tempT2D[0] << endl;
						cout << "wi Right2 " << EdgeWi[0] << " " << tempT2D[1] << endl;
						cout << "wi Right3 " << Bx[i] << " " << By[i] << endl << endl;
#endif // DEBUG
						break;
					}

				}
				else
				{
					if ((HeH1_FromVetx.idx() == TargetVet.idx()) && (HeH1_ToVetx.idx() == NowVertex.idx()))
					{
						MyMesh::Point EdgeWi = BeSelectModel.model.mesh.property(Wi, Eh);
						ThisVertexWi = EdgeWi[0];
#ifdef DEBUG
						cout << "wi Left " << ThisVertexWi << endl << endl;
#endif // DEBUG
						for (int j = 0; j < InnerPoint.size(); j++)
						{
							if (InnerPoint[j] == TargetVet.idx())
							{
								ThisA_Array[j] = ThisVertexWi;
								break;
							}
						}
						break;
					}
					else if ((HeH1_FromVetx.idx() == NowVertex.idx()) && (HeH1_ToVetx.idx() == TargetVet.idx()))
					{
						MyMesh::Point EdgeWi = BeSelectModel.model.mesh.property(Wi, Eh);
						ThisVertexWi = EdgeWi[0];
#ifdef DEBUG
						cout << "wi Left " << ThisVertexWi << endl << endl;
#endif // DEBUG
						for (int j = 0; j < InnerPoint.size(); j++)
						{
							if (InnerPoint[j] == TargetVet.idx())
							{
								ThisA_Array[j] = ThisVertexWi;
								break;
							}
						}
						break;
					}
				}
			}
			ABigWi += ThisVertexWi;
		}
		Bx[i] = Bx[i] / ABigWi;
		By[i] = By[i] / ABigWi;
		for (int j = 0; j < InnerPoint.size(); j++)
		{
			if (i != j)
			{
				A.insert(i, j) = ((-ThisA_Array[j]) / ABigWi);
			}
		}
		cout << "Line " << i << endl;
	}



#ifdef DEBUG
	cout << "A" << endl;
	cout << A;
	cout << "Bx" << endl;
	cout << Bx;
	cout << "By" << endl;
	cout << By;
#endif // DEBUG
	A.makeCompressed();
	linearSolver.compute(A);
	VectorXd Xx = linearSolver.solve(Bx);
	linearSolver.compute(A);
	VectorXd Xy = linearSolver.solve(By);
	cout << "liner Solve!!" << endl;
	for (int i = 0; i < InnerPoint.size(); i++)
	{
		MyMesh::TexCoord2D UV;
		MyMesh::VertexHandle innerPoint = BeSelectModel.model.mesh.vertex_handle(InnerPoint[i]);
		UV[0] = Xx[i];
		UV[1] = Xy[i];
		BeSelectModel.model.mesh.set_texcoord2D(innerPoint, UV);
#ifdef DEBUG
		cout << "InnerUV " << UV[0] << " " << UV[1] << endl;
#endif // DEBUG

		BeSelectModel.model.mesh.request_face_normals();
		BeSelectModel.model.mesh.update_normals();
		BeSelectModel.model.mesh.release_face_normals();

		BeSelectModel.MY_LoadToShader();
	}
}

void magic_delete()
{
	model.selectedFace.clear();
	model.selectedPoint.clear();
	model.selectPoint_Seq.clear();
	BeSelectModel.model.mesh.clear();
	BeSelectModel.model.mesh.ClearMesh();
	InnerPoint.clear();
	OuterPoint.clear();
	OuterLengh = 0;
}

//針對兩個面，直接對應點的ID
bool IsFaceConnect(int ID_X, int ID_Y)
{
	MyMesh::FHandle FH_X = BeSelectModel.model.mesh.face_handle(ID_X);
	MyMesh::FHandle FH_Y = BeSelectModel.model.mesh.face_handle(ID_Y);
	int ConnectPointCount = 0;
	for (MyMesh::FVIter FVI_X = BeSelectModel.model.mesh.fv_begin(FH_X); FVI_X != BeSelectModel.model.mesh.fv_end(FH_X); ++FVI_X)
	{
		for (MyMesh::FVIter FVI_Y = BeSelectModel.model.mesh.fv_begin(FH_Y); FVI_Y != BeSelectModel.model.mesh.fv_end(FH_Y); ++FVI_Y)
		{
			//在 id 上是同一個點
			if (FVI_X->idx() == FVI_Y->idx())
			{
				ConnectPointCount++;
			}
			//在距離上是同一個點
			MyMesh::VHandle VH1 = BeSelectModel.model.mesh.vertex_handle(FVI_X->idx());
			MyMesh::VHandle VH2 = BeSelectModel.model.mesh.vertex_handle(FVI_Y->idx());
			MyMesh::Point P1 = BeSelectModel.model.mesh.point(VH1);
			MyMesh::Point P2 = BeSelectModel.model.mesh.point(VH2);
			float diff_x = (P1[0] - P2[0]) * (P1[0] - P2[0]);
			float diff_y = (P1[1] - P2[1]) * (P1[1] - P2[1]);
			float diff_z = (P1[2] - P2[2]) * (P1[2] - P2[2]);
			if (sqrt(diff_x + diff_y + diff_z) < 0.01)
			{
				ConnectPointCount++;
			}
			if (ConnectPointCount == 2)
			{
				return true;
			}
		}
	}
	return false;
}
//DFS尋找
void DFS(std::vector<std::vector<int>>& M, std::vector<bool>& visited, int i)
{
	visited[i] = true;
	FriendZone[FriendCount].push_back(i);
	//尋找這個人的朋友
	for (int j = 0; j < M.size(); j++)
	{
		//沒有交集或已經被找過
		if (M[i][j] == 0 || visited[j] == true) continue;
		//繼續尋找
		DFS(M, visited, j);
	}
}
//分類
int FindFriend(std::vector<std::vector<int>>& M)
{
	int n = M.size();
	if (n == 0) return -1;

	//有沒有被找過
	std::vector<bool> visited(n, false);
	//
	for (int i = 0; i < n; i++)
	{
		FriendZone.push_back(std::vector<int>());
		//如果被找過
		if (visited[i] == true) continue;
		//沒被找過
		DFS(M, visited, i);
		FriendCount++;
	}

	return FriendCount;
}
//無
void detectRoof()
{

}

//use Find friend algothim
void MergeBoundary()
{
	//產生朋友矩陣
	//用ID產生
	int n_faces = BeSelectModel.model.mesh.n_faces();
	std::vector<std::vector<int>> M;
	M.resize(n_faces);

	for (int i = 0; i < Idx.size(); i++)
	{
		M[i].resize(Idx.size());
	}
	for (int i = 0; i < n_faces; i++)
	{
		for (int j = 0; j < n_faces; j++)
		{
			//如果兩個有相連
			if (i != j && IsFaceConnect(i, j))
			{
				M[i][j] = 1;
				M[j][i] = 1;
			}
			else
			{
				M[i][j] = 0;
				M[j][i] = 0;
			}
		}
	}

	//產生完矩陣
	int ClusterCount = FindFriend(M);
	cout << "Cluster " << ClusterCount << "\n";
	//重新填色
	for (int i = 0; i < FriendZone.size(); i++)
	{
		MyMesh::TexCoord2D TC;
		TC[0] = colormap[i][0];
		TC[1] = colormap[i][1];
		for (int j = 0; j < FriendZone[i].size(); j++)
		{
			MyMesh::FHandle FH = BeSelectModel.model.mesh.face_handle(FriendZone[i][j]);
			for (MyMesh::FVIter FVI = BeSelectModel.model.mesh.fv_begin(FH); FVI != BeSelectModel.model.mesh.fv_end(FH); FVI++)
			{
				MyMesh::VHandle VH = BeSelectModel.model.mesh.vertex_handle(FVI->idx());
				BeSelectModel.model.mesh.set_texcoord2D(VH, TC);
			}
		}
	}

	BeSelectModel.MY_LoadToShader();
}

//use halfedge find bounding
void caluBoundary()
{
	//for all vertex find convel
	std::vector<std::vector<MyMesh::HalfedgeHandle>> BoundaryHalfEdgeHandler;
	std::vector<MyMesh::HalfedgeHandle> NOToundary;

	for (MyMesh::HIter HI = BeSelectModel.model.mesh.halfedges_begin(); HI != BeSelectModel.model.mesh.halfedges_end(); HI++)
	{
		MyMesh::HalfedgeHandle HFH = BeSelectModel.model.mesh.halfedge_handle(HI->idx());

		//確認halfedge是不是 已經在不是Boundary
		std::vector<MyMesh::HalfedgeHandle>::iterator it = std::find(NOToundary.begin(), NOToundary.end(), HFH);
		if (it != NOToundary.end())
		{
			continue;
		}
		//確認halfedge是不是 已經在Boundary列表裡
		bool isFoundinBoundaryHalfEdgeHandler = false;
		for (int i = 0; i < BoundaryHalfEdgeHandler.size(); i++)
		{
			std::vector<MyMesh::HalfedgeHandle>::iterator it = std::find(BoundaryHalfEdgeHandler[i].begin(), BoundaryHalfEdgeHandler[i].end(), HFH);
			if (it != BoundaryHalfEdgeHandler[i].end())
			{
				isFoundinBoundaryHalfEdgeHandler = true;
				continue;
			}
		}
		if (isFoundinBoundaryHalfEdgeHandler)
		{
			continue;
		}
		//從來沒找過的halfedge
		if (BeSelectModel.model.mesh.is_boundary(HFH))
		{
			//找到第一個邊界
			std::vector<MyMesh::HalfedgeHandle> VhEH;
			VhEH.push_back(HFH);
			BoundaryHalfEdgeHandler.push_back(VhEH);
			HFH = BeSelectModel.model.mesh.next_halfedge_handle(HFH);
			//直接找出一串邊界
			for (;;)
			{
				if (HFH == BoundaryHalfEdgeHandler[BoundaryHalfEdgeHandler.size() - 1][0])
				{
					break;
				}
				BoundaryHalfEdgeHandler[BoundaryHalfEdgeHandler.size() - 1].push_back(HFH);
				HFH = BeSelectModel.model.mesh.next_halfedge_handle(HFH);
			}
		}
		else
		{
			NOToundary.push_back(HFH);
		}
	}

	//直接給出點的陣列
	//刪掉直線上的點
	//依照角度計算
	std::vector < std::vector<MyMesh::Point> > EdgePointSet;
	for (int i = 0; i < BoundaryHalfEdgeHandler.size(); i++)
	{
		EdgePointSet.push_back(std::vector<MyMesh::Point>());
		for (int j = 0; j < BoundaryHalfEdgeHandler[i].size(); j++)
		{
			MyMesh::VHandle VH_Last = BeSelectModel.model.mesh.from_vertex_handle(BoundaryHalfEdgeHandler[i][j % BoundaryHalfEdgeHandler[i].size()]);
			MyMesh::VHandle VH_This = BeSelectModel.model.mesh.from_vertex_handle(BoundaryHalfEdgeHandler[i][(j + 1) % BoundaryHalfEdgeHandler[i].size()]);
			MyMesh::VHandle VH_Next = BeSelectModel.model.mesh.from_vertex_handle(BoundaryHalfEdgeHandler[i][(j + 2) % BoundaryHalfEdgeHandler[i].size()]);

			MyMesh::Point P_Last = BeSelectModel.model.mesh.point(VH_Last);
			MyMesh::Point P_This = BeSelectModel.model.mesh.point(VH_This);
			MyMesh::Point P_Next = BeSelectModel.model.mesh.point(VH_Next);
			//是轉角
			if (PointAngle(P_Last, P_Next, P_This) < 175)
			{
				EdgePointSet[i].push_back(P_This);
			}
		}
	}

	//找每個點的深度
	//尋找點座標範圍內 xz相近  的點
	//找出內群裡最高的
	double diff = 0.01;

	BoundingPointheight.resize(EdgePointSet.size());
	for (int i = 0; i < EdgePointSet.size(); i++)
	{
		BoundingPointheight[i].resize(EdgePointSet[i].size());
		for (int j = 0; j < EdgePointSet[i].size(); j++)
		{
			BoundingPointheight[i][j] = DeepY;
		}
	}

	for (int i = 0; i < EdgePointSet.size(); i++)
	{
		for (int j = 0; j < EdgePointSet[i].size(); j++)
		{
			//已經找過
			if (BoundingPointheight[i][j] > 199)
			{
				continue;
			}
			MyMesh::Point P_Mid = EdgePointSet[i][j];

			for (int k = 0; k < EdgePointSet.size(); k++)
			{
				for (int LL = 0; LL < EdgePointSet[k].size(); LL++)
				{
					if (i != k)
					{
						MyMesh::Point P_Compare = EdgePointSet[k][LL];
						double Distence = sqrt(pow(P_Compare[0] - P_Mid[0], 2) + pow(P_Compare[2] - P_Mid[2], 2));
						if (Distence < diff)
						{
							if (P_Compare[1] < P_Mid[1])
							{
								if (P_Compare[1] > BoundingPointheight[i][j])
								{
									BoundingPointheight[i][j] = P_Compare[1];
									BoundingPointheight[k][LL] = 200;
								}
							}
						}
					}
				}
			}
		}
	}

	//HowToDraw
	//產生一系列 只有邊界的mesh
	for (int i = 0; i < BoundaryHalfEdgeHandler.size(); i++)
	{
		ALLModel.push_back(MeshObject());
		ALLModel[i].model.mesh.clear();
		ALLModel[i].model.mesh.ClearMesh();
		ALLModel[i].model.mesh.request_vertex_texcoords2D();
	}
	//存點 做concave hull
	std::fstream PointListFile;
	PointListFile.open("./Dfile/Point_List.txt", ios::out);
	for (int i = 0; i < BoundaryHalfEdgeHandler.size(); i++)
	{
		//set vertex
		MyMesh::TexCoord2D TC;
		TC[0] = 0.5;
		TC[1] = 0.1;
		std::vector<MyMesh::VertexHandle> VHnadleVector;

		//全部點的線
		/*for (int j = 0; j < BoundaryHalfEdgeHandler[i].size(); j++)
		{
			MyMesh::VHandle VH = BeSelectModel.model.mesh.from_vertex_handle(BoundaryHalfEdgeHandler[i][j]);
			MyMesh::Point P = BeSelectModel.model.mesh.point(VH);
			VHnadleVector.push_back(ALLModel[i].model.mesh.add_vertex(P));
		}*/


		int PointLenght = EdgePointSet[i].size();

		//篩選出來的點
		for (int j = 0; j < PointLenght; j++)
		{
			MyMesh::Point P = EdgePointSet[i][j];
			PointListFile << P[0] << " " << P[2] << " ";

			//直接變最高
			//P[1] = HeightY;

			VHnadleVector.push_back(ALLModel[i].model.mesh.add_vertex(P));
		}
		PointListFile << "\n";

		//向下拉出的點
		for (int j = 0; j < PointLenght; j++)
		{
			MyMesh::Point P = EdgePointSet[i][j];
			/*if (BoundingPointheight[i][j] < 199)
			{
				P[1] = BoundingPointheight[i][j];
			}*/
			P[1] = DeepY;
			VHnadleVector.push_back(ALLModel[i].model.mesh.add_vertex(P));
		}
		std::vector<MyMesh::VertexHandle> face_vhandles;

		//理論上需要做面高度的分類並產生對應 屋頂union區域 再擷取出 屋頂區域下拉面 在從屋頂下拉面，做同樣的整個流程
		//
		//

		for (int j = 0; j < PointLenght; j++)
		{
			//face 0 down
			face_vhandles.clear();
			face_vhandles.push_back(VHnadleVector[j]);
			face_vhandles.push_back(VHnadleVector[(j + 1) % PointLenght]);
			face_vhandles.push_back(VHnadleVector[j + PointLenght]);
			ALLModel[i].model.mesh.add_face(face_vhandles);
			//cout << "FACE " << ID->first - 1 << "FACE0 Down Is be Rebuild" << endl;
			// 
			//face 0 up
			face_vhandles.clear();
			face_vhandles.push_back(VHnadleVector[(j + 1) % PointLenght + PointLenght]);
			face_vhandles.push_back(VHnadleVector[j + PointLenght]);
			face_vhandles.push_back(VHnadleVector[(j + 1) % PointLenght]);
			ALLModel[i].model.mesh.add_face(face_vhandles);
			//cout << "FACE " << ID->first - 1 << "FACE0 UP Is be Rebuild" << endl;
		}
	}

	//saving OBJ
	std::fstream ObjFile;
	ObjFile.open("./Dfile/NewModel.obj", ios::out);
	int VCount = BeSelectModel.model.mesh.n_vertices();
	int NowVerticesCount = 1;
	std::string Face_string = "";
	for (MyMesh::VIter VI = BeSelectModel.model.mesh.vertices_begin(); VI != BeSelectModel.model.mesh.vertices_end(); VI++)
	{
		MyMesh::VHandle VH = BeSelectModel.model.mesh.vertex_handle(VI->idx());
		MyMesh::Point P = BeSelectModel.model.mesh.point(VH);
		ObjFile << "v " << P[0] << " " << P[1] << " " << P[2] << "\n";
	}
	for (MyMesh::FIter FI = BeSelectModel.model.mesh.faces_begin(); FI != BeSelectModel.model.mesh.faces_end(); FI++)
	{
		std::vector<int> FVIndex;
		MyMesh::FHandle FH = BeSelectModel.model.mesh.face_handle(FI->idx());
		for (MyMesh::FVIter FVI = BeSelectModel.model.mesh.fv_begin(FH); FVI != BeSelectModel.model.mesh.fv_end(FH); FVI++)
		{
			FVIndex.push_back(FVI->idx() + 1);
		}
		Face_string += ("f " + std::to_string(FVIndex[0]) + " " + std::to_string(FVIndex[1]) + " " + std::to_string(FVIndex[2]) + "\n");
	}

#pragma region saveDownFace
#ifdef SAVEDOWNFACE
	for (int i = 0; i < BoundaryHalfEdgeHandler.size(); i++)
	{
		for (MyMesh::FIter FI = ALLModel[i].model.mesh.faces_begin(); FI != ALLModel[i].model.mesh.faces_end(); FI++)
		{
			std::vector<int> FVIndex;
			MyMesh::FHandle FH = ALLModel[i].model.mesh.face_handle(FI->idx());
			for (MyMesh::FVIter FVI = ALLModel[i].model.mesh.fv_begin(FH); FVI != ALLModel[i].model.mesh.fv_end(FH); FVI++)
			{
				FVIndex.push_back(FVI->idx() + NowVerticesCount + VCount);
			}
			Face_string += ("f " + std::to_string(FVIndex[0]) + " " + std::to_string(FVIndex[1]) + " " + std::to_string(FVIndex[2]) + "\n");
		}


		for (MyMesh::VIter VI = ALLModel[i].model.mesh.vertices_begin(); VI != ALLModel[i].model.mesh.vertices_end(); VI++)
		{
			MyMesh::VHandle VH = ALLModel[i].model.mesh.vertex_handle(VI->idx());
			MyMesh::Point P = ALLModel[i].model.mesh.point(VH);
			ObjFile << "v " << P[0] << " " << P[1] << " " << P[2] << "\n";
			NowVerticesCount++;
		}
	}
#endif // SAVEDOWNFACE
#pragma endregion
	ObjFile << Face_string;;
	ObjFile.close();

}

void NewDetectRoof()
{
	/*float DepthMap[600][600] = { 0 };
	float IDMap[600][600] = {0 };*/
	unsigned char* data = new unsigned char[360000];
	float* Rawdata = new float[360000];

	unsigned char* Colordata = new unsigned char[360000 * 3];
	int* RawIdxdata = new int[360000];
	float MaxDepth = -1100000;
	float MinDepth = 1100000;
	map<int, double> IdxDepth;
	std::map<int, std::vector<double>> ALL_Idx_Depth;
	std::fstream ObjFile;
	std::string Face_index = "";
	//std::fstream IdxFile;
	glm::mat4 mvMat = meshWindowCam.GetViewMatrix() * meshWindowCam.GetModelMatrix();
	glm::mat4 pMat = meshWindowCam.GetProjectionMatrix(aspect);
	float depthValue = 0;
	ObjFile.open("./Dfile/NewModel.obj", ios::out);
	//DepthFile.open("./Dfile/DepthFile.txt", ios::out);


	//depthFile << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

	for (int i = 0; i < 600; i++)
	{
		for (int j = 0; j < 600; j++)
		{
			//depth map
			glReadPixels(i, j, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthValue);
			float D = pickingTexture.ReadTexture(i, j);
			//cout << D << " ";
			if (D != 0)
			{
				if (depthValue < MinDepth)
				{
					MinDepth = depthValue;
				}
				if (depthValue > MaxDepth)
				{
					MaxDepth = depthValue;
				}
			}
			//IdxFile << depthValue << " ";
			//DepthFile << D << " ";

			if (D == 0)
			{

			}

			Rawdata[(i)+(599 - j) * 600] = depthValue;

			RawIdxdata[(i)+(599 - j) * 600] = D;

			if (Idx.find(D) == Idx.end())
			{
				Idx[D] = 1;
				ALL_Idx_Depth[D].push_back(depthValue);
			}
			else
			{
				Idx[D] ++;
				ALL_Idx_Depth[D].push_back(depthValue);
			}
		}
		//cout << "\n";
		//IdxFile << "\n";
		//DepthFile << "\n";
		if (i % 60 == 0)
		{
			cout << i / 60 << "/10" << "\n";
		}
	}

	//IdxFile.close();
	//DepthFile.close();

	double diff = MaxDepth - MinDepth;
	for (int i = 0; i < 360000; i++)
	{
		double tmp = Rawdata[i] - MinDepth;
		data[i] = (char)((tmp / diff) * 256.0);
		Colordata[i * 3 + 0] = (char)(colormap[RawIdxdata[i]][0] * 256.0);
		Colordata[i * 3 + 1] = (char)(colormap[RawIdxdata[i]][1] * 256.0);
		Colordata[i * 3 + 2] = (char)(colormap[RawIdxdata[i]][2] * 256.0);
	}
	//stbi_flip_vertically_on_write(true);
	stbi_write_png("Fileeee.png", 600, 600, 1, data, 0);
	stbi_write_png("Fileeee_Color.png", 600, 600, 3, Colordata, 0);
	stbi_image_free(data);
	stbi_image_free(Colordata);
	Idx.erase(0);
	ALL_Idx_Depth.erase(0);

	//平均深度
	for (map<int, std::vector<double>>::iterator ID = ALL_Idx_Depth.begin(); ID != ALL_Idx_Depth.end(); ID++)
	{
		double totalDepth = 0;
		for (int i = 0; i < ID->second.size(); i++)
		{
			totalDepth += ID->second[i];
		}
		ID->second.push_back(totalDepth / (double)ID->second.size());
	}

	for (map<int, int>::iterator ID = Idx.begin(); ID != Idx.end(); ID++)
	{
#ifdef CREATE_FACE_DEBUG
		cout << ID->first - 1 << " " << ID->second << "\n";
#endif // CREATE_FACE_DEBUG
		if (ID->first != 0)
		{
			//ALLModel.push_back(MeshObject());
			//ALLModel[ALLModel.size() - 1].model.mesh.clear();
			//ALLModel[ALLModel.size() - 1].model.mesh.ClearMesh();
			//ALLModel[ALLModel.size() - 1].model.mesh.request_vertex_texcoords2D();// _vertex_texcoords2D
			model.AddSelectedFace(ID->first - 1);
		}
	}
	//找最高與最低

	for (MyMesh::FIter FI = model.model.mesh.faces_begin(); FI != model.model.mesh.faces_end(); FI++)
	{
		MyMesh::FHandle FH = model.model.mesh.face_handle(FI->idx());
		for (MyMesh::FVIter FV1 = model.model.mesh.fv_begin(FH); FV1 != model.model.mesh.fv_end(FH); ++FV1)
		{
			MyMesh::VHandle VH = model.model.mesh.vertex_handle(FV1->idx());
			MyMesh::Point P = model.model.mesh.point(VH);
			if (P[1] < DeepY)
			{
				DeepY = P[1];
			}
			if (P[1] > HeightY)
			{
				HeightY = P[1];
			}
		}
	}
	cout << "DeepY " << DeepY << "\n";
	//mesh TopDown

	//舊vertex 與 新vertex對應
	//給舊vertex Index回傳新vertex Index
	//排除重複點
	std::map<int, int> VectorSerise;
	std::vector <MyMesh::VertexHandle> vhandle;
	MyMesh::TexCoord2D TC;
	TC[0] = 0;
	TC[1] = 1;
	BeSelectModel.model.mesh.request_vertex_texcoords2D();
	//直接連接依照每個面，建造向下bounding box
	for (map<int, int>::iterator ID = Idx.begin(); ID != Idx.end(); ID++)
	{
		MyMesh::FHandle FH = model.model.mesh.face_handle(ID->first - 1);
		std::vector<MyMesh::VertexHandle> VHV;
		for (MyMesh::FVIter FV1 = model.model.mesh.fv_begin(FH); FV1 != model.model.mesh.fv_end(FH); ++FV1)
		{
			if (VectorSerise.find(FV1->idx()) == VectorSerise.end())
			{
				MyMesh::VHandle VH = model.model.mesh.vertex_handle(FV1->idx());
				MyMesh::Point P = model.model.mesh.point(VH);
				int Size = VectorSerise.size();
				VectorSerise[FV1->idx()] = Size;
				vhandle.push_back(BeSelectModel.model.mesh.add_vertex(P));
				BeSelectModel.model.mesh.set_texcoord2D(vhandle[vhandle.size() - 1], TC);
			}
		}
	}
	TC[0] = 1;
	TC[1] = 1;

	//int VSize = vhandle.size();
	//for (int i = 0; i < VSize; i++)
	//{
	//	MyMesh::VHandle VH = model.model.mesh.vertex_handle(vhandle[i].idx());
	//	MyMesh::Point P = model.model.mesh.point(VH);
	//	P[1] = DeepY;
	//	vhandle.push_back(BeSelectModel.model.mesh.add_vertex(P));
	//	BeSelectModel.model.mesh.set_texcoord2D(vhandle[vhandle.size() - 1], TC);
	//}

	//產生vertex handler
	//需要產生兩倍的vertex 屋頂與牆壁
	//===========================================================================================//地板高度

	//屋頂===================================================================================
	//地板===================================================================================
//#ifdef GENERATE_FLOOR
	/*TC[0] = 1;
	TC[1] = 1;
	for (map<int, int>::iterator VIS = VectorSerise.begin(); VIS != VectorSerise.end(); VIS++)
	{
		MyMesh::VertexHandle VH = model.model.mesh.vertex_handle(VIS->first);
		MyMesh::Point P = model.model.mesh.point(VH);
		P[1] = DeepY;
		vhandle.push_back(BeSelectModel.model.mesh.add_vertex(P));
		BeSelectModel.model.mesh.set_texcoord2D(vhandle[vhandle.size() - 1], TC);
	}*/
	//#endif // GENERATE_FLOOR
		//連接面產生小box===================================================================================
		//connect point to face to beselectModel===================================================================================
	std::vector<MyMesh::VertexHandle>  face_vhandles;
	int count = 0;

	for (map<int, int>::iterator ID = Idx.begin(); ID != Idx.end(); ID++)
	{
		if (ID->first != 0)
		{
			//舊三角面對應點id
			std::vector<int> Face_vertex_index;
			MyMesh::FHandle FH = model.model.mesh.face_handle(ID->first - 1);
			//std::vector <MyMesh::VertexHandle> All_Vhandle;
			//TC[0] = 1;
			//TC[1] = 0;
			//double AvgDepth = 0;
			////Find Avg Depth
			//for (MyMesh::FVIter FVI = model.model.mesh.fv_begin(FH); FVI != model.model.mesh.fv_end(FH); FVI++)
			//{
			//	MyMesh::VertexHandle VH = model.model.mesh.vertex_handle(FVI->idx());
			//	MyMesh::Point P = model.model.mesh.point(VH);
			//	AvgDepth += P[1];
			//}
			//AvgDepth /= 3;
			//int ClusterSize = 5;
			//double ClusterDeep = (HeightY - DeepY) / (double)ClusterSize;
			//int Custer = 0;
			//for (int i = 0; i < ClusterSize; i++)
			//{
			//	if (((ClusterDeep * (i + 1) + DeepY) > AvgDepth) && (AvgDepth > (ClusterDeep * i + DeepY)))
			//	{
			//		AvgDepth = (ClusterDeep * (i + 0.5)) + DeepY;
			//		Custer = i;
			//		break;
			//	}
			//}
			//TC[0] = colormap[Custer][0];
			//TC[1] = colormap[Custer][1];

			for (MyMesh::FVIter FVI = model.model.mesh.fv_begin(FH); FVI != model.model.mesh.fv_end(FH); FVI++)
			{
				Face_vertex_index.push_back(FVI->idx());

				//MyMesh::VertexHandle VH = model.model.mesh.vertex_handle(FVI->idx());
				//MyMesh::Point P = model.model.mesh.point(VH);
				//int ClusterSize = 5;
				//double ClusterDeep = (HeightY - DeepY) / (double)ClusterSize;
				////int Custer = 0;
				//for (int i = 0; i < ClusterSize; i++)
				//{
				//	if (((ClusterDeep * (i + 1) + DeepY) > P[1]) && (P[1] > (ClusterDeep * i + DeepY)))
				//	{
				//		P[1] = (ClusterDeep * (i + 0.5)) + DeepY;
				//		Custer = i;
				//		break;
				//	}
				//}
				////P[1] = AvgDepth;
				//All_Vhandle.push_back(ALLModel[count].model.mesh.add_vertex(P));
				//ALLModel[count].model.mesh.set_texcoord2D(All_Vhandle[All_Vhandle.size() - 1], TC);
			}
			//TC[0] = 1;
			//TC[1] = 1;
		/*	for (MyMesh::FVIter FVI = model.model.mesh.fv_begin(FH); FVI != model.model.mesh.fv_end(FH); FVI++)
			{
				MyMesh::VertexHandle VH = model.model.mesh.vertex_handle(FVI->idx());
				MyMesh::Point P = model.model.mesh.point(VH);
				P[1] = DeepY;
				All_Vhandle.push_back(ALLModel[count].model.mesh.add_vertex(P));
				ALLModel[count].model.mesh.set_texcoord2D(All_Vhandle[All_Vhandle.size() - 1], TC);
			}*/
#pragma region box
			//top
			face_vhandles.clear();
			face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[0]]]);
			face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[1]]]);
			face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[2]]]);
			BeSelectModel.model.mesh.add_face(face_vhandles);

			//cout << "Index " << Face_vertex_index[0] << " " << VectorSerise[Face_vertex_index[0]] << "\n";
			//cout << "Index " << Face_vertex_index[1] << " " << VectorSerise[Face_vertex_index[1]] << "\n";
			//cout << "Index " << Face_vertex_index[2] << " " << VectorSerise[Face_vertex_index[2]] << "\n";




#ifdef CREATE_FACE_DEBUG
			cout << "FACE " << ID->first - 1 << "TOP Is be Rebuild" << endl;
#endif // CREATE_FACE_DEBUG
			////for loop
			////face 0 down
			//face_vhandles.clear();
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[1]]]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[0]]]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[0]] + HalfSize]);
			//BeSelectModel.model.mesh.add_face(face_vhandles);
			//cout << "FACE " << ID->first-1 << "FACE0 Down Is be Rebuild" << endl;
			////face 0 up
			//face_vhandles.clear();
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[1]]]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[0]] + HalfSize]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[1]] + HalfSize]);
			//BeSelectModel.model.mesh.add_face(face_vhandles);
			//cout << "FACE " << ID->first-1 << "FACE0 UP Is be Rebuild" << endl;
			////face 1 down
			//face_vhandles.clear();
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[2]]]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[1]]]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[1]] + HalfSize]);
			//BeSelectModel.model.mesh.add_face(face_vhandles);
			//cout << "FACE " << ID->first-1 << "FACE1 Down Is be Rebuild" << endl;
			////face 1 up
			//face_vhandles.clear();
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[2]]]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[1]] + HalfSize]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[2]] + HalfSize]);
			//BeSelectModel.model.mesh.add_face(face_vhandles);
			//cout << "FACE " << ID->first-1 << "FACE1 UP Down Is be Rebuild" << endl;
			//
			////face 2 down
			//face_vhandles.clear();
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[0]]]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[2]]]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[2]] + HalfSize]);
			//BeSelectModel.model.mesh.add_face(face_vhandles);
			//cout << "FACE " << ID->first-1 << "FACE2 Down Is be Rebuild" << endl;
			////face 2 up
			//face_vhandles.clear();
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[0]]]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[2]] + HalfSize]);
			//face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[0]] + HalfSize]);
			//BeSelectModel.model.mesh.add_face(face_vhandles);
			//cout << "FACE " << ID->first-1 << "FACE2 UP Is be Rebuild" << endl;

			//top
			/*face_vhandles.clear();
			face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[0]] + VSize]);
			face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[1]] + VSize]);
			face_vhandles.push_back(vhandle[VectorSerise[Face_vertex_index[2]] + VSize]);
			BeSelectModel.model.mesh.add_face(face_vhandles);*/
#pragma endregion

#pragma region newbox
			//			int NHS = 3;
			//			//top
			//			face_vhandles.clear();
			//			face_vhandles.push_back(All_Vhandle[0]);
			//			face_vhandles.push_back(All_Vhandle[1]);
			//			face_vhandles.push_back(All_Vhandle[2]);
			//			ALLModel[count].model.mesh.add_face(face_vhandles);
			//#ifdef CREATE_FACE_DEBUG
			//			cout << "FACE " << ID->first - 1 << "TOP Is be Rebuild" << endl;
			//#endif // CREATE_FACE_DEBUG
			//			//for loop
			//			//face 0 down
			//			face_vhandles.clear();
			//			face_vhandles.push_back(All_Vhandle[1]);
			//			face_vhandles.push_back(All_Vhandle[0]);
			//			face_vhandles.push_back(All_Vhandle[0 + NHS]);
			//			ALLModel[count].model.mesh.add_face(face_vhandles);
			//#ifdef CREATE_FACE_DEBUG
			//			cout << "FACE " << ID->first - 1 << "FACE0 Down Is be Rebuild" << endl;
			//#endif // CREATE_FACE_DEBUG
			//			//face 0 up
			//			face_vhandles.clear();
			//			face_vhandles.push_back(All_Vhandle[1]);
			//			face_vhandles.push_back(All_Vhandle[0 + NHS]);
			//			face_vhandles.push_back(All_Vhandle[1 + NHS]);
			//			ALLModel[count].model.mesh.add_face(face_vhandles);
			//#ifdef CREATE_FACE_DEBUG
			//			cout << "FACE " << ID->first - 1 << "FACE0 UP Is be Rebuild" << endl;
			//#endif // CREATE_FACE_DEBUG
			//			//face 1 down
			//			face_vhandles.clear();
			//			face_vhandles.push_back(All_Vhandle[2]);
			//			face_vhandles.push_back(All_Vhandle[1]);
			//			face_vhandles.push_back(All_Vhandle[1 + NHS]);
			//			ALLModel[count].model.mesh.add_face(face_vhandles);
			//#ifdef CREATE_FACE_DEBUG
			//			cout << "FACE " << ID->first - 1 << "FACE1 Down Is be Rebuild" << endl;
			//#endif // CREATE_FACE_DEBUG
			//			//face 1 up
			//			face_vhandles.clear();
			//			face_vhandles.push_back(All_Vhandle[2]);
			//			face_vhandles.push_back(All_Vhandle[1 + NHS]);
			//			face_vhandles.push_back(All_Vhandle[2 + NHS]);
			//			ALLModel[count].model.mesh.add_face(face_vhandles);
			//#ifdef CREATE_FACE_DEBUG
			//			cout << "FACE " << ID->first - 1 << "FACE1 UP Down Is be Rebuild" << endl;
			//#endif // CREATE_FACE_DEBUG
			//			//face 2 down
			//			face_vhandles.clear();
			//			face_vhandles.push_back(All_Vhandle[0]);
			//			face_vhandles.push_back(All_Vhandle[2]);
			//			face_vhandles.push_back(All_Vhandle[2 + NHS]);
			//			ALLModel[count].model.mesh.add_face(face_vhandles);
			//#ifdef CREATE_FACE_DEBUG
			//			cout << "FACE " << ID->first - 1 << "FACE2 Down Is be Rebuild" << endl;
			//#endif // CREATE_FACE_DEBUG
			//			//face 2 up
			//			face_vhandles.clear();
			//			face_vhandles.push_back(All_Vhandle[0]);
			//			face_vhandles.push_back(All_Vhandle[2 + NHS]);
			//			face_vhandles.push_back(All_Vhandle[0 + NHS]);
			//			ALLModel[count].model.mesh.add_face(face_vhandles);
			//#ifdef CREATE_FACE_DEBUG
			//			cout << "FACE " << ID->first - 1 << "FACE2 UP Is be Rebuild" << endl;
			//#endif // CREATE_FACE_DEBUG
			//			//top
			//			face_vhandles.clear();
			//			face_vhandles.push_back(All_Vhandle[0 + NHS]);
			//			face_vhandles.push_back(All_Vhandle[2 + NHS]);
			//			face_vhandles.push_back(All_Vhandle[1 + NHS]);
			//			ALLModel[count].model.mesh.add_face(face_vhandles);
			//#pragma endregion
			//#ifdef CREATE_FACE_DEBUG
			//			cout << "FACE " << ID->first - 1 << " Is be Rebuild" << endl;
			//#endif // CREATE_FACE_DEBUG
			//			count++;
		}
	}

	BeSelectModel.model.mesh.request_face_normals();
	BeSelectModel.model.mesh.update_normals();
	BeSelectModel.model.mesh.release_face_normals();
	BeSelectModel.MY_LoadToShader();

	int NowVerticesCount = 1;

#ifdef MultiOBJ
	for (int i = 0; i < ALLModel.size(); i++)
	{
		ALLModel[i].model.mesh.request_face_normals();
		ALLModel[i].model.mesh.update_normals();
		ALLModel[i].model.mesh.release_face_normals();
		ALLModel[i].MY_LoadToShader();

		for (MyMesh::FIter FI = ALLModel[i].model.mesh.faces_begin(); FI != ALLModel[i].model.mesh.faces_end(); FI++)
		{
			std::vector<int> FVIndex;
			MyMesh::FHandle FH = ALLModel[i].model.mesh.face_handle(FI->idx());
			for (MyMesh::FVIter FVI = ALLModel[i].model.mesh.fv_begin(FH); FVI != ALLModel[i].model.mesh.fv_end(FH); FVI++)
			{
				FVIndex.push_back(FVI->idx() + NowVerticesCount);
			}
			Face_index += ("f " + std::to_string(FVIndex[0]) + " " + std::to_string(FVIndex[1]) + " " + std::to_string(FVIndex[2]) + "\n");
		}

		for (MyMesh::VIter VI = ALLModel[i].model.mesh.vertices_begin(); VI != ALLModel[i].model.mesh.vertices_end(); VI++)
		{
			MyMesh::VHandle VH = ALLModel[i].model.mesh.vertex_handle(VI->idx());
			MyMesh::Point P = ALLModel[i].model.mesh.point(VH);
			ObjFile << "v " << P[0] << " " << P[1] << " " << P[2] << "\n";
			NowVerticesCount++;
		}
	}
	ObjFile << Face_index;
	ObjFile.close();
#endif // MultiOBJ

	for (MyMesh::VIter VI = BeSelectModel.model.mesh.vertices_begin(); VI != BeSelectModel.model.mesh.vertices_end(); VI++)
	{
		MyMesh::VHandle VH = BeSelectModel.model.mesh.vertex_handle(VI->idx());
		MyMesh::Point P = BeSelectModel.model.mesh.point(VH);
		ObjFile << "v " << P[0] << " " << P[1] << " " << P[2] << "\n";
	}

	for (MyMesh::FIter FI = BeSelectModel.model.mesh.faces_begin(); FI != BeSelectModel.model.mesh.faces_end(); FI++)
	{
		std::vector<int> FVIndex;
		MyMesh::FHandle FH = BeSelectModel.model.mesh.face_handle(FI->idx());
		for (MyMesh::FVIter FVI = BeSelectModel.model.mesh.fv_begin(FH); FVI != BeSelectModel.model.mesh.fv_end(FH); FVI++)
		{
			FVIndex.push_back(FVI->idx() + 1);
		}
		ObjFile << ("f " + std::to_string(FVIndex[0]) + " " + std::to_string(FVIndex[1]) + " " + std::to_string(FVIndex[2]) + "\n");
	}

	ObjFile.close();

	Showwwwwwwwww = 1000;
	//model.AddSelectedFace(0);
}


//切換相機方向 需要確定 牆壁面方向 牆壁大小(中心點)
//Face_Diraction 單一 xyz 向量 向牆壁方向       Fce_Size  xyz 左上 xyz 右上 xyz 左下 xyz 右下  共四個xyz
void ChangeCameraLook(glm::vec3 Face_Diraction, std::vector<glm::vec3> Face_Size)
{
	//將面的上下左右加起來做平均，算中心點
	glm::vec3 Four(4);
	glm::vec3 Face_Mid = (Face_Size[0] + Face_Size[1] + Face_Size[2] + Face_Size[3]) / Four;

	cout << Face_Mid[0]<<" " << Face_Mid[1] << " " << Face_Mid[2] << endl;

	//切換相機方向
	glm::vec3 Campos(Face_Mid - Face_Diraction * Four + Face_Mid);
	vec3 up = vec3(0, 1, 0);
	glm::mat4 CamLook = lookAt(Campos, Face_Mid, up);
	CoutMat4(CamLook);
		
	//meshWindowCam.SetViewMatrix(CamLook);
	glm::mat4 IdM(1);
	meshWindowCam.SetModelratatioMatrix(CamLook);
	meshWindowCam.SetModelTranslateMatrix(IdM);
}


//對牆壁照深度圖 需要確定 牆壁面方向 牆壁大小(中心點)
//Face_Diraction 單一 xyz 向量 向牆壁方向       Fce_Size  xyz 左上 xyz 右上 xyz 左下 xyz 右下  共四個xyz
void NewDetectWall(glm::vec3 Face_Diraction, std::vector<glm::vec3> Face_Size)
{

	//切換相機位置與方向
	ChangeCameraLook(Face_Diraction,Face_Size);
	//拿到這個方向，用相機看過去得到的面與深度
#pragma region  Camera GetDepth Map
	//灰階深度圖
	unsigned char* data = new unsigned char[360000];
	//深度raw data
	float* Rawdata = new float[360000];
	//index彩色可視化
	unsigned char* Colordata = new unsigned char[360000 * 3];
	//原始index raycast 到的資料
	int* RawIdxdata = new int[360000];

	float MaxDepth = -1100000;
	float MinDepth = 1100000;
	//index 對應深度
	map<int, double> IdxDepth;
	//
	std::map<int, std::vector<double>> ALL_Idx_Depth;
	//for shader rendering
	glm::mat4 mvMat = meshWindowCam.GetViewMatrix() * meshWindowCam.GetModelMatrix();
	glm::mat4 pMat = meshWindowCam.GetProjectionMatrix(aspect);
	float depthValue = 0;
	//depthFile << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
	for (int i = 0; i < 600; i++)
	{
		for (int j = 0; j < 600; j++)
		{
			//depth map
			glReadPixels(i, j, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthValue);
			float D = pickingTexture.ReadTexture(i, j);
			//cout << D << " ";
			if (D != 0)
			{
				if (depthValue < MinDepth)
				{
					MinDepth = depthValue;
				}
				if (depthValue > MaxDepth)
				{
					MaxDepth = depthValue;
				}
			}

			Rawdata[(i)+(599 - j) * 600] = depthValue;
			RawIdxdata[(i)+(599 - j) * 600] = D;
			if (Idx.find(D) == Idx.end())
			{
				Idx[D] = 1;
				ALL_Idx_Depth[D].push_back(depthValue);
			}
			else
			{
				Idx[D] ++;
				ALL_Idx_Depth[D].push_back(depthValue);
			}
		}
		if (i % 60 == 0)
		{
			cout << i / 60 << "/10" << "\n";
		}
		double diff = MaxDepth - MinDepth;
		for (int i = 0; i < 360000; i++)
		{
			double tmp = Rawdata[i] - MinDepth;
			data[i] = (char)((tmp / diff) * 256.0);
			Colordata[i * 3 + 0] = (char)(colormap[RawIdxdata[i]][0] * 256.0);
			Colordata[i * 3 + 1] = (char)(colormap[RawIdxdata[i]][1] * 256.0);
			Colordata[i * 3 + 2] = (char)(colormap[RawIdxdata[i]][2] * 256.0);
		}
		//stbi_flip_vertically_on_write(true);
		stbi_write_png("Fileeee.png", 600, 600, 1, data, 0);
		stbi_write_png("Fileeee_Color.png", 600, 600, 3, Colordata, 0);
		stbi_image_free(data);
		stbi_image_free(Colordata);
		Idx.erase(0);
		ALL_Idx_Depth.erase(0);
	}
#pragma endregion

	//對於斜面 整個斜面分成同一類，儘管高度不同
	//再做Kmeans時，必須分在同一類

	//面需要推的方向 相機指向vector乘上深度
	//id 與 深度 與 面積 做判斷
	//



}



//each SideFace do camera depth thing
void Create_FaceCluster_BoundingBox()
{


}




#pragma region OneDimKMeans

//一維 K-means id 
//ALL_Idx_Depth		idx 與 那個idx深度對應(最後一個為平均深度)
//idx id 與重複次數
void data_2_K_means(std::map<int, std::vector<double>> ALL_Idx_Depth, std::map<int, int> idx, int ClusterNum)
{



}

//用kx 分群
std::vector<std::vector<double>> one_dim_K_means_cluster(std::vector<double> x, std::vector<double> kx, int seed)
{
	std::vector<std::vector<double>> team;
	for (int i = 0; i < seed; i++)
	{
		team.push_back(std::vector<double>());
	}
	//依照原始點對分點的距離 做最近點分群
	for (int i = 0; i < x.size(); i++)
	{
		double min_dis = 999999999;
		int smallKJ = 0;
		for (int j = 0; j < seed; j++)
		{
			 double dis =  abs(x[i] - kx[j]);
			if (dis < min_dis)
			{
				min_dis = dis;
				smallKJ = j;
			}
		}
		//找出最近k點
		team[smallKJ].push_back(i);
	}
	//回傳所有點分群結果
	return team;
}

std::vector<double> one_dim_K_means_re_seed(std::vector<std::vector<double>> Team, std::vector<double> kx, int seed)
{

}

//x 初始所有 點的值
//kx 初始分群點
//seed 分群數量
//fig 疊代次數
std::vector<std::vector<double>> one_dim_K_means(std::vector<double> x, std::vector<double> kx, int fig, int seed)
{
	std::vector<std::vector<double>> Team = one_dim_K_means_cluster(x, kx, seed);
	std::vector<double> nkx = one_dim_K_means_re_seed(Team, kx, seed);

	double Error = 0.01;
	int Done = true;
	for (int i = 0; i < seed; i++)
	{
		if (abs(nkx[i] - kx[i]) < Error)
		{
			Done = false;
		}
	}

	if (Done == false)
	{
		one_dim_K_means(x, nkx, fig += 1, seed);
	}
	else
	{
		return Team;
	}
}

#pragma endregion