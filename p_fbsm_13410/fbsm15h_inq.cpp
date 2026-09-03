/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 历史配料单查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15h_inq)

int f_fbsm15h_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";

	CDbCommand cmd_inq(conn);
	CString date_c = "";
	CString date_e = "";
	CString backlog_ea = "";
	CString st_no = " ";
	CString compose_list_no = "";
	CString datetime = CDateTime::Now().ToString("yyyyMMdd");
	try
	{
		Log::Trace("", "", "datetime = {0}", datetime);
		if (bcls_rec->Tables[0].Columns.Contains("DATE_C"))
			date_c = bcls_rec->Tables[0].Rows[0]["DATE_C"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("DATE_E"))
			date_e = bcls_rec->Tables[0].Rows[0]["DATE_E"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("BACKLOG_EA"))
			backlog_ea = bcls_rec->Tables[0].Rows[0]["BACKLOG_EA"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
			compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();

		CString where = "";
		if (st_no.Trim() != "")
		{
			where += " and ST_NO='" + st_no + "'";
		}
		if (date_c.Trim() != "")
		{
			where += " and DATE_C>='" + date_c.SubstringNE(2, 6) + "'";
		}
		if (date_e.Trim() != "")
		{
			where += " and DATE_C<='" + date_e.SubstringNE(2, 6) + "'";
		}
		if (backlog_ea.Trim() != "")
		{
			where += " and BACKLOG_EA='" + backlog_ea + "'";
		}
		if (compose_list_no.Trim() != "")
		{
			where += " and COMPOSE_LIST_NO='" + compose_list_no + "'";
		}

		// 组装最终SQL
		sqlstr = " select distinct COMPOSE_LIST_NO,ST_NO,'20' || DATE_C AS DATE_C,BACKLOG_EA,SINGLECOST,COST_DG from tfbsm14a where 1=1 " + where +
				 " order by COMPOSE_LIST_NO desc ";
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr = {0}", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
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
