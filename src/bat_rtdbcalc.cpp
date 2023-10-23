#include "bat_rtdbcalc.h"
#include "sys/eproc.h"
#include "eco/estring.h"
#include "deelx.hpp"
#include "stringutils.h"
#include "bat_math.h"
#include "bat_config.h"
#include "bat_taos.h"
#include "bat_prewarning.h"

int regex_match(const char* str, const char* reg)
{
	std::map<int, int> map_pos;
	int beginPos = 0;
	int endPos = 0;
	const char* val = str;

	// 表达式对象
	CRegexpT <char> re(reg);

	// 是否找到匹配
	while (re.Match(val, endPos).IsMatched())
	{
		MatchResult result = re.Match(val, endPos);

		beginPos = result.GetStart();
		endPos = result.GetEnd();

		map_pos[beginPos] = endPos - beginPos;
		//vec_pos.push_back(endPos);
	}

	char buf[128];
	memset(buf, 0, sizeof(buf));
	std::string res;
	for (int i = 0; i < strlen(val);)
	{
		if (map_pos.find(i) != map_pos.end()) //special text
		{
			memset(buf, 0, sizeof(buf));
			strncpy(buf, val + i, map_pos[i]);
			res += buf;
			res += "\n";
			i += map_pos[i];
		}
		else if (val[i] > 0 && val[i] < 127) //ASCII 1bit
		{
			memset(buf, 0, sizeof(buf));
			strncpy(buf, val + i, 1);
			res += buf;
			res += "\n";
			++i;
		}
		else //characters 2bits
		{
			memset(buf, 0, sizeof(buf));
			strncpy(buf, val + i, 2);
			res += buf;
			res += "\n";
			i += 2;
		}
	}

	return 0;
}

int BatRtCalc::init()
{
	log_info("BatRtCalc, init");

	int ret_code = 0;

	ret_code = m_public_rtdb.Init();
	if (ret_code < 0)
	{
		log_error("BatRtCalc, rtdb init error! ret_code = %d", ret_code);
		return ret_code;
	}

	std::string ctx_app(EProc::GetCtxApp());
	ret_code = m_public_rtdb.Open(ctx_app.c_str());
	if (ret_code < 0)
	{
		log_error("BatRtCalc, rtdb open error! ret_code = %d", ret_code);
		return ret_code;
	}

	ret_code = m_opt_comm.Init(ctx_app.c_str());
	if (ret_code < 0) {
		log_error("BatRtCalc, m_opt_comm init error! ret_code = %d", ret_code);
		return ret_code;
	}

	return 0;
}

int BatRtCalc::read_mea(vector<pnt_ana> &pnts, int64_t container, const char* postfix /* = "" */)
{
	if (strcmp(postfix, "") == 0)
	{
		pnt_ana_container_cond cond;
		cond.container_id = container;
		return read_mea(pnts, cond);
	}
	else
	{
		pnt_ana_con_post_cond cond;
		cond.container_id = container;
		ECO_NCPY_CHAR_ARRAY(cond.postfix, postfix);
		return read_mea(pnts, cond);
	}

	return pnts.size();
}

int BatRtCalc::read_mea(vector<pnt_dig> &pnts, int64_t container, const char* postfix /* = "" */)
{
	if (strcmp(postfix, "") == 0)
	{
		pnt_dig_container_cond cond;
		cond.container_id = container;
		return read_mea(pnts, cond);
	}
	else
	{
		pnt_dig_con_post_cond cond;
		cond.container_id = container;
		ECO_NCPY_CHAR_ARRAY(cond.postfix, postfix);
		return read_mea(pnts, cond);
	}

	return pnts.size();
}

int BatRtCalc::read_mea(vector<pnt_mix> &pnts, int64_t container, const char* postfix /* = "" */)
{
	if (strcmp(postfix, "") == 0)
	{
		pnt_mix_container_cond cond;
		cond.container_id = container;
		return read_mea(pnts, cond);
	}
	else
	{
		pnt_mix_con_post_cond cond;
		cond.container_id = container;
		ECO_NCPY_CHAR_ARRAY(cond.postfix, postfix);
		return read_mea(pnts, cond);
	}

	return pnts.size();
}

type_vt BatRtCalc::get_vbase(int id)
{
	type_vt vt;

	if (id < 0)
		return vt;

	int ret_code = m_public_rtdb.GetByKey(id, vt);
	if (ret_code < 0) {
		log_error("get vbase by id error! ret_code = %d", ret_code);
		return vt;
	}

	return vt;
}

int64_t BatRtCalc::read_tpl_dev(int64_t container_id, const char *tpl_name)
{
	int64_t id = -1;
	if (container_id <= 0)
		return id;

	if (strcmp(tpl_name, "") == 0)
		return id;

	obj_dev_tpl_cond cond;
	cond.container_id = container_id;
	ECO_NCPY_CHAR_ARRAY(cond.tp_name, tpl_name);

	vector<obj_dev> devs;

	int ret_code = m_public_rtdb.Get(cond, devs);
	if (ret_code < 0)
	{
		log_error("read tpl dev error, container_id = %lld, tp_name = %s, ret_code = %d ", container_id, tpl_name, ret_code);
		return ret_code;
	}

	if (devs.size() != 1)
	{
		log_error("tpl dev is empty, container_id = %lld, tp_name = %s, ret_code = %d ", container_id, tpl_name, ret_code);
		return -1;
	}

	return devs[0].id;
}

void BatRtCalc::dynamic_seq_warping(map<string, map<int, double>> &pfx2idxval)
{
	int max_count = 0;
	map<string, map<int, double>>::iterator it = pfx2idxval.begin();
	for (; it != pfx2idxval.end(); ++it)
	{
		const string &postfix = it->first;
		map<int, double> &map_idx2val = it->second;
		max_count = map_idx2val.size() > max_count ? map_idx2val.size() : max_count;
		//cell_postfix2variance[postfix] = BatMath::variance(map_idx2val);
	}

	for (it = pfx2idxval.begin(); it != pfx2idxval.end(); ++it)
	{
		const string &postfix = it->first;
		map<int, double> &map_idx2val = it->second;
		if (map_idx2val.size() == max_count)
			continue;

		double ratio = 1.0 * map_idx2val.size() / max_count;
		map<int, double> new_map;

		int first_idx = map_idx2val.begin()->first;
		for (int k = first_idx; k < first_idx + max_count; ++k)
		{
			int idx = lround(k * ratio);
			double value = 0;
			if (map_idx2val.count(idx) == 0)
			{
				/// idx在map中找不到时，自动找到前一个，如idx=0时，向后找一个 Meong 2022-8-31 ///
				if (idx == 0)
				{
					++idx;
					if (map_idx2val.count(idx) != 0)
					{
						value = map_idx2val[idx];
					}
					else
					{
						/// 再找不到就没有办法了 Meong 2022-8-31 ///
					}
				}
				else
				{
					--idx;
					if (map_idx2val.count(idx) != 0)
					{
						value = map_idx2val[idx];
					}
					else
					{
						/// 再找不到就没有办法了 Meong 2022-8-31 ///
					}
				}
			}
			else
			{
				value = map_idx2val[idx];
			}

			if (k == first_idx + max_count - 1)
			{
				/// 最后一个强制使用map中最后一个值 Meong 2022-8-31 ///
				map<int, double>::iterator it_idx = map_idx2val.end();
				--it_idx;
				value = it_idx->second;
			}

			new_map[k] = value;
		}
		it->second = new_map;
	}
}

void BatRtCalc::dynamic_seq_warping(map<string, vector<double>> &pfx2val)
{
	int max_count = 0;
	map<string, vector<double>>::iterator it = pfx2val.begin();
	for (; it != pfx2val.end(); ++it)
	{
		const string &postfix = it->first;
		max_count = it->second.size() > max_count ? it->second.size() : max_count;
	}

	for (it = pfx2val.begin(); it != pfx2val.end(); ++it)
	{
		const string &postfix = it->first;
		vector<double> &vec_val = it->second;
		if (vec_val.size() == max_count)
			continue;

		double ratio = 1.0 * vec_val.size() / max_count;
		vector<double> new_vec(max_count);
		for (int k = 0; k < max_count; ++k)
		{
			int idx = lround(k*ratio);
			idx = idx < 0 ? 0 : idx >= vec_val.size() ? vec_val.size() - 1 : idx;
			new_vec[k] = vec_val[idx];
		}
		it->second = new_vec;
	}
}

BatRtSohCalc::BatRtSohCalc()
{
}

BatRtSohCalc::~BatRtSohCalc()
{
}

int BatRtSohCalc::calc()
{
	log_info("BatRtSohCalc, start calc");

	int ret_code = 0;
	ret_code = read_data();
	if (ret_code < 0)
	{
		log_error("BatRtSohCalc, read data error! ret_code = %d", ret_code);
		return ret_code;
	}

	map<int64_t, double> map_stsoh;

	map<int64_t, vector<int>>::iterator it = map_st2bms.begin();
	for (; it != map_st2bms.end(); ++it)
	{
		station_bess &st = m_station[it->first];
		int64_t st_id = st.id;
		vector<int>& bms_idx = it->second;

		double rated_soc_sum = 0;
		double soh_sum = 0;

		vector<double> vec_soc_ratio;
		vector<double> vec_soh;
		for (int j = 0; j < bms_idx.size(); ++j)
		{
			bms &obj = m_bms[bms_idx[j]];
			rated_soc_sum += obj.rated_soc;
			if (map_dev2soh.count(obj.id) == 0)
			{
				log_warn("bms not have soh pnt, use default value 0, bms_id=%lld", obj.id);
				vec_soh.push_back(0);
			}
			else
			{
				double soh = map_dev2soh[obj.id].value;
				if (soh < 0)
				{
					log_warn("bms soh < 0, bms_id=%lld, actual_value=%lf", obj.id, soh);
					soh = 0;
				}
				else if (soh > 100)
				{
					log_warn("bms soh > 100, bms_id=%lld, actual_value=%lf", obj.id, soh);
					soh = 100;
				}
				vec_soh.push_back(soh);
				soh_sum += map_dev2soh[obj.id].value;
			}
		}

		if (soh_sum == 0)
		{
			map_stsoh[st_id] = 0;
			continue;
		}

		if (rated_soc_sum == 0)
		{
			for (int j = 0; j < bms_idx.size(); ++j)
				vec_soc_ratio.push_back(1);
		}
		else
		{
			for (int j = 0; j < bms_idx.size(); ++j)
			{
				bms &obj = m_bms[bms_idx[j]];
				double r = obj.rated_soc / rated_soc_sum;
				vec_soc_ratio.push_back(r);
			}
		}
		
		if (vec_soc_ratio.size() != vec_soh.size())
		{
			log_error("soc size (%d) != soh size (%d)", vec_soc_ratio.size(), vec_soh.size());
			continue;
		}

		double tmp_sum = 0;
		for (int j = 0; j < vec_soh.size(); ++j)
		{
			tmp_sum += vec_soh[j] * vec_soc_ratio[j];
		}

		double soh = tmp_sum;
		if (soh < 0)
		{
			log_warn("st soh < 0, st_id=%lld, actual_value=%lf", st_id, soh);
			soh = 0;
		}
		else if (soh > 100)
		{
			log_warn("st soh > 100, st_id=%lld, actual_value=%lf", st_id, soh);
			soh = 100;
		}

		log_info("BatRtSohCalc, station=%lld, soh=%lf", st_id, soh);
		map_stsoh[st_id] = soh;
	}

	/// 回写厂站soh数据 Meong 2022-8-22 ///
	write_mea(map_stsoh);

	log_info("BatRtSohCalc, calc finished");

	return 0;
}

void BatRtSohCalc::clear()
{
	m_station.clear();
	m_bms.clear();
	m_batcluster.clear();
	m_batcell.clear();
	
	map_st2soh.clear();
	map_dev2soh.clear();

	map_st2idx.clear(), map_bms2idx.clear(), map_cluster2idx.clear(), map_cell2idx.clear();
	map_st2bms.clear(), map_bms2cluster.clear(), map_cluster2cell.clear();
	map_bms2st.clear(), map_cluster2bms.clear(), map_cell2cluster.clear();

}

int BatRtSohCalc::read_data()
{
	log_info("BatRtSohCalc, read data");
	
	clear();

	int ret_code = 0;

	/// 1、读所有station_bess Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_station);
	if (ret_code < 0)
	{
		log_error("BatRtSohCalc, read station_bess error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 2、读所有BMS Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_bms);
	if (ret_code < 0)
	{
		log_error("BatRtSohCalc, read bms error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 3、读所有电池簇 Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_batcluster);
	if (ret_code < 0)
	{
		log_error("BatRtSohCalc, read batcluster error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 4、读所有电芯 Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_batcell);
	if (ret_code < 0)
	{
		log_error("BatRtSohCalc, read batcell error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 5、读厂站SOH点 Meong 2022-8-22 ///
	read_st_mea();

	/// 6、处理索引关系 Meong 2022-8-22 ///
	create_index();

	/// 7、读量测 Meong 2022-8-22 ///
	read_mea();

	return 0;
}

int BatRtSohCalc::read_st_mea()
{
	obj_dev_tpl_cond cond;
	for (int i = 0; i < m_station.size(); ++i)
	{
		station_bess &st = m_station[i];

		int64_t dev_id = read_tpl_dev(st.id, "bess_measuration");
		if (dev_id < 0)
		{
			log_error("BatRtSohCalc, read station tpl dev error, id = %lld, tpl_name = bess_measuration", st.id);
			continue;
		}

		read_mea(map_st2soh, dev_id, st.id, "soh");
	}

	return 0;
}

int BatRtSohCalc::create_index()
{
	int ret_code = 0;

	map<int64_t, int> map_sub2st;
	for (int i = 0; i < m_station.size(); ++i)
	{
		station_bess &obj = m_station[i];
		map_st2idx[obj.id] = i;
	}

	vector<obj_dev> sub_bess;
	obj_dev_filter_cond cond;
	ECO_NCPY_CHAR_ARRAY(cond.filter, "sub_bess");
	ret_code = m_public_rtdb.Get(cond, sub_bess);
	if (ret_code < 0)
	{
		log_error("read sub_bess error, ret_code = %d", ret_code);
		return ret_code;
	}

	for (int i = 0; i < sub_bess.size(); ++i)
	{
		obj_dev &obj = sub_bess[i];

		if (map_st2idx.count(obj.container_id) == 0)
			continue;

		map_sub2st[obj.id] = map_st2idx[obj.container_id];
	}

	for (int i = 0; i < m_bms.size(); ++i)
	{
		bms &obj = m_bms[i];
		map_bms2idx[obj.id] = i;

		/// 建立BMS与厂站的关系 Meong 2022-8-22 ///
		if (map_sub2st.count(obj.container_id) == 0)
		{
			log_error("bms container error, id = %lld, container_id = %lld", obj.id, obj.container_id);
			continue;
		}

		map_bms2st[obj.id] = map_sub2st[obj.container_id];
		map_st2bms[map_sub2st[obj.container_id]].push_back(i);
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

	return 0;
}

int BatRtSohCalc::read_mea()
{
	map_dev2soh.clear();

	for (int i = 0; i < m_bms.size(); ++i)
	{
		read_mea(map_dev2soh, m_bms[i].id, -1, "soh");
	}

	for (int i = 0; i < m_batcluster.size(); ++i)
	{
		read_mea(map_dev2soh, m_batcluster[i].id, -1, "soh");
	}

	for (int i = 0; i < m_batcell.size(); ++i)
	{
		read_mea(map_dev2soh, m_batcell[i].id, -1, "soh");
	}

	return 0;
}

int BatRtSohCalc::read_mea(map<int64_t, pnt_ana> &pnts, int64_t container, int64_t real_container/* = -1*/, const char* postfix /*= ""*/)
{
	vector<pnt_ana> vec;

	BatRtCalc::read_mea(vec, container, postfix);

	int64_t c = container;
	if (real_container > 0)
		c = real_container;

	for (int i = 0; i < vec.size(); ++i)
	{
		pnts[c] = vec[i];
	}

	return vec.size();
}

int BatRtSohCalc::write_mea(const map<int64_t, double> &v)
{
	log_info("BatRtSohCalc, write soh measuration");

	int ret_code = 0;

	map<int64_t, double>::const_iterator it = v.cbegin();
	for (; it != v.cend(); ++it)
	{
		int64_t st_id = it->first;
		double value = it->second;

		if (map_st2soh.count(st_id) == 0)
		{
			log_error("BatRtSohCalc, station_bess not have soh pnt, station id = %lld", st_id);
			continue;
		}

		const pnt_ana &p = map_st2soh[st_id];
		if (p.value == value)
		{
			log_debug("BatRtSohCalc, station soh value same, station id = %lld, value = %lf", st_id, value);
			continue;
		}

		char str_tag[128];
		memset(str_tag, 0, 128);
		eco::nsprintf(str_tag, 128, "pnt_ana.%lld.value", p.id);
		ret_code = m_opt_comm.Manset("MT_OPT_PINHIBIT_REF", 1, str_tag, value, "", "bat_server");
		if (ret_code < 0)
		{
			log_error("BatRtSohCalc, opt_comm manset error，ret_code=%d, tag=%s, value=%lf", ret_code, str_tag, value);
			continue;
		}
	}

	return 0;
}

BatRtCellEvaluate::BatRtCellEvaluate()
{
}

BatRtCellEvaluate::~BatRtCellEvaluate()
{
}

int BatRtCellEvaluate::calc()
{
	log_info("BatRtCellEvaluate, calc start");

	int ret_code = 0;

	time_t t = time(0);
	char t_ch[MAX_DATETIME_LEN];
	strftime(t_ch, sizeof(t_ch), "%Y-%m-%d %H:%M:%S", localtime(&t));

	clear();

	ret_code = init();
	if (ret_code < 0)
	{
		log_error("BatRtCellEvaluate, init error! ret_code = %d", ret_code);
		return ret_code;
	}

	ret_code = read_data();
	if (ret_code < 0)
	{
		log_error("BatRtCellEvaluate, read data error! ret_code = %d", ret_code);
		return ret_code;
	}

	map<string, map<int, double>> bms_pfx2idx; /// 电池堆测类型-idx-值 Meong 2022-8-25 ///
	map<string, double> bms_pfx2var;// 电池堆各量测类型方差值
	map<string, double> bms_score_ratio;
	vector<double> bms_score;

	for (int i = 0; i < m_bms.size(); ++i)
	{
		bms &b = m_bms[i];

		const vector<pnt_ana> &bms_pnts = map_dev2meas[b.id];
		for (int j = 0; j < bms_pnts.size(); ++j)
		{
			int64_t id = bms_pnts[j].id;
			const char* postfix = bms_pnts[j].postfix;
			const double &v = bms_pnts[j].value;

			if (BatConfig::get().get_ratio(string("bms.") + postfix) != 0)
			{
				/// 只有配置了ratio的电池堆量测才纳入计算 Meong 2022-9-1 ///
				bms_pfx2idx[postfix][i] = v;
			}
		}
		
		/// bms下有电池簇则对下属电池簇进行分析 Meong 2022-9-1 ///
		if (map_bms2cluster.count(b.id) > 0)
		{
			map<string, map<int, double>> cluster_pfx2idx; /// 电池簇量测类型/方差-idx-值 Meong 2022-8-25 ///
			map<string, double> cluster_pfx2var;// 电池堆下电池簇各量测类型方差值
			map<string, double> cluster_score_ratio;
			vector<double> cluster_score;

			const vector<int> &cluster_idx = map_bms2cluster[b.id];
			for (int j = 0; j < cluster_idx.size(); ++j)
			{
				int clu_idx = cluster_idx[j];
				batcluster &cluster = m_batcluster[clu_idx];

				map<string, map<int, double>> cell_pfx2idx; /// 电芯量测类型-idx-值 Meong 2022-8-25 ///
				map<string, double> cell_pfx2var;// 电池簇下电芯各量测类型方差值
				map<string, double> cell_score_ratio;
				vector<double> cell_score;

				/// 从电池簇的量测点里找出电池簇本身的量测以及下属的电芯的量测 Meong 2022-8-25 ///
				const vector<pnt_ana> &cluster_pnts = map_dev2meas[cluster.id];
				for (int k = 0; k < cluster_pnts.size(); ++k)
				{
					int64_t id = cluster_pnts[k].id;
					const char* postfix = cluster_pnts[k].postfix;
					const double &v = cluster_pnts[k].value;

					if (strncmp(postfix, "Batcell", 7) == 0 || strncmp(postfix, "BatCell", 7) == 0)
					{
						string p = StringUtils::RemoveString(postfix, "Batcell");
						p = StringUtils::RemoveString(p, "BatCell");
						vector<string> l = StringUtils::split(p, '-');
						if (l.size() != 2)
						{
							log_warn("BatRtCellEvaluate, pnt postfix is invalid, pnt id=%lld, postfix=%s", id, postfix);
							continue;
						}

						int idx = atoi(l[1].c_str());
						if (idx == 0)
						{
							log_warn("BatRtCellEvaluate, pnt postfix format is invalid, pnt id=%lld, postfix=%s", id, postfix);
							continue;
						}
						cell_pfx2idx[l[0]][idx] = v;
					}
					else if (BatConfig::get().get_ratio(string("batcluster.") + postfix) != 0)
					{
						/// 只有配置了ratio的电池簇才纳入计算 Meong 2022-9-1 ///
						cluster_pfx2idx[postfix][clu_idx] = v;
					}
				}

				/// 建立电芯模型且把量测挂的电芯上的情况暂时不考虑 Meong 2022-8-25 ///
				/// 电池簇下如有电芯则对下属的电芯进行分析 Meong 2022-9-1 ///
				if (cell_pfx2idx.size() > 0)
				{
					/// 读取各计算比例因子并计算方差及综合评分 Meong 2022-8-25 ///
					map<string, map<int, double>>::iterator it = cell_pfx2idx.begin();
					int max_count = 0;

					map<int, int> idx2eva_cell;
					for (; it != cell_pfx2idx.end(); ++it)
					{
						const string &postfix = it->first;
						map<int, double> &map_idx2val = it->second;

						/// 查找电芯分值占比 Meong 2022-8-31 ///
						cell_score_ratio[postfix] = BatConfig::get().get_ratio(string("batcell.") + postfix);

						/// 计算电芯不同量测标准差 Meong 2022-8-31 ///
						double var = BatMath::coefficient_variation(map_idx2val);
						cell_pfx2var[postfix] = var;

						/// 变异系数值判断并预警 Meong 2023-7-6 ///
						double threshold = BatConfig::get().get_variation(string("batcell.") + postfix);
						if (var > threshold)
						{
							ret_code = BatPrewarning::get().insert_warning(b.id, cluster.id, -1, PWVariation, string("Batcell") + postfix, var, threshold);
							if (ret_code < 0)
							{
								log_error("BatRtCellEvaluate, insert cluster prewarning data error, bms id=%lld, cluster id=%lld", b.id, cluster.id);
							}
						}
					}

					cell_score = data_eval(cell_pfx2idx, cell_score_ratio);
					if (cell_score.size() != 0)
					{
						double threshold = BatConfig::get().get_score("batcell");

						int data_size = cell_pfx2idx.begin()->second.size();
						for (int k = 0; k < data_size; ++k)
						{
							map<string, double> pfx2val;
							int idx = -1;
							for (map<string, map<int, double>>::iterator it_clu = cell_pfx2idx.begin(); it_clu != cell_pfx2idx.end(); ++it_clu)
							{
								const string& postfix = it_clu->first;
								map<int, double> &m = it_clu->second;
								double v = get_map_value(m, k, 0);
								idx = get_map_key(m, k, idx);
								pfx2val[postfix] = v;
							}

							if (idx == -1)
							{
								log_warn("BatRtCellEvaluate, cell idx=-1, cluster id=%lld, k:%d", cluster.id, k);
								continue;
							}

							bat_eva_cell eva_cell;
							eva_cell.id = cluster.id * 1000 + idx;
							eva_cell.container_id = cluster.id;
							eco::nsprintf(eva_cell.description, MAX_DESCRIPTION_LEN, "BatCell-%d", idx);
							eco::nsprintf(eva_cell.update_dt, MAX_DATETIME_LEN, t_ch);
							eva_cell.score = cell_score[k];

							map<string, double>::iterator it = pfx2val.begin();
							while (it != pfx2val.end())
							{
								set_mea_val(eva_cell, it->first.c_str(), it->second);
								++it;
							}
							m_result_cell.push_back(eva_cell);

							/// 综合评分数值判断并预警 Meong 2023-7-6 ///
							if (eva_cell.score < threshold)
							{
								ret_code = BatPrewarning::get().insert_warning(b.id, cluster.id, idx, PWVarScore, "", eva_cell.score, threshold);
								if (ret_code < 0)
								{
									log_error("BatRtCellEvaluate, insert cell score prewarning data error, bms id=%lld, cluster id=%lld, cell idx=%d", b.id, cluster.id, idx);
								}
							}
						}
					}					

					/// 电池簇下属电芯方差作为电池簇指标 Meong 2022-8-31 ///
					for (map<string, double>::iterator it = cell_pfx2var.begin(); it != cell_pfx2var.end(); ++it)
					{
						const string &postfix = it->first;
						const double &var = it->second;

						string var_pfx = string("var.") + postfix;
						cluster_pfx2idx[var_pfx][clu_idx] = var;
					}
				}
			}

			double cluster_ratio = 0;
			int cluster_var_count = 0;
			for (map<string, map<int, double>>::iterator it = cluster_pfx2idx.begin(); it != cluster_pfx2idx.end(); ++it)
			{
				const string& postfix = it->first;
				map<int, double> &map_idx2val = it->second;

				if (strncmp(postfix.c_str(), "var.", 4) == 0)
				{
					/// 电芯方差个数统计 Meong 2022-9-1 ///
					++cluster_var_count;
					continue;
				}
				else
				{
					/// 查找电池簇分值占比 Meong 2022-8-31 ///
					double r = BatConfig::get().get_ratio(string("batcluster.") + postfix);;
					cluster_score_ratio[postfix] = r;
					cluster_ratio += r;

					double var = BatMath::coefficient_variation(map_idx2val);
					cluster_pfx2var[postfix] = var;

					/// 变异系数值判断并预警 Meong 2023-7-6 ///
					double threshold = BatConfig::get().get_variation(string("batcluster.") + postfix);
					if (var > threshold)
					{
						ret_code = BatPrewarning::get().insert_warning(b.id, -1, -1, PWVariation, postfix, var, threshold);
						if (ret_code < 0)
						{
							log_error("BatRtCellEvaluate, insert bms prewarning data error, bms id=%lld", b.id);
						}
					}
				}
			}

			if (cluster_var_count == 0)
				cluster_ratio = 0;
			else
				cluster_ratio = (DEFAULT_VAR_RATIO * cluster_ratio) / cluster_var_count;

			/// 电池簇下的电芯方差占比50% Meong 2022-9-1 ///
			for (map<string, map<int, double>>::iterator it = cluster_pfx2idx.begin(); it != cluster_pfx2idx.end(); ++it)
			{
				const string& postfix = it->first;
				if (strncmp(postfix.c_str(), "var.", 4) == 0)
				{
					/// 电芯方差占比 Meong 2022-9-1 ///
					cluster_score_ratio[postfix] = cluster_ratio;
				}
			}

			cluster_score = data_eval(cluster_pfx2idx, cluster_score_ratio);
			if (cluster_score.size() != 0)
			{
				double threshold = BatConfig::get().get_score("batcluster");

				int data_size = cluster_pfx2idx.begin()->second.size();
				for (int i = 0; i < data_size; ++i)
				{
					map<string, double> pfx2val;
					int idx = -1;
					for (map<string, map<int, double>>::iterator it_clu = cluster_pfx2idx.begin(); it_clu != cluster_pfx2idx.end(); ++it_clu)
					{
						const string& postfix = it_clu->first;
						map<int, double> &m = it_clu->second;
						double v = get_map_value(m, i, 0);
						idx = get_map_key(m, i, idx);
						pfx2val[postfix] = v;
					}

					if (idx == -1)
					{
						log_warn("BatRtCellEvaluate, cluster idx=-1, bms id=%lld, i:%d", b.id, i);
						continue;
					}

					batcluster &cluster = m_batcluster[idx];
					bat_eva_cluster eva_cluster;

					eva_cluster.id = cluster.id;
					eva_cluster.container_id = cluster.container_id;
					strncpy(eva_cluster.description, cluster.description, MAX_DESCRIPTION_LEN);
					eco::nsprintf(eva_cluster.update_dt, MAX_DATETIME_LEN, t_ch);
					eva_cluster.score = cluster_score[i];

					log_info("BatRtCellEvaluate, batcluster evaluate, name: %s, id: %lld, score: %lf", cluster.description, cluster.id, cluster_score[i]);

					map<string, double>::iterator it = pfx2val.begin();
					while (it != pfx2val.end())
					{
						set_mea_val(eva_cluster, it->first.c_str(), it->second);
						set_calc_var(eva_cluster, it->first.c_str(), it->second);
						++it;
					}
					m_result_cluster.push_back(eva_cluster);

					/// 综合评分数值判断并预警 Meong 2023-7-6 ///
					if (eva_cluster.score < threshold)
					{
						ret_code = BatPrewarning::get().insert_warning(b.id, cluster.id, -1, PWVarScore, "", eva_cluster.score, threshold);
						if (ret_code < 0)
						{
							log_error("BatRtCellEvaluate, insert cluster score prewarning data error, bms id=%lld, cluster id=%lld", b.id, cluster.id);
						}
					}
				}
			}			

			/// 电池堆下属电池簇方差作为电池堆指标 Meong 2022-8-31 ///
			for (map<string, double>::iterator it = cluster_pfx2var.begin(); it != cluster_pfx2var.end(); ++it)
			{
				const string &postfix = it->first;
				const double &var = it->second;

				string var_pfx = string("var.") + postfix;
				bms_pfx2idx[var_pfx][i] = var;
			}
		}
	}

	double bms_ratio = 0;
	int bms_var_count = 0;
	for (map<string, map<int, double>>::iterator it = bms_pfx2idx.begin(); it != bms_pfx2idx.end(); ++it)
	{
		const string& postfix = it->first;
		map<int, double> &map_idx2val = it->second;

		if (strncmp(postfix.c_str(), "var.", 4) == 0)
		{
			/// 电芯方差个数统计 Meong 2022-9-1 ///
			++bms_var_count;
			continue;
		}
		else
		{
			/// 查找电池簇分值占比 Meong 2022-8-31 ///
			double r = BatConfig::get().get_ratio(string("bms.") + postfix);;
			bms_score_ratio[postfix] = r;
			bms_ratio += r;
			
			double var = BatMath::coefficient_variation(map_idx2val);
			bms_pfx2var[postfix] = var;
			
			/// 变异系数值判断并预警 Meong 2023-7-6 ///
			double threshold = BatConfig::get().get_variation(string("bms.") + postfix);
			if (var > threshold)
			{
				ret_code = BatPrewarning::get().insert_warning(-1, -1, -1, PWVariation, postfix, var, threshold);
				if (ret_code < 0)
				{
					log_error("BatRtCellEvaluate, insert bms prewarning data error");
				}
			}
		}
	}

	if (bms_var_count == 0)
		bms_ratio = 0;
	else
		bms_ratio /= (DEFAULT_VAR_RATIO * bms_var_count);

	/// 电池堆下的电池簇方差占比50% Meong 2022-9-1 ///
	for (map<string, map<int, double>>::iterator it = bms_pfx2idx.begin(); it != bms_pfx2idx.end(); ++it)
	{
		const string& postfix = it->first;
		if (strncmp(postfix.c_str(), "var.", 4) == 0)
		{
			/// 电芯方差占比 Meong 2022-9-1 ///
			bms_score_ratio[postfix] = bms_ratio;
		}
	}

	bms_score = data_eval(bms_pfx2idx, bms_score_ratio);
	if (bms_score.size() != 0)
	{
		double threshold = BatConfig::get().get_score("bms");

		int data_size = bms_pfx2idx.begin()->second.size();
		for (int i = 0; i < data_size; ++i)
		{
			map<string, double> pfx2val;
			int idx = -1;
			for (map<string, map<int, double>>::iterator it_clu = bms_pfx2idx.begin(); it_clu != bms_pfx2idx.end(); ++it_clu)
			{
				const string& postfix = it_clu->first;
				map<int, double> &m = it_clu->second;
				double v = get_map_value(m, i, 0);
				idx = get_map_key(m, i, idx);
				pfx2val[postfix] = v;
			}

			if (idx == -1)
			{
				log_warn("");
				continue;
			}

			bms &b = m_bms[idx];
			bat_eva_bms eva_bms;

			eva_bms.id = b.id;
			eva_bms.container_id = b.container_id;
			strncpy(eva_bms.description, b.description, MAX_DESCRIPTION_LEN);
			eco::nsprintf(eva_bms.update_dt, MAX_DATETIME_LEN, t_ch);
			eva_bms.score = bms_score[i];

			log_info("BatRtCellEvaluate, bms evaluate, name: %s, id: %lld, score: %lf", b.description, b.id, bms_score[i]);

			map<string, double>::iterator it = pfx2val.begin();
			while (it != pfx2val.end())
			{
				set_mea_val(eva_bms, it->first.c_str(), it->second);
				set_calc_var(eva_bms, it->first.c_str(), it->second);
				++it;
			}
			m_result_bms.push_back(eva_bms);

			/// 综合评分数值判断并预警 Meong 2023-7-6 ///
			if (eva_bms.score < threshold)
			{
				ret_code = BatPrewarning::get().insert_warning(b.id, -1, -1, PWVarScore, "", eva_bms.score, threshold);
				if (ret_code < 0)
				{
					log_error("BatRtCellEvaluate, insert bms score prewarning data error, bms id=%lld", b.id);
				}
			}
		}
	}

	double st_score = BatMath::mean(bms_score);
	log_info("BatRtCellEvaluate, station score: %lf", st_score);

	write_data();
	ret_code = write_taosdb();
	if (ret_code < 0)
	{
		log_error("BatRtCellEvaluate, write to taosdb error, ret_code = %d", ret_code);
		return ret_code;
	}

	log_info("BatRtCellEvaluate, calc finish");
	return 0;
}

int BatRtCellEvaluate::init()
{
	int ret_code = 0;

	ret_code = m_bat_rtdb.Init();
	if (ret_code < 0)
	{
		log_error("BatRtCellEvaluate, rtdb init error! ret_code = %d", ret_code);
		return ret_code;
	}

	std::string ctx_app(EProc::GetCtxApp());
	ret_code = m_bat_rtdb.Open(ctx_app.c_str());
	if (ret_code < 0)
	{
		log_error("BatRtCellEvaluate, rtdb open error! ret_code = %d", ret_code);
		return ret_code;
	}

	ret_code = create_bat_rtdb();
	if (ret_code < 0)
	{
		log_error("BatRtCellEvaluate, create bat rtdb error! ret_code = %d", ret_code);
		return ret_code;
	}

	ret_code = create_taosdb();
	if (ret_code < 0)
	{
		log_error("BatRtCellEvaluate, create taosdb error! ret_code = %d", ret_code);
		return ret_code;
	}

	return 0;
}

int BatRtCellEvaluate::create_bat_rtdb()
{
	int ret_code = 0;

	if (!m_bat_rtdb.IsTableExist("bat_eva_bms"))
	{
		ret_code = m_bat_rtdb.CreateTableByStruct<bat_eva_bms>();
		if (ret_code < 0) {
			log_error("BatRtCellEvaluate, create rtdb table bat_eva_bms error! ret_code = %d", ret_code);
			return ret_code;
		}
	}

	if (!m_bat_rtdb.IsTableExist("bat_eva_cluster"))
	{
		ret_code = m_bat_rtdb.CreateTableByStruct<bat_eva_cluster>();
		if (ret_code < 0) {
			log_error("BatRtCellEvaluate, create rtdb table bat_eva_cluster error! ret_code = %d", ret_code);
			return ret_code;
		}
	}

	if (!m_bat_rtdb.IsTableExist("bat_eva_cell"))
	{
		ret_code = m_bat_rtdb.CreateTableByStruct<bat_eva_cell>();
		if (ret_code < 0) {
			log_error("BatRtCellEvaluate, create rtdb table bat_eva_cell error! ret_code = %d", ret_code);
			return ret_code;
		}
	}

	return 0;
}

int BatRtCellEvaluate::create_taosdb()
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
	log_error("BatRtCellEvaluate, create bat_analysis db error");
	return -1;
	}*/

	// use database
	memset(sql, 0, sql_len);
	eco::nsprintf(sql, sql_len, "USE %s;", db_name.c_str());
	if (taos.modify(sql) < 0)
	{
		log_error("BatRtCellEvaluate, use bat_analysis db error");
		return -1;
	}

	// create stable rt_eva_bms
	memset(sql, 0, sql_len);
	eco::nsprintf(sql, sql_len, "CREATE STABLE IF NOT EXISTS %s.rt_eva_bms \
								(ts TIMESTAMP, v DOUBLE, i DOUBLE, t DOUBLE, soc DOUBLE, soh DOUBLE, \
								var_v DOUBLE, var_i DOUBLE, var_t DOUBLE, var_soc DOUBLE, var_soh DOUBLE, score DOUBLE) \
								tags (id BIGINT, name BINARY(100), container_id BIGINT, container_tablename BINARY(32));",
								db_name.c_str());
	if (taos.modify(sql) < 0)
	{
		log_error("BatRtCellEvaluate, create stable %s.rt_eva_bms error", db_name.c_str());
		return -1;
	}

	// create stable rt_eva_cluster
	memset(sql, 0, sql_len);
	eco::nsprintf(sql, sql_len, "CREATE STABLE IF NOT EXISTS %s.rt_eva_cluster \
								(ts TIMESTAMP, v DOUBLE, i DOUBLE, t DOUBLE, soc DOUBLE, soh DOUBLE, \
								var_v DOUBLE, var_i DOUBLE, var_t DOUBLE, var_soc DOUBLE, var_soh DOUBLE, score DOUBLE) \
								tags (id BIGINT, name BINARY(100), container_id BIGINT, container_tablename BINARY(32));",
								db_name.c_str());
	if (taos.modify(sql) < 0)
	{
		log_error("BatRtCellEvaluate, create stable %s.rt_eva_cluster error", db_name.c_str());
		return -1;
	}

	// create stable rt_eva_cell
	memset(sql, 0, sql_len);
	eco::nsprintf(sql, sql_len, "CREATE STABLE IF NOT EXISTS %s.rt_eva_cell \
								(ts TIMESTAMP, v DOUBLE, i DOUBLE, t DOUBLE, soc DOUBLE, soh DOUBLE, score DOUBLE) \
								tags (id BIGINT, name BINARY(100), container_id BIGINT, container_tablename BINARY(32));",
								db_name.c_str());
	if (taos.modify(sql) < 0)
	{
		log_error("BatRtCellEvaluate, create stable %s.rt_eva_cell error", db_name.c_str());
		return -1;
	}

	return 0;
}

void BatRtCellEvaluate::clear()
{
	m_bms.clear();
	m_batcluster.clear();
	m_batcell.clear();

	map_dev2meas.clear();
	map_dev2meascore.clear();
	map_dev2score.clear();

	map_bms2idx.clear(), map_cluster2idx.clear(), map_cell2idx.clear();
	map_bms2cluster.clear(), map_cluster2cell.clear();
	map_cluster2bms.clear(), map_cell2cluster.clear();

	m_result_bms.clear();
	m_result_cluster.clear();
	m_result_cell.clear();
}

int BatRtCellEvaluate::read_data()
{
	log_info("BatRtCellEvaluate, read data");

	int ret_code = 0;

	/// 读所有BMS Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_bms);
	if (ret_code < 0)
	{
		log_error("BatRtCellEvaluate, read bms error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 读所有电池簇 Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_batcluster);
	if (ret_code < 0)
	{
		log_error("BatRtCellEvaluate, read batcluster error, ret_code = %d", ret_code);
		return ret_code;
	}

	/// 读所有电芯 Meong 2022-8-22 ///
	ret_code = m_public_rtdb.Get(m_batcell);
	if (ret_code < 0)
	{
		log_error("BatRtCellEvaluate, read batcell error, ret_code = %d", ret_code);
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
			log_warn("BatRtCellEvaluate, batcluster relation_id is invalid, id = %lld, relation_id = %lld", obj.id, obj.relation_id);
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
			log_warn("BatRtCellEvaluate, batcell relation_id is invalid, id = %lld, relation_id = %lld", obj.id, obj.relation_id);
			continue;
		}

		map_cell2cluster[obj.id] = map_cluster2idx[obj.relation_id];
		map_cluster2cell[obj.relation_id].push_back(i);
	}
	
	/// 读量测 Meong 2022-8-22 ///
	read_mea();

	return 0;
}

int BatRtCellEvaluate::read_mea()
{
	vector<pnt_ana> pnts;
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

vector<double> BatRtCellEvaluate::data_eval(map<string, map<int, double>> &pfx2idxval, map<string, double> &pfx2ratio)
{
	vector<double> score;

	if (pfx2idxval.size() == 0 || pfx2ratio.size() == 0)
		return score;

	/// 1 动态序列对齐 Meong 2022-8-31 ///
	dynamic_seq_warping(pfx2idxval);

	/// 2 数据偏差与几何距离计算 Meong 2022-8-31 ///
	map<string, vector<double>> pfx2diff;
	map<string, map<int, double>>::iterator it;
	for (it = pfx2idxval.begin(); it != pfx2idxval.end(); ++it)
	{
		const string &postfix = it->first;
		map<int, double> &map_idx2val = it->second;
		vector<double> &vec_val = pfx2diff[postfix];

		double m = BatMath::mean(map_idx2val);
		for (map<int, double>::iterator it_cell = map_idx2val.begin(); it_cell != map_idx2val.end(); ++it_cell)
		{
			/// 离散度用变异系数评判 Meong 2023-5-29 ///
			if (strncmp(postfix.c_str(), "var.", 4) == 0)
			{
				vec_val.push_back((1 - it_cell->second) * 100);
			}
			else
			{
				double v = it_cell->second;
				if (m != 0)
					vec_val.push_back((m - fabs(it_cell->second - m)) / m * 100);
				else
					vec_val.push_back(100);
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

int BatRtCellEvaluate::write_data()
{
	log_info("BatRtCellEvaluate, write result data to rtdb");

	BatRtCalc::write_rdbdata(m_bat_rtdb, m_result_bms, true);
	BatRtCalc::write_rdbdata(m_bat_rtdb, m_result_cluster, true);
	BatRtCalc::write_rdbdata(m_bat_rtdb, m_result_cell, true);

	return 0;
}

int BatRtCellEvaluate::write_taosdb()
{
	if (!BatConfig::get().get_taos_enable())
		return 0;

	log_info("BatRtCellEvaluate, write result data to taosdb");

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
			const bat_eva_bms &b = m_result_bms[i];
			char s[sql_len] = { 0 };
			eco::nsprintf(s, sql_len, "bms_%lld USING %s.rt_eva_bms TAGS(%lld, '%s', %lld, '') "\
									  	"VALUES ('%s', %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf)",
										b.id, db_name.c_str(), b.id, b.description, b.container_id, 
										b.update_dt, b.v, b.i, b.t, b.soc, b.soh, b.var_v, b.var_i, b.var_t, b.var_soc, b.var_soh, b.score);
			slist[i] = s;
		}
		sql += StringUtils::join(slist, ' ');
		sql += ";";
		ret_code = taos.modify(sql.c_str());
		if (ret_code < 0)
		{
			log_error("BatRtCellEvaluate, insert bms data to %s.rt_eva_bms error, ret_code = %d", db_name.c_str(), ret_code);
		}
	}

	if (m_result_cluster.size() > 0)
	{
		string sql = "INSERT INTO ";
		vector<string> slist(m_result_cluster.size());
		for (int i = 0; i < m_result_cluster.size(); ++i)
		{
			const bat_eva_cluster &b = m_result_cluster[i];
			char s[sql_len] = { 0 };
			eco::nsprintf(s, sql_len, "batcluster_%lld USING %s.rt_eva_cluster TAGS(%lld, '%s', %lld, '') "\
									  	"VALUES ('%s', %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf)",
										b.id, db_name.c_str(), b.id, b.description, b.container_id, 
										b.update_dt, b.v, b.i, b.t, b.soc, b.soh, b.var_v, b.var_i, b.var_t, b.var_soc, b.var_soh, b.score);
			slist[i] = s;
		}
		sql += StringUtils::join(slist, ' ');
		sql += ";";
		ret_code = taos.modify(sql.c_str());
		if (ret_code < 0)
		{
			log_error("BatRtCellEvaluate, insert batcluster data to %s.rt_eva_cluster error, ret_code = %d", db_name.c_str(), ret_code);
		}
	}
	
	/// todo 数据量过大，电芯数据暂不写入 Meong 2023-5-30 ///

	return 0;
}
