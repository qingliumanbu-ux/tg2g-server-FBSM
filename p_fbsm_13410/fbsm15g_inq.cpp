/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15g_inq)

int f_fbsm15g_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	
	CDbCommand cmd_inq(conn);
	CString date_time = "";
	CString compose_list_no = "";
	CString st_no = "";
	CString backlog_ea = " ";
	CString material_code = " ";
	CString comm_fmly_code = " ";

	CString datetime = CDateTime::Now().ToString("yyyyMMdd");
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("DATE_TIME"))
			date_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME"].ToString();
		if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
			compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("BACKLOG_EA"))
			backlog_ea = bcls_rec->Tables[0].Rows[0]["BACKLOG_EA"].ToString().TrimOrBlank().ToUpper();
		Log::Trace("", "", "date_time = {0}", date_time);
		Log::Trace("", "", "st_no = {0}", st_no);
		
		sqlstr = " select COMM_FMLY_CODE,MATERIAL_CODE from TFBSM11 where ST_NO='" + st_no + "' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			comm_fmly_code = cmd_inq.GetString(1);
			material_code = cmd_inq.GetString(2);
		}
		cmd_inq.Close();

		// 确定t01查询使用的ST_NO：先查当前，没有则中类代表，再没有则大类代表
		CString t01_st_no = st_no;
		sqlstr = "SELECT COUNT(*) FROM TFBSM01 WHERE ST_NO = '" + st_no + "' AND (BACKLOG_EA = ' ' OR BACKLOG_EA = '" + backlog_ea + "')";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			if (cmd_inq.GetString(1) == "0")
			{
				cmd_inq.Close();
				// 找中类代表
				sqlstr = "SELECT ST_NO FROM TFBSM11 WHERE MATERIAL_CODE = '" + material_code + "' AND MARK_POS_CODE = '1'";
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					t01_st_no = cmd_inq.GetString(1);
				}
				else
				{
					cmd_inq.Close();
					// 找大类代表
					sqlstr = "SELECT ST_NO FROM TFBSM11 WHERE COMM_FMLY_CODE = '" + comm_fmly_code + "' AND MARK_POS_CODE = '2'";
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.ExecuteReader();
					if (cmd_inq.Read())
					{
						t01_st_no = cmd_inq.GetString(1);
					}
				}
			}
		}
		cmd_inq.Close();
		Log::Trace("", __FUNCTION__, "t01使用ST_NO = {0}", t01_st_no);

		
		sqlstr = " WITH t01 AS ("
			" SELECT MAT_CODE, MAT_NAME, STATION_ID, '" + st_no + "' AS ST_NO, BACKLOG_EA, BACK_C2"
			" FROM ("
			" SELECT a.MAT_CODE, a.MAT_NAME, a.STATION_ID, a.ST_NO, a.BACKLOG_EA, NVL(m50.MAT_TYPE_PL1, a.BACK_C2) AS BACK_C2,"
			" ROW_NUMBER() OVER (PARTITION BY a.MAT_CODE ORDER BY CASE WHEN a.BACKLOG_EA = '" + backlog_ea + "' THEN 1 ELSE 2 END) AS rn"
			" FROM TFBSM01 a"
			" LEFT JOIN TMMSM50 m50 ON m50.MAT_CODE = a.MAT_CODE"
			" WHERE a.ST_NO = '" + t01_st_no + "'"
			" AND (a.BACKLOG_EA = ' ' OR a.BACKLOG_EA = '" + backlog_ea + "')"
			" )"
			" WHERE rn = 1"
			" ),"
			" t18 AS ("
			" SELECT * FROM TFBSM18"
			" WHERE COMPOSE_LIST_NO = '" + compose_list_no + "'"
			" AND ST_NO = '" + st_no + "'"
			" AND BACKLOG_EA = '" + backlog_ea + "'"
			" )"
			" SELECT"
			" b.REC_CREATOR,"
			" b.REC_CREATE_TIME,"
			" b.REC_REVISOR,"
			" b.REC_REVISE_TIME,"
			" b.FACTORY_DIV,"
			" NVL(a.MAT_CODE, b.MAT_CODE) AS MAT_CODE,"
			" NVL(a.MAT_NAME, b.MAT_NAME) AS MAT_NAME,"
			" b.STOCK_NAME,"
			" NVL(b.STOCK_WT, 0) AS STOCK_WT,"
			" b.LOT_NO,"
			" b.STK_NO,"
			" b.WEIGHT,"
			" b.C_VALUE,"
			" b.SI_VALUE,"
			" b.MN_VALUE,"
			" b.P_VALUE,"
			" b.S_VALUE,"
			" b.CR_VALUE,"
			" b.NI_VALUE,"
			" b.MO_VALUE,"
			" b.CU_VALUE,"
			" b.CO_VALUE,"
			" b.BUNKER_TYPE,"
			" b.QUALITY_BATCH_NO,"
			" b.STOCK_WT_QC,"
			" NVL(b.DATE_C, (SELECT MAX(DATE_C) FROM t18)) AS DATE_C,"
			" b.PRICE,"
			" NVL(a.STATION_ID, b.STATION_ID) AS STATION_ID,"
			" NVL(a.ST_NO, b.ST_NO) AS ST_NO,"
			" NVL(b.BACKLOG_EA, '" + backlog_ea + "') AS BACKLOG_EA,"
			" b.SEQ_NO,"
			" NVL(b.BACK_C2, a.BACK_C2) AS BACK_C2,"
			" NVL(b.COMPOSE_LIST_NO, '" + compose_list_no + "') AS COMPOSE_LIST_NO,"
			" b.TI_VALUE,"
			" b.NB_VALUE,"
			" b.AL_VALUE,"
			" b.B_VALUE,"
			" b.V_VALUE,"
			" b.CA_VALUE,"
			" b.N_VALUE"
			" FROM t18 b"
			" FULL OUTER JOIN t01 a"
			" ON a.MAT_CODE = b.MAT_CODE"
			" ORDER BY CASE WHEN b.MAT_CODE IS NULL THEN 1 ELSE 0 END,"
			" NVL(a.MAT_CODE, b.MAT_CODE)";

		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr = {0}", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();
		bcls_ret->Tables.Add();
		
		sqlstr = " SELECT * FROM TFBSM19 WHERE COMPOSE_LIST_NO='" + compose_list_no  + "' and ST_NO='" + st_no + "' and BACKLOG_EA='" + backlog_ea + "'";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[1]);
		cmd_inq.Close();
		
		bcls_ret->Tables.Add();
		sqlstr = " SELECT * FROM TFBSM26 WHERE COMPOSE_LIST_NO='" + compose_list_no  + "' and ST_NO='" + st_no + "'";
		Log::Trace("", __FUNCTION__, "inDMST02.GX_MAT.sqlstr = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[2]);
		cmd_inq.Close();
		
		bcls_ret->Tables.Add();
		sqlstr = " SELECT * FROM TFBSM27 WHERE COMPOSE_LIST_NO='" + compose_list_no  + "' and ST_NO='" + st_no + "' and BACKLOG_EA='" + backlog_ea + "'";
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "inDMST02.GX_WEIGHT.sqlstr = [{0}]", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[3]);
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
