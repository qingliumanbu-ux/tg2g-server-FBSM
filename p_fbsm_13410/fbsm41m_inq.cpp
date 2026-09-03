/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 电炉配料单查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm41m_inq)

int f_fbsm41m_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString begin_date = "";
	CString end_date = "";
	CString st_no = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMdd");
	try
	{

		begin_date = bcls_rec->Tables[0].Rows[0]["BEGIN_DATE"].ToString();
		end_date = bcls_rec->Tables[0].Rows[0]["END_DATE"].ToString();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString();
		Log::Trace("", __FUNCTION__, "st_no = [{0}]]", st_no);

		sqlstr = " SELECT * FROM TFBSM43 T43 "
				 " WHERE 1=1"
				 " AND T43.CHECK_FLAG = '1' ";
		if (end_date.Trim() != "")
			sqlstr = sqlstr + " AND T43.CHECK_DATE<=@end_date";
		if (begin_date.Trim() != "")
			sqlstr = sqlstr + " AND T43.CHECK_DATE>=@begin_date";
		if (st_no.Trim() != "")
			sqlstr = sqlstr + " AND T43.ST_NO=@st_no";

		sqlstr = sqlstr + " ORDER BY ST_NO, CHECK_DATE DESC, COMPOSE_LIST_NO_EAF ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("end_date", end_date);
		cmd_inq.Parameters.Set("begin_date", begin_date);
		cmd_inq.Parameters.Set("st_no", st_no);
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
