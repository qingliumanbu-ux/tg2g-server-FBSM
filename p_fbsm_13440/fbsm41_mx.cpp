/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2026
Author:      XXX
Version:     1.0
Date:        2026/04/02
Description: 电炉料篮模型调用程序
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm41_mx)

/* ***** 外部函数声明 ***** */
// 模型计算
int f_fbsm41_mx_ins(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);

int f_fbsm41_mx(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
    CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	try
	{
        Log::Trace("", "", "开始处理");
		doFlag = f_fbsm41_mx_ins(bcls_rec, bcls_ret, conn);
		if (doFlag < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}
        Log::Trace("", "", "处理结束");
	}
    catch (CDbException &ex)
    {
        CFormattable arguments[] = {ex.GetCode(), ex.GetMsg()};
        CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);
        CString str = sqlstr + "\r\n" + ex.GetMsg();
        strncpy(s.sysmsg, (const char *)str, sizeof(s.sysmsg) - 1);
        s.flag = -1;
        doFlag = -1;
    }
    catch (CApplicationException &ex)
    {
        s.flag = ex.GetCode();
        strncpy(s.msg, ex.GetMsg(), sizeof(s.msg) - 1);
        doFlag = -1;
    }
    catch (CException &ex)
    {
        strcpy(s.msg, ex.GetMsg());
        s.flag = ex.GetCode();
        doFlag = -1;
    }
    return doFlag;
}
