/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2026/3/5
Description: 配料单的物料代码和库存信息
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm31z_material_inq)

int f_fbsm31z_material_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int i = 1;
	CString sqlstr = " ";
	int v_count = 0;
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_q(conn);
	CDbCommand cmd(conn);
	CString station_id = "";
	CString st_no = "";
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();

		Log::Trace("", __FUNCTION__, "st_no = [{0}],station_id = [{1}]", st_no, station_id);

		// 查询对应的st_no 是否有配料
		sqlstr = " select count(1) "
			" from tfbsm10 "
			" where st_no = @st_no ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("st_no", st_no);
		v_count = cmd_inq.ExecuteScalar().ToInt32();
		Log::Trace("", __FUNCTION__, "v_count = [{0}]", v_count);
		cmd_inq.Close();

		// 用出钢记号查不到，说明是非品种钢，用ALL来查
		if (v_count == 0)
		{
			st_no = "ALL";
		}

		sqlstr = " SELECT "
			" ROW_NUMBER() OVER (ORDER BY t10.BACK_C2, t10.MAT_CODE) AS SEQ_ID,"
			" t10.MAT_NAME,"
			" t10.MAT_CODE,"
			" t10.BACK_C2 AS BACK_C2,"
			" t50.YEILD_MINUS AS MAT_YEILD"
			" FROM TFBSM10 t10  "
			" LEFT JOIN TMMSM50 t50 "
			" ON t10.MAT_CODE = t50.MAT_CODE"
			" where 1=1 "
			" and t10.st_no = @st_no "
			" ORDER BY t10.BACK_C2, t10.MAT_CODE ";

		Log::Trace("", __FUNCTION__, "BOF");
		cmd_inq_q.SetCommandText(sqlstr);
		cmd_inq_q.Parameters.Set("st_no", st_no);
		cmd_inq_q.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq_q.Close();

		Log::Trace("", __FUNCTION__, "行数 = [{0}]", bcls_ret->Tables[0].Rows.get_Count());

		
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
