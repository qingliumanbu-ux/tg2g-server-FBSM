/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2026/03/16
Description: 不锈钢模板库配料单查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15m_inq)

int f_fbsm15m_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString date_c = "";
	CString date_e = "";
	CString backlog_ea = "";
	CString st_no = " ";
	CString seq_no = "";
	CString mb_mark = "";
	CString comm_fmly_code = "";
	CString material_code = "";
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
		/*if (bcls_rec->Tables[0].Columns.Contains("SEQ_NO"))
			seq_no = bcls_rec->Tables[0].Rows[0]["SEQ_NO"].ToString().TrimOrBlank().ToUpper();*/
		if (bcls_rec->Tables[0].Columns.Contains("COMM_FMLY_CODE"))
			comm_fmly_code = bcls_rec->Tables[0].Rows[0]["COMM_FMLY_CODE"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("MATERIAL_CODE"))
			material_code = bcls_rec->Tables[0].Rows[0]["MATERIAL_CODE"].ToString().TrimOrBlank().ToUpper();

		// 动态拼接 t12 条件
		CString where12 = "";
		if (st_no.Trim() != "") {
			where12 += " and t12.ST_NO = '" + st_no + "'";
		}
		if (backlog_ea.Trim() != "") {
			where12 += " and t12.BACKLOG_EA = '" + backlog_ea + "'";
		}
		if (comm_fmly_code.Trim() != "") {
			where12 += " and t11.COMM_FMLY_CODE = '" + comm_fmly_code + "'";
		}
		if (material_code.Trim() != "") {
			where12 += " and t12.MATERIAL_CODE = '" + material_code + "'";
		}
		if (date_c.Trim() != "") {
			where12 += " and t12.MB_CREATE_TIME >= '" + date_c + "'";
		}
		if (date_e.Trim() != "") {
			where12 += " and t12.MB_CREATE_TIME <= '" + date_e + "'";
		}

		// 动态拼接 t31 条件（只有 ST_NO 和 BACKLOG_EA）
		CString where31 = "";
		if (st_no.Trim() != "") {
			where31 += " and t31.ST_NO = '" + st_no + "'";
		}
		if (backlog_ea.Trim() != "") {
			where31 += " and t31.BACKLOG_EA = '" + backlog_ea + "'";
		}
		if (date_c.Trim() != "") {
			where31 += " and t31.MB_CREATE_TIME >= '" + date_c + "'";
		}
		if (date_e.Trim() != "") {
			where31 += " and t31.MB_CREATE_TIME <= '" + date_e + "'";
		}

		// 组装 SQL
		sqlstr = " SELECT * FROM ( "
			" SELECT DISTINCT "
			"   t12.COMPOSE_LIST_NO, "
			"   t12.COST_DG, "
			"   t12.SINGLECOST, "
			"   t12.MB_CREATOR, "
			"   t12.MB_CREATE_TIME, "
			"   t12.BACKLOG_EA, "
			"   t12.MATERIAL_CODE, "
			"   t11.COMM_FMLY_CODE, "
			"   t12.DATE_TIME, "
			"   t12.ST_NO, "
			"   t12.REMARK_PS, "
			"   t12.BACK_AOD, "
			"   t12.BACK_BOF, "
			"   t12.BACK_IF, "
			"   t12.BACK_EAF, "
			"   '12' as SOURCE_TABLE "
			" FROM TFBSM12 t12 "
			" LEFT JOIN TFBSM11 t11 ON t11.MATERIAL_CODE = t12.MATERIAL_CODE "
			" WHERE 1=1 "
			"   AND t12.MB_MARK = '1' " + where12 +
			" UNION ALL "
			" SELECT DISTINCT "
			"   t31.COMPOSE_LIST_NO, "
			"   t31.COST_DG, "
			"   t31.SINGLECOST, "
			"   t31.MB_CREATOR, "
			"   t31.MB_CREATE_TIME, "
			"   t31.BACKLOG_EA, "
			"   NULL as MATERIAL_CODE, "
			"   NULL as COMM_FMLY_CODE, "
			"   NULL as DATE_TIME, "
			"   t31.ST_NO, "
			"   t31.REMARK_PS, "
			"   NULL as BACK_AOD, "
			"   t31.BACK_BOF, "
			"   NULL as BACK_IF, "
			"   NULL as BACK_EAF, "
			"   '31' as SOURCE_TABLE "
			" FROM TFBSM31 t31 "
			" WHERE 1=1 "
			"   AND t31.MB_MARK = '1' " + where31 +
			" ) ORDER BY COMPOSE_LIST_NO DESC ";
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
