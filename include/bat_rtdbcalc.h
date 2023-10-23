/*
*  desc : 电池实时数据分析服务类
*  time	: 2022-8-18
*  auth	: Meong
*/
#ifndef BAT_RTDBCALC_H
#define BAT_RTDBCALC_H

#include <vector>
#include <map>
#include "bat_calc.h"
#include "bat_rtdbdef.h"
#include "scada_api.h"
#include "stringutils.h"

using std::vector;
using std::map;

/*
*  desc : Bat实时计算基类
*  para : 
*  time	: 2022-8-22
*  auth	: Meong
*/
class BatRtCalc : public BatCalc
{
public:
	BatRtCalc() {
		init();
	};

	virtual ~BatRtCalc() {};

protected:
	int read_mea(vector<pnt_ana> &pnts, int64_t container, const char* postfix = "");
	int read_mea(vector<pnt_dig> &pnts, int64_t container, const char* postfix = "");
	int read_mea(vector<pnt_mix> &pnts, int64_t container, const char* postfix = "");

	type_vt get_vbase(int id);
	int64_t read_tpl_dev(int64_t container_id, const char *tpl_name);

	template<typename S, typename T>
	int write_rdbdata(RTDB<S> &rtdb, const vector<T>& vec, bool clear = false, bool overwrite = false) {
		if (!vec.size())
			return 0;

		const char* table_name = rtdb.GetBindTabName(&(vec[0]));
		int ret_code = 0;
		if (clear) {
			rtdb.Delete(table_name, true);
		}

		ClassArchive class_archive;
		class_archive.Archive<T>();
		std::vector<std::string > var_name_list;
		class_archive.GetVarNameList(var_name_list);
		std::string cols = StringUtils::join(var_name_list, ',');

		for (int i = 0; i < vec.size(); ++i)
		{
			ret_code = rtdb.Add(vec[i], overwrite);
			if (ret_code < 0) {
				log_error("Write rdb data error, table: %s, id: %lld, ret_code: %d ", table_name, vec[i].id, ret_code);
				continue;
			}
		}
		log_info("Write rdb table: %s, data count: %d", table_name, vec.size());

		return 0;
	}

	template<typename T> 
	void creat_index(const vector<T> &vec, map<int64_t, int> &map_idx) {
		for (int i = 0; i < vec.size(); ++i) {
			map_idx[vec[i].id] = i;
		}
	}

	template<typename T>
	void creat_pnt_index(const vector<T> &vec, map<string, int> &map_idx) {
		for (int i = 0; i < vec.size(); ++i) {
			if (strcmp(vec[i].postfix, "") == 0)
				continue;

			map_idx[vec[i].postfix] = i;
		}
	}

	/// 动态序列对齐算法 Meong 2023-6-2 ///
	void dynamic_seq_warping(map<string, map<int, double>> &pfx2idxval);
	void dynamic_seq_warping(map<string, vector<double>> &pfx2val);

	RTDB<PHY_MODEL_AGENT> m_public_rtdb;
	COptComm m_opt_comm;

private:
	int init();

	template<typename T, typename C>
	int read_mea(vector<T> &pnts, const C& cond)
	{
		int ret_code = m_public_rtdb.Get(cond, pnts);
		if (ret_code < 0)
		{
			log_warn("BatRtCalc, read pnt error");
			return ret_code;
		}

		return pnts.size();
	}
};

/*
*  desc : 实时SOH统计计算
*  para : 
*  time	: 2022-8-22
*  auth	: Meong
*/
class BatRtSohCalc : public BatRtCalc
{
public:
	BatRtSohCalc();
	~BatRtSohCalc();

	int calc();

private:
	void clear();
	int read_data();
	int create_index();
	int read_st_mea();
	int read_mea();
	int read_mea(map<int64_t, pnt_ana> &pnts, int64_t container, int64_t real_container = -1, const char* postfix = "");
	int write_mea(const map<int64_t, double> &v);

	vector<station_bess> m_station;
	vector<bms> m_bms;
	vector<batcluster> m_batcluster;
	vector<batcell> m_batcell;

	map<int64_t, pnt_ana>		map_st2soh;
	map<int64_t, pnt_ana>		map_dev2soh;

	map<int64_t, int>			map_st2idx, map_bms2idx, map_cluster2idx, map_cell2idx;
	map<int64_t, vector<int>>	map_st2bms, map_bms2cluster, map_cluster2cell;
	map<int64_t, int>			map_bms2st, map_cluster2bms, map_cell2cluster;

};

/*
*  desc : 实时电芯状态评价分析
*  para :
*  time	: 2022-8-22
*  auth	: Meong
*/
class BatRtCellEvaluate : public BatRtCalc
{
public:
	BatRtCellEvaluate();
	~BatRtCellEvaluate();

	int calc();

private:
	int init();
	int create_bat_rtdb();
	int create_taosdb();
	void clear();
	int read_data();
	int read_mea();
	vector<double> data_eval(map<string, map<int, double>> &pfx2idxval, map<string, double> &pfx2ratio);
	int write_data();
	int write_taosdb();

	double get_map_value(const map<int, double> &m, int idx, double default_value = 0)
	{
		double v = default_value;
		map<int, double>::const_iterator it = m.cbegin();
		int i = 0;
		while (it != m.cend())
		{
			if (idx == i++)
				return it->second;

			++it;
		}
		return v;
	}

	int get_map_key(const map<int, double> &m, int idx, int default_value = 0)
	{
		int k = default_value;
		map<int, double>::const_iterator it = m.cbegin();
		int i = 0;
		while (it != m.cend())
		{
			if (idx == i++)
				return it->first;

			++it;
		}
		return k;
	}

	template<typename T>
	void set_mea_val(T &t, const char* postfix, const double& v)
	{
		if (!postfix)
			return;

		if (strcmp(postfix, "v") == 0 || strcmp(postfix, "V") == 0)
			t.v = v;
		else if (strcmp(postfix, "i") == 0 || strcmp(postfix, "I") == 0)
			t.i = v;
		else if (strcmp(postfix, "t") == 0 || strcmp(postfix, "T") == 0)
			t.t = v;
		else if (strcmp(postfix, "soc") == 0 || strcmp(postfix, "SOC") == 0)
			t.soc = v;
		else if (strcmp(postfix, "soh") == 0 || strcmp(postfix, "SOH") == 0)
			t.soh = v;
	}

	template<typename T>
	void set_calc_var(T &t, const char* postfix, const double& v)
	{
		if (!postfix)
			return;

		if (strncmp(postfix, "var.", 4) != 0)
			return;

		if (strcmp(postfix, "var.v") == 0 || strcmp(postfix, "var.V") == 0)
			t.var_v = v;
		else if (strcmp(postfix, "var.i") == 0 || strcmp(postfix, "var.I") == 0)
			t.var_i = v;
		else if (strcmp(postfix, "var.t") == 0 || strcmp(postfix, "var.T") == 0)
			t.var_t = v;
		else if (strcmp(postfix, "var.soc") == 0 || strcmp(postfix, "var.SOC") == 0)
			t.var_soc = v;
		else if (strcmp(postfix, "var.soh") == 0 || strcmp(postfix, "var.SOH") == 0)
			t.var_soh = v;
	}

	RTDB<BAT_EVA_AGENT> m_bat_rtdb;

	vector<bms> m_bms;
	vector<batcluster> m_batcluster;
	vector<batcell> m_batcell;

	map<int64_t, vector<pnt_ana>> map_dev2meas;
	map<int64_t, map<string, vector<double>>> map_dev2meascore;
	map<int64_t, vector<double>> map_dev2score;

	map<int64_t, int>			 map_bms2idx, map_cluster2idx, map_cell2idx;
	map<int64_t, vector<int>>	 map_bms2cluster, map_cluster2cell;
	map<int64_t, int>			 map_cluster2bms, map_cell2cluster;

	vector<bat_eva_bms> m_result_bms;
	vector<bat_eva_cluster> m_result_cluster;
	vector<bat_eva_cell> m_result_cell;
};

/*
*  desc : 虚拟电厂等效参数计算
*  para :
*  time	: 2023-6-13
*  auth	: Meong
*/
class BatRtVpp : public BatRtCalc
{
public:
	BatRtVpp();
	~BatRtVpp();

	int calc();

private:
	int init();
	int create_vpp_rtdb();
	int read_data();
	void clear();

	int calc_station();
	int calc_generator();

	int write_data();

	RTDB<VPP_AGENT>				m_vpp_rtdb;

	vector<station_bess>		m_station;
	vector<pqvc_connectpoint>	m_pqvc;

	vector<agvc_station_bess>	m_agvc_station;
	vector<agvc_pqvc_connectpoint> m_agvc_pqvc;

	vector<vpp_station>			m_result_st;
	vector<vpp_generator>		m_result_gen;

	map<int64_t, int>			map_stidx, map_pqvcidx, map_agvcstidx, map_agvcpqvcidx;
};

#endif