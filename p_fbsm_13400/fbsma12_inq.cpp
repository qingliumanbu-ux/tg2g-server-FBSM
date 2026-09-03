/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: H5静态表查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsma12_inq)

int f_fbsma12_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString date_c = "";
	CString date_c_1 = "";
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("DATE_C"))
			date_c = bcls_rec->Tables[0].Rows[0]["DATE_C"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("DATE_C_1"))
			date_c_1 = bcls_rec->Tables[0].Rows[0]["DATE_C_1"].ToString().TrimOrBlank().ToUpper();
		sqlstr = " SELECT * FROM( "
				 "   select DATE_TIME,DATE_C,FURNACE_COUNT,ST_NO,REMARK_PS,BACKLOG_EA,CC_MACH_NO,REC_CREATOR,REC_CREATE_TIME,REC_REVISOR,REC_REVISE_TIME from tfbsm12 WHERE SEQ_NOW='1' "
				 "   UNION ALL "
				 "   select DATE_TIME,DATE_C,FURNACE_COUNT,ST_NO,REMARK_PS,BACKLOG_EA,CC_MACH_NO,REC_CREATOR,REC_CREATE_TIME,REC_REVISOR,REC_REVISE_TIME from tfbsm31 WHERE SEQ_NOW='1' "
				 " ) WHERE 1=1 ";
		if (date_c.Trim() != "" && date_c_1.Trim() == "")
		{
			sqlstr += " and DATE_TIME>='" + date_c + "' ";
		}
		else if (date_c_1.Trim() != "" && date_c.Trim() == "")
		{
			sqlstr += " and DATE_TIME<='" + date_c_1 + "' ";
		}
		else if (date_c_1.Trim() != "" && date_c.Trim() != "")
		{
			sqlstr += " and DATE_TIME>='" + date_c + "' and DATE_TIME<='" + date_c_1 + "' ";
		}
		sqlstr += " order by CC_MACH_NO,DATE_C ";
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
