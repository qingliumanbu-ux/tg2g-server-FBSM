/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2026/6/3
Description: 每日配料规则维护查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm23_inq)

int f_fbsm23_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";

	CDbCommand cmd_inq(conn);
	CString date_c = "";
	CString datetime = CDateTime::Now().ToString("yyyyMMdd");
	try
	{
		Log::Trace("", "", "datetime = {0}", datetime);
		if (bcls_rec->Tables[0].Columns.Contains("DATE_C"))
			date_c = bcls_rec->Tables[0].Rows[0]["DATE_C"].ToString().TrimOrBlank().ToUpper();
		CString where = "";
		if (date_c.Trim() != "")
		{
			where += " and DATE_C>='" + date_c.SubstringNE(2, 6) + "'";
		}

		// 组装最终SQL
		sqlstr = " select ST_NO,MATERIAL_CODE,DATE_C,SUM(FURNACE_COUNT) AS FURNACE_COUNT from tfbsm12 "
			"  where DATE_C='" + date_c.SubstringNE(2, 6) + "'"
			"  group by ST_NO,MATERIAL_CODE,DATE_C ";
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr = {0}", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
	}
	catch (CDbException &ex)
	{
		CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
		CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char *)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException &ex)
	{
		s.flag = ex.GetCode();
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
