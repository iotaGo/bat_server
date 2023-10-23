#include "bat_rtdbcalc.h"
#include "bat_math.h"
#include "bat_config.h"
#include "sys/eproc.h"
#include <math.h>


BatRtVpp::BatRtVpp()
{
}

BatRtVpp::~BatRtVpp()
{
}

int BatRtVpp::calc()
{
	log_info("BatRtdbVpp, calc start");

	int ret_code = 0;
	ret_code = init();
	if (ret_code < 0)
	{
		log_error("BatRtdbVpp, init error! ret_code = %d", ret_code);
		return ret_code;
	}

	ret_code = read_data();
	if (ret_code < 0)
	{
		log_error("BatRtdbVpp, read data error! ret_code = %d", ret_code);
		return ret_code;
	}

	/// 7. 计算厂站等效参数 Meong 2022-9-13 ///
	calc_station();

	/// 8. 计算机组等效参数 Meong 2022-9-13 ///
	calc_generator();

	/// 9. 输出计算结果 Meong 2022-9-15 ///
	write_data();

	log_info("BatRtdbVpp, calc finish");
	return 0;
}

int BatRtVpp::init()
{
	log_info("BatRtdbVpp init");

	int ret_code = 0;

	ret_code = m_vpp_rtdb.Init();
	if (ret_code < 0)
	{
		log_error("BatRtdbVpp, rtdb init error! ret_code = %d", ret_code);
		return ret_code;
	}

	std::string ctx_app(EProc::GetCtxApp());
	ret_code = m_vpp_rtdb.Open(ctx_app.c_str());
	if (ret_code < 0)
	{
		log_error("BatRtdbVpp, rtdb open error! ret_code = %d", ret_code);
		return ret_code;
	}

	ret_code = create_vpp_rtdb();
	if (ret_code < 0)
	{
		log_error("BatRtdbVpp, create vpp rtdb error! ret_code = %d", ret_code);
		return ret_code;
	}

	return 0;
}

int BatRtVpp::create_vpp_rtdb()
{
	log_info("BatRtdbVpp create vpp rtdb");

	int ret_code = 0;

	if (!m_vpp_rtdb.IsTableExist("vpp_station"))
	{
		ret_code = m_vpp_rtdb.CreateTableByStruct<vpp_station>();
		if (ret_code < 0) {
			log_error("BatRtdbVpp, create rtdb table vpp_station error! ret_code = %d", ret_code);
			return ret_code;
		}
	}

	if (!m_vpp_rtdb.IsTableExist("vpp_generator"))
	{
		ret_code = m_vpp_rtdb.CreateTableByStruct<vpp_generator>();
		if (ret_code < 0) {
			log_error("BatRtdbVpp, create rtdb table vpp_generator error! ret_code = %d", ret_code);
			return ret_code;
		}
	}

	return 0;
}

int BatRtVpp::read_data()
{
	log_info("BatRtdbVpp read data");

	int ret_code = 0;

	clear();

	ret_code = m_public_rtdb.Get(m_station);
	if (ret_code < 0)
	{
		log_error("BatRtdbVpp, read station error! ret_code = %d", ret_code);
		return -1;
	}
	creat_index(m_station, map_stidx);

	ret_code = m_public_rtdb.Get(m_pqvc);
	if (ret_code < 0)
	{
		log_error("BatRtdbVpp, read pqvc error! ret_code = %d", ret_code);
		return -1;
	}
	creat_index(m_pqvc, map_pqvcidx);

	ret_code = m_vpp_rtdb.Get(m_agvc_station);
	if (ret_code < 0)
	{
		log_error("BatRtdbVpp, read agvc station error! ret_code = %d", ret_code);
		return -1;
	}
	creat_index(m_agvc_station, map_agvcstidx);

	ret_code = m_vpp_rtdb.Get(m_agvc_pqvc);
	if (ret_code < 0)
	{
		log_error("BatRtdbVpp, read agvc pqvc error! ret_code = %d", ret_code);
		return -1;
	}
	creat_index(m_agvc_pqvc, map_agvcpqvcidx);

	return 0;
}

void BatRtVpp::clear()
{
	m_station.clear();
	m_pqvc.clear();
	m_agvc_station.clear();
	m_agvc_pqvc.clear();
	m_result_st.clear();
	m_result_gen.clear();

	map_stidx.clear();
	map_pqvcidx.clear();
	map_agvcstidx.clear();
	map_agvcpqvcidx.clear();
}

int BatRtVpp::calc_station()
{
	log_info("BatRtdbVpp, calc station");

	int ret_code = 0;
	for (int i = 0; i < m_station.size(); ++i)
	{
		const station_bess &st = m_station[i];
		if (map_agvcstidx.count(st.id) == 0)
		{
			log_error("staion bess not match agvc station, id=%lld", st.id);
			continue;
		}

		const agvc_station_bess &agvc_st = m_agvc_station[map_agvcstidx[st.id]];
		int64_t dev_id = read_tpl_dev(st.id, "bess_measuration");
		if (dev_id < 0)
		{
			log_error("read station tpl dev error, id = %lld, tpl_name = bess_measuration", st.id);
			continue;
		}

		map<string, int> map_anapfx2idx, map_digpfx2idx;
		vector<pnt_ana> pnt_anas;
		vector<pnt_dig> pnt_digs;

		read_mea(pnt_anas, dev_id);
		creat_pnt_index(pnt_anas, map_anapfx2idx);

		read_mea(pnt_digs, dev_id);
		creat_pnt_index(pnt_digs, map_digpfx2idx);

		/// 参数计算 Meong 2022-9-13 ///
		vpp_station vpp_st;
		vpp_st.id = st.id;
		ECO_NCPY_CHAR_ARRAY(vpp_st.description, st.description);

		// 有功
		if (map_anapfx2idx.count("p") != 0)
			vpp_st.p = pnt_anas[map_anapfx2idx["p"]].value;
		// 无功
		if (map_anapfx2idx.count("q") != 0)
			vpp_st.q = pnt_anas[map_anapfx2idx["q"]].value;
		// 电压
		if (map_anapfx2idx.count("uab") != 0)
			vpp_st.v = pnt_anas[map_anapfx2idx["uab"]].value;
		// 电流
		if (map_anapfx2idx.count("ia") != 0)
			vpp_st.i = pnt_anas[map_anapfx2idx["ia"]].value;
		// 频率
		if (map_anapfx2idx.count("freq") != 0)
			vpp_st.f = pnt_anas[map_anapfx2idx["freq"]].value;
		// 有功上调节功率
		if (map_anapfx2idx.count("pdischargemax") != 0 && map_anapfx2idx.count("p") != 0)
		{
			double v = pnt_anas[map_anapfx2idx["pdischargemax"]].value - pnt_anas[map_anapfx2idx["p"]].value;
			vpp_st.p_incre = v;
			// 有功上调节闭锁
			if (v < DEFAULT_VAR_PERCISION)
				vpp_st.p_incre_lock = 1;
		}
		// 有功下调节功率
		if (map_anapfx2idx.count("pchargemax") != 0 && map_anapfx2idx.count("p") != 0)
		{
			double v = pnt_anas[map_anapfx2idx["pchargemax"]].value + pnt_anas[map_anapfx2idx["p"]].value;
			vpp_st.p_decre = v;
			// 有功下调节闭锁
			if (v < DEFAULT_VAR_PERCISION)
				vpp_st.p_decre_lock = 1;
		}
		// 有功调节速率，默认1s额定功率调节速率
		vpp_st.p_ratio = st.rated_p;
		// 无功上调节功率
		if (map_anapfx2idx.count("qincre") != 0)
			vpp_st.q_incre = pnt_anas[map_anapfx2idx["qincre"]].value;
		// 无功下调节功率
		if (map_anapfx2idx.count("qdecre") != 0)
			vpp_st.q_decre = pnt_anas[map_anapfx2idx["qdecre"]].value;
		// 无功上调节闭锁
		if (map_digpfx2idx.count("avc_up_lock") != 0)
			vpp_st.q_incre_lock = pnt_digs[map_digpfx2idx["avc_up_lock"]].value;
		// 无功下调节闭锁
		if (map_digpfx2idx.count("avc_dn_lock") != 0)
			vpp_st.q_decre_lock = pnt_digs[map_digpfx2idx["avc_dn_lock"]].value;

		// 无功调节速率，默认1s调节到无功调整限值
		vpp_st.q_ratio = st.adjq_uplimit;
		// 整站运行状态
		if (map_anapfx2idx.count("runstate") != 0)
		{
			vpp_st.run_state = (int)pnt_anas[map_anapfx2idx["runstate"]].value;
			// 整站告警状态
			vpp_st.alarm_state = vpp_st.run_state == AgvcAlarmStatus ? 1 : 0;
			// 整站故障状态
			vpp_st.fault_state = vpp_st.run_state == AgvcFaultStatus ? 1 : 0;
		}
		// 有功指令异常闭锁
		vpp_st.p_order_lock = 0;
		// 无功指令异常闭锁
		if (map_digpfx2idx.count("avc_command_lock") != 0)
			vpp_st.q_order_lock = pnt_digs[map_digpfx2idx["avc_command_lock"]].value;
		// 电网紧急控制状态
		if (map_digpfx2idx.count("ywhlock") != 0)
			vpp_st.emergency = pnt_digs[map_digpfx2idx["ywhlock"]].value;

		m_result_st.push_back(vpp_st);
	}

	log_info("BatRtdbVpp, calc station finished");
	return 0;
}

int BatRtVpp::calc_generator()
{
	log_info("BatRtdbVpp, calc generator");

	for (int i = 0; i < m_pqvc.size(); ++i)
	{
		const pqvc_connectpoint &pqvc = m_pqvc[i];
		if (map_agvcpqvcidx.count(pqvc.id) == 0)
		{
			log_error("pqvc not match agvc pqvc, id=%lld", pqvc.id);
			continue;
		}

		const agvc_pqvc_connectpoint &agvc_pqvc = m_agvc_pqvc[map_agvcpqvcidx[pqvc.id]];
		int64_t dev_id = read_tpl_dev(pqvc.id, "pqvc_measuration");
		if (dev_id < 0)
		{
			log_error("read pqvc tpl dev error, id = %lld, tpl_name = pqvc_measuration", pqvc.id);
			continue;
		}

		map<string, int> map_anapfx2idx, map_digpfx2idx;
		vector<pnt_ana> pnt_anas;
		vector<pnt_dig> pnt_digs;

		read_mea(pnt_anas, dev_id);
		creat_pnt_index(pnt_anas, map_anapfx2idx);

		read_mea(pnt_digs, dev_id);
		creat_pnt_index(pnt_digs, map_digpfx2idx);

		/// 参数计算 Meong 2022-9-13 ///
		vpp_generator vpp_gen;
		vpp_gen.id = pqvc.id;
		ECO_NCPY_CHAR_ARRAY(vpp_gen.description, pqvc.description);

		// 额定有功
		vpp_gen.rated_p = pqvc.rated_p;
		// 额定无功
		vpp_gen.rated_q = pqvc.rated_q;
		// 额定电压
		vpp_gen.rated_v = get_vbase(pqvc.vt_id).vbase;
		// 额定电流
		double sn = sqrt(vpp_gen.rated_p*vpp_gen.rated_p + vpp_gen.rated_q*vpp_gen.rated_q);
		if (vpp_gen.rated_v > 0)
		{
			vpp_gen.rated_i = sn / vpp_gen.rated_v / 1.732;
		}
		// 额定功率因数
		vpp_gen.rated_cos = 1.0;
		// 额定频率
		vpp_gen.rated_f = 50.0;
		// 额定转速 rpm
		vpp_gen.rated_n = 3000;
		// 有功功率
		if (map_anapfx2idx.count("p") != 0)
			vpp_gen.p = pnt_anas[map_anapfx2idx["p"]].value;
		// 无功功率
		if (map_anapfx2idx.count("q") != 0)
			vpp_gen.q = pnt_anas[map_anapfx2idx["q"]].value;
		// 电压
		if (map_anapfx2idx.count("uab") != 0)
			vpp_gen.v = pnt_anas[map_anapfx2idx["uab"]].value;
		// 电流
		if (map_anapfx2idx.count("ia") != 0)
			vpp_gen.i = pnt_anas[map_anapfx2idx["ia"]].value;
		// 功率因数
		if (sn > 0)
			vpp_gen.cos = vpp_gen.p / sn;
		// 频率
		if (map_anapfx2idx.count("freq") != 0)
			vpp_gen.f = pnt_anas[map_anapfx2idx["freq"]].value;
		// 转速
		vpp_gen.n = vpp_gen.p / vpp_gen.rated_p * vpp_gen.rated_n;
		// 接线方式
		vpp_gen.type = 1;
		// 可靠性指标
		vpp_gen.reliability = 99999;
		// 启动方式
		vpp_gen.start_type = 1;

		m_result_gen.push_back(vpp_gen);
	}

	log_info("BatRtdbVpp, calc generator finished");
	return 0;
}

int BatRtVpp::write_data()
{
	log_info("BatRtdbVpp, write result data");

	write_rdbdata(m_vpp_rtdb, m_result_st, true, true);
	write_rdbdata(m_vpp_rtdb, m_result_gen, true, true);

	return 0;
}