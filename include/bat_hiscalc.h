/*
*  desc : 电池历史数据分析服务类
*  time	: 2022-8-18
*  auth	: Meong
*/
#ifndef BAT_HISCALC_H
#define BAT_HISCALC_H

#include "bat_rtdbcalc.h"
#include "db_com/query_sample_cli.h"
#include "db_com/db_client.h"
#include "db_com/query_sample_cli_com.h"
#include "db_com/query_sample_cli.h"
#include "rtdb/rtdb_client_base.h"
#include <map>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::string;

class BatHisCalc : public BatRtCalc
{
public:
	BatHisCalc() {
		init();
	};
	virtual ~BatHisCalc() {};

protected:
	CQuerySampleCli m_sample_cli;
	DbClient m_dbClient;
	RTDB_ClientBase m_rtdbBase;
	SysRun m_sys_run;

	// tag pnt_ana.11020000001245.value
	int query_sample(map<string, vector<double>>&out, const vector<string> &tags, int64_t start_time, int64_t end_time, int time_pace = 15, int mode = 4);
	int query_sample(vector<vector<double>>&out, const vector<string> &tags, int64_t start_time, int64_t end_time, int time_pace = 15, int mode = 4);
	int query_sample(vector<TRHDBSampleResult> &out, const vector<string> &tags, int64_t start_time, int64_t end_time, int time_pace = 15, int mode = 4);

private:
	int init();
};


/*
*  desc : 电池历史运行状态评估
*  para : 
*  time	: 2022-8-22
*  auth	: Meong
*/
class BatHisCellEvaluate : public BatHisCalc
{
public:
	BatHisCellEvaluate();
	virtual ~BatHisCellEvaluate();

	int calc();

protected:
	int init();
	int create_his_rtdb();
	int create_taosdb();
	void clear();
	int read_data();
	int read_mea();
	int write_data();
	int write_taosdb();
	int find_sliding_window(const bms &b, const pcs &p, int64_t &start_time, int64_t &end_time);
	int64_t find_dev_pnt(int64_t dev_id, const char* postfix);
	void tag_sort_by_idx(vector<string> &old, map<string, int> &tag2idx, map<int, int>& seq2idx);
	int corelation_analysis(const bms &b, const int64_t &start_time, const int64_t &end_time, const int interval = 300);
	vector<double> data_eval(map<string, vector<double>> &pfx2val, map<string, double> &pfx2ratio);

	template<typename T>
	void set_calc_pear(T &t, const char* postfix, const double& v)
	{
		if (!postfix)
			return;

		if (strncmp(postfix, "pear.", 5) != 0)
			return;

		if (strcmp(postfix, "pear.v") == 0 || strcmp(postfix, "pear.V") == 0)
			t.pear_v = v;
		else if (strcmp(postfix, "pear.i") == 0 || strcmp(postfix, "pear.I") == 0)
			t.pear_i = v;
		else if (strcmp(postfix, "pear.t") == 0 || strcmp(postfix, "pear.T") == 0)
			t.pear_t = v;
		else if (strcmp(postfix, "pear.soc") == 0 || strcmp(postfix, "pear.SOC") == 0)
			t.pear_soc = v;
		else if (strcmp(postfix, "pear.soh") == 0 || strcmp(postfix, "pear.SOH") == 0)
			t.pear_soh = v;
	}

	RTDB<BAT_HIS_EVA_AGENT> m_bat_rtdb;

	vector<pcs> m_pcs;
	vector<bms> m_bms;
	vector<batcluster> m_batcluster;
	vector<batcell> m_batcell;

	map<int64_t, vector<pnt_ana>> map_dev2meas;

	map<int64_t, int>			 map_pcs2idx, map_bms2idx, map_cluster2idx, map_cell2idx;
	map<int64_t, vector<int>>	 map_pcs2bms, map_bms2cluster, map_cluster2cell;
	map<int64_t, int>			 map_bms2pcs, map_cluster2bms, map_cell2cluster;

	int m_pcs_idx, m_bms_idx;
	char t_ch[MAX_DATETIME_LEN];

	vector<bat_his_eva_bms> m_result_bms;
	vector<bat_his_eva_cluster> m_result_cluster;
	vector<bat_his_eva_cell> m_result_cell;
};

/*
*  desc : 电池历史统计与预警计算
*  para :
*  time	: 2023-6-13
*  auth	: Meong
*/
class BatHisStatisticCalc : public BatHisCalc
{
public:
	BatHisStatisticCalc();
	~BatHisStatisticCalc();

	virtual int calc();

private:
	void clear();
	int table_exists();
	int read_data();
	int read_mea();
	int read_db();
	int data_statistics(const bms &b, const int64_t &start_time, const int64_t &end_time);
	int merge_data(int64_t bms_id, map<int64_t, map<string, vector<int>>> &map_cluster);
	int insert_all(int64_t bms_id, int64_t cluster_id, int cell_count);
	int insert_one(int64_t bms_id, int64_t cluster_id, int cell_index);
	int calc_data(int64_t bms_id, map<int64_t, map<string, vector<int>>> &map_cluster);
	int write_data();

	vector<bms> m_bms;
	vector<batcluster> m_batcluster;
	vector<bat_his_statistics> m_dbdata, m_insertdata;
	vector<string> m_insert, m_update;

	map<int64_t, vector<pnt_ana>> map_dev2meas;

	map<int64_t, int>			 map_bms2idx, map_cluster2idx;
	map<int64_t, vector<int>>	 map_bms2cluster;
	map<int64_t, int>			 map_cluster2bms;

	map<int64_t, vector<int>>	 map_bms2his, map_cluster2his;

	char t_ch[MAX_DATETIME_LEN];
};

#endif