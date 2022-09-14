#include"FaceData.h"

//�p��Z��
double FaceData::FaceDistance(FaceData A, FaceData B)
{
	int TotalPointCount = A.pointcount * B.pointcount;
	//Depth �Z��
	double DepthTerm = 0;
	//normal �@�P
	int NormalDiffFlag = 0;
	if (glm::length(A.realNormal - B.realNormal) < 0.0001)
	{
		double DepthDis = A.Depth[0] - B.Depth[0];
		DepthTerm = DepthDis * TotalPointCount;
	}
	else //normal ���@��
	{
		NormalDiffFlag = 1;
	}

	double Ax = A.AvgUV[0] * A.pointcount;
	double Ay = A.AvgUV[1] * A.pointcount;

	double Bx = B.AvgUV[0] * B.pointcount;
	double By = B.AvgUV[1] * B.pointcount;

	double UVterm = sqrt((Ax - Bx) * (Ax - Bx) + (Ay - By) * (Ay - By));

	//UV �Z��
	//double UVterm = 0;
#pragma omp parallel for
	for (int i = 0; i < A.pointcount; i++)
	{
#pragma omp parallel for
		for (int j = 0; j < B.pointcount; j++)
		{
			// UV Distance
			double DisX = A.UV[i][0] - B.UV[j][0];
			double DisY = A.UV[i][1] - B.UV[j][1];
			double UVDistance = sqrt(DisX * DisX + DisY * DisY);
			//Depth Distance
			//UVterm += UVDistance;

			if (NormalDiffFlag == 1)
			{
				double DepthDis = A.Depth[i] - B.Depth[j];
				DepthTerm += DepthDis;
			}
		}
	}

	//UV Normalize?
	//UVterm /= (double)(TotalPointCount);

	//Noraml �Z��
	double NormalTerm = glm::length(A.realNormal - B.realNormal) * ((double)(TotalPointCount));

	if (A.CheckConnect(B))
	{
		double QEMError = A.QEM(B);
	}
	//�u���Z��Term
	return abs(DepthTerm) + abs(UVterm) + abs(NormalTerm);
}



FaceData FaceData::FaceAdd(FaceData, FaceData)
{
	//���s��

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

	//bounding �p��A���bbounding���������L
	//FD X�Ӥj	X�Ӥp
	if ((this->Max[0] + FilterSize) < (FD.Min[0] - FilterSize) || (this->Min[0] - FilterSize) > (FD.Max[0] + FilterSize))
	{
		return false;
	}
	//FD Y�Ӥj Y�Ӥp
	if ((this->Max[1] + FilterSize) < (FD.Min[1] - FilterSize) || (this->Min[1] - FilterSize) > (FD.Max[1] + FilterSize))
	{
		return false;
	}


	int* RawIdxdata = new int[360000];

#pragma omp parallel for
	for (int i = 0; i < 360000; i++)
	{
		RawIdxdata[i] = 0;
	}

	//�Ĥ@�ӭ��e�Ĥ@�M
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

	//���o������{�� A
	Eigen::Vector4d APlan(ANormal[0], ANormal[1], ANormal[2], 0);
	APlan[4] = ANormal[0] * -APoint[0] + ANormal[1] * -APoint[1] + ANormal[2] * -APoint[2];
	AQ = APlan * APlan.transpose();

	//������{��B
	Eigen::Vector4d BPlan(BNormal[0], BNormal[1], BNormal[2], 0);
	BPlan[4] = BNormal[0] * -BPoint[0] + BNormal[1] * -BPoint[1] + BNormal[2] * -BPoint[2];
	BQ = BPlan * BPlan.transpose();
	
	Eigen::Vector4d B(0, 0, 0, 1);
	Eigen::Matrix4d Qbar = AQ + BQ;
	Eigen::Matrix4d Qab = Qbar;
	Qab.row(3) = B;
	Eigen::Vector4d NewPoint = Qab.partialPivLu().solve(B);

	double QEMError = NewPoint.transpose() * (Qbar * NewPoint);
	return QEMError;
}
