/*
*  desc : 电池分析基础类
*  time	: 2022-8-18
*  auth	: Meong
*/
#ifndef BAT_CALC_H
#define BAT_CALC_H

#include "bat_rtdbdef.h"

class BatCalc
{
public:
	BatCalc() {};
	virtual ~BatCalc() {};

	virtual int calc() { return 0; };

private:

};

#endif