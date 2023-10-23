#include "bat_config.h"
#include "sys/sysrun.h"
#include "elog.h"
#include "stringutils.h"
#include <ctime>
#include "time.h"

BatConfig::BatConfig()
{
	taos_enable = 0;
	taos_ip_host = "localhost";
	taos_port = 0;
	taos_user = "root";
	taos_passwd = "taosdata";

	init();
}

BatConfig::~BatConfig()
{

}

int BatConfig::init()
{
	int ret_code = 0;

	/// 读配置文件 Meong 2022-8-8 ///
	std::string ini_file_path;
	ret_code = SysEnv::GetConfFilePath("agvcsrv/bat_server.ini", ini_file_path);
	if (ret_code < 0 && ini_file_path.empty())
	{
		log_error("BatServer, get config file path failed. Er:%d, file:%s", ret_code, ini_file_path.c_str());
		return ret_code;
	}

	CSimpleIni spini;
	spini.Reset();
	ret_code = spini.LoadFile(ini_file_path.c_str());
	if (ret_code != SI_OK)
	{
		log_error("BatServer, load config file failed. Er:%d, File path:%s", ret_code, ini_file_path.c_str());
		return ret_code;
	}

	CSimpleIni::TNamesDepend key_list;
	spini.GetAllKeys("BatRtSohCalc", key_list);
	CSimpleIni::TNamesDepend::iterator keyiter = key_list.begin();

	for (; keyiter != key_list.end(); ++keyiter)
	{
		const char * cur_key = keyiter->pItem;
		if (strcmp(cur_key, "interval") == 0)
		{
			string str = spini.GetValue("BatRtSohCalc", cur_key, "");
			int interval = -1;
			string unit;
			if (!StringUtils::time_split(str, interval, unit))
			{
				log_error("read BatRtSohCalc.%s error, value: %s", cur_key, str.c_str());
				continue;
			}
			
			log_info("read BatRtSohCalc.%s: %d %s", cur_key, interval, unit.c_str());
			m_interval["BatRtSohCalc"] = interval;
			m_unit["BatRtSohCalc"] = unit;
		}
	}

	spini.GetAllKeys("BatRtCellEvaluate", key_list);
	keyiter = key_list.begin();
	for (; keyiter != key_list.end(); ++keyiter)
	{
		const char * cur_key = keyiter->pItem;
		if (strcmp(cur_key, "interval") == 0)
		{
			string str = spini.GetValue("BatRtCellEvaluate", cur_key, "");
			int interval = -1;
			string unit;
			if (!StringUtils::time_split(str, interval, unit))
			{
				log_error("read BatRtCellEvaluate.%s error, value: %s", cur_key, str.c_str());
				continue;
			}

			log_info("read BatRtCellEvaluate.%s: %d %s", cur_key, interval, unit.c_str());
			m_interval["BatRtCellEvaluate"] = interval;
			m_unit["BatRtCellEvaluate"] = unit;
		}
		else if (strncmp(cur_key, "ratio.", 6) == 0)
		{
			double ratio = spini.GetDoubleValue("BatRtCellEvaluate", cur_key, 0.1);
			log_info("read BatRtCellEvaluate.%s: %lf", cur_key, ratio);
			string k = StringUtils::RemoveString(cur_key, "ratio.");
			m_ratio[k] = ratio;
		}
		else if (strncmp(cur_key, "variation.", 10) == 0)
		{
			double var = spini.GetDoubleValue("BatRtCellEvaluate", cur_key, 0.1);
			log_info("read BatRtCellEvaluate.%s: %lf", cur_key, var);
			string k = StringUtils::RemoveString(cur_key, "variation.");
			m_variation[k] = var;
		}
		else if (strncmp(cur_key, "score.", 6) == 0)
		{
			double var = spini.GetDoubleValue("BatRtCellEvaluate", cur_key, 80);
			log_info("read BatRtCellEvaluate.%s: %lf", cur_key, var);
			string k = StringUtils::RemoveString(cur_key, "score.");
			m_score[k] = var;
		}
	}

	spini.GetAllKeys("BatRtVpp", key_list);
	keyiter = key_list.begin();
	for (; keyiter != key_list.end(); ++keyiter)
	{
		const char * cur_key = keyiter->pItem;
		if (strcmp(cur_key, "interval") == 0)
		{
			string str = spini.GetValue("BatRtVpp", cur_key, "");
			int interval = -1;
			string unit;
			if (!StringUtils::time_split(str, interval, unit))
			{
				log_error("read BatRtVpp.%s error, value: %s", cur_key, str.c_str());
				continue;
			}

			log_info("read BatRtVpp.%s: %d %s", cur_key, interval, unit.c_str());
			m_interval["BatRtVpp"] = interval;
			m_unit["BatRtVpp"] = unit;
		}
	}

	spini.GetAllKeys("BatHisCellEvaluate", key_list);
	keyiter = key_list.begin();
	for (; keyiter != key_list.end(); ++keyiter)
	{
		const char * cur_key = keyiter->pItem;
		if (strcmp(cur_key, "interval") == 0)
		{
			string str = spini.GetValue("BatHisCellEvaluate", cur_key, "");
			int interval = -1;
			string unit;
			if (!StringUtils::time_split(str, interval, unit))
			{
				log_error("read BatHisCellEvaluate.%s error, value: %s", cur_key, str.c_str());
				continue;
			}

			log_info("read BatHisCellEvaluate.%s: %d %s", cur_key, interval, unit.c_str());
			m_interval["BatHisCellEvaluate"] = interval;
			m_unit["BatHisCellEvaluate"] = unit;
		}
		else if (strncmp(cur_key, "ratio.", 6) == 0)
		{
			double v = spini.GetDoubleValue("BatHisCellEvaluate", cur_key, 0.1);
			log_info("read BatHisCellEvaluate.%s: %lf", cur_key, v);
			string k = StringUtils::RemoveString(cur_key, "ratio.");
			string key = string("his") + k;
			m_ratio[key] = v;
		}
		else if (strncmp(cur_key, "corelation.", 11) == 0)
		{
			double v = spini.GetDoubleValue("BatHisCellEvaluate", cur_key, 0.9);
			log_info("read BatHisCellEvaluate.%s: %lf", cur_key, v);
			string k = StringUtils::RemoveString(cur_key, "corelation.");
			m_corelation[k] = v;
		}
		else if (strncmp(cur_key, "score.", 6) == 0)
		{
			double v = spini.GetDoubleValue("BatHisCellEvaluate", cur_key, 80);
			log_info("read BatHisCellEvaluate.%s: %lf", cur_key, v);
			string k = StringUtils::RemoveString(cur_key, "score.");
			string key = string("his") + k;
			m_score[key] = v;
		}
	}

	spini.GetAllKeys("TaosDB", key_list);
	keyiter = key_list.begin();
	for (; keyiter != key_list.end(); ++keyiter)
	{
		const char * cur_key = keyiter->pItem; 
		if (strcmp(cur_key, "enable") == 0)
		{
			taos_enable = spini.GetLongValue("TaosDB", cur_key, 0);
		}
		else if (strcmp(cur_key, "ip_hostname") == 0)
		{
			taos_ip_host = spini.GetValue("TaosDB", cur_key, "localhost");
		}
		else if (strcmp(cur_key, "port") == 0)
		{
			taos_port = spini.GetLongValue("TaosDB", cur_key, 0);
		}
		else if (strcmp(cur_key, "user") == 0)
		{
			taos_user = spini.GetValue("TaosDB", cur_key, "root");
		}
		else if (strcmp(cur_key, "passwd") == 0)
		{
			taos_passwd = spini.GetValue("TaosDB", cur_key, "taosdata");
		}
		else if (strcmp(cur_key, "db_name") == 0)
		{
			taos_db = spini.GetValue("TaosDB", cur_key, "smartsys_his");
		}
	}

	return 0;
}

bool BatConfig::time_to_calc(const string& key)
{
	if (key.empty())
		return false;

	int interval = get_interval(key);
	if (interval == -1)
		return false;

	string unit = get_time_unit(key);
	if (unit.empty())
		return false;

	time_t now_time = time(0);
	tm *t = localtime(&now_time);
	if (unit == "s")
		return t->tm_sec % interval == 0;
	else if (unit == "m")
		return t->tm_min % interval == 0;
	else if (unit == "h")
		return t->tm_hour % interval == 0;
	else if (unit == "d")
		return t->tm_mday % interval == 0;
	else if (unit == "M")
		return t->tm_mon % interval == 0;
	else if (unit == "Y")
		return t->tm_year % interval == 0;

	return false;
}

int BatConfig::get_interval(const string &key)
{
	map<string, int>::const_iterator cit = m_interval.find(key);
	if (cit == m_interval.cend())
		return DAFAULT_CALC_INTERVAL;

	return cit->second;
}

string BatConfig::get_time_unit(const string &key)
{
	map<string, string>::const_iterator cit = m_unit.find(key);
	if (cit == m_unit.cend())
		return "";

	return cit->second;
}

double BatConfig::get_ratio(const string& key)
{
	map<string, double>::const_iterator cit = m_ratio.find(key);
	if (cit == m_ratio.cend()) {
		return 0;
	}

	return cit->second;
}

double BatConfig::get_score(const string& key)
{
	map<string, double>::const_iterator cit = m_score.find(key);
	if (cit == m_score.cend()) {
		return 80;/// 得分默认值为80 Meong 2023-7-6 ///
	}

	return cit->second;
}

double BatConfig::get_variation(const string& key)
{
	map<string, double>::const_iterator cit = m_variation.find(key);
	if (cit == m_variation.cend()) {
		return 0.1; /// 变异系数默认值为0.1 Meong 2023-7-6 ///
	}

	return cit->second;
}

double BatConfig::get_corelation(const string& key)
{
	map<string, double>::const_iterator cit = m_corelation.find(key);
	if (cit == m_corelation.cend()) {
		return 0.9; /// 相关系数默认值为0.9 Meong 2023-7-6 ///
	}

	return cit->second;
}

bool BatConfig::get_taos_enable() const
{
	return taos_enable == 1;
}

string BatConfig::get_taos_url() const
{
	return taos_ip_host;
}

int BatConfig::get_taos_port() const
{
	return taos_port;
}

string BatConfig::get_taos_user() const 
{ 
	return taos_user; 
}

string BatConfig::get_taos_passwd() const 
{ 
	return taos_passwd; 
}

string BatConfig::get_taos_dbname() const 
{ 
	return taos_db; 
}