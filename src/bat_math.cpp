#include "bat_math.h"
#include <numeric>
#include "Eigen/Dense"

using Eigen::MatrixXd;
using Eigen::RowVectorXd;

/// 变异系数 Meong 2023-5-29 ///
double BatMath::coefficient_variation(vector<double> &v)
{
	double m = mean(v);
	double std_dev = std_deviation(v);

	if (fabs(m) < DEFAULT_VAR_PERCISION)
		return 0;

	double w = std_dev / fabs(m);
	w = w > 1 ? 1 : w < 0 ? 0 : w;

	return w;
}

double BatMath::std_sigma(double v, double mean, double std_dev)
{
	if (fabs(mean) < DEFAULT_VAR_PERCISION)
		return 0;

	if (fabs(std_dev) < DEFAULT_VAR_PERCISION)
		return 0;

	return fabs(v - mean) / std_dev;
}

double BatMath::std_deviation(vector<double> &v)
{
	if (!v.size())
		return 0;

	if (v.size() == 1)
	{
		return 0;
	}

	double d = 0;
	double m = mean(v);

	for (int i = 0; i < v.size(); ++i)
	{
		d += (v[i] - m)*(v[i] - m);
	}

	/// 总体方差时用n，样本方差时用n-1 Meong 2023-5-29 ///
	//d /= (v.size() - 1);
	d /= (v.size());
	d = sqrt(d);

	if (d < DEFAULT_VAR_PERCISION)
		d = 0;

	return d;
}

double BatMath::mean(vector<double> &v)
{
	if (v.size() == 0)
		return 0;

	double sum = accumulate(v.begin(), v.end(), 0.0);
	return sum / v.size();
}

double BatMath::weight_average(vector<double>&v, vector<double>&w, double ratio/* = 1*/)
{
	if (v.size() == 0)
		return 0;

	if (v.size() != w.size())
		return 0;

	double w_sum = accumulate(w.begin(), w.end(), 0.0);
	if (w_sum != 1)
		normalized(w);

	double v_sum = accumulate(v.begin(), v.end(), 0.0);
	double weight_sum = 0;
	for (int i = 0; i < v.size(); ++i)
	{
		weight_sum += v[i] * w[i];
	}

	return weight_sum * ratio;
}

void BatMath::normalized(vector<double> &v, double ratio /*= 1*/)
{
	double sum = accumulate(v.begin(), v.end(), 0.0);
	for (int i = 0; i < v.size(); ++i)
	{
		if (sum == 0)
			v[i] = 0;
		else
			v[i] = ratio * v[i] / sum;
	}
}

int BatMath::pearson_corelation(const vector<vector<double>> &data_in, vector<double> &data_out)
{
	data_out.clear();
	if (data_in.size() == 0)
	{
		return -1;
	}
	if (data_in[0].size() == 0)
	{
		return -1;
	}

	int r = data_in[0].size();	//矩阵行数
	int c = data_in.size();		//矩阵列数

	MatrixXd mt = MatrixXd::Zero(r, c);
	for (int i = 0; i < r; ++i)
	{
		for (int j = 0; j < c; ++j)
		{
			mt(i, j) = data_in[j][i];
		}
	}

	/// 矩阵平均值向量 Meong 2023-5-23 ///
	MatrixXd meanVec = mt.colwise().mean();
	RowVectorXd meanVecRow(Eigen::RowVectorXd::Map(meanVec.data(), mt.cols()));
	MatrixXd zeroMeanMat = mt;
	/// 平均偏差 Meong 2023-5-23 ///
	zeroMeanMat.rowwise() -= meanVecRow;
	/// 转置矩阵 Meong 2023-5-23 ///
	MatrixXd conjMat = zeroMeanMat.adjoint();
	/// 标准差 Meong 2023-5-23 ///
	MatrixXd meanMatPow = zeroMeanMat.cwiseAbs2();
	MatrixXd meanMatSumVec = meanMatPow.colwise().sum() / (mt.rows() == 1 ? (mt.rows()) : (mt.rows() - 1));
	MatrixXd meanMatSqrt = meanMatSumVec.cwiseSqrt();
	MatrixXd std = meanMatSqrt.adjoint() * meanMatSqrt;
	/// 协方差 Meong 2023-5-23 ///
	MatrixXd covMat;
	if (mt.rows() == 1)
		covMat = (conjMat*zeroMeanMat) / double(mt.rows());
	else
		covMat = (conjMat*zeroMeanMat) / double(mt.rows() - 1);
	/// 相关系数矩阵 Meong 2023-5-23 ///
	MatrixXd pearson = MatrixXd::Ones(mt.cols(), mt.cols());
	for (int i = 0; i < covMat.rows(); ++i)
	{
		for (int j = 0; j < covMat.cols(); ++j)
		{
			if (std(i,j) == 0 || fabs(std(i,j)) < DEFAULT_VAR_PERCISION)
				continue;

			pearson(i, j) = covMat(i, j) / std(i, j);
		}
	}
	//MatrixXd pearson = covMat.array() / std.array();
	/// 平均相关系数 Meong 2023-5-23 ///
	MatrixXd meanPearson = pearson.colwise().mean();
	RowVectorXd meanPearsonRow(Eigen::RowVectorXd::Map(meanPearson.data(), meanPearson.cols()));

	for (int i = 0; i < meanPearsonRow.size(); ++i)
	{
		data_out.push_back(meanPearsonRow(i));
	}

	return 0;
}