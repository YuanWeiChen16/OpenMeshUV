#include"FaceData.h"

//計算距離
double FaceData::FaceDistance(FaceData A, FaceData B)
{
	int TotalPointCount = A.pointcount * B.pointcount;
	//Depth 距離
	double DepthTerm = 0;
	//normal 一致
	int NormalDiffFlag = 0;
	if (glm::length(A.realNormal - B.realNormal) < 0.0001)
	{
		double DepthDis = A.Depth[0] - B.Depth[0];
		DepthTerm = DepthDis * TotalPointCount;
	}
	else //normal 不一樣
	{
		NormalDiffFlag = 1;
	}

	double Ax = A.AvgUV[0] * A.pointcount;
	double Ay = A.AvgUV[1] * A.pointcount;

	double Bx = B.AvgUV[0] * B.pointcount;
	double By = B.AvgUV[1] * B.pointcount;

	double UVterm = sqrt((Ax - Bx) * (Ax - Bx) + (Ay - By) * (Ay - By));

	//UV 距離
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

	//Noraml 距離
	double NormalTerm = glm::length(A.realNormal - B.realNormal) * ((double)(TotalPointCount));

	//真正距離Term
	return abs(DepthTerm) + abs(UVterm) + abs(NormalTerm);

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
			for (int count = 0; count < this->pointcount-1; count++)
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
	for (int count = 0; count < FD.pointcount-1; count++)
	{
		glm::vec2 temp = FD.UV[count];
		RawIdxdata[(int)(temp.x) + (int)(temp.y) * 600] += 1;
	}
	int Connect = 0;
	for (int i = 0; i < 360000; i++)
	{
		if (RawIdxdata[i] > 1 )
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

