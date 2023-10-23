#include "bat_prewarning.h"
#include "elog.h"
#include "bat_config.h"
#include "bat_taos.h"
#include "eco/estring.h"

#if _MSC_VER >= 1600      
#pragma execution_character_set("utf-8")      
#endif  

BatPrewarning::PreWarningInfo BatPrewarning::prewarning_info[PWTypeCount] = {
	PreWarningInfo(PWVariation, "离散度", "数据离散度过大"),
	PreWarningInfo(PWCorelation, "相关性", "数据相关性差"),
	PreWarningInfo(PWThrehouldUp, "阈值上限", "数据超阈值上限"),
	PreWarningInfo(PWThrehouldDn, "阈值下限", "数据超阈值下限"),
	PreWarningInfo(PWVarScore, "数据一致性评价", "数据一致性评分过低"),
	PreWarningInfo(PWCoScore, "数据相关性评价", "数据相关性评分过低"),
	PreWarningInfo(PWUnknown, "未知", "数据异常,原因未知")
};

BatPrewarning::PreWarningPostfix BatPrewarning::prewarning_postfix[PWPostfixCount] = {
	PreWarningPostfix(PW_I, "i","电流"),
	PreWarningPostfix(PW_V, "v", "电压"),
	PreWarningPostfix(PW_T, "t", "温度"),
	PreWarningPostfix(PW_Soc, "soc", "SOC"),
	PreWarningPostfix(PW_Soh, "soh", "SOH"),
	PreWarningPostfix(PW_CellV, "BatcellV", "电芯电压"),
	PreWarningPostfix(PW_CellI, "BatcellI", "电芯电流"),
	PreWarningPostfix(PW_CellT, "BatcellT", "电芯温度"),
	PreWarningPostfix(PW_CellSoc, "BatcellSoc", "电芯SOC"),
	PreWarningPostfix(PW_CellSoh, "BatcellSoh", "电芯SOH"),
	PreWarningPostfix(PW_PostfixUnknown, "unknow", "未知量测类型")
};

BatPrewarning::BatPrewarning()
{
	init();
}

BatPrewarning::~BatPrewarning()
{
}

BatPrewarning& BatPrewarning::get()
{
	static BatPrewarning instance;
	return instance;
}

int BatPrewarning::init()
{
	if (!BatConfig::get().get_taos_enable())
		return 0;

	int ret_code = create_taosdb();
	if (ret_code < 0)
	{
		log_error("BatPrewarning, create taosdb error, ret_code: %d", ret_code);
		return ret_code;
	}

	return 0;
}

int BatPrewarning::insert_warning(int64_t bms_id, int64_t cluster_id, int cell_index, int type, const string& postfix, double value, double threhould, string info /*= ""*/)
{
	if (!BatConfig::get().get_taos_enable())
		return 0;

	/*if (bms_id <= 0)
	{
	log_error("BatPrewarning, insert bat prewarning data error, reason: bms id error, id: %lld", bms_id);
	return -1;
	}*/

	string db_name = BatConfig::get().get_taos_dbname();
	if (db_name.empty())
		db_name = "bat_analysis";

	if (duplicate(bms_id, cluster_id, cell_index, type, postfix, value, threhould, info))
		return 0;

	BatTaos &taos = BatTaos::get();
	const int sql_len = 512;
	char sql[sql_len];

	char table_name[100] = { 0 };
	if (bms_id <= 0)
		eco::nsprintf(table_name, 100, "prewarning_batall");
	else if (cluster_id <= 0)
		eco::nsprintf(table_name, 100, "prewarning_%lld", bms_id);
	else if (cell_index <= 0)
		eco::nsprintf(table_name, 100, "prewarning_%lld", cluster_id);
	else
		eco::nsprintf(table_name, 100, "prewarning_%lld_%d", cluster_id, cell_index);

	if (info.empty())
	{
		if ((type > 0 && type < PWTypeCount))
			info = str2pfxname(postfix) + prewarning_info[type].desc;
		else
			info = str2pfxname(postfix) + prewarning_info[PWUnknown].desc;
	}

	eco::nsprintf(sql, sql_len, "INSERT INTO %s USING %s.bat_prewarning TAGS(%lld, %lld, %d) VALUES (NOW, %d, %lf, %lf, '%s')",
		table_name, db_name.c_str(), bms_id, cluster_id, cell_index, type, value, threhould, info.c_str());

	int ret_code = taos.modify(sql);
	if (ret_code < 0)
	{
		log_error("BatPrewarning, insert bat prewarning data to %s.bat_prewarning error, ret_code: %d, sql: %s", db_name.c_str(), ret_code, sql);
		return -1;
	}

	return 0;
}

int BatPrewarning::create_taosdb()
{
	if (!BatConfig::get().get_taos_enable())
		return 0;

	string db_name = BatConfig::get().get_taos_dbname();
	if (db_name.empty())
		db_name = "bat_analysis";

	BatTaos &taos = BatTaos::get();

	const int sql_len = 512;
	char sql[sql_len];

	// use database
	memset(sql, 0, sql_len);
	eco::nsprintf(sql, sql_len, "USE %s;", db_name.c_str());
	if (taos.modify(sql) < 0)
	{
		log_error("BatPrewarning, use %s db error", db_name.c_str());
		return -1;
	}

	// create stable bat_prewarning
	memset(sql, 0, sql_len);
	eco::nsprintf(sql, sql_len, "CREATE STABLE IF NOT EXISTS %s.bat_prewarning \
								(ts TIMESTAMP, type INT, value DOUBLE, threshold DOUBLE, info_str BINARY(256)) \
								tags (bms_id BIGINT, cluster_id BIGINT, cell_index INT);",
								db_name.c_str());
	if (taos.modify(sql) < 0)
	{
		log_error("BatPrewarning, create stable %s.bat_prewarning error", db_name.c_str());
		return -1;
	}

	return 0;
}

string BatPrewarning::str2pfxname(const string& postfix)
{
	if (postfix.empty())
		return "";

	for (int i = 0; i < PWPostfixCount; ++i)
	{
		if (postfix == prewarning_postfix[i].postfix)
			return prewarning_postfix[i].desc;
	}

	return prewarning_postfix[PW_PostfixUnknown].desc;
}

bool BatPrewarning::duplicate(int64_t& bms_id, int64_t& cluster_id, int& cell_index, int& type, const string& postfix, double& value, double& threhould, string& info)
{
	const int len = 128;
	char key[len] = { 0 };
	char val[len] = { 0 };

	eco::nsprintf(key, len, "%lld_%lld_%d_%d_%s", bms_id, cluster_id, cell_index, type, postfix.c_str());
	eco::nsprintf(val, len, "%lf_%lf_%s", value, threhould, info.c_str());
	if (m_record.count(key) == 0)
	{
		m_record[key] = val;
		return false;
	}
	else
	{
		if (m_record[key] != val){
			m_record[key] = val;
			return false;
		}
		else {
			return true;
		}
	}

	return true;
}