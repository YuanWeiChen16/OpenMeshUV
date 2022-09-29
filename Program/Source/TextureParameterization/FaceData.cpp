#include"FaceData.h"

//計算距離
double FaceData::FaceDistance(FaceData A, FaceData B)
{
	if (A.PreDis.find(B.ID) != A.PreDis.end())
	{
		return A.PreDis[B.ID];
	}

	if ((A.ID == -1) || (B.ID == -1))
	{
		return 10000000000000000000;
	}

	if (A.Grapth[B.ID] > 5)
	{
		return 10000000000000000000;
	}




#define Test
#ifdef Test

	int TotalPointCount = A.pointcount * B.pointcount;
	//Depth 距離
	double DepthTerm = 0;
	//normal 一致
	int NormalDiffFlag = 0;
	if (glm::length(A.realNormal - B.realNormal) < 0.01)
	{
		double DepthDis = A.Depth[0] - B.Depth[0];
		DepthTerm = DepthDis * TotalPointCount;
	}
	else //normal 不一樣
	{
		NormalDiffFlag = 1;
	}

	double Ax = A.AvgUV[0];
	double Ay = A.AvgUV[1];

	double Bx = B.AvgUV[0];
	double By = B.AvgUV[1];

	double UVterm = sqrt((Ax - Bx) * (Ax - Bx) + (Ay - By) * (Ay - By));

	//UV 距離
//	//double UVterm = 0;
//#pragma omp parallel for
//	for (int i = 0; i < A.pointcount; i++)
//	{
//#pragma omp parallel for
//		for (int j = 0; j < B.pointcount; j++)
//		{
//			// UV Distance
//			double DisX = A.UV[i][0] - B.UV[j][0];
//			double DisY = A.UV[i][1] - B.UV[j][1];
//			double UVDistance = sqrt(DisX * DisX + DisY * DisY);
//			//Depth Distance
//			//UVterm += UVDistance;
//
//			if (NormalDiffFlag == 1)
//			{
//				double DepthDis = A.Depth[i] - B.Depth[j];
//				DepthTerm += DepthDis;
//			}
//		}
//	}

	//Noraml 距離 0~
	double NormalTerm = glm::length(A.realNormal - B.realNormal) * 100000;
#endif // Test
	double QEMError = A.QEM(B);
	QEMError = QEMError * 10000;


	//double REALDISTANCE = abs(DepthTerm) + abs(UVterm) + abs(NormalTerm) + abs(QEMError * 10000);
	//double REALDISTANCE = QEMError * QEMError * QEMError * QEMError;
	double REALDISTANCE = NormalTerm;
	//UV Normalize?
	// 
	//UVterm /= (double)(TotalPointCount);
	/*REALDISTANCE = 100000000;
	for (int i = 0; i < A.ConnectFace.size(); i++)
	{
		if (A.ConnectFace[i] == B.ID)
		{
			REALDISTANCE = 0.01;
		}
	}*/
	//真正距離Term
	return REALDISTANCE;
}

FaceData FaceData::FaceAdd(FaceData, FaceData)
{
	//重新取

	return FaceData();
}

FaceData FaceData::FaceAvg(std::vector<FaceData> FD)
{
	FaceData TempFaceData;

	TempFaceData.ID = -1000;

	TempFaceData.Depth = std::vector<double>();
	TempFaceData.UV = std::vector<glm::vec2>();
	TempFaceData.pointcount = 0;
	TempFaceData.realNormal = glm::vec3();

	//for (int i = 0; i < FD.size(); i++)
	//{
	//	TempFaceData.Depth.insert(TempFaceData.Depth.begin(), FD[i].Depth.begin(), FD[i].Depth.end());
	//	TempFaceData.UV.insert(TempFaceData.UV.begin(), FD[i].UV.begin(), FD[i].UV.end());
	//	TempFaceData.pointcount += FD.
	//}

	return FaceData();
}

bool FaceData::CheckConnect(FaceData FD)
{

	int FilterSize = 1;

	//bounding 計算，不在bounding的直接跳過
	//FD X太大	X太小
	if ((this->Max[0] + FilterSize) < (FD.Min[0] - FilterSize) || (this->Min[0] - FilterSize) > (FD.Max[0] + FilterSize))
	{
		return false;
	}
	//FD Y太大 Y太小
	if ((this->Max[1] + FilterSize) < (FD.Min[1] - FilterSize) || (this->Min[1] - FilterSize) > (FD.Max[1] + FilterSize))
	{
		return false;
	}
	for (int i = 0; i < this->ConnectFace.size(); i++)
	{
		if (this->ConnectFace[i] == FD.ID)
		{
			return true;
		}
	}

	int* RawIdxdata = new int[360000];

#pragma omp parallel for
	for (int i = 0; i < 360000; i++)
	{
		RawIdxdata[i] = 0;
	}

	//第一個面畫第一遍
	//#pragma omp parallel for
	for (int i = -FilterSize; i <= FilterSize; i++)
	{
		//#pragma omp parallel for
		for (int j = -FilterSize; j <= FilterSize; j++)
		{
			//#pragma omp parallel for
			for (int count = 0; count < this->pointcount - 1; count++)
			{
				glm::vec2 temp = this->UV[count];
				temp[0] += i;
				temp[1] += j;
				if (temp[0] > 599)
				{
					temp[0] = 599;
				}
				if (temp[1] > 599)
				{
					temp[1] = 599;
				}

				if (temp[0] < 0)
				{
					temp[0] = 0;
				}
				if (temp[1] < 0)
				{
					temp[1] = 0;
				}
				RawIdxdata[(int)(temp.x) + (int)(temp.y) * 600] = 1;
			}
		}
	}

	//#pragma omp parallel for
	for (int count = 0; count < FD.pointcount - 1; count++)
	{
		glm::vec2 temp = FD.UV[count];
		RawIdxdata[(int)(temp.x) + (int)(temp.y) * 600] += 1;
	}
	int Connect = 0;
	for (int i = 0; i < 360000; i++)
	{
		if (RawIdxdata[i] > 1)
		{
			Connect = 1;
			break;
		}
	}
	free(RawIdxdata);

	if (Connect == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

double FaceData::QEM(FaceData FD)
{
	Eigen::Matrix4d AQ = Eigen::Matrix4d::Zero();
	Eigen::Matrix4d BQ = Eigen::Matrix4d::Zero();
	glm::vec3 APoint = this->position;
	glm::vec3 ANormal = this->realNormal;

	glm::vec3 BPoint = FD.position;
	glm::vec3 BNormal = FD.realNormal;

	//取得平面方程式 A
	Eigen::Vector4d APlan(ANormal[0], ANormal[1], ANormal[2], 0);
	APlan[3] = ANormal[0] * -APoint[0] + ANormal[1] * -APoint[1] + ANormal[2] * -APoint[2];
	AQ = APlan * APlan.transpose();

	//平面方程式B
	Eigen::Vector4d BPlan(BNormal[0], BNormal[1], BNormal[2], 0);
	BPlan[3] = BNormal[0] * -BPoint[0] + BNormal[1] * -BPoint[1] + BNormal[2] * -BPoint[2];
	BQ = BPlan * BPlan.transpose();

	Eigen::Vector4d B(0, 0, 0, 1);
	Eigen::Matrix4d Qbar = AQ + BQ;
	Eigen::Matrix4d Qab = Qbar;
	Qab.row(3) = B;

	Eigen::Vector4d NewPoint = Qab.partialPivLu().solve(B);
	//Eigen::Vector4d NewPoint = ;

	//for (int i = 0; i < 4; i++)
	{
		//if (isnan(NewPoint[i]))
		{
			glm::vec3 temp = (APoint + BPoint);
			/*temp = (APoint - BPoint);
			temp[0] /= (double)(this->pointcount + FD.pointcount);
			temp[1] /= (double)(this->pointcount + FD.pointcount);
			temp[2] /= (double)(this->pointcount + FD.pointcount);
			temp *= this->pointcount;
			temp += BPoint;*/

			//A     B
			//

			//NewPoint = Eigen::Vector4d(temp[0],temp[1],temp[2], 1);
			NewPoint = Eigen::Vector4d(temp[0] / 2.0, temp[1] / 2.0, temp[2] / 2.0, 1.0);
		}
	}
	double QEMError = NewPoint.transpose() * (Qbar * NewPoint);


	double Mid2APlane = abs(APlan[0] * NewPoint[0] + APlan[1] * NewPoint[1] + APlan[2] * NewPoint[2] + APlan[3]);
	double Mid2BPlane = abs(BPlan[0] * NewPoint[0] + BPlan[1] * NewPoint[1] + BPlan[2] * NewPoint[2] + BPlan[3]);


	QEMError = Mid2APlane + Mid2BPlane;


	//std::cout << QEMError;

	return QEMError;
}
