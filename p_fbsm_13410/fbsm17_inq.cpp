/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm17_inq)

int f_fbsm17_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
		
			Log::Trace("", __FUNCTION__, "start");
			begin_date = bcls_rec->Tables[0].Rows[0]["BEGIN_DATE"].ToString().SubstringNE(0, 8);
			Log::Trace("", __FUNCTION__, "BEGIN_DATE = [{0}]",begin_date);
			end_date = bcls_rec->Tables[0].Rows[0]["END_DATE"].ToString().SubstringNE(0, 8);
			Log::Trace("", __FUNCTION__, "END_DATE = [{0}]",end_date);
			if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
				st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString();
			Log::Trace("", __FUNCTION__, "st_no = [{0}]],st_no = [{1}]]", st_no.SubstringNE(1, 1), st_no.SubstringNE(2, 1));

			sqlstr = " select t.*,t2.MATERIAL_CODE,t2.comm_fmly_code,t2.material_code,t2.ST_NO_DESC"
				",(SELECT DESCRIP FROM TFBSM02 WHERE CODE = t.BACKLOG_EA) as BACKLOG_EA_DESC"
				" from "
				" (SELECT ST_NO,BACKLOG_EA,DATE_TIME,DATE_C,COUNT(1) AS FURNACE_COUNT,REMARK_PS,COMPOSE_LIST_NO,SINGLECOST,COST_DG,CHECK_FLAG,CHECK_MAKE,CHECK_DATE,ERROR_REMARK,ORIGIN_CODE"
				" FROM TFBSM12 "
				" WHERE 1=1"
				;
			if (end_date.Trim() != "")
				sqlstr = sqlstr + " AND DATE_TIME<=@end_date";
			if (begin_date.Trim() != "")
				sqlstr = sqlstr + " AND DATE_TIME>=@begin_date";
			if (st_no.Trim() != "")
				sqlstr = sqlstr + " AND st_no=@st_no";
			
			sqlstr = sqlstr + " GROUP BY ST_NO, BACKLOG_EA, DATE_TIME,DATE_C, REMARK_PS, COMPOSE_LIST_NO,  SINGLECOST, COST_DG, CHECK_FLAG, CHECK_MAKE, CHECK_DATE,ERROR_REMARK,ORIGIN_CODE"
				" ORDER BY DATE_TIME DESC, ST_NO, BACKLOG_EA ) t left join TFBSM11 t2 on t.st_no=t2.st_no"
				" ORDER BY t.DATE_TIME DESC, t.ST_NO, t.BACKLOG_EA,COMPOSE_LIST_NO DESC"
				;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("end_date", end_date);
		cmd_inq.Parameters.Set("begin_date", begin_date);
		cmd_inq.Parameters.Set("st_no", st_no);
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
