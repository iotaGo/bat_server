/*
*  desc : 电池分析服务类
*  time	: 2022-8-18
*  auth	: Meong
*/
#ifndef BAT_SERVER_H
#define BAT_SERVER_H

#include <vector>
#include <map>
#include <string>
#include "eco/json_support.h"
#include "sys/sysrun.h"
#include "rtdb/rtdb.h"
#include "emsg/emsg.h"
#include "bat_config.h"
#include "bat_rtdbdef.h"

using std::vector;
using std::map;
using std::string;

class BatServer
{
public:
	BatServer();
	~BatServer();

	int Init();
	void Run();

private:
	SysRunSvr	m_sys;
	bool		m_onDuty;
	map<string, bool> m_calculated;
};

#endif