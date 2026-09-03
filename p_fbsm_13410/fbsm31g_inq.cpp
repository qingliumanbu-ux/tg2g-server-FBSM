/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 碳钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm31g_inq)

int f_fbsm31g_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	CString date_time = "";       // 日期（对应表字段DATE_C）
	CString compose_list_no = ""; // 配料单号
	CString st_no = "";           // 出钢记号（输入）
	CString query_st_no = "";     // 实际查询用的ST_NO（品种钢=原值，非品种钢=ALL）
	CString backlog_ea = " ";     // 钢区工序途径

	try
	{
		// ==================== 1. 获取输入参数 ====================
		if (bcls_rec->Tables[0].Columns.Contains("DATE_TIME"))
			date_time = bcls_rec->Tables[0].Rows[0]["DATE_TIME"].ToString().TrimOrBlank();
		if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
			compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("BACKLOG_EA"))
			backlog_ea = bcls_rec->Tables[0].Rows[0]["BACKLOG_EA"].ToString().TrimOrBlank().ToUpper();

		Log::Trace("", "", "date_time = {0}", date_time);
		Log::Trace("", "", "compose_list_no = {0}", compose_list_no);
		Log::Trace("", "", "st_no = {0}", st_no);
		Log::Trace("", "", "backlog_ea = {0}", backlog_ea);

		// ==================== 2. 品种钢判断 ====================
		// 规则：ST_NO在TFBSM10能查到数据→品种钢（query_st_no=原值）；否则→非品种钢（query_st_no=ALL）
		query_st_no = st_no;
		if (st_no != "" && st_no != "ALL")
		{
			sqlstr = "SELECT COUNT(1) FROM TFBSM10 WHERE ST_NO = '" + st_no + "'";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				if (cmd_inq.GetDecimal(1) <= 0)
				{
					query_st_no = "ALL"; // 非品种钢，统一用ALL查
					Log::Trace("", __FUNCTION__, "ST_NO=[{0}]在TFBSM10中不存在，非品种钢，查询ST_NO使用ALL", st_no);
				}
				else
				{
					Log::Trace("", __FUNCTION__, "ST_NO=[{0}]在TFBSM10中存在，品种钢，查询ST_NO使用[{0}]", st_no);
				}
			}
			cmd_inq.Close();
		}
		else if (st_no == "")
		{
			query_st_no = "ALL";
		}

		// ==================== 3. 查询TFBSM33（LG_PSSM计划过程数据） ====================
		bcls_ret->Tables.Add();
		sqlstr = "SELECT * FROM TFBSM33 WHERE 1=1 ";
		if (compose_list_no != "")
		{
			sqlstr += "AND COMPOSE_LIST_NO = '" + compose_list_no + "' ";
		}
		if (st_no != "")
		{
			sqlstr += "AND ST_NO = '" + st_no + "' ";
		}
		if (date_time != "")
		{
			sqlstr += "AND DATE_C = '" + date_time.SubstringNE(2, 6) + "' ";
		}
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "TFBSM33.sqlstr = [{0}]", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		// ==================== 4. 查询TFBSM34（MatTable物料库存过程数据） ====================
		bcls_ret->Tables.Add();
		sqlstr = "SELECT * FROM TFBSM34 WHERE 1=1 ";
		if (compose_list_no != "")
		{
			sqlstr += "AND COMPOSE_LIST_NO = '" + compose_list_no + "' ";
		}
		if (query_st_no != "")
		{
			sqlstr += "AND ST_NO = '" + query_st_no + "' ";
		}
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "TFBSM34.sqlstr = [{0}]", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[1]);
		cmd_inq.Close();

		// ==================== 5. 查询TFBSM35（CHARGE_RANGE投料范围过程数据） ====================
		bcls_ret->Tables.Add();
		sqlstr = "SELECT * FROM TFBSM35 WHERE 1=1 ";
		if (compose_list_no != "")
		{
			sqlstr += "AND COMPOSE_LIST_NO = '" + compose_list_no + "' ";
		}
		if (query_st_no != "")
		{
			sqlstr += "AND ST_NO = '" + query_st_no + "' ";
		}
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "TFBSM35.sqlstr = [{0}]", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[2]);
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
