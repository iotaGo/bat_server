/*
*  desc : AGVC实时库数据定义
*  time	: 2022-3-16
*  auth	: Meong
*/
#ifndef AGVC_RTDBDEF_H
#define AGVC_RTDBDEF_H

#include "agvc_def.h"
#include "rtdb/rtdb_core.h"
#include "bat_config.h"

struct station_bess
{
	station_bess() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		memset(filter, 0, MAX_FILTER_LEN);
		vt_id = 0;
		rated_p = rated_q = rated_soc = adjq_uplimit = 0;
	}
	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	char	filter[MAX_FILTER_LEN];
	int		vt_id;									//额定电压等级
	double	rated_p;								//额定有功
	double	rated_q;								//额定无功
	double	rated_soc;								//额定SOC
	double	adjq_uplimit;							//无功调整限值

	CLASS_VAR_REGISTER(id, description, filter, vt_id, rated_p, rated_q, rated_soc, adjq_uplimit);
};

struct voltagelevel
{
	voltagelevel() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		container_id = -1;
		memset(container_tablename, 0, MAX_TABLENAME_LEN);
	}
	int64_t id;
	char description[MAX_DESCRIPTION_LEN];
	int64_t container_id;
	char container_tablename[MAX_TABLENAME_LEN];
	CLASS_VAR_REGISTER(id, description, container_id, container_tablename);
};

struct obj_dev
{
	obj_dev() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		memset(filter, 0, MAX_FILTER_LEN);
		obj_type = -1;
		container_id = -1;
		memset(container_tablename, 0, MAX_TABLENAME_LEN);
		vt_id = 0;
	}
	int64_t id;
	char description[MAX_DESCRIPTION_LEN];
	char filter[MAX_FILTER_LEN];
	int32_t obj_type;
	int64_t container_id;
	char container_tablename[MAX_TABLENAME_LEN];
	int32_t vt_id;
	CLASS_VAR_REGISTER(id, description, filter, obj_type, container_id, container_tablename, vt_id);
};

struct obj_dev_filter_cond
{
	char filter[MAX_FILTER_LEN];
	CLASS_VAR_REGISTER(filter);
};

struct obj_dev_tpl_cond
{
	int64_t container_id;
	char tp_name[MAX_TPNAME_LEN];
	CLASS_VAR_REGISTER(container_id, tp_name);
};

struct pqvc_connectpoint
{
	pqvc_connectpoint() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		obj_type = -1;
		container_id = -1;
		memset(container_tablename, 0, MAX_TABLENAME_LEN);
		vt_id = -1;
		relation_id = -1;
		memset(relation_tablename, 0, MAX_TABLENAME_LEN);
		ctrlpoint_type = -1;
		rated_p = rated_q = rated_soc = 0;
	}
	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int32_t obj_type;
	int64_t container_id;
	char	container_tablename[MAX_TABLENAME_LEN];
	int32_t vt_id;
	int64_t	relation_id;							//容器ID
	char	relation_tablename[MAX_TABLENAME_LEN];	//容器表名
	int		ctrlpoint_type;							//并网控制点类型
	double	rated_p;								//额定有功容量
	double	rated_q;								//额定有功容量
	double	rated_soc;								//额定SOC

	CLASS_VAR_REGISTER(id, description, obj_type, container_id, container_tablename, vt_id, relation_id, relation_tablename, ctrlpoint_type, rated_p, rated_q, rated_soc);
};

struct pqvc_connectpoint_container_cond
{
	int64_t container_id;
	CLASS_VAR_REGISTER(container_id);
};

struct pqvc_connectpoint_type_cond
{
	int32_t ctrlpoint_type;
	CLASS_VAR_REGISTER(ctrlpoint_type);
};

struct auxiliary_bess
{
	auxiliary_bess() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		obj_type = -1;
		container_id = -1;
		memset(container_tablename, 0, MAX_TABLENAME_LEN);
		vt_id = -1;
		relation_id = -1;
		memset(relation_tablename, 0, MAX_TABLENAME_LEN);
		pair_up = -1;
		rated_p = rated_q = rated_soc = soc_uplimit = soc_dnlimit = 0;
	}
	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int32_t obj_type;
	int64_t container_id;
	char	container_tablename[MAX_TABLENAME_LEN];
	int		vt_id;									//额定电压等级
	double	rated_p;								//额定有功
	double	rated_q;								//额定无功
	double	rated_soc;								//额定SOC
	int64_t	relation_id;							//关联ID
	char	relation_tablename[MAX_TABLENAME_LEN];	//关联表名
	int64_t pair_up;								//主备机ID
	double	soc_uplimit;							//soc上限
	double	soc_dnlimit;							//soc下限

	CLASS_VAR_REGISTER(id, description, obj_type, container_id, container_tablename, vt_id, rated_p, rated_q, rated_soc, relation_id, relation_tablename, pair_up, soc_uplimit, soc_dnlimit);
};

struct pcs
{
	pcs() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		obj_type = -1;
		container_id = -1;
		memset(container_tablename, 0, MAX_TABLENAME_LEN);
		vt_id = -1;
		relation_id = -1;
		memset(relation_tablename, 0, MAX_TABLENAME_LEN);
		rated_sn = rated_p = rated_q = p_value = 0;
	}
	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int32_t obj_type;
	int64_t container_id;
	char	container_tablename[MAX_TABLENAME_LEN];
	int		vt_id;									//额定电压等级
	double	rated_sn;								//额定视在功率
	double	rated_p;								//额定有功
	double	rated_q;								//额定无功
	int64_t	relation_id;							//关联ID
	char	relation_tablename[MAX_TABLENAME_LEN];	//关联表名
	double	p_value;								//有功功率值

	CLASS_VAR_REGISTER(id, description, obj_type, container_id, container_tablename, vt_id, rated_sn, rated_p, rated_q, relation_id, relation_tablename, p_value);
};

struct bms
{
	bms() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		obj_type = -1;
		container_id = -1;
		memset(container_tablename, 0, MAX_TABLENAME_LEN);
		vt_id = -1;
		relation_id = -1;
		memset(relation_tablename, 0, MAX_TABLENAME_LEN);
		rated_v = rated_soc = soc_uplimit = soc_dnlimit = soc_value = 0;
	}
	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int32_t obj_type;
	int64_t container_id;
	char	container_tablename[MAX_TABLENAME_LEN];
	int		vt_id;									//额定电压等级
	int64_t	relation_id;							//关联ID
	char	relation_tablename[MAX_TABLENAME_LEN];	//关联表名
	double	rated_v;								//额定电压
	double	rated_soc;								//额定SOC
	double	soc_uplimit;							//soc上限
	double	soc_dnlimit;							//soc下限
	double	soc_value;								//soc值

	CLASS_VAR_REGISTER(id, description, obj_type, container_id, container_tablename, vt_id, relation_id, relation_tablename, rated_v, rated_soc, soc_uplimit, soc_dnlimit, soc_value);
};

struct batcluster
{
	batcluster() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		obj_type = -1;
		container_id = -1;
		memset(container_tablename, 0, MAX_TABLENAME_LEN);
		vt_id = -1;
		relation_id = -1;
		memset(relation_tablename, 0, MAX_TABLENAME_LEN);
		rated_v = rated_soc = soc_uplimit = soc_dnlimit = 0;
	}
	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int32_t obj_type;
	int64_t container_id;
	char	container_tablename[MAX_TABLENAME_LEN];
	int		vt_id;									//额定电压等级
	int64_t	relation_id;							//关联ID
	char	relation_tablename[MAX_TABLENAME_LEN];	//关联表名
	double	rated_v;								//额定电压
	double	rated_soc;								//额定SOC
	double	soc_uplimit;							//soc上限
	double	soc_dnlimit;							//soc下限

	CLASS_VAR_REGISTER(id, description, obj_type, container_id, container_tablename, vt_id, relation_id, relation_tablename, rated_v, rated_soc, soc_uplimit, soc_dnlimit);
};

struct batcell
{
	batcell() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		obj_type = -1;
		container_id = -1;
		memset(container_tablename, 0, MAX_TABLENAME_LEN);
		vt_id = -1;
		relation_id = -1;
		memset(relation_tablename, 0, MAX_TABLENAME_LEN);
		rated_v = rated_soc = soc_uplimit = soc_dnlimit = 0;
	}
	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int32_t obj_type;
	int64_t container_id;
	char	container_tablename[MAX_TABLENAME_LEN];
	int		vt_id;									//额定电压等级
	int64_t	relation_id;							//关联ID
	char	relation_tablename[MAX_TABLENAME_LEN];	//关联表名
	double	rated_v;								//额定电压
	double	rated_soc;								//额定SOC
	double	soc_uplimit;							//soc上限
	double	soc_dnlimit;							//soc下限

	CLASS_VAR_REGISTER(id, description, obj_type, container_id, container_tablename, vt_id, relation_id, relation_tablename, rated_v, rated_soc, soc_uplimit, soc_dnlimit);
};

struct pnt
{
	pnt() {
		id = 0;
		status = 0;
		memset(postfix, 0, MAX_POSTFIX_LEN);
		container_id = -1;
		memset(container_tablename, 0, MAX_TABLENAME_LEN);
	}
	int64_t id;
	int status;
	char postfix[MAX_POSTFIX_LEN];
	int64_t container_id;
	char container_tablename[MAX_TABLENAME_LEN];
};

struct pnt_ana : public pnt
{
	pnt_ana() {
		value = 0;
	}
	double value;
	CLASS_VAR_REGISTER(id, value, status, postfix, container_id, container_tablename);
};

struct pnt_dig : public pnt
{
	pnt_dig() {
		value = 0;
	}
	int value;
	CLASS_VAR_REGISTER(id, value, status, postfix, container_id, container_tablename);
};

/// pnt_mix表结构与pnt_mix一致，子类需要重新注册变量值 Meong 2022-4-24 ///
struct pnt_mix : public pnt_dig
{
	CLASS_VAR_REGISTER(id, value, status, postfix, container_id, container_tablename);
};

struct type_vt
{
	type_vt() {
		id = 0;
		vbase = h_limit = l_limit = 0;
	}

	int32_t id;
	double vbase;
	double h_limit;
	double l_limit;
	CLASS_VAR_REGISTER(id, vbase, h_limit, l_limit);
};

struct pnt_ana_container_cond
{
	int64_t container_id;
	CLASS_VAR_REGISTER(container_id);
};

struct pnt_ana_postfix_cond
{
	char postfix[MAX_POSTFIX_LEN];
	CLASS_VAR_REGISTER(postfix);
};

struct pnt_ana_con_post_cond
{
	int64_t container_id;
	char postfix[MAX_POSTFIX_LEN];
	CLASS_VAR_REGISTER(container_id, postfix);
};

struct pnt_dig_container_cond
{
	int64_t container_id;
	CLASS_VAR_REGISTER(container_id);
};

struct pnt_dig_postfix_cond
{
	char postfix[MAX_POSTFIX_LEN];
	CLASS_VAR_REGISTER(postfix);
};

struct pnt_dig_con_post_cond
{
	int64_t container_id;
	char postfix[MAX_POSTFIX_LEN];
	CLASS_VAR_REGISTER(container_id, postfix);
};

struct pnt_mix_container_cond
{
	int64_t container_id;
	CLASS_VAR_REGISTER(container_id);
};

struct pnt_mix_postfix_cond
{
	char postfix[MAX_POSTFIX_LEN];
	CLASS_VAR_REGISTER(postfix);
};

struct pnt_mix_con_post_cond
{
	int64_t container_id;
	char postfix[MAX_POSTFIX_LEN];
	CLASS_VAR_REGISTER(container_id, postfix);
};

struct PHY_MODEL_AGENT
{
	RTDB_CLASS_BIND(station_bess, "station_bess");
	RTDB_CLASS_BIND(voltagelevel, "con_voltagelevel");
	RTDB_CLASS_BIND(pqvc_connectpoint, "pqvc_connectpoint");
	RTDB_CLASS_BIND(auxiliary_bess, "auxiliary_bess");
	RTDB_CLASS_BIND(pcs, "pcs");
	RTDB_CLASS_BIND(bms, "bms");
	RTDB_CLASS_BIND(batcluster, "batcluster");
	RTDB_CLASS_BIND(batcell, "batcell");
	RTDB_CLASS_BIND(obj_dev, "obj_dev");
	RTDB_CLASS_BIND(pnt_ana, "pnt_ana");
	RTDB_CLASS_BIND(pnt_dig, "pnt_dig");
	RTDB_CLASS_BIND(pnt_mix, "pnt_mix");
	RTDB_CLASS_BIND(type_vt, "type_vt");

	/// select condition Meong 2022-3-16 ///
	RTDB_CLASS_BIND(pqvc_connectpoint_container_cond, "pqvc_connectpoint");
	RTDB_CLASS_BIND(pqvc_connectpoint_type_cond, "pqvc_connectpoint");
	RTDB_CLASS_BIND(obj_dev_filter_cond, "obj_dev");
	RTDB_CLASS_BIND(obj_dev_tpl_cond, "obj_dev");
	RTDB_CLASS_BIND(pnt_ana_container_cond, "pnt_ana");
	RTDB_CLASS_BIND(pnt_ana_postfix_cond, "pnt_ana");
	RTDB_CLASS_BIND(pnt_ana_con_post_cond, "pnt_ana");
	RTDB_CLASS_BIND(pnt_dig_container_cond, "pnt_dig");
	RTDB_CLASS_BIND(pnt_dig_postfix_cond, "pnt_dig");
	RTDB_CLASS_BIND(pnt_dig_con_post_cond, "pnt_dig");
	RTDB_CLASS_BIND(pnt_mix_container_cond, "pnt_mix");
	RTDB_CLASS_BIND(pnt_mix_postfix_cond, "pnt_mix");
	RTDB_CLASS_BIND(pnt_mix_con_post_cond, "pnt_mix");
};

struct bat_eva_bms
{
	bat_eva_bms()
	{
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		v = i = t = soc = soh = var_v = var_i = var_t = var_soc = var_soh = score = 0;
	}

	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int64_t	container_id;
	double	v;
	double	i;
	double	t;
	double	soc;
	double	soh;
	double	var_v;
	double	var_i;
	double	var_t;
	double	var_soc;
	double	var_soh;
	double	score;
	char	update_dt[MAX_DATETIME_LEN];
	CLASS_VAR_REGISTER(id, description, container_id, v, i, t, soc, soh, var_v, var_i, var_t, var_soc, var_soh, score, update_dt);
};

struct bat_eva_cluster
{
	bat_eva_cluster()
	{
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		v = i = t = soc = soh = var_v = var_i = var_t = var_soc = var_soh = score = 0;
	}

	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int64_t	container_id;
	double	v;
	double	i;
	double	t;
	double	soc;
	double	soh;
	double	var_v;
	double	var_i;
	double	var_t;
	double	var_soc;
	double	var_soh;
	double	score;
	char	update_dt[MAX_DATETIME_LEN];
	CLASS_VAR_REGISTER(id, description, container_id, v, i, t, soc, soh, var_v, var_i, var_t, var_soc, var_soh, score, update_dt);
};

struct bat_eva_cell
{
	bat_eva_cell()
	{
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		v = i = t = soc = soh = score = 0;
	}

	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int64_t	container_id;
	double	v;
	double	i;
	double	t;
	double	soc;
	double	soh;
	double	score;
	char	update_dt[MAX_DATETIME_LEN];
	CLASS_VAR_REGISTER(id, description, container_id, v, i, t, soc, soh, score, update_dt);
};

struct BAT_EVA_AGENT
{
	RTDB_CLASS_BIND(bat_eva_bms, "bat_eva_bms");
	RTDB_CLASS_BIND(bat_eva_cluster, "bat_eva_cluster");
	RTDB_CLASS_BIND(bat_eva_cell, "bat_eva_cell");
};

struct vpp_station
{
	vpp_station() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		p = q = v = i = f = p_incre = p_decre = p_ratio = q_incre = q_decre = q_ratio = 0;
		run_state = alarm_state = fault_state = p_incre_lock = p_decre_lock = q_incre_lock = q_decre_lock = p_order_lock = q_order_lock = emergency = 0;
	}

	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	double	p;					// 有功功率
	double	q;					// 无功功率
	double	v;					// 电压
	double	i;					// 电流
	double	f;					// 频率
	double	p_incre;			// 有功上调节功率
	double	p_decre;			// 有功下调节功率
	double	p_ratio;			// 有功调节速率
	double	q_incre;			// 无功上调节功率
	double	q_decre;			// 无功下调节功率
	double	q_ratio;			// 无功调节速率
	int		run_state;			// 整站运行状态
	int		alarm_state;		// 整站告警状态
	int		fault_state;		// 整站故障状态
	int		p_incre_lock;		// 有功上调节闭锁
	int		p_decre_lock;		// 有功下调节闭锁
	int		q_incre_lock;		// 无功上调节闭锁
	int		q_decre_lock;		// 无功下调节闭锁
	int		p_order_lock;		// 有功指令异常闭锁
	int		q_order_lock;		// 无功指令异常闭锁
	int		emergency;			// 电网紧急控制状态

	CLASS_VAR_REGISTER(id, description, p, q, v, i, f, p_incre, p_decre, p_ratio, q_incre, q_decre, q_ratio, run_state, alarm_state, fault_state,
		p_incre_lock, p_decre_lock, q_incre_lock, q_decre_lock, p_order_lock, q_order_lock, emergency);
};

struct vpp_generator
{
	vpp_generator() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		rated_p = rated_q = rated_v = rated_i = rated_cos = rated_f = 0;
		rated_n = 0;
		p = q = v = i = cos = f = reliability = 0;
		n = type = start_type = 0;
	}

	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	double	rated_p;			// 额定有功
	double	rated_q;			// 额定无功
	double	rated_v;			// 额定电压
	double	rated_i;			// 额定电流
	double	rated_cos;			// 额定功率因数
	double	rated_f;			// 额定频率
	int		rated_n;			// 额定转速
	double	p;					// 有功功率
	double	q;					// 无功功率
	double	v;					// 电压
	double	i;					// 电流
	double	cos;				// 功率因数
	double	f;					// 频率
	int		n;					// 转速
	int		type;				// 接线方式
	double	reliability;		// 可靠性指标
	int		start_type;			// 启动方式

	CLASS_VAR_REGISTER(id, description, rated_p, rated_q, rated_v, rated_i, rated_cos, rated_f, rated_n, 
		p, q, v, i, cos, f, n, type, reliability, start_type);
};

struct agvc_station_bess 
{
	agvc_station_bess()
	{
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		uab = ubc = uca = ia = ib = ic = freq = p = q = 0;
		ywh_lock = 0;
		state = AgvcStopStatus;
		protect = AgvcNormal;
		soc = 0;
		pin_limit = pout_limit = qin_limit = qout_limit = 0;
		qincre_available = qdecre_available = qcapacitive_max = qinductive_max = 0;
		agc_no_charge = agc_no_discharge = 0;
		pset_remain = qset_remain = 0;
		agc_charge_finished = agc_discharge_finished = 0;
		agc_no_charge = agc_no_discharge = agc_charge_finished = agc_discharge_finished = 0;
	}
	int64_t	id;										//设备ID
	char	description[MAX_DESCRIPTION_LEN];		//设备名称

	double	uab;									//并网点Uab
	double	ubc;									//并网点Ubc
	double	uca;									//并网点Uca
	double	ia;										//并网点Ia
	double	ib;										//并网点Ib
	double	ic;										//并网点Ic
	double	freq;									//并网点频率
	double	p;										//并网点有功
	double	q;										//并网点无功

	int		ywh_lock;								//全站闭锁标志位 0代表非闭锁状态，1代表全站闭锁，用于处理源网荷系统信号
	int		state;									//EMS->调度	系统运行状态，0-故障，1-停机，2-运行，3-充电，4-放电
	int		protect;								//EMS->调度	系统故障状态，0-正常，1-故障

	double	soc;									//平均SOC
	double	pin_limit;								//厂站可输入有功功率
	double	pout_limit;								//厂站可输出有功功率
	double	qin_limit;								//厂站可输入无功功率
	double	qout_limit;								//厂站可输出无功功率

	double	qincre_available;						//厂站可增无功
	double	qdecre_available;						//厂站可减无功
	double	qcapacitive_max;						//厂站可提供最大容性无功
	double	qinductive_max;							//厂站可提供最大感性无功

	double	pset_remain;							//有功缺额
	double	qset_remain;							//无功缺额

	int		agc_no_charge;							//AGC禁充状态，0-未禁充，1-禁充
	int		agc_no_discharge;						//AGC禁放状态，0-示禁放，1-禁放

	int		agc_charge_finished;					//AGC充电完成
	int		agc_discharge_finished;					//AGC放电完成

	CLASS_VAR_REGISTER(id, description, uab, ubc, uca, ia, ib, ic, freq, p, q,
		ywh_lock, state, protect, soc, pin_limit, pout_limit, qin_limit, qout_limit,
		qincre_available, qdecre_available, qcapacitive_max, qinductive_max, pset_remain, qset_remain,
		agc_no_charge, agc_no_discharge, agc_charge_finished, agc_discharge_finished);
};

struct agvc_pqvc_connectpoint
{
	agvc_pqvc_connectpoint() {
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		uab = ubc = uca = ia = ib = ic = freq = p = q = 0;
		state = AgvcStopStatus;
		protect = AgvcNormal;
		soc = 0;
		pin_limit = pout_limit = qin_limit = qout_limit = 0;
		qincre_available = qdecre_available = qcapacitive_max = qinductive_max = 0;
		agc_no_charge = agc_no_discharge = 0;
		agc_charge_finished = agc_discharge_finished = 0;
		agc_no_charge = agc_no_discharge = agc_charge_finished = agc_discharge_finished = 0;
	}
	int64_t	id;										//设备ID
	char	description[MAX_DESCRIPTION_LEN];		//设备名称

	double	uab;									//并网点Uab
	double	ubc;									//并网点Ubc
	double	uca;									//并网点Uca
	double	ia;										//并网点Ia
	double	ib;										//并网点Ib
	double	ic;										//并网点Ic
	double	freq;									//并网点频率
	double	p;										//并网点有功
	double	q;										//并网点无功

	int		state;									//EMS->调度	系统运行状态，0-故障，1-停机，2-运行，3-充电，4-放电
	int		protect;								//EMS->调度	系统故障状态，0-正常，1-故障

	double	soc;									//平均SOC
	double	pin_limit;								//可输入有功功率
	double	pout_limit;								//可输出有功功率
	double	qin_limit;								//可输入无功功率
	double	qout_limit;								//可输出无功功率

	double	qincre_available;						//可增无功
	double	qdecre_available;						//可减无功
	double	qcapacitive_max;						//厂站可提供最大容性无功
	double	qinductive_max;							//厂站可提供最大感性无功

	int		agc_no_charge;							//AGC禁充状态，0-未禁充，1-禁充
	int		agc_no_discharge;						//AGC禁放状态，0-示禁放，1-禁放

	int		agc_charge_finished;					//AGC充电完成
	int		agc_discharge_finished;					//AGC放电完成

	CLASS_VAR_REGISTER(id, description, uab, ubc, uca, ia, ib, ic, freq, p, q, 
		state, protect, soc, pin_limit, pout_limit, qin_limit, qout_limit, qincre_available, qdecre_available, 
		qcapacitive_max, qinductive_max, agc_no_charge, agc_no_discharge, agc_charge_finished, agc_discharge_finished);
};

struct VPP_AGENT
{
	RTDB_CLASS_BIND(vpp_station, "vpp_station");
	RTDB_CLASS_BIND(vpp_generator, "vpp_generator");
	RTDB_CLASS_BIND(agvc_station_bess, "agvc_station_bess");
	RTDB_CLASS_BIND(agvc_pqvc_connectpoint, "agvc_pqvc_connectpoint");
};

struct bat_his_eva_bms
{
	bat_his_eva_bms() 
	{
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		start_dt = end_dt = 0;
		pear_v = pear_i = pear_t = pear_soc = pear_soh = score = 0;
		memset(update_dt, 0, MAX_DATETIME_LEN);
	}

	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int64_t	container_id;
	int64_t start_dt;
	int64_t end_dt;
	double	pear_v;
	double	pear_i;
	double	pear_t;
	double	pear_soc;
	double	pear_soh;
	double	score;
	char	update_dt[MAX_DATETIME_LEN];
	CLASS_VAR_REGISTER(id, description, container_id, start_dt, end_dt, pear_v, pear_i, pear_t, pear_soc, pear_soh, score, update_dt);
};

struct bat_his_eva_cluster
{
	bat_his_eva_cluster() 
	{
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		start_dt = end_dt = 0;
		pear_v = pear_i = pear_t = pear_soc = pear_soh = score = 0;
		memset(update_dt, 0, MAX_DATETIME_LEN);
	}

	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int64_t	container_id;
	int64_t start_dt;
	int64_t end_dt;
	double	pear_v;
	double	pear_i;
	double	pear_t;
	double	pear_soc;
	double	pear_soh;
	double	score;
	char	update_dt[MAX_DATETIME_LEN];
	CLASS_VAR_REGISTER(id, description, container_id, start_dt, end_dt, pear_v, pear_i, pear_t, pear_soc, pear_soh, score, update_dt);
};

struct bat_his_eva_cell
{
	bat_his_eva_cell() 
	{
		id = 0;
		memset(description, 0, MAX_DESCRIPTION_LEN);
		start_dt = end_dt = 0;
		pear_v = pear_i = pear_t = pear_soc = pear_soh = score = 0;
		memset(update_dt, 0, MAX_DATETIME_LEN);
	}

	int64_t id;
	char	description[MAX_DESCRIPTION_LEN];
	int64_t	container_id;
	int64_t start_dt;
	int64_t end_dt;
	double	pear_v;
	double	pear_i;
	double	pear_t;
	double	pear_soc;
	double	pear_soh;
	double	score;
	char	update_dt[MAX_DATETIME_LEN];
	CLASS_VAR_REGISTER(id, description, container_id, start_dt, end_dt, pear_v, pear_i, pear_t, pear_soc, pear_soh, score, update_dt);
};

struct BAT_HIS_EVA_AGENT
{
	RTDB_CLASS_BIND(bat_his_eva_bms, "bat_his_eva_bms");
	RTDB_CLASS_BIND(bat_his_eva_cluster, "bat_his_eva_cluster");
	RTDB_CLASS_BIND(bat_his_eva_cell, "bat_his_eva_cell");
};

struct bat_his_statistics
{
	bat_his_statistics()
	{
		bms_id = cluster_id = -1;
		cell_index = -1;
		vmax_n = vmin_n = tmax_n = tmin_n = 0;
		vmax_p = vmin_p = tmax_p = tmin_p = 0;

		memset(update_dt, 0, MAX_DATETIME_LEN);
	}

	int64_t bms_id;
	int64_t cluster_id;
	int		cell_index;
	int		vmax_n;
	double	vmax_p;
	int		vmin_n;
	double	vmin_p;
	int		tmax_n;
	double	tmax_p;
	int		tmin_n;
	double	tmin_p;
	char	update_dt[MAX_DATETIME_LEN];
	CLASS_VAR_REGISTER(bms_id, cluster_id, cell_index, vmax_n, vmax_p, vmin_n, vmin_p, tmax_n, tmax_p, tmin_n, tmin_p, update_dt);
};

#endif