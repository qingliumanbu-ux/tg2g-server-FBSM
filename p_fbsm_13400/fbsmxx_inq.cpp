/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsmxx_inq)

int f_fbsmxx_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString date_c = "";
	CString date_c_1 = "";
	CString st_no = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMdd");
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("DATE_C"))
			date_c = bcls_rec->Tables[0].Rows[0]["DATE_C"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("DATE_C_1"))
			date_c_1 = bcls_rec->Tables[0].Rows[0]["DATE_C_1"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		sqlstr = "select distinct DATE_C,ST_NO,COMPOSE_LIST_NO,BACKLOG_EA from tfbsmxx WHERE 1 = 1";
		if (date_c.Trim() != ""  && date_c_1.Trim() == ""){
			sqlstr += " and DATE_C='" + date_c + "' ";
		}
		else if (date_c_1.Trim() != "" && date_c.Trim() == ""){
			sqlstr += " and DATE_C<='" + date_c_1 + "' ";
		}
		else if (date_c_1.Trim() != "" && date_c.Trim() != ""){
			sqlstr += " and DATE_C>='" + date_c + "' and DATE_C<='" + date_c_1 + "' ";
		}
		/*else{
		sqlstr += " and A.DATE_TIME='" + datetime + "' ";
		}*/
		if (st_no.Trim() != ""){
			sqlstr += " and ST_NO like '%" + st_no + "%' ";
		}
		sqlstr += "ORDER BY  DATE_C ";
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
