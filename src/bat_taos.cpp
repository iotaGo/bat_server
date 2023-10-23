#include "bat_taos.h"
#include "bat_config.h"
#include "elog.h"

const int retry_times = 5;

BatTaos& BatTaos::get()
{
	static BatTaos t;
	return t;
}

BatTaos::BatTaos()
{
	m_ptaos = NULL;
	init();
}

BatTaos::~BatTaos()
{
	if (m_ptaos)
	{
		taos_close(m_ptaos);
		taos_cleanup();
		m_ptaos = NULL;
	}
}

int BatTaos::init()
{	
	int ret_code = connect();
	if (ret_code < 0)
	{
		log_error("BatTaos, init error");
		return ret_code;
	}

	return 0;
}

int BatTaos::connect(bool force_reconn)
{
	BatConfig& config = BatConfig::get();
	string url = config.get_taos_url();
	int port = config.get_taos_port();
	string user = config.get_taos_user();
	string pass = config.get_taos_passwd();
	string db_name = config.get_taos_dbname();

	return connect(url.c_str(), port, user.c_str(), pass.c_str(), db_name.c_str(), force_reconn);
}

int BatTaos::connect(const char *url, int port, const char *user, const char *passwd, const char *db_name, bool force_reconn/* = false*/)
{
	if (m_ptaos == NULL)
	{
		m_ptaos = taos_connect(url, user, passwd, db_name, port);
		if (!m_ptaos)
		{
			log_error("BatTaos, connect to taosdb error, ip:%s, port:%d, db_name:%s, user:%s, reason:%s", 
				url, port, db_name, user, taos_errstr(NULL));
			return -1;
		}
	}
	else if (force_reconn)
	{
		taos_close(m_ptaos);
		taos_cleanup();

		m_ptaos = taos_connect(url, user, passwd, db_name, port);
		if (!m_ptaos)
		{
			log_error("BatTaos, reconnect to taosdb error, ip:%s, port:%d, db_name:%s, user:%s, reason:%s", 
				url, port, db_name, user, taos_errstr(NULL));
			return -1;
		}
	}

	log_info("BatTaos, taos server info: %s, client info: %s", taos_get_server_info(m_ptaos), taos_get_client_info());

	return 0;
}

int BatTaos::query(const char *sql)
{
	int ret_code = 0;

	if (!m_ptaos)
	{
		ret_code = connect();
		if (ret_code < 0)
		{
			log_error("BatTaos, connect error");
			return ret_code;
		}
	}

	// todo

	return 0;
}

int BatTaos::modify(const char *sql)
{
	int ret_code = 0;

	if (!m_ptaos)
	{
		ret_code = connect();
		if (ret_code < 0)
		{
			log_error("BatTaos, connect error");
			return ret_code;
		}
	}

	int       i;
	TAOS_RES *pSql = NULL;
	int32_t   code = -1;

	for (i = 0; i < retry_times; i++) {
		if (NULL != pSql) {
			taos_free_result(pSql);
			pSql = NULL;
		}

		pSql = taos_query(m_ptaos, sql);
		code = taos_errno(pSql);
		if (0 == code) {
			break;
		}
	}

	if (code != 0) {
		log_error("BatTaos, failed to modify, reason: %s, sql: %s", taos_errstr(pSql), sql);
		taos_free_result(pSql);
		taos_close(m_ptaos);
		m_ptaos = NULL;
		return -1;
	}

	taos_free_result(pSql);

	return 0;
}
