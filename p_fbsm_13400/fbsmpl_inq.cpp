/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: H5静态表查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsmpl_inq)

int f_fbsmpl_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString mat_code = "";
	CString comm_fmly_code = "";
	CString material_code = "";
	CString st_no = "";
	CString backlog_ea = "";
	CString table_id = "";
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("MAT_CODE"))
			mat_code = bcls_rec->Tables[0].Rows[0]["MAT_CODE"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("COMM_FMLY_CODE"))
			comm_fmly_code = bcls_rec->Tables[0].Rows[0]["COMM_FMLY_CODE"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("MATERIAL_CODE"))
			material_code = bcls_rec->Tables[0].Rows[0]["MATERIAL_CODE"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("BACKLOG_EA"))
			backlog_ea = bcls_rec->Tables[0].Rows[0]["BACKLOG_EA"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[1].Columns.Contains("TABLE_ID"))
			table_id = bcls_rec->Tables[1].Rows[0]["TABLE_ID"].ToString().TrimOrBlank().ToUpper();

		Log::Trace("", "", "linke = {0},table_id = [{1}]", __LINE__, table_id);

		if (table_id.Trim() == "" )
		{
			strcpy(s.msg, "表名未传入!!!");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		if (comm_fmly_code.Trim() == "" && st_no.Trim() != "")
		{
			sqlstr = "SELECT COMM_FMLY_CODE FROM TFBSM11 WHERE ST_NO = '" + st_no + "'";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "查询大类sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				comm_fmly_code = cmd_inq.GetString(1);
			}
			cmd_inq.Close();
		}

		if (table_id == "TFBSM05")
		{
			sqlstr = " SELECT T11.*,T06.backlog_ea "
				" FROM  TFBSM11 T11 "
				" LEFT JOIN TFBSM06 T06 "
				" ON T11.ST_NO = T06.ST_NO "
				" WHERE 1=1 ";
			if (comm_fmly_code.Trim() != "")
			{
				sqlstr += " and T11.comm_fmly_code like'%" + comm_fmly_code + "%' ";
			}
			if (mat_code.Trim() != "")
			{
				sqlstr += " and T11.mat_code like'%" + mat_code + "%' ";
			}
			if (material_code.Trim() != "")
			{
				sqlstr += " and T11.material_code like'%" + material_code + "%' ";
			}
			if (st_no.Trim() != "")
			{
				sqlstr += " and T11.st_no like'%" + st_no + "%' ";
			}
			if (backlog_ea.Trim() != "")
			{
				sqlstr += " and T06.backlog_ea like'%" + backlog_ea + "%' ";
			}

			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "st_no查询sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		}
		else if (table_id == "TFBSM01")
		{
			sqlstr = " SELECT * "
				" FROM  TFBSM11 T11 "
				" WHERE 1=1 ";
			if (comm_fmly_code.Trim() != "")
			{
				sqlstr += " and T11.comm_fmly_code like'%" + comm_fmly_code + "%' ";
			}
			if (mat_code.Trim() != "")
			{
				sqlstr += " and T11.mat_code like'%" + mat_code + "%' ";
			}
			if (material_code.Trim() != "")
			{
				sqlstr += " and T11.material_code like'%" + material_code + "%' ";
			}
			if (st_no.Trim() != "")
			{
				sqlstr += " and T11.st_no like'%" + st_no + "%' ";
			}
			if (backlog_ea.Trim() != "")
			{
				sqlstr += " and T11.backlog_ea like'%" + backlog_ea + "%' ";
			}

			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "st_no查询sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);

		}

		
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
		strncpy(s.msg, ex.GetMsg(), sizeof(s.msg) - 1);
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
