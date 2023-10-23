#include "sys/eproc.h"
#include "bat_server.h"

#if defined (__unix)
void Stop(int signo)
{
	printf("stop bat_server");
	_exit(0);
}

#endif // _unix
int main(int argc, char *argv[])
{
	log_reg(argc, argv);
	log_info("battery server log reg sucess!");

	int ret_code = EProc::Init(argc, argv);
	if (ret_code < 0)
	{
		log_error("main error, er:%d", ret_code);
		exit(0);
	}

	ret_code = EProc::Reg(EPROC_RESIDENT, 5000);
	if (ret_code < 0)
	{
		log_error("main, eproc reg error, er:%d", ret_code);
		exit(0);
	}

#if defined __unix
	signal(SIGINT, Stop);
#endif

	BatServer server;
	ret_code = server.Init();
	if (ret_code < 0)
	{
		log_error("main, bat_server init error, er:%d", ret_code);
		exit(0);
	}
	server.Run();

	return 0;
}