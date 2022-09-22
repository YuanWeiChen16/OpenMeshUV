#pragma once
#include <Common.h>
#include <iostream>

#include <ViewManager.h>
#include <AntTweakBar/AntTweakBar.h>
#include <ResourcePath.h>
#include <fstream>
#include <string>
#include <map>

#include <omp.h>

#include <Eigen/Sparse>
#include <Eigen/Dense>

//同一面但是在2D投影上分成兩塊，在這裡也會分成兩塊
class FaceData {
public:
	int ID = -1;
	//raycast到的點的數量
	int pointcount = 0;
	//raycast到的點的深度
	std::vector<double> Depth;
	
	//點在圖上的UV位置，計算在圖上位置
	std::vector<glm::vec2> UV;	


	//與normal組成 平面方程式計算QEM
	//點為三角面的在2D投影的中心點
	glm::vec3 position;
	//平面normal
	glm::vec3 realNormal;

	//
	glm::vec2 AvgUV;

	//面與連接關系?
	//目前只記錄ID
	std::vector<int> ConnectFace;
	//與其他面在UV上的graph 距離
	std::map<int, int> Grapth;
	//2D UV bounding box
	//加速計算
	glm::vec2 Max;
	glm::vec2 Min;

	//已經算過的dis
	std::map<int, double> PreDis;



	//計算兩面的距離，需考慮多種情況
	double FaceDistance(FaceData, FaceData);
	// 
	//

	double QEM(FaceData);

	FaceData FaceAdd(FaceData, FaceData);
	FaceData FaceAvg(std::vector<FaceData>);

	bool CheckConnect(FaceData);
	


};