#pragma once
#include <Common.h>
#include <iostream>

#include <ViewManager.h>
#include <AntTweakBar/AntTweakBar.h>
#include <ResourcePath.h>
#include <fstream>
#include <string>
#include <map>

#include <Eigen/Sparse>
class FaceData {
public:
	int ID = -1;
	//raycast到的點的數量
	int pointcount = 0;
	//raycast到的點的深度
	std::vector<double> Depth;
	//raycast到的點的Normal
	std::vector<glm::vec3> Normal;
	//平均normal
	glm::vec3 realNormal;


	double FaceData::operator-(FaceData& FaceData);
	FaceData FaceData::operator+(FaceData& FaceData);
};