/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15r_inq)

int f_fbsm15r_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
		Log::Trace("", "", "datetime = {0}", datetime);



		if (bcls_rec->Tables[0].Columns.Contains("DATE_C"))
			date_c = bcls_rec->Tables[0].Rows[0]["DATE_C"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("DATE_C_1"))
			date_c_1 = bcls_rec->Tables[0].Rows[0]["DATE_C_1"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		sqlstr = " SELECT t12.*, t11.BIG_CLASS_NAME,t11.COMM_FMLY_CODE,t11.MAT_DIVISION_DESC,t0.ST_NO_DESC FROM tfbsm12 t12 LEFT JOIN tfbsm11 t11 ON t12.ST_NO = t11.ST_NO LEFT JOIN  TQMTS0X t0 ON t12.ST_NO = t0.ST_NO WHERE 1 = 1";
		if (date_c.Trim() != ""  && date_c_1.Trim() == ""){
			sqlstr += " and DATE_TIME='" + date_c + "' ";
		}
		else if (date_c_1.Trim() != "" && date_c.Trim() == ""){
			sqlstr += " and DATE_TIME<='" + date_c_1 + "' ";
		}
		else if (date_c_1.Trim() != "" && date_c.Trim() != ""){
			sqlstr += " and DATE_TIME>='" + date_c + "' and DATE_TIME<='" + date_c_1 + "' ";
		}
		/*else{
		sqlstr += " and A.DATE_TIME='" + datetime + "' ";
		}*/
		if (st_no.Trim() != ""){
			sqlstr += " and t12.ST_NO like '%" + st_no + "%' ";
		}
		sqlstr += "  order by DATE_C DESC, t12.ST_NO, BACKLOG_EA,SEQ_NO ";
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
