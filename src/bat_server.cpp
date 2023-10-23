#include <ctime>
#include <vector>
#include "bat_server.h"
#include "sys/sysrun.h"
#include "sys/eproc.h"
#include "eco/file_op.h"
#include "bat_rtdbdef.h"
#include "bat_rtdbcalc.h"
#include "bat_hiscalc.h"

#if _MSC_VER >= 1600      
#pragma execution_character_set("utf-8")      
#endif  

using std::vector;

BatServer::BatServer()
{
	m_onDuty = false;
}

BatServer::~BatServer()
{

}

int BatServer::Init()
{
	/// init sysrun Meong 2022-3-16 ///
	int ret_code = m_sys.Init();
	if (ret_code < 0)
	{
		log_error("SysRunSvr Init error! ret_code = %d", ret_code);
		return ret_code;
	}

	return 0;
}

void BatServer::Run()
{
	bool started = false;
	BatConfig &config = BatConfig::get();

	while (true)
	{
		if (m_sys.IsOnDuty(EProc::GetCtxApp().c_str()))
		{
			if (!m_onDuty)
			{
				log_info("bat_server change from standby to be on duty!");
				/// info agvc Meong 2022-3-16 ///
			}
			m_onDuty = true;
		}
		else
		{
			if (m_onDuty)
			{
				log_info("bat_server change from on duty to standby!");
				/// info agvc Meong 2022-3-16 ///
			}
			m_onDuty = false;
		}

		log_debug("bat_server start: %d, on duty: %d", (int)started, (int)m_onDuty);

		int ret_code = EProc::Report();
		if (ret_code < 0)
		{
			log_warn("bat_server ProcReport error! ret_code:%d", ret_code);
		}

		SysEnv::Sleep(200);

		if (m_onDuty == false)
			continue;

		if (config.time_to_calc("BatRtSohCalc"))
		{
			if (m_calculated["BatRtSohCalc"] == false)
			{
				BatRtSohCalc soh_calc;
				soh_calc.calc();
				m_calculated["BatRtSohCalc"] = true;
			}
		}
		else
			m_calculated["BatRtSohCalc"] = false;

		if (config.time_to_calc("BatRtCellEvaluate"))
		{
			if (m_calculated["BatRtCellEvaluate"] == false)
			{
				BatRtCellEvaluate cell_eva;
				cell_eva.calc();
				m_calculated["BatRtCellEvaluate"] = true;
			}
		}
		else
			m_calculated["BatRtCellEvaluate"] = false;

		/// 间隔单位s Meong 2022-9-15 ///
		if (config.time_to_calc("BatRtVpp"))
		{
			if (m_calculated["BatRtVpp"] == false)
			{
				BatRtVpp bat_vpp;
				bat_vpp.calc();
				m_calculated["BatRtVpp"] = true;
			}
		}
		else
		{
			m_calculated["BatRtVpp"] = false;
		}

		if (config.time_to_calc("BatHisCellEvaluate"))
		{
			if (m_calculated["BatHisCellEvaluate"] == false)
			{
				BatHisCellEvaluate bat_hiseva;
				bat_hiseva.calc();
				BatHisStatisticCalc bat_hisstatic;
				bat_hisstatic.calc();

				m_calculated["BatHisCellEvaluate"] = true;
			}
		}
		else
		{
			m_calculated["BatHisCellEvaluate"] = false;
		}
	}
}