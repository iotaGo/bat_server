/*
*  desc : taos库接口
*  para : 
*  time	: 2023-5-29
*  auth	: Meong
*/
#ifndef BAT_TAOS_H
#define BAT_TAOS_H

#include "taos.h"
#include "taoserror.h"
#include <string>

using std::string;

class BatTaos
{
public:
	static BatTaos& get();
	~BatTaos();

	int connect(bool force_reconn = false);
	int connect(const char *url, int port, const char *user, const char *passwd, const char *db_name, bool force_reconn = false);
	int query(const char *sql);
	int modify(const char *sql);

private:
	BatTaos();
	BatTaos(const BatTaos& c) {};
	int init();

private:
	TAOS *m_ptaos;
};


#endif