#include <fstream>
#include "stdafx.h"
#include "plugin.h"

#include "MemLeaker.h"


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

PDATAIOFUNC	 g_pFuncCallBack;

//获取回调函数
void RegisterDataInterface(PDATAIOFUNC pfn)
{
	g_pFuncCallBack = pfn;
}

//注册插件信息
void GetCopyRightInfo(LPPLUGIN info)
{
	//填写基本信息
	strcpy(info->Name,"获取股票信息");
	strcpy(info->Dy,"US");
	strcpy(info->Author,"john");
	strcpy(info->Period,"短线");
	strcpy(info->Descript,"获取股票信息");
	strcpy(info->OtherInfo,"获取股票信息");

	//填写参数信息
	info->ParamNum = 0;//表示0个参数

}

////////////////////////////////////////////////////////////////////////////////
//自定义实现细节函数(可根据选股需要添加)

const	BYTE	g_nAvoidMask[]={0xF8,0xF8,0xF8,0xF8};	// 无效数据标志(系统定义)

char* g_nFatherCode[] = { "999999", "399001", "399005", "399006" };
int g_FatherUpPercent[] = {-1, -1, -1, -1};


BOOL fEqual(float a, float b)
{
	const float fJudge = 0.01;
	float fValue = 0.0;

	if (a > b)
		fValue = a - b;
	else 
		fValue = b - a;

	if (fValue > fJudge)
		return FALSE;

	return TRUE;
}


BOOL dateEqual(NTime t1, NTime t2)
{
	if (t1.year != t2.year || t1.month != t2.month || t1.day != t2.day)
		return FALSE;

	return TRUE;
}


NTime dateInterval(NTime nLeft, NTime nRight)
{
	NTime nInterval;
	memset(&nInterval, 0, sizeof(NTime));
	
	unsigned int iLeft = 0;
	unsigned int iRight = 0;
	unsigned int iInterval = 0;

	const unsigned int cDayofyear = 365;
	const unsigned int cDayofmonth = 30;

	iLeft = nLeft.year*cDayofyear + nLeft.month*cDayofmonth + nLeft.day;
	iRight = nRight.year*cDayofyear + nRight.month*cDayofmonth + nRight.day;

	iInterval = (iLeft > iRight) ? iLeft - iRight : iRight - iLeft;

	nInterval.year = iInterval / cDayofyear;
	iInterval = iInterval % cDayofyear;
	nInterval.month = iInterval / cDayofmonth;
	iInterval = iInterval % cDayofmonth;
	nInterval.day = iInterval;

	return nInterval;
}


/* 过滤函数
   返回值：以S和*开头的股票 或者 上市不满一年，返回FALSE，否则返回TRUE
*/
BOOL filterStock(char * Code, short nSetCode, NTime time1, NTime time2, BYTE nTQ)
{
	if (NULL == Code)
		return FALSE;
	
	int iInfoNum = 2;
	LPSTOCKINFO pStockInfo = new STOCKINFO[iInfoNum];
	memset(pStockInfo, 0, iInfoNum*sizeof(STOCKINFO));

	long readnum = g_pFuncCallBack(Code, nSetCode, STKINFO_DAT, pStockInfo, iInfoNum, time1, time2, nTQ, 0);
	if (readnum <= 0)
	{
		delete[] pStockInfo;
		pStockInfo = NULL;
		return FALSE;
	}
	if ('S' == pStockInfo->Name[0] || '*' == pStockInfo->Name[0])
	{
		delete[] pStockInfo;
		pStockInfo = NULL;
		return FALSE;
	}

	NTime startDate, dInterval;
	memset(&startDate, 0, sizeof(NTime));
	memset(&dInterval, 0, sizeof(NTime));

	long lStartDate = pStockInfo->J_start;
	startDate.year = lStartDate / 10000;
	lStartDate = lStartDate % 10000;
	startDate.month = lStartDate / 100;
	lStartDate = lStartDate % 100;
	startDate.day = lStartDate;

	dInterval = dateInterval(startDate, time2);

	if (dInterval.year < 2)
	{
		delete[] pStockInfo;
		pStockInfo = NULL;
		return FALSE;
	}

	delete[] pStockInfo;
	pStockInfo = NULL;

	return TRUE;
}



BOOL InputInfoThenCalc1(char * Code,short nSetCode,int Value[4],short DataType,short nDataNum,BYTE nTQ,unsigned long unused) //按最近数据计算
{
	BOOL nRet = FALSE;

	int iInfoNum = 2;
	LPSTOCKINFO pStockInfo = new STOCKINFO[iInfoNum];
	memset(pStockInfo, 0, iInfoNum*sizeof(STOCKINFO));

	NTime tmpTime={0};

	long readnum = g_pFuncCallBack(Code, nSetCode, STKINFO_DAT, pStockInfo, iInfoNum, tmpTime, tmpTime, nTQ, 0);
	if (readnum < 0)
	{
		delete[] pStockInfo;
		pStockInfo = NULL;
		return FALSE;
	}

	try{
		std::ofstream out;
		out.open("PickInfo.txt", std::ios::app);
		char szBuff[256] = {0};
		sprintf(szBuff, "%s  %s \n\n", Code, pStockInfo->Name);
		out << szBuff;
		out.close();

		nRet = TRUE;
	} 
	catch(...)
	{
		OutputDebugString(L"Exception!\n");
	}

	return nRet;
}

BOOL InputInfoThenCalc2(char * Code,short nSetCode,int Value[4],short DataType,NTime time1,NTime time2,BYTE nTQ,unsigned long unused)  //选取区段
{
	BOOL nRet = FALSE;
	
	return nRet;
}
