/*
*  desc : BAT预警存储类
*  time	: 2022-3-16
*  auth	: Meong
*/
#ifndef BAT_PREWARNING_H
#define BAT_PREWARNING_H

#include <string>
#include <map>
#include "bat_rtdbdef.h"

using std::string;
using std::map;

enum PREWARINING_TYPE
{
	PWVariation,	// 离散度
	PWCorelation,	// 相关性
	PWThrehouldUp,	// 阈值上限
	PWThrehouldDn,	// 阈值下限
	PWVarScore,		// 数据一致性评分较低
	PWCoScore,		// 数据相关性评分较低
	PWUnknown,

	PWTypeCount
};

enum PREWARINING_POSTFIX
{
	PW_I,
	PW_V,
	PW_T,
	PW_Soc,
	PW_Soh,
	PW_CellV,
	PW_CellI,
	PW_CellT,
	PW_CellSoc,
	PW_CellSoh,
	PW_PostfixUnknown,

	PWPostfixCount
};

class BatPrewarning
{
public:
	struct PreWarningInfo
	{
		PREWARINING_TYPE type;
		string name;
		string desc;

		PreWarningInfo(PREWARINING_TYPE t, const string& n, const string &d)
			: type(t), name(n), desc(d) {};
	};

	struct PreWarningPostfix
	{
		PREWARINING_POSTFIX type;
		string postfix;
		string desc;

		PreWarningPostfix(PREWARINING_POSTFIX t, const string& p, const string& d)
			: type(t), postfix(p), desc(d) {};
	};

public:
	static BatPrewarning& get();
	~BatPrewarning();

	int insert_warning(int64_t bms_id, int64_t cluster_id, int cell_index, int type, const string& postfix, double value, double threhould, string info="");

private:
	BatPrewarning();
	BatPrewarning(const BatPrewarning& c) {};

	int init();
	int create_taosdb();
	string str2pfxname(const string& postfix);
	inline bool duplicate(int64_t& bms_id, int64_t& cluster_id, int& cell_index, int& type, const string& postfix, double& value, double& threhould, string& info);

	map<string, string> m_record;

	static PreWarningInfo prewarning_info[PWTypeCount];
	static PreWarningPostfix prewarning_postfix[PWPostfixCount];
};

#endif 