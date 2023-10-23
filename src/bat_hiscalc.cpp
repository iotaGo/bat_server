#include "bat_hiscalc.h"
#include "sys/eproc.h"
#include "eco/estring.h"
#include "bat_math.h"
#include "bat_taos.h"
#include "bat_prewarning.h"
#include <numeric>

int BatHisCalc::init()
{
	log_info("BatHisCalc, init");

	int ret_code = 0;

	// dbclient init
	ret_code = m_dbClient.ConnectDuty();
	if (ret_code < 0)
	{
		log_error("BatHisCalc, dbclient init error! ret_code = %d", ret_code);
		return ret_code;
	}
	
	// CQuerySampleCli init
	ret_code = m_sample_cli.InitQuerySampleCli("bat_server", NULL, &m_dbClient, &m_rtdbBase);
	if (ret_code < 0)
	{
		log_error("BatHisCalc, CQuerySampleCli init error! ret_code = %d", ret_code);
		return ret_code;
	}

	// sys run init
	ret_code = m_sys_run.Init();
	if (ret_code < 0)
	{
		log_error("BatHisCalc, sysrun init error! ret_code = %d", ret_code);
		return ret_code;
	}

	return 0;
}

int BatHisCalc::query_sample(map<string, vector<double>>&out, const vector<string> &tags, int64_t start_time, int64_t end_time, int time_pace /*= 15*/, int mode/* = 4*/)
{
	std::vector<TSampleInfoNew> in_para_rhdb; //GetRHDBSampleResult in para
	std::vector<TRHDBSampleResult> out_para_rhdb; //GetRHDBSampleResult out para

	for (int i = 0; i < tags.size(); ++i)
	{
		TSampleInfoNew info;
		info.start_time = start_time;
		info.end_time = end_time;
		info.time_pace = time_pace;
		strncpy(info.key_id, tags[i].c_str(), sizeof(info.key_id));
		info.sample_mode = mode;
		info.app_id = m_sys_run.GetAppID("scada");
		in_para_rhdb.push_back(info);
	}
	
	int ret_code = m_sample_cli.GetRHDBSampleResult(in_para_rhdb, out_para_rhdb);
	if (ret_code < 0) // ?? out_para.size() == in_para.size()
	{
		log_error("BatHisCalc, query sample value error, ret_code=%d", ret_code);
		return ret_code;
	}

	for (int i = 0; i < out_para_rhdb.size(); i++)
	{
		const TRHDBSampleResult& ret = out_para_rhdb[i];
		if (ret.success_flag < 0)
			continue;

		string key = ret.key_id;
		vector<double> &vec = out[key];

		const VecTRHDBSampleValue& vs_value = ret.vec_value;
		for (int k = 0; k < vs_value.size(); k++)
		{
			double value = vs_value[k].value;
			vec.push_back(value);
		}
	}

	return 0;
}

int BatHisCalc::query_sample(vector<vector<double>> &out, const vector<string> &tags, int64_t start_time, int64_t end_time, int time_pace /* = 15 */, int mode /* = 4 */)
{
	std::vector<TSampleInfoNew> in_para_rhdb; //GetRHDBSampleResult in para
	std::vector<TRHDBSampleResult> out_para_rhdb; //GetRHDBSampleResult out para

	for (int i = 0; i < tags.size(); ++i)
	{
		TSampleInfoNew info;
		info.start_time = start_time;
		info.end_time = end_time;
		info.time_pace = time_pace;
		strncpy(info.key_id, tags[i].c_str(), sizeof(info.key_id));
		info.sample_mode = mode;
		info.app_id = m_sys_run.GetAppID("scada");
		in_para_rhdb.push_back(info);
	}

	int ret_code = m_sample_cli.GetRHDBSampleResult(in_para_rhdb, out_para_rhdb);
	if (ret_code < 0) // ?? out_para.size() == in_para.size()
	{
		log_error("BatHisCalc, query sample value error, ret_code=%d", ret_code);
		return ret_code;
	}
	
	map<string, int> idx;
	for (int i = 0; i < out_para_rhdb.size(); i++)
	{
		const TRHDBSampleResult& ret = out_para_rhdb[i];
		if (ret.success_flag < 0)
			continue;

		string key = ret.key_id;
		idx[key] = i;
	}

	out.clear();
	out.resize(tags.size());
	for (int i = 0; i < tags.size(); ++i)
	{
		if (idx.count(tags[i]) == 0)
			continue;

		int j = idx[tags[i]];
		const TRHDBSampleResult& ret = out_para_rhdb[j];

		VecTRHDBSampleValue vs_value = ret.vec_value;
		out[i].clear();
		out[i].resize(vs_value.size());
		for (int k = 0; k < vs_value.size(); k++)
		{
			double value = vs_value[k].value;
			out[i][k] = value;
		}
	}

	return 0;
}

int BatHisCalc::query_sample(vector<TRHDBSampleResult> &out, const vector<string> &tags, int64_t start_time, int64_t end_time, int time_pace /* = 15 */, int mode /* = 4 */)
{
	std::vector<TSampleInfoNew> in_para_rhdb; //GetRHDBSampleResult in para

	for (int i = 0; i < tags.size(); ++i)
	{
		TSampleInfoNew info;
		info.start_time = start_time;
		info.end_time = end_time;
		info.time_pace = time_pace;
		strncpy(info.key_id, tags[i].c_str(), sizeof(info.key_id));
		info.sample_mode = mode;
		info.app_id = m_sys_run.GetAppID("scada");
		in_para_rhdb.push_back(info);
	}

	int ret_code = m_sample_cli.GetRHDBSampleResult(in_para_rhdb, out);
	if (ret_code < 0) // ?? out_para.size() == in_para.size()
	{
		log_error("BatHisCalc, query sample value error, ret_code=%d", ret_code);
		return ret_code;
	}

	return 0;
}

BatHisCellEvaluate::BatHisCellEvaluate()
{
	clear();
}

BatHisCellEvaluate::~BatHisCellEvaluate()
{
}

void BatHisCellEvaluate::clear()
{
	m_pcs.clear();
	m_bms.clear();
	m_batcluster.clear();
	m_batcell.clear();

	map_dev2meas.clear();

	map_pcs2idx.clear(), map_bms2idx.clear(), map_cluster2idx.clear(), map_cell2idx.clear();
	map_pcs2bms.clear(), map_bms2cluster.clear(), map_cluster2cell.clear();
	map_bms2pcs.clear() ,map_cluster2bms.clear(), map_cell2cluster.clear();

	m_pcs_idx = -1, m_bms_idx = -1;

	m_result_bms.clear(), m_result_cluster.clear(), m_result_cell.clear();
}

int BatHisCellEvaluate::init()
{
	int ret_code = 0;

	// bat his rtdb init
	ret_code = m_bat_rtdb.Init();
	if (ret_code < 0)
	{
		log_error("BatHisCellEvaluate, rtdb init error! ret_code = %d", ret_code);
		return ret_code;
	}

	// bat his rtdb open
	std::string ctx_app(EProc::GetCtxApp());
	ret_code = m_bat_rtdb.Open(ctx_app.c_str());
	if (ret_code < 0)
	{
		log_error("BatHisCellEvaluate, rtdb open error! ret_code = %d", ret_code);
		return ret_code;
	}

	// create bat his rtdb 
	ret_code = create_his_rtdb();
	if (ret_code < 0)
	{
		log_error("BatHisCellEvaluate, create bat rtdb error! ret_code = %d", ret_code);
		return ret_code;
	}

	ret_code = create_taosdb();
	if (ret_code < 0)
	{
		log_error("BatHisCellEvaluate, create taosdb error! ret_code = %d", ret_code);
		return ret_code;
	}

	return 0;
}

int BatHisCellEvaluate::create_his_rtdb()
{
	int ret_code = 0;

	if (!m_bat_rtdb.IsTableExist("bat_his_eva_bms"))
	{
		ret_code = m_bat_rtdb.CreateTableByStruct<bat_his_eva_bms>();
		if (ret_code < 0) {
			log_error("BatHisCellEvaluate, create rtdb table bat_eva_bms error! ret_code = %d", ret_code);
			return ret_code;
		}
	}

	if (!m_bat_rtdb.IsTableExist("bat_his_eva_cluster"))
	{
		ret_code = m_bat_rtdb.CreateTableByStruct<bat_his_eva_cluster>();
		if (ret_code < 0) {
			log_error("BatHisCellEvaluate, create rtdb table bat_eva_cluster error! ret_code = %d", ret_code);
			return ret_code;
		}
	}

	if (!m_bat_rtdb.IsTableExist("bat_his_eva_cell"))
	{
		ret_code = m_bat_rtdb.CreateTableByStruct<bat_his_eva_cell>();
		if (ret_code < 0) {
			log_error("BatHisCellEvaluate, create rtdb table bat_eva_cell error! ret_code = %d", ret_code);
			return ret_code;
		}
	}

	return 0;
}

int BatHisCellEvaluate::create_taosdb()
{
	if (!BatConfig::get().get_taos_enable())
		return 0;

	string db_name = BatConfig::get().get_taos_dbname();
	if (db_name.empty())
		db_name = "bat_analysis";

	BatTaos &taos = BatTaos::get();
	const int sql_len = 512;
	char sql[sql_len];

	// create database
	/*if (taos.modify("CREATE database IF NOT EXISTS bat_analysis;") < 0)
	{
	log_error("BatHisCellEvaluate, create bat_analysis db error");
	return -1;
	}*/

	// use database
	memset(sql, 0, sql_len);
	eco::nsprintf(sql, sql_len, "USE %s;", db_name.c_str());
	if (taos.modify(sql) < 0)
	{
		log_error("BatHisCellEvaluate, use %s db error", db_name.c_str());
		return -1;
	}

	// create stable his_eva_bms
	memset(sql, 0, sql_len);
	eco::nsprintf(sql, sql_len, "CREATE STABLE IF NOT EXISTS %s.his_eva_bms \
								(ts TIMESTAMP, start_ts TIMESTAMP, pear_v DOUBLE, pear_i DOUBLE, \
								pear_t DOUBLE, pear_soc DOUBLE, pear_soh DOUBLE, score DOUBLE) \
								tags (id BIGINT, name BINARY(100), container_id BIGINT, container_tablename BINARY(32));", 
								db_name.c_str());
	if (taos.modify(sql) < 0)
	{
		log_error("BatHisCellEvaluate, create stable %s.his_eva_bms error", db_name.c_str());
		return -1;
	}

	// create stable his_eva_cluster
	memset(sql, 0, sql_len);
	eco::nsprintf(sql, sql_len, "CREATE STABLE IF NOT EXISTS %s.his_eva_cluster \
								(ts TIMESTAMP, start_ts TIMESTAMP, pear_v DOUBLE, pear_i DOUBLE, \
								pear_t DOUBLE, pear_soc DOUBLE, pear_soh DOUBLE, score DOUBLE) \
								tags (id BIGINT, name BINARY(100), container_id BIGINT, container_tablename BINARY(32));",
								db_name.c_str());

	if (taos.modify(sql) < 0)
	{
		log_error("BatHisCellEvaluate, create stable %s.his_eva_cluster error", db_name.c_str());
		return -1;
	}

	// create stable his_eva_cell
	memset(sql, 0, sql_len);
	eco::nsprintf(sql, sql_len, "CREATE STABLE IF NOT EXISTS %s.his_eva_cell \
								(ts TIMESTAMP, start_ts TIMESTAMP, pear_v DOUBLE, pear_i DOUBLE, \
								pear_t DOUBLE, pear_soc DOUBLE, pear_soh DOUBLE, score DOUBLE) \
								tags (id BIGINT, name BINARY(100), container_id BIGINT, container_tablename BINARY(32));", 
								db_name.c_str());
	if (taos.modify(sql) < 0)
	{
		log_error("BatHisCellEvaluate, create stable %s.his_eva_cell error", db_name.c_str());
		return -1;
	}

	return 0;
}

int BatHisCellEvaluate::read_data()
{
	log_info("BatHisCellEvaluate, read data");

	int ret_code = 0;
	/// 读所有PCS Meong 2023-5-22 ///
	ret_code = m_public_rtdb.Get(m_pcs);
	if (ret_code < 0)
	{
		log_error("BatHisCellEvaluate, read pcs error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 读所有BMS Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_bms);
	if (ret_code < 0)
	{
		log_error("BatHisCellEvaluate, read bms error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 读所有电池簇 Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_batcluster);
	if (ret_code < 0)
	{
		log_error("BatHisCellEvaluate, read batcluster error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 读所有电芯 Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_batcell);
	if (ret_code < 0)
	{
		log_error("BatHisCellEvaluate, read batcell error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 建立索引关系 Meong 2022-8-25 ///
	for (int i = 0; i < m_pcs.size(); ++i)
	{
		pcs &obj = m_pcs[i];
		map_pcs2idx[obj.id] = i;
	}

	for (int i = 0; i < m_bms.size(); ++i)
	{
		bms &obj = m_bms[i];
		map_bms2idx[obj.id] = i;

		if (map_pcs2idx.count(obj.relation_id) == 0)
		{
			log_warn("bms relation_id is invalid, id = %lld, relation_id = %lld", obj.id, obj.relation_id);
			continue;
		}

		map_bms2pcs[obj.id] = map_pcs2idx[obj.relation_id];
		map_pcs2bms[obj.relation_id].push_back(i);
	}

	for (int i = 0; i < m_batcluster.size(); ++i)
	{
		batcluster &obj = m_batcluster[i];
		map_cluster2idx[obj.id] = i;

		if (map_bms2idx.count(obj.relation_id) == 0)
		{
			log_warn("batcluster relation_id is invalid, id = %lld, relation_id = %lld", obj.id, obj.relation_id);
			continue;
		}

		map_cluster2bms[obj.id] = map_bms2idx[obj.relation_id];
		map_bms2cluster[obj.relation_id].push_back(i);
	}

	for (int i = 0; i < m_batcell.size(); ++i)
	{
		batcell &obj = m_batcell[i];
		map_cell2idx[obj.id] = i;

		if (map_cluster2idx.count(obj.relation_id) == 0)
		{
			log_warn("batcell relation_id is invalid, id = %lld, relation_id = %lld", obj.id, obj.relation_id);
			continue;
		}

		map_cell2cluster[obj.id] = map_cluster2idx[obj.relation_id];
		map_cluster2cell[obj.relation_id].push_back(i);
	}

	/// 读量测 Meong 2022-8-22 ///
	read_mea();

	return 0;
}

int BatHisCellEvaluate::read_mea()
{
	vector<pnt_ana> pnts;

	for (int i = 0; i < m_pcs.size(); ++i)
	{
		pnts.clear();
		BatRtCalc::read_mea(pnts, m_pcs[i].id, "p");
		for (int j = 0; j < pnts.size(); ++j)
		{
			if (strcmp(pnts[j].postfix, "") == 0)
				continue;
			map_dev2meas[m_pcs[i].id].push_back(pnts[j]);
		}
	}

	for (int i = 0; i < m_bms.size(); ++i)
	{
		pnts.clear();
		BatRtCalc::read_mea(pnts, m_bms[i].id);
		for (int j = 0; j < pnts.size(); ++j)
		{
			if (strcmp(pnts[j].postfix, "") == 0)
				continue;
			map_dev2meas[m_bms[i].id].push_back(pnts[j]);
		}
	}

	for (int i = 0; i < m_batcluster.size(); ++i)
	{
		pnts.clear();
		BatRtCalc::read_mea(pnts, m_batcluster[i].id);
		for (int j = 0; j < pnts.size(); ++j)
		{
			if (strcmp(pnts[j].postfix, "") == 0)
				continue;
			map_dev2meas[m_batcluster[i].id].push_back(pnts[j]);
		}
	}

	for (int i = 0; i < m_batcell.size(); ++i)
	{
		pnts.clear();
		BatRtCalc::read_mea(pnts, m_batcell[i].id);
		for (int j = 0; j < pnts.size(); ++j)
		{
			if (strcmp(pnts[j].postfix, "") == 0)
				continue;
			map_dev2meas[m_batcell[i].id].push_back(pnts[j]);
		}
	}

	return 0;
}

int BatHisCellEvaluate::write_data()
{
	log_info("BatHisCellEvaluate, write result data to rtdb");

	BatRtCalc::write_rdbdata(m_bat_rtdb, m_result_bms, true);
	BatRtCalc::write_rdbdata(m_bat_rtdb, m_result_cluster, true);
	BatRtCalc::write_rdbdata(m_bat_rtdb, m_result_cell, true);

	return 0;
}

int BatHisCellEvaluate::write_taosdb()
{
	if (!BatConfig::get().get_taos_enable())
		return 0;

	log_info("BatHisCellEvaluate, write result data to taosdb");

	string db_name = BatConfig::get().get_taos_dbname();
	if (db_name.empty())
		db_name = "bat_analysis";

	BatTaos &taos = BatTaos::get();
	int ret_code = 0;

	const int sql_len = 512;
	if (m_result_bms.size() > 0)
	{
		string sql = "INSERT INTO ";
		vector<string> slist(m_result_bms.size());

		for (int i = 0; i < m_result_bms.size(); ++i)
		{
			const bat_his_eva_bms &b = m_result_bms[i];
			char s[sql_len] = { 0 };

			eco::nsprintf(s, sql_len, "his_bms_%lld USING %s.his_eva_bms TAGS(%lld, '%s', %lld, '') "\
									  	"VALUES (%lld, %lld, %lf, %lf, %lf, %lf, %lf, %lf)",
										b.id, db_name.c_str(), b.id, b.description, b.container_id,
										b.end_dt*1000, b.start_dt*1000, b.pear_v, b.pear_i, b.pear_t, b.pear_soc, b.pear_soh, b.score);
			slist[i] = s;
		}
		sql += StringUtils::join(slist, ' ');
		sql += ";";
		ret_code = taos.modify(sql.c_str());
		if (ret_code < 0)
		{
			log_error("BatHisCellEvaluate, insert bms data to %s.his_eva_bms error, ret_code = %d", db_name.c_str(), ret_code);
		}
	}
	
	if (m_result_cluster.size() > 0)
	{
		string sql = "INSERT INTO ";
		vector<string> slist(m_result_cluster.size());
		for (int i = 0; i < m_result_cluster.size(); ++i)
		{
			// todo 电芯数据量过多，考虑分多批写入
			const bat_his_eva_cluster &b = m_result_cluster[i];
			char s[sql_len] = { 0 };

			eco::nsprintf(s, sql_len, "his_batcluster_%lld USING %s.his_eva_cluster TAGS(%lld, '%s', %lld, '') "\
									  "VALUES (%lld, %lld, %lf, %lf, %lf, %lf, %lf, %lf)",
									  b.id, db_name.c_str(), b.id, b.description, b.container_id, 
									  b.end_dt*1000, b.start_dt*1000, b.pear_v, b.pear_i, b.pear_t, b.pear_soc, b.pear_soh, b.score);
			slist[i] = s;
		}
		sql += StringUtils::join(slist, ' ');
		sql += ";";
		ret_code = taos.modify(sql.c_str());
		if (ret_code < 0)
		{
			log_error("BatHisCellEvaluate, insert batcluster data to %s.his_eva_cluster error, ret_code = %d", db_name.c_str(), ret_code);
		}
	}

	if (m_result_cell.size() > 0)
	{
		/// cell数据过多，分批入库，taosdb sql一次只能小于1M Meong 2023-5-31 ///
		const int batch_count = 3000;
		vector<string> slist(batch_count);
		for (int i = 0; i < m_result_cell.size(); ++i)
		{
			const bat_his_eva_cell &b = m_result_cell[i];
			int idx = i % batch_count;
			char s[sql_len] = { 0 };

			eco::nsprintf(s, sql_len, "his_batcell_%lld USING %s.his_eva_cell TAGS(%lld, '%s', %lld, '') "\
				"VALUES (%lld, %lld, %lf, %lf, %lf, %lf, %lf, %lf)",
				b.id, db_name.c_str(), b.id, b.description, b.container_id,
				b.end_dt*1000, b.start_dt*1000, b.pear_v, b.pear_i, b.pear_t, b.pear_soc, b.pear_soh, b.score);

			slist[idx] = s;

			if (idx >= batch_count - 1 || i == m_result_cell.size() - 1)
			{
				// storage
				string sql = "INSERT INTO ";
				sql += StringUtils::join(slist, ' ');
				sql += ";";
				ret_code = taos.modify(sql.c_str());
				if (ret_code < 0)
				{
					log_error("BatHisCellEvaluate, insert batcell data to %s.his_eva_cell error, ret_code = %d", db_name.c_str(), ret_code);
				}

				slist.clear();
				slist.resize(batch_count);
			}
		}
	}

	return 0;
}

int BatHisCellEvaluate::find_sliding_window(const bms &b, const pcs &p, int64_t &start_time, int64_t &end_time)
{
	return 0;

	const static double SOC_VAL = 60;
	const static double P_VAL = 0.1;
	const static int INTERVAL = 15;

	// get bms soc and pcs p from pnt_ana
	int64_t bms_soc_id = find_dev_pnt(b.id, "soc");
	int64_t pcs_p_id = find_dev_pnt(p.id, "p");
	if (bms_soc_id < 0 || pcs_p_id)
	{
		log_warn("BatHisCellEvaluate, bms soc or pcs p pnt not exists, bms id:%lld, pcs id:%lld", b.id, p.id);
		start_time = end_time = 0;
		return -1;
	}

	const int tag_length = 100;
	char tag_bms_soc[tag_length] = { 0 };
	char tag_pcs_p[tag_length] = { 0 };
	eco::nsprintf(tag_bms_soc, tag_length, "pnt_ana.%lld.value", bms_soc_id);
	eco::nsprintf(tag_pcs_p, tag_length, "pnt_ana.%lld.value", pcs_p_id);

	vector<string> tags;
	tags.push_back(tag_bms_soc);
	tags.push_back(tag_pcs_p);

	vector<TRHDBSampleResult> sdata;
	int ret_code = query_sample(sdata, tags, start_time, end_time, INTERVAL);
	if (ret_code < 0 || tags.size() != 2)
	{
		log_warn("BatHisCellEvaluate, query bms soc and pcs p sample value error, bms tag:%s, pcs tag:%s", tag_bms_soc, tag_pcs_p);
		start_time = end_time = 0;
		return -1;
	}

	const VecTRHDBSampleValue& res_bms_soc = sdata[0].vec_value;
	const VecTRHDBSampleValue& res_pcs_p = sdata[1].vec_value;

	vector<int> soc_mark;
	for (int i = 1; i < res_bms_soc.size(); ++i)
	{
		/// soc≈60% && p>0.1×P_rate Meong 2023-5-23 ///
		if (((res_bms_soc[i - 1].value < SOC_VAL && res_bms_soc[i].value >= SOC_VAL)
			|| (res_bms_soc[i - 1].value >= SOC_VAL && res_bms_soc[i].value < SOC_VAL))
			&& res_pcs_p[i].value > m_pcs[m_pcs_idx].p_value * P_VAL)
		{
			soc_mark.push_back(i);
		}
	}

	// todo 判断PCS功率与电池SOC寻找窗口

	return 0;
}

int64_t BatHisCellEvaluate::find_dev_pnt(int64_t dev_id, const char* postfix)
{
	if (strcmp(postfix, "") == 0)
		return -1;

	if (map_dev2meas.count(dev_id) == 0)
		return -1;

	int64_t id = 0;
	const vector<pnt_ana> &pnts = map_dev2meas[dev_id];
	for (int i = 0; i < pnts.size(); ++i)
	{
		if (strcmp(pnts[i].postfix, postfix) != 0)
			continue;

		id = pnts[i].id;
		break;
	}

	return id;
}


void BatHisCellEvaluate::tag_sort_by_idx(vector<string> &old, map<string, int> &tag2idx, map<int, int>& seq2idx)
{
	map<int, string> m;
	map<int, int> s;
	int constant = 10000;
	for (int i = 0; i < old.size(); ++i)
	{
		if (tag2idx.count(old[i]) == 0)
			m[constant++] = old[i];
		else
			m[tag2idx[old[i]]] = old[i];
	}

	old.clear();
	old.resize(m.size());
	int i = 0;
	for (map<int, string>::iterator it = m.begin(); it != m.end(); ++it, ++i)
	{
		old[i] = it->second;
		s[i] = it->first;
	}

	/// 取最长序列，避免长短不一序列合并后变成更长序列 Meong 2023-6-2 ///
	if (s.size() > seq2idx.size())
		seq2idx = s;
}

int BatHisCellEvaluate::corelation_analysis(const bms &b, const int64_t &start_time, const int64_t &end_time, const int interval /*= 300*/)
{
	/// 读电池堆下属电池簇 Meong 2023-5-23 ///
	if (map_bms2cluster.count(b.id) == 0)
	{
		log_warn("BatHisCellEvaluate, bms not has any cluster dev, bms:%lld", b.id);
		return -1;
	}

	int ret_code = -1;

	/// 读电池簇电压、电流、SOC、温度量测点 Meong 2023-5-23 ///
	vector<string> v_tag, i_tag, soc_tag, t_tag;
	map<string, vector<string>> cluster_pfx2tags;
	map<string, int> cluster_tag2idx;
	map<string, vector<string>>::iterator it_cluster;
	map<string, double> cluster_score_ratio;
	vector<double> cluster_score;

	const vector<int> &cluster_idx = map_bms2cluster[b.id];
	for (int i = 0; i < cluster_idx.size(); ++i)
	{
		const int tag_len = 100;
		int clu_idx = cluster_idx[i];
		const batcluster &cluster = m_batcluster[clu_idx];
		char tag[tag_len] = { 0 };
		map<string, double> cell_score_ratio;
		vector<double> cell_score;

		/// 电池簇及下属电芯量测点查找 Meong 2023-5-26 ///
		map<string, vector<string>> cell_pfx2tags;
		map<string, vector<string>>::iterator it_cell;
		map<string, int> cell_tag2idx;
		const vector<pnt_ana> &cluster_pnts = map_dev2meas[cluster.id];
		for (int k = 0; k < cluster_pnts.size(); ++k)
		{
			int64_t id = cluster_pnts[k].id;
			const char* postfix = cluster_pnts[k].postfix;
			if (strcmp(postfix, "") == 0)
				continue;

			if (strncmp(postfix, "Batcell", 7) == 0 || strncmp(postfix, "BatCell", 7) == 0)
			{
				string p = StringUtils::RemoveString(postfix, "Batcell");
				p = StringUtils::RemoveString(p, "BatCell");
				vector<string> l = StringUtils::split(p, '-');
				if (l.size() != 2)
				{
					log_warn("BatHisCellEvaluate, pnt postfix is invalid, pnt id=%lld, postfix=%s", id, postfix);
					continue;
				}

				if (BatConfig::get().get_ratio(string("batcell.") + l[0]) == 0)
				{
					continue;
				}

				int idx = atoi(l[1].c_str());
				if (idx == 0)
				{
					log_warn("BatHisCellEvaluate, pnt postfix fomat is invalid, pnt id=%lld, postfix=%s", id, postfix);
					continue;
				}

				memset(tag, 0, tag_len);
				eco::nsprintf(tag, tag_len, "pnt_ana.%lld.value", id);
				cell_pfx2tags[l[0]].push_back(tag);
				cell_tag2idx[tag] = idx;
			}
			else if (BatConfig::get().get_ratio(string("batcluster.") + postfix) != 0)
			{
				memset(tag, 0, tag_len);
				eco::nsprintf(tag, tag_len, "pnt_ana.%lld.value", id);
				cluster_pfx2tags[postfix].push_back(tag);
				cluster_tag2idx[tag] = clu_idx;
			}
		}

		/// 根据电芯量测点查询时序数据并计算时序数据的皮尔逊相关系数 Meong 2023-5-23 ///
		map<string, vector<double>> cell_pfx2pear;
		map<int, int> cell_seq2idx;
		for (it_cell = cell_pfx2tags.begin(); it_cell != cell_pfx2tags.end(); ++it_cell)
		{
			const string &postfix = it_cell->first;
			vector<string> &tags = it_cell->second;
			string p = string("pear.") + postfix;

			/// 查找电芯分值占比 Meong 2022-8-31 ///
			cell_score_ratio[p] = BatConfig::get().get_ratio(string("hisbatcell.") + postfix);

			// tags需要按idx排序一下
			tag_sort_by_idx(tags, cell_tag2idx, cell_seq2idx);
			vector<vector<double>> out;
			ret_code = query_sample(out, tags, start_time, end_time, interval);
			if (ret_code < 0)
			{
				log_warn("BatHisCellEvaluate, batcell sample data query error, cluster id=%lld, postfix=%s", cluster.id, postfix.c_str());
				continue;
			}

			ret_code = BatMath::pearson_corelation(out, cell_pfx2pear[p]);
			if (ret_code < 0)
			{
				cell_pfx2pear.erase(cell_pfx2pear.find(p));
				log_warn("BatHisCellEvaluate, batcell corelation coefficient calc error, cluster id=%lld, postfix=%s", cluster.id, postfix.c_str());
				continue;
			}
		}

		cell_score = data_eval(cell_pfx2pear, cell_score_ratio);
		if (cell_score.size() != 0)
		{
			double threshold = BatConfig::get().get_score("hisbatcell");

			int data_size = cell_pfx2pear.begin()->second.size();
			for (int k = 0; k < data_size; ++k)
			{
				map<string, double> pfx2val;
				for (map<string, vector<double>>::iterator it = cell_pfx2pear.begin(); it != cell_pfx2pear.end(); ++it)
				{
					const string &postfix = it->first;
					vector<double> &vec_val = it->second;
					pfx2val[postfix] = vec_val[k];
				}

				if (cell_seq2idx.count(k) == 0)
				{
					log_warn("BatHisCellEvaluate, cell idx=-1, cluster id=%lld, k:%d", cluster.id, k);
					continue;
				}
				int idx = cell_seq2idx[k];

				bat_his_eva_cell eva_cell;
				eva_cell.id = cluster.id * 1000 + idx;
				eva_cell.container_id = cluster.id;
				eco::nsprintf(eva_cell.description, MAX_DESCRIPTION_LEN, "BatCell-%d", idx);
				eva_cell.start_dt = start_time;
				eva_cell.end_dt = end_time;
				eco::nsprintf(eva_cell.update_dt, MAX_DATETIME_LEN, t_ch);
				eva_cell.score = cell_score[k];

				map<string, double>::iterator it = pfx2val.begin();
				while (it != pfx2val.end())
				{
					set_calc_pear(eva_cell, it->first.c_str(), it->second);
					if (StringUtils::startswith(it->first, "pear."))
					{
						string postfix = it->first;
						vector<string> s = StringUtils::split(postfix, '.');
						double threshold = BatConfig::get().get_corelation(string("batcell.") + s[1]);
						double val = it->second;
						if (val < threshold)
						{
							ret_code = BatPrewarning::get().insert_warning(b.id, cluster.id, idx, PWCorelation, string("Batcell") + s[1], it->second, threshold);
							if (ret_code < 0)
							{
								log_error("BatHisCellEvaluate, insert batcell prewarning data error");
							}
						}
					}
					++it;
				}
				m_result_cell.push_back(eva_cell);

				/// 综合评分数值判断并预警 Meong 2023-7-6 ///
				if (eva_cell.score < threshold)
				{
					ret_code = BatPrewarning::get().insert_warning(b.id, cluster.id, idx, PWCoScore, "", eva_cell.score, threshold);
					if (ret_code < 0)
					{
						log_error("BatHisCellEvaluate, insert cell score prewarning data error, bms id=%lld, cluster id=%lld, cell idx=%d", b.id, cluster.id, idx);
					}
				}
			}
		}
	}

	/// 根据电池簇量测点查询时序数据并计算时序数据的皮尔逊相关系数 Meong 2023-5-23 ///
	map<string, vector<double>> cluster_pfx2pear;
	map<int, int> cluster_seq2idx;
	for (it_cluster = cluster_pfx2tags.begin(); it_cluster != cluster_pfx2tags.end(); ++it_cluster)
	{
		const string &postfix = it_cluster->first;
		vector<string> &tags = it_cluster->second;
		string p = string("pear.") + postfix;

		cluster_score_ratio[p] = BatConfig::get().get_ratio(string("hisbatcluster.") + postfix);

		// tags需要按idx排序一下
		tag_sort_by_idx(tags, cluster_tag2idx, cluster_seq2idx);
		vector<vector<double>> out;
		ret_code = query_sample(out, tags, start_time, end_time, interval);
		if (ret_code < 0)
		{
			log_warn("BatHisCellEvaluate, batcluster sample data query error, bms id=%lld, postfix=%s", b.id, postfix.c_str());
			continue;
		}

		ret_code = BatMath::pearson_corelation(out, cluster_pfx2pear[p]);
		if (ret_code < 0)
		{
			cluster_pfx2pear.erase(cluster_pfx2pear.find(p));
			log_warn("BatHisCellEvaluate, batcell corelation coefficient calc error, cluster id=%lld, postfix=%s", b.id, postfix.c_str());
			continue;
		}
	}

	int score_min_idx = -1;
	double score_min = 101;
	cluster_score = data_eval(cluster_pfx2pear, cluster_score_ratio);
	if (cluster_score.size() != 0)
	{
		double threshold = BatConfig::get().get_score("hisbatcluster");

		int data_size = cluster_pfx2pear.begin()->second.size();
		for (int k = 0; k < data_size; ++k)
		{
			map<string, double> pfx2val;
			for (map<string, vector<double>>::iterator it = cluster_pfx2pear.begin(); it != cluster_pfx2pear.end(); ++it)
			{
				const string &postfix = it->first;
				vector<double> &vec_val = it->second;
				pfx2val[postfix] = vec_val[k];
			}

			if (cluster_seq2idx.count(k) == 0)
			{
				log_warn("BatHisCellEvaluate, cluster idx=-1, bms id=%lld, k:%d", b.id, k);
				continue;
			}
			int idx = cluster_seq2idx[k];

			batcluster &cluster = m_batcluster[idx];
			bat_his_eva_cluster eva_cluster;

			eva_cluster.id = cluster.id;
			eva_cluster.container_id = cluster.container_id;
			strncpy(eva_cluster.description, cluster.description, MAX_DESCRIPTION_LEN);
			eva_cluster.start_dt = start_time;
			eva_cluster.end_dt = end_time;
			eco::nsprintf(eva_cluster.update_dt, MAX_DATETIME_LEN, t_ch);
			eva_cluster.score = cluster_score[k];
			
			if (cluster_score[k] < score_min)
			{
				score_min = cluster_score[k];
				score_min_idx = m_result_cluster.size();
			}

			log_info("BatHisCellEvaluate, batcluster evaluate, name: %s, id: %lld, score: %lf", cluster.description, cluster.id, cluster_score[k]);

			map<string, double>::iterator it = pfx2val.begin();
			while (it != pfx2val.end())
			{
				set_calc_pear(eva_cluster, it->first.c_str(), it->second);

				if (StringUtils::startswith(it->first, "pear."))
				{
					string postfix = it->first;
					vector<string> s = StringUtils::split(postfix, '.');
					double threshold = BatConfig::get().get_corelation(string("batcell.") + s[1]);
					if (it->second < threshold)
					{
						ret_code = BatPrewarning::get().insert_warning(b.id, cluster.id, -1, PWCorelation, s[1], it->second, threshold);
						if (ret_code < 0)
						{
							log_error("BatHisCellEvaluate, insert batcell prewarning data error");
						}
					}
				}

				++it;
			}
			m_result_cluster.push_back(eva_cluster);

			/// 综合评分数值判断并预警 Meong 2023-7-6 ///
			if (eva_cluster.score < threshold)
			{
				ret_code = BatPrewarning::get().insert_warning(b.id, cluster.id, -1, PWCoScore, "", eva_cluster.score, threshold);
				if (ret_code < 0)
				{
					log_error("BatHisCellEvaluate, insert cluster score prewarning data error, bms id=%lld, cluster id=%lld", b.id, cluster.id);
				}
			}
		}
	}

	// 根据木桶效应对BMS进行历史评价，选取分值最低的电池簇的数据作为电池堆的评价结果
	if (score_min_idx >= 0)
	{
		bat_his_eva_bms eva_bms;
		const bat_his_eva_cluster &cluster = m_result_cluster[score_min_idx];

		eva_bms.id = b.id;
		eva_bms.container_id = b.container_id;
		strncpy(eva_bms.description, b.description, MAX_DESCRIPTION_LEN);
		eva_bms.start_dt = start_time;
		eva_bms.end_dt = end_time;
		eco::nsprintf(eva_bms.update_dt, MAX_DATETIME_LEN, t_ch);
		eva_bms.score = score_min;
		eva_bms.pear_v = cluster.pear_v;
		eva_bms.pear_i = cluster.pear_i;
		eva_bms.pear_t = cluster.pear_t;
		eva_bms.pear_soc = cluster.pear_soc;
		eva_bms.pear_soh = cluster.pear_soh;

		m_result_bms.push_back(eva_bms);

		double threshold = BatConfig::get().get_score("hisbms");
		/// 综合评分数值判断并预警 Meong 2023-7-6 ///
		if (eva_bms.score < threshold)
		{
			ret_code = BatPrewarning::get().insert_warning(b.id, -1, -1, PWCoScore, "", eva_bms.score, threshold);
			if (ret_code < 0)
			{
				log_error("BatHisCellEvaluate, insert bms score prewarning data error, bms id=%lld", b.id);
			}
		}
	}

	return 0;
}

vector<double> BatHisCellEvaluate::data_eval(map<string, vector<double>> &pfx2val, map<string, double> &pfx2ratio)
{
	vector<double> score;

	if (pfx2val.size() == 0 || pfx2ratio.size() == 0)
		return score;

	/// 1 动态序列对齐 Meong 2023-6-2 ///
	dynamic_seq_warping(pfx2val);

	/// 2 相关系数与得分换算 Meong 2023-6-2 ///
	map<string, vector<double>> pfx2diff;
	map<string, vector<double>>::iterator it;
	for (it = pfx2val.begin(); it != pfx2val.end(); ++it)
	{
		const string &postfix = it->first;
		vector<double> &vec = it->second;
		vector<double> &vec_val = pfx2diff[postfix];
		for (int i = 0; i < vec.size(); ++i)
		{
			if (strncmp(postfix.c_str(), "pear.", 5) == 0)
			{
				vec_val.push_back(vec[i] * 100);
			}
		}
	}

	/// 3 评价加权归一化 Meong 2022-8-31 ///
	map<string, double> score_ratio = pfx2ratio;
	BatMath::normalized(score_ratio);

	/// 4 数据评价 Meong 2022-8-31 ///
	int data_size = pfx2diff.begin()->second.size();
	for (int i = 0; i < data_size; ++i)
	{
		vector<double> v;
		vector<double> w;
		for (map<string, vector<double>>::iterator it = pfx2diff.begin(); it != pfx2diff.end(); ++it)
		{
			const string &postfix = it->first;
			double ratio = score_ratio[postfix];
			v.push_back(it->second[i]);
			w.push_back(ratio);
		}
		double score_tmp = BatMath::weight_average(v, w);
		score.push_back(score_tmp < 0 ? 0 : (score_tmp > 100 ? 100 : score_tmp));
	}

	return score;
}

int BatHisCellEvaluate::calc()
{
	log_info("BatHisCellEvaluate, calc start");

	int ret_code = 0;

	time_t t = time(0);
	memset(t_ch, 0, MAX_DATETIME_LEN);
	strftime(t_ch, sizeof(t_ch), "%Y-%m-%d %H:%M:%S", localtime(&t));

	clear();

	ret_code = init();
	if (ret_code < 0)
	{
		log_error("BatHisCellEvaluate, init error! ret_code = %d", ret_code);
		return ret_code;
	}

	ret_code = read_data();
	if (ret_code < 0)
	{
		log_error("BatHisCellEvaluate, read data error! ret_code = %d", ret_code);
		return ret_code;
	}

	// start_time and end_time
	time_t now = time_t(0);
	time(&now); /*获取time_t类型的当前时间*/
	time_t ago = now - 24 * 60 * 60;

	for (int i = 0; i < m_bms.size(); ++i)
	{
		bms& b = m_bms[i];
		if (map_bms2pcs.count(b.id) == 0) 
		{
			log_warn("BatHisCellEvaluate, bms:%lld not relation with pcs", b.id);
			continue;
		}

		pcs& p = m_pcs[map_bms2pcs[b.id]];

		m_bms_idx = i;
		m_pcs_idx = map_bms2pcs[b.id];

		/// 暂时不做滑动窗口选取，前一日全部运行数据分析，5min一个点 Meong 2023-5-23 ///
		/// 滑动窗口选取 Meong 2023-5-22 ///
		ret_code = find_sliding_window(b, p, ago, now);
		if (ret_code < 0 || ago == now)
		{
			log_warn("BatHisCellEvaluate, find sliding window error, bms:%lld, pcs:%lld", b.id, p.id);
			continue;
		}

		ret_code = corelation_analysis(b, ago, now, 300);
		if (ret_code < 0)
		{
			log_warn("BatHisCellEvaluate, corelation coefficient calc error, bms:%lld, pcs:%lld", b.id, p.id);
			continue;
		}
	}

	write_data();
	ret_code = write_taosdb();
	if (ret_code < 0)
	{
		log_error("BatHisCellEvaluate, write to taosdb error, ret_code = %d", ret_code);
		return ret_code;
	}

	log_info("BatHisCellEvaluate, calc finish");

	return 0;
}


BatHisStatisticCalc::BatHisStatisticCalc()
{
}

BatHisStatisticCalc::~BatHisStatisticCalc()
{
}

void BatHisStatisticCalc::clear()
{
	m_bms.clear();
	m_batcluster.clear();

	m_dbdata.clear();
	m_insertdata.clear();

	map_dev2meas.clear();

	map_bms2idx.clear();
	map_cluster2idx.clear();
	map_bms2cluster.clear();
	map_cluster2bms.clear();

	map_bms2his.clear();
	map_cluster2his.clear();
}

int BatHisStatisticCalc::read_data()
{
	log_info("BatHisStatisticCalc, read data");

	int ret_code = 0;

	/// 读所有BMS Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_bms);
	if (ret_code < 0)
	{
		log_error("BatHisStatisticCalc, read bms error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 读所有电池簇 Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_batcluster);
	if (ret_code < 0)
	{
		log_error("BatHisStatisticCalc, read batcluster error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 建立索引关系 Meong 2022-8-25 ///
	for (int i = 0; i < m_bms.size(); ++i)
	{
		bms &obj = m_bms[i];
		map_bms2idx[obj.id] = i;
	}

	for (int i = 0; i < m_batcluster.size(); ++i)
	{
		batcluster &obj = m_batcluster[i];
		map_cluster2idx[obj.id] = i;

		if (map_bms2idx.count(obj.relation_id) == 0)
		{
			log_warn("batcluster relation_id is invalid, id = %lld, relation_id = %lld", obj.id, obj.relation_id);
			continue;
		}

		map_cluster2bms[obj.id] = map_bms2idx[obj.relation_id];
		map_bms2cluster[obj.relation_id].push_back(i);
	}

	/// 读量测 Meong 2022-8-22 ///
	ret_code = read_mea();
	if (ret_code < 0)
	{
		log_error("BatHisStatisticCalc, read measuration error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 读数据库记录 Meong 2023-6-27 ///
	ret_code = read_db();
	if (ret_code < 0)
	{
		log_error("BatHisStatisticCalc, read db error, ret_code = %d", ret_code);
		return ret_code;
	}

	return 0;
}

int BatHisStatisticCalc::read_mea()
{
	vector<pnt_ana> pnts;
	for (int i = 0; i < m_batcluster.size(); ++i)
	{
		pnts.clear();
		BatRtCalc::read_mea(pnts, m_batcluster[i].id);
		for (int j = 0; j < pnts.size(); ++j)
		{
			if (strcmp(pnts[j].postfix, "") == 0)
				continue;
			map_dev2meas[m_batcluster[i].id].push_back(pnts[j]);
		}
	}

	return 0;
}

int BatHisStatisticCalc::read_db()
{
	const char* sql = "select bms_id, cluster_id, cell_index, vmax_n, vmax_p, vmin_n, vmin_p, tmax_n, tmax_p, tmin_n, tmin_p from bat_his_statistics;";
	TwoDimArray result;
	int ret_code = m_dbClient.SelectBySQL(sql, result);
	if (ret_code < 0)
	{
		log_error("BatHisStatisticCalc, select data from bat_his_statistics error! ret_code = %d, sql = %s", ret_code, sql);
		return ret_code;
	}

	m_dbdata.clear();
	if (result.Row() == 0)
		return 0;

	m_dbdata.resize(result.Row());
	for (int i = 0; i < result.Row(); ++i)
	{
		bat_his_statistics &d = m_dbdata[i];
		d.bms_id = result[i][0].i64_val;
		d.cluster_id = result[i][1].i64_val;
		d.cell_index = result[i][2].i32_val;
		d.vmax_n = result[i][3].i32_val;
		d.vmax_p = result[i][4].f64_val;
		d.vmin_n = result[i][5].i32_val;
		d.vmin_p = result[i][6].f64_val;
		d.tmax_n = result[i][7].i32_val;
		d.tmax_p = result[i][8].f64_val;
		d.tmin_n = result[i][9].i32_val;
		d.tmin_p = result[i][10].f64_val;

		if (d.bms_id > 0)
			map_bms2his[d.bms_id].push_back(i);
		if (d.cluster_id > 0)
			map_cluster2his[d.cluster_id].push_back(i);
	}
	result.Clear();

	return 0;
}

int BatHisStatisticCalc::table_exists()
{
	/// 表是否存在 Meong 2023-6-27 ///
	const char* sql = "select bms_id from bat_his_statistics;";
	//bat_his_statistics sta;
	TwoDimArray result;
	int ret_code = m_dbClient.SelectBySQL(sql, result);
	if (ret_code < 0)
	{
		result.Clear();
		log_error("BatHisStatisticCalc, table bat_his_statistics not exists");
		return ret_code;
	}

	return 0;
}

int BatHisStatisticCalc::data_statistics(const bms &b, const int64_t &start_time, const int64_t &end_time)
{
	/// 读电池堆下属电池簇 Meong 2023-5-23 ///
	if (map_bms2cluster.count(b.id) == 0)
	{
		log_warn("BatHisStatisticCalc, bms not has any cluster dev, bms:%lld", b.id);
		return -1;
	}

	int ret_code = -1;

	map<int64_t, map<string, vector<int>>> map_clusterstat;

	const vector<int> &cluster_idx = map_bms2cluster[b.id];
	for (int i = 0; i < cluster_idx.size(); ++i)
	{
		const int tag_len = 100;
		int clu_idx = cluster_idx[i];
		const batcluster &cluster = m_batcluster[clu_idx];
		int cell_count = 0;
		char tag[tag_len] = { 0 };
		/// 读电池簇最大电压电芯编号，最低电压电芯编号，最高温度电芯编号，最低温度电压编号 Meong 2023-6-27 ///
		vector<string> tags;
		map<int, string> map_tag;
		const vector<pnt_ana> &cluster_pnts = map_dev2meas[cluster.id];
		for (int k = 0; k < cluster_pnts.size(); ++k)
		{
			int64_t id = cluster_pnts[k].id;
			const char* postfix = cluster_pnts[k].postfix;
			if (strcmp(postfix, "") == 0)
				continue;

			/// 判断电芯数量 Meong 2023-6-28 ///
			if (strncmp(postfix, "BatcellV", 7) == 0 || strncmp(postfix, "BatCellV", 7) == 0)
			{
				vector<string> l = StringUtils::split(postfix, '-');
				if (l.size() != 2)
				{
					log_warn("BatHisStatisticCalc, pnt postfix is invalid, pnt id=%lld, postfix=%s", id, postfix);
					continue;
				}

				int idx = atoi(l[1].c_str());
				if (idx == 0)
				{
					log_warn("BatHisStatisticCalc, pnt postfix fomat is invalid, pnt id=%lld, postfix=%s", id, postfix);
					continue;
				}

				cell_count = cell_count < idx ? idx : cell_count;
			}
			else if (strcmp(postfix, "vmax_pos") == 0)
			{
				memset(tag, 0, tag_len);
				eco::nsprintf(tag, tag_len, "pnt_ana.%lld.value", id);
				tags.push_back(tag);
				map_tag[tags.size() - 1] = "vmax_pos";
			}
			else if (strcmp(postfix, "vmin_pos") == 0)
			{
				memset(tag, 0, tag_len);
				eco::nsprintf(tag, tag_len, "pnt_ana.%lld.value", id);
				tags.push_back(tag);
				map_tag[tags.size() - 1] = "vmin_pos";
			}
			else if (strcmp(postfix, "tmax_pos") == 0)
			{
				memset(tag, 0, tag_len);
				eco::nsprintf(tag, tag_len, "pnt_ana.%lld.value", id);
				tags.push_back(tag);
				map_tag[tags.size() - 1] = "tmax_pos";
			}
			else if (strcmp(postfix, "tmin_pos") == 0)
			{
				memset(tag, 0, tag_len);
				eco::nsprintf(tag, tag_len, "pnt_ana.%lld.value", id);
				tags.push_back(tag);
				map_tag[tags.size() - 1] = "tmin_pos";
			}
		}

		/// 时序库读历史数据并统计计算 Meong 2023-6-28 ///
		if (tags.size() == 0 || cell_count == 0)
			continue;

		vector<vector<double>> out;
		ret_code = query_sample(out, tags, start_time, end_time, -1, RHDB_ALL_DATA);
		if (ret_code < 0)
		{
			log_warn("BatHisStatisticCalc, batcluster sample data query error, bms id=%lld, cluster id=%lld", b.id, cluster.id);
			continue;
		}

		map<string, vector<int>> &map_type2count = map_clusterstat[cluster.id];
		for (int k = 0; k < tags.size(); ++k)
		{
			string type = map_tag[k];
			if (type.empty())
				continue;

			vector<int> &vec = map_type2count[type];
			if (vec.size() == 0) {
				vec.resize(cell_count);
			}

			const vector<double> &data = out[k];
			for (int m = 0; m < data.size(); ++m)
			{
				int val = static_cast<int>(data[m]);

				/// val的值为BatCellV-编号，编号起始值为1 Meong 2023-6-30 ///
				if (val <= 0 || val > vec.size())
					continue;

				++vec[val-1];
			}
		}
	}

	ret_code = merge_data(b.id, map_clusterstat);
	if (ret_code < 0)
	{
		log_error("BatHisStatisticCalc, merge data error, bms id:%lld", b.id);
		return -1;
	}

	ret_code = calc_data(b.id, map_clusterstat);
	if (ret_code < 0)
	{
		log_error("BatHisStatisticCalc, calc data error, bms id:%lld", b.id);
		return -1;
	}

	ret_code = write_data();
	if (ret_code < 0)
	{
		log_error("BatHisStatisticCalc, write data error, bms id:%lld", b.id);
		return -1;
	}

	return 0;
}

int BatHisStatisticCalc::merge_data(int64_t bms_id, map<int64_t, map<string, vector<int>>> &map_cluster)
{
	map<int64_t, map<string, vector<int>>>::iterator it = map_cluster.begin();
	for (; it != map_cluster.end(); ++it)
	{
		int64_t id = it->first;
		map<string, vector<int>> &map_type2count = it->second;
		map<string, vector<int>>::iterator it_type = map_type2count.begin();

		if (map_cluster2his.count(id) == 0)
		{
			/// 插入全部数据 Meong 2023-6-30 ///
			insert_all(bms_id, id, it_type->second.size());
			continue;
		}
		
		const vector<int> &dbcluvec = map_cluster2his[id];
		map<int, int> dbclucell_idx;
		for (int i = 0; i < dbcluvec.size(); ++i)
		{
			const bat_his_statistics &dbc = m_dbdata[dbcluvec[i]];
			if (dbc.cell_index <= 0)
				continue;

			dbclucell_idx[dbc.cell_index] = dbcluvec[i];
		}

		for (it_type = map_type2count.begin(); it_type != map_type2count.end(); ++it_type)
		{
			const string &type = it_type->first;
			vector<int> &vec = it_type->second;
			for (int i = 0; i < vec.size(); ++i)
			{
				if (dbclucell_idx.count(i + 1) == 0)
				{
					/// 插入电芯数据 Meong 2023-6-30 ///
					insert_one(bms_id, id, i + 1);
					continue;
				}

				const bat_his_statistics &dbc = m_dbdata[dbclucell_idx[i + 1]];
				if (type == "vmax_pos")
				{
					vec[i] += dbc.vmax_n;
				}
				else if (type == "vmin_pos")
				{
					vec[i] += dbc.vmin_n;
				}
				else if (type == "tmax_pos")
				{
					vec[i] += dbc.tmax_n;
				}
				else if (type == "tmin_pos")
				{
					vec[i] += dbc.tmin_n;
				}
			}
		}
	}

	return 0;
}

int BatHisStatisticCalc::insert_all(int64_t bms_id, int64_t cluster_id, int cell_count)
{
	if (bms_id <= 0 || cluster_id <= 0 || cell_count <= 0)
		return -1;

	const int sql_len = 256;
	char sql[sql_len];
	for (int i = 0; i < cell_count; ++i)
	{
		memset(sql, 0, sql_len);
		eco::nsprintf(sql, sql_len, "insert into bat_his_statistics(bms_id, cluster_id, cell_index, update_dt) values(%lld, %lld, %d, '%s')",
			bms_id, cluster_id, i + 1, t_ch);

		m_insert.push_back(sql);
	}

	return 0;
}

int BatHisStatisticCalc::insert_one(int64_t bms_id, int64_t cluster_id, int cell_index)
{
	if (bms_id <= 0 || cluster_id <= 0 || cell_index <= 0)
		return -1;

	const int sql_len = 256;
	char sql[sql_len] = { 0 };

	eco::nsprintf(sql, sql_len, "insert into bat_his_statistics(bms_id, cluster_id, cell_index, update_dt) values(%lld, %lld, %d, '%s')",
		bms_id, cluster_id, cell_index, t_ch);

	m_insert.push_back(sql);

	return 0;
}

int BatHisStatisticCalc::calc_data(int64_t bms_id, map<int64_t, map<string, vector<int>>> &map_cluster)
{
	const int sql_len = 512;
	char sql[sql_len] = { 0 };

	map<int64_t, map<string, vector<int>>>::iterator it = map_cluster.begin();
	for (; it != map_cluster.end(); ++it)
	{
		int64_t id = it->first;
		map<string, vector<int>> &map_type2count = it->second;
		map<string, vector<int>>::iterator it_type = map_type2count.begin();
		int cell_count = it_type->second.size();

		map<string, double> map_count;
		for (it_type = map_type2count.begin(); it_type != map_type2count.end(); ++it_type)
		{
			const string &type = it_type->first;
			vector<int> &vec = it_type->second;
			double sum = accumulate(vec.begin(), vec.end(), 0.0);
			map_count[type] = sum;
		}

		for (int i = 0; i < cell_count; ++i)
		{
			vector<string> set_str;
			for (it_type = map_type2count.begin(); it_type != map_type2count.end(); ++it_type)
			{
				const string &type = it_type->first;
				vector<int> &vec = it_type->second;
				double count = map_count[type];
				if (count == 0)
					continue;

				if (type == "vmax_pos")
				{
					char s[128] = { 0 };
					eco::nsprintf(s, 128, "vmax_n=%d, vmax_p=%0.2lf", vec[i], vec[i] / count);
					set_str.push_back(s);
				}
				else if (type == "vmin_pos")
				{
					char s[128] = { 0 };
					eco::nsprintf(s, 128, "vmin_n=%d, vmin_p=%0.2lf", vec[i], vec[i] / count);
					set_str.push_back(s);
				}
				else if (type == "tmax_pos")
				{
					char s[128] = { 0 };
					eco::nsprintf(s, 128, "tmax_n=%d, tmax_p=%0.2lf", vec[i], vec[i] / count);
					set_str.push_back(s);
				}
				else if (type == "tmin_pos")
				{
					char s[128] = { 0 };
					eco::nsprintf(s, 128, "tmin_n=%d, tmin_p=%0.2lf", vec[i], vec[i] / count);
					set_str.push_back(s);
				}
			}

			if (set_str.size() == 0)
				continue;

			string str = StringUtils::join(set_str, ',');
			memset(sql, 0, sql_len);
			eco::nsprintf(sql, sql_len, "update bat_his_statistics set %s,update_dt='%s' where bms_id=%lld and cluster_id=%lld and cell_index=%d",
				str.c_str(), t_ch, bms_id, id, i + 1);

			m_update.push_back(sql);
		}
	}

	return 0;
}

int BatHisStatisticCalc::write_data()
{
	const int commit_count = 3000;
	int ret_code = 0;
	for (int i = 0; i < m_insert.size(); ++i)
	{
		int commit_flag = 0;
		if (i == commit_count - 1 || i == m_insert.size() - 1)
		{
			commit_flag = 1;
		}
		ret_code = m_dbClient.ExecuteSql(m_insert[i].c_str(), commit_flag);
		if (ret_code < 0)
		{
			log_error("insert data error, ret_code: %d, error str: %s", ret_code, m_dbClient.GetLastErrorStr().c_str());
			continue;
		}
	}

	for (int i = 0; i < m_update.size(); ++i)
	{
		int commit_flag = 0;
		if (i == commit_count - 1 || i == m_update.size() - 1)
		{
			commit_flag = 1;
		}
		ret_code = m_dbClient.ExecuteSql(m_update[i].c_str(), commit_flag);
		if (ret_code < 0)
		{
			log_error("update data error, ret_code: %d, error str: %s", ret_code, m_dbClient.GetLastErrorStr().c_str());
			continue;
		}
	}

	return 0;
}

int BatHisStatisticCalc::calc()
{
	log_info("BatHisStatisticCalc, calc start");

	int ret_code = 0;

	time_t t = time(0);
	memset(t_ch, 0, MAX_DATETIME_LEN);
	strftime(t_ch, sizeof(t_ch), "%Y-%m-%d %H:%M:%S", localtime(&t));

	clear();

	ret_code = table_exists();
	if (ret_code < 0)
	{
		log_error("BatHisStatisticCalc, table bat_his_statistics not exists! ret_code = %d", ret_code);
		return ret_code;
	}

	ret_code = read_data();
	if (ret_code < 0)
	{
		log_error("BatHisStatisticCalc, read data error! ret_code = %d", ret_code);
		return ret_code;
	}

	// start_time and end_time
	time_t now = time_t(0);
	time(&now); /*获取time_t类型的当前时间*/
	time_t ago = now - 24 * 60 * 60;

	for (int i = 0; i < m_bms.size(); ++i)
	{
		bms& b = m_bms[i];

		ret_code = data_statistics(b, ago, now);
		if (ret_code < 0)
		{
			log_warn("BatHisStatisticCalc, data statistics calc error, bms:%lld", b.id);
			continue;
		}
	}

	return 0;
}