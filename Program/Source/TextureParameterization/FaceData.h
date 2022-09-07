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
class FaceData {
public:
	int ID = -1;
	//raycast到的點的數量
	int pointcount = 0;
	//raycast到的點的深度
	std::vector<double> Depth;
	
	//點在圖上的UV位置，計算在圖上位置
	std::vector<glm::vec2> UV;	

	glm::vec3 position;
	//平均normal
	glm::vec3 realNormal;
	//
	glm::vec2 AvgUV;

	//面與連接關系?
	//只記錄ID
	std::vector<int> ConnectFace;

	//計算兩面的距離，需考慮多種情況
	double FaceDistance(FaceData, FaceData);
	// 
	//
	FaceData FaceAdd(FaceData, FaceData);
	FaceData FaceAvg(std::vector<FaceData>);

	bool CheckConnect(FaceData);
};