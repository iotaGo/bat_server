/*
*  desc : BAT配置类
*  time	: 2022-3-16
*  auth	: Meong
*/
#ifndef BAT_CONFIG_H
#define BAT_CONFIG_H

#include <string>
#include <map>

using std::string;
using std::map;

#define DAFAULT_CALC_INTERVAL 15
#define DEFAULT_VAR_RATIO 1
#define DEFAULT_VAR_PERCISION 0.00001
#define MAX_DATETIME_LEN 32

class BatConfig
{
public:
	~BatConfig();

	static BatConfig& get() {
		static BatConfig config;
		return config;
	}

	/// 是否开始计算 Meong 2023-6-5 ///
	bool time_to_calc(const string& key);

	/// 获取计算间隔时间 Meong 2022-8-25 ///
	int get_interval(const string &key);
	/// 获取计算间隔时间单位 Meong 2023-6-5 ///
	string get_time_unit(const string &key);

	/// 获取计算各量测比例因子 Meong 2022-8-25 ///
	double get_ratio(const string& key);

	/// 获取综合得分值 Meong 2023-7-6 ///
	double get_score(const string& key);

	/// 获取变异系数值 Meong 2023-7-6 ///
	double get_variation(const string& key);

	/// 获取相关系数值 Meong 2023-7-7 ///
	double get_corelation(const string& key);

	/// 获取taos使能 Meong 2023-5-31 ///
	bool get_taos_enable() const;
	/// 获取taos库地址或hostname Meong 2023-5-29 ///
	string get_taos_url() const;
	/// 获取taos库端口 Meong 2023-5-29 ///
	int get_taos_port() const;
	/// 获取taos用户名 Meong 2023-5-29 ///
	string get_taos_user() const;
	/// 获取taos用户密码 Meong 2023-5-29 ///
	string get_taos_passwd() const;
	/// 获取taos连接库 Meong 2023-5-29 ///
	string get_taos_dbname() const;

private:
	BatConfig();
	BatConfig(const BatConfig& c) {};
	int init();

	map<string, int> m_interval;
	map<string, string> m_unit;
	map<string, double> m_ratio;
	map<string, double> m_variation;
	map<string, double> m_corelation;
	map<string, double> m_score;

	int taos_enable;
	string taos_ip_host;
	int taos_port;
	string taos_user;
	string taos_passwd;
	string taos_db;
};

#endif 