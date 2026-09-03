/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 电炉配料单查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm41_inq)

int f_fbsm41_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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

		begin_date = bcls_rec->Tables[0].Rows[0]["BEGIN_DATE"].ToString().SubstringNE(0, 8);
		end_date = bcls_rec->Tables[0].Rows[0]["END_DATE"].ToString().SubstringNE(0, 8);
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString();
		Log::Trace("", __FUNCTION__, "st_no = [{0}]],st_no = [{1}]]", st_no.SubstringNE(1, 1), st_no.SubstringNE(2, 1));

		sqlstr = " SELECT T12.ST_NO,T12.BACKLOG_EA,T12.DATE_TIME,T12.DATE_C,T12.SEQ_NO,T12.REMARK_PS,T12.COMPOSE_LIST_NO,"
				 " T12.SINGLECOST,T12.COST_DG,T43.ERROR_REMARK,T12.ORIGIN_CODE,"
				 " (SELECT COUNT(1) FROM TFBSM12 T12_2 WHERE T12_2.COMPOSE_LIST_NO = T12.COMPOSE_LIST_NO) AS FURNACE_COUNT,"
				 " T11.MATERIAL_CODE,T11.comm_fmly_code,T11.ST_NO_DESC,"
				 " T12.COMPOSE_LIST_NO_EAF AS COMPOSE_LIST_NO_EAF,T43.DEV_CODE,T43.REMARK,T43.RESP,T43.SHIFT_GROUP,"
				 " T43.CHECK_FLAG,T43.CHECK_MAKE,T43.CHECK_DATE,T43.EAF_PROC_NO,"
				 " (SELECT DESCRIP FROM TFBSM02 WHERE CODE = T12.BACKLOG_EA) AS BACKLOG_EA_DESC"
				 " FROM TFBSM12 T12 "
				 " LEFT JOIN TFBSM11 T11 ON T12.ST_NO = T11.ST_NO "
				 " INNER JOIN TFBSM02 T02 ON T02.CODE = T12.BACKLOG_EA AND T02.BACKLOG_EA LIKE '%EAF%' "
				 " LEFT JOIN TFBSM43 T43 ON T43.COMPOSE_LIST_NO_EAF = T12.COMPOSE_LIST_NO_EAF "
				 " WHERE 1=1"
				 " AND (T12.CHECK_FLAG = '1' OR T12.COMPOSE_LIST_NO = ' ')";
		if (end_date.Trim() != "")
			sqlstr = sqlstr + " AND T12.DATE_TIME<=@end_date";
		if (begin_date.Trim() != "")
			sqlstr = sqlstr + " AND T12.DATE_TIME>=@begin_date";
		if (st_no.Trim() != "")
			sqlstr = sqlstr + " AND T12.ST_NO=@st_no";

		sqlstr = sqlstr + " ORDER BY CASE WHEN T12.COMPOSE_LIST_NO = ' ' THEN 1 ELSE 0 END, T12.DATE_TIME DESC, T12.ST_NO, T12.BACKLOG_EA, T12.SEQ_NO ";
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
