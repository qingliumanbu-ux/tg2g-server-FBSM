/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 钢种静态数据查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm00_inq)

int f_fbsm00_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString comm_fmly_code = "";
	CString material_code = "";
	CString st_no = "";
	CString backlog_ea = "";
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("COMM_FMLY_CODE"))
			comm_fmly_code = bcls_rec->Tables[0].Rows[0]["COMM_FMLY_CODE"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("MATERIAL_CODE"))
			material_code = bcls_rec->Tables[0].Rows[0]["MATERIAL_CODE"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("BACKLOG_EA"))
			backlog_ea = bcls_rec->Tables[0].Rows[0]["BACKLOG_EA"].ToString().TrimOrBlank().ToUpper();

		if (st_no.Trim() != "")
		{
			sqlstr = "SELECT MATERIAL_CODE, COMM_FMLY_CODE FROM TFBSM11 WHERE ST_NO = '" + st_no + "'";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", __FUNCTION__, "根据ST_NO补全参数sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				material_code = cmd_inq.GetString(1);
				comm_fmly_code = cmd_inq.GetString(2);
			}
			cmd_inq.Close();
		}
		else if (st_no.Trim() == "" && material_code.Trim() != "")
		{
			sqlstr = "SELECT COMM_FMLY_CODE FROM TFBSM11 WHERE MATERIAL_CODE = '" + material_code + "'";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", __FUNCTION__, "根据MATERIAL_CODE补全参数sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				comm_fmly_code = cmd_inq.GetString(1);
			}
			cmd_inq.Close();
		}

		Log::Trace("", __FUNCTION__, "补全后查询条件st_no = {0}", st_no);
		Log::Trace("", __FUNCTION__, "补全后查询条件material_code = {0}", material_code);
		Log::Trace("", __FUNCTION__, "补全后查询条件comm_fmly_code = {0}", comm_fmly_code);

		sqlstr = " SELECT DISTINCT "
			"     t11.ST_NO, "
			"     t11.MATERIAL_CODE, "
			"     t11.COMM_FMLY_CODE, "
			"     NVL(t05.BACKLOG_EA, ' ') AS BACKLOG_EA, "
			"     CASE "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM01 t1 WHERE t1.ST_NO = t11.ST_NO AND t1.BACKLOG_EA = t05.BACKLOG_EA) THEN 1 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM01 t1 WHERE t1.ST_NO = t11.ST_NO) THEN 2 "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM01 t1 WHERE t1.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.MATERIAL_CODE = t11.MATERIAL_CODE AND t11_sub.MARK_POS_CODE = '1') AND t1.BACKLOG_EA = t05.BACKLOG_EA) THEN 3 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM01 t1 WHERE t1.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.MATERIAL_CODE = t11.MATERIAL_CODE AND t11_sub.MARK_POS_CODE = '1')) THEN 4 "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM01 t1 WHERE t1.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.COMM_FMLY_CODE = t11.COMM_FMLY_CODE AND t11_sub.MARK_POS_CODE = '2') AND t1.BACKLOG_EA = t05.BACKLOG_EA) THEN 5 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM01 t1 WHERE t1.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.COMM_FMLY_CODE = t11.COMM_FMLY_CODE AND t11_sub.MARK_POS_CODE = '2')) THEN 6 "
			"         ELSE 0 "
			"     END AS TFBSM01, "
			"     CASE "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM04 t4 WHERE t4.ST_NO = t11.ST_NO AND t4.BACKLOG_EA = t05.BACKLOG_EA) THEN 1 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM04 t4 WHERE t4.ST_NO = t11.ST_NO) THEN 2 "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM04 t4 WHERE t4.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.MATERIAL_CODE = t11.MATERIAL_CODE AND t11_sub.MARK_POS_CODE = '1') AND t4.BACKLOG_EA = t05.BACKLOG_EA) THEN 3 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM04 t4 WHERE t4.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.MATERIAL_CODE = t11.MATERIAL_CODE AND t11_sub.MARK_POS_CODE = '1')) THEN 4 "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM04 t4 WHERE t4.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.COMM_FMLY_CODE = t11.COMM_FMLY_CODE AND t11_sub.MARK_POS_CODE = '2') AND t4.BACKLOG_EA = t05.BACKLOG_EA) THEN 5 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM04 t4 WHERE t4.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.COMM_FMLY_CODE = t11.COMM_FMLY_CODE AND t11_sub.MARK_POS_CODE = '2')) THEN 6 "
			"         ELSE 0 "
			"     END AS TFBSM04, "
			"     CASE "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM05 t5 WHERE t5.ST_NO = t11.ST_NO AND t5.BACKLOG_EA = t05.BACKLOG_EA) THEN 1 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM05 t5 WHERE t5.ST_NO = t11.ST_NO) THEN 2 "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM05 t5 WHERE t5.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.MATERIAL_CODE = t11.MATERIAL_CODE AND t11_sub.MARK_POS_CODE = '1') AND t5.BACKLOG_EA = t05.BACKLOG_EA) THEN 3 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM05 t5 WHERE t5.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.MATERIAL_CODE = t11.MATERIAL_CODE AND t11_sub.MARK_POS_CODE = '1')) THEN 4 "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM05 t5 WHERE t5.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.COMM_FMLY_CODE = t11.COMM_FMLY_CODE AND t11_sub.MARK_POS_CODE = '2') AND t5.BACKLOG_EA = t05.BACKLOG_EA) THEN 5 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM05 t5 WHERE t5.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.COMM_FMLY_CODE = t11.COMM_FMLY_CODE AND t11_sub.MARK_POS_CODE = '2')) THEN 6 "
			"         ELSE 0 "
			"     END AS TFBSM05, "
			"     CASE "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM09 t9 WHERE t9.ST_NO = t11.ST_NO AND t9.BACKLOG_EA = t05.BACKLOG_EA) THEN 1 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM09 t9 WHERE t9.ST_NO = t11.ST_NO) THEN 2 "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM09 t9 WHERE t9.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.MATERIAL_CODE = t11.MATERIAL_CODE AND t11_sub.MARK_POS_CODE = '1') AND t9.BACKLOG_EA = t05.BACKLOG_EA) THEN 3 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM09 t9 WHERE t9.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.MATERIAL_CODE = t11.MATERIAL_CODE AND t11_sub.MARK_POS_CODE = '1')) THEN 4 "
			"         WHEN t05.BACKLOG_EA IS NOT NULL AND EXISTS (SELECT 1 FROM TFBSM09 t9 WHERE t9.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.COMM_FMLY_CODE = t11.COMM_FMLY_CODE AND t11_sub.MARK_POS_CODE = '2') AND t9.BACKLOG_EA = t05.BACKLOG_EA) THEN 5 "
			"         WHEN EXISTS (SELECT 1 FROM TFBSM09 t9 WHERE t9.ST_NO IN (SELECT ST_NO FROM TFBSM11 t11_sub WHERE t11_sub.COMM_FMLY_CODE = t11.COMM_FMLY_CODE AND t11_sub.MARK_POS_CODE = '2')) THEN 6 "
			"         ELSE 0 "
			"     END AS TFBSM09 "
			" FROM TFBSM11 t11 "
			" LEFT JOIN ( "
			"     SELECT DISTINCT ST_NO, BACKLOG_EA FROM TFBSM05 "
			"     UNION ALL "
			"     SELECT DISTINCT t11.ST_NO, t05.BACKLOG_EA "
			"     FROM TFBSM11 t11 "
			"     JOIN TFBSM11 rep ON t11.MATERIAL_CODE = rep.MATERIAL_CODE AND rep.MARK_POS_CODE = '1' "
			"     JOIN TFBSM05 t05 ON rep.ST_NO = t05.ST_NO "
			"     WHERE NOT EXISTS (SELECT 1 FROM TFBSM05 t WHERE t.ST_NO = t11.ST_NO) "
			"     UNION ALL "
			"     SELECT DISTINCT t11.ST_NO, t05.BACKLOG_EA "
			"     FROM TFBSM11 t11 "
			"     JOIN TFBSM11 rep ON t11.COMM_FMLY_CODE = rep.COMM_FMLY_CODE AND rep.MARK_POS_CODE = '2' "
			"     JOIN TFBSM05 t05 ON rep.ST_NO = t05.ST_NO "
			"     WHERE NOT EXISTS (SELECT 1 FROM TFBSM05 t WHERE t.ST_NO = t11.ST_NO) "
			"     AND NOT EXISTS ( "
			"         SELECT 1 FROM TFBSM11 r JOIN TFBSM05 t ON r.ST_NO = t.ST_NO "
			"         WHERE r.MATERIAL_CODE = t11.MATERIAL_CODE AND r.MARK_POS_CODE = '1' "
			"     ) "
			" ) t05 ON t11.ST_NO = t05.ST_NO "
			" WHERE 1=1 ";

		if (st_no.Trim() != "")
		{
			sqlstr += " AND t11.ST_NO LIKE '%' || @ST_NO || '%' ";
			cmd_inq.Parameters.Set("ST_NO", st_no);
		}
		if (material_code.Trim() != "")
		{
			sqlstr += " AND t11.MATERIAL_CODE LIKE '%' || @MATERIAL_CODE || '%' ";
			cmd_inq.Parameters.Set("MATERIAL_CODE", material_code);
		}
		if (comm_fmly_code.Trim() != "")
		{
			sqlstr += " AND t11.COMM_FMLY_CODE LIKE '%' || @COMM_FMLY_CODE || '%' ";
			cmd_inq.Parameters.Set("COMM_FMLY_CODE", comm_fmly_code);
		}
		if (backlog_ea.Trim() != "")
		{
			sqlstr += " AND NVL(t05.BACKLOG_EA, ' ') LIKE '%' || @BACKLOG_EA || '%' ";
			cmd_inq.Parameters.Set("BACKLOG_EA", backlog_ea);
		}

		sqlstr += " ORDER BY t11.ST_NO, NVL(t05.BACKLOG_EA, ' ') ";

		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "Main query sqlstr = {0}", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
	}
	catch (CDbException &ex)
	{
		CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
		CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char *)str, sizeof(s.sysmsg) - 1);
		s.sysmsg[sizeof(s.sysmsg) - 1] = '\0';
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

