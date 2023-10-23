/*
*  desc : 电池常用计算类
*  time	: 2022-8-18
*  auth	: Meong
*/
#ifndef BAT_MATH_H
#define BAT_MATH_H

#include <vector>
#include <map>
#include <math.h>
#include <bat_config.h>

using std::vector;
using std::map;

class BatMath
{
public:
	/// 变异系数计算 Meong 2023-5-29 ///
	static double coefficient_variation(vector<double> &v);
	template<typename T>
	static double coefficient_variation(map<T, double> &v) {
		vector<double> vec;
		for (typename map<T, double>::iterator it = v.begin(); it != v.end(); ++it)
		{
			vec.push_back(it->second);
		}

		return coefficient_variation(vec);
	}

	/// 标准差率计算 Meong 2023-5-29 ///
	static double std_sigma(double v, double mean, double std_dev);

	/// 标准差计算 Meong 2023-5-29 ///
	static double std_deviation(vector<double> &v);
	template<typename T>
	static double std_deviation(map<T, double> &v) {
		vector<double> vec;
		for (typename map<T, double>::iterator it = v.begin(); it != v.end(); ++it)
		{
			vec.push_back(it->second);
		}

		return std_deviation(vec);
	}

	/// 均值计算 Meong 2023-5-29 ///
	static double mean(vector<double> &v);

	template<typename T>
	static double mean(map<T, double> &v) {
		vector<double> vec;
		for (typename map<T, double>::iterator it = v.begin(); it != v.end(); ++it)
		{
			vec.push_back(it->second);
		}

		return mean(vec);
	}

	/// 权重归一化计算 Meong 2023-5-29 ///
	static double weight_average(vector<double>& v, vector<double>&w, double ratio = 1);

	/// 归一化计算 Meong 2023-5-29 ///
	static void normalized(vector<double> &v, double ratio = 1);

	template <typename T>
	static void normalized(map<T, double> &v, double ratio = 1) {
		vector<double> vec;
		for (typename map<T, double>::iterator it = v.begin(); it != v.end(); ++it)
		{
			vec.push_back(it->second);
		}

		normalized(vec, ratio);
		int i = 0;
		for (typename map<T, double>::iterator it = v.begin(); it != v.end(); ++it, ++i)
		{
			it->second = vec[i];
		}
	}

	/// 皮尔逊相关系数计算 Meong 2023-5-29 ///
	static int pearson_corelation(const vector<vector<double>> &data_in, vector<double> &data_out);
};

#endif