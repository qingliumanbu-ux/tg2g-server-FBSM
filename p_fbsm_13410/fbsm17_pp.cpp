/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 计划匹配配料单
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm17_pp)

int f_fbsm17_pp(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int i = 1;	
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd(conn);
	CString compose_list_no = "";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CModel tfbsm12("TFBSM12");
	CModel tfbsm14a("TFBSM14A");
	try
	{
		//获取是那个配料单
		compose_list_no = bcls_rec->Tables["LD"].Rows[0]["COMPOSE_LIST_NO"].ToString();
		sqlstr = " select CHECK_FLAG,CHECK_MAKE,CHECK_DATE,ORIGIN_CODE,BACKLOG_EA,SINGLECOST,COST_DG "
			" from tfbsm14a"
			" where compose_list_no = @compose_list_no"
			;
		
		cmd_inq.SetCommandText(sqlstr); 
		cmd_inq.Parameters.Set("compose_list_no", compose_list_no);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			if (cmd_inq.GetString(1) != "1")
			{
				sprintf(s.msg, "配料单号:[" + compose_list_no + "]未审核,不能匹配使用。");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			tfbsm14a["CHECK_FLAG"] = cmd_inq.GetString(1);
			tfbsm14a["CHECK_MAKE"] = cmd_inq.GetString(2);
			tfbsm14a["CHECK_DATE"] = cmd_inq.GetString(3);
			tfbsm14a["ORIGIN_CODE"] = cmd_inq.GetString(4);
			tfbsm14a["BACKLOG_EA"] = cmd_inq.GetString(5);
			tfbsm14a["SINGLECOST"] = cmd_inq.GetDecimal(6);
			tfbsm14a["COST_DG"] = cmd_inq.GetDecimal(7);
		}
		cmd_inq.Close();

		//返回配料单
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "COMPOSE_LIST_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACKLOG_EA");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO");
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "DATE_TIME");
		bcls_ret->Tables[0].Rows.Add();
		

		for (int p = 0; p < bcls_rec->Tables["Main"].Rows.get_Count(); p++)
		{
			tfbsm12.Reset();
			tfbsm12.MergeFrom(bcls_rec->Tables["Main"].Rows[p]);
			tfbsm12.TrimOrBlank();
			if (tfbsm14a["BACKLOG_EA"].ToString() != tfbsm12["BACKLOG_EA"].ToString())
			{
				sprintf(s.msg, "配料单号的路径与需匹配的钢种[" + tfbsm12["ST_NO"].ToString() + "]对应的路径不同,不能匹配使用。");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			sqlstr = " update tfbsm12 set COMPOSE_LIST_NO = @compose_list_no_new"
				",CHECK_FLAG = @check_flag"
				",CHECK_MAKE = @check_make"
				",CHECK_DATE = @check_date"
				",ORIGIN_CODE = @origin_code"
				",SINGLECOST = @singlecost"
				",COST_DG = @cost_dg"
				" where 1=1"
				" AND COMPOSE_LIST_NO = compose_list_no"
				" AND BACKLOG_EA = @backlog_ea"
				" AND ST_NO = @st_no"
				" AND DATE_TIME = @date_time"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("st_no", tfbsm12["ST_NO"].ToString());
			cmd_inq.Parameters.Set("compose_list_no", tfbsm12["COMPOSE_LIST_NO"].ToString());
			cmd_inq.Parameters.Set("backlog_ea", tfbsm12["BACKLOG_EA"].ToString());
			cmd_inq.Parameters.Set("date_time", tfbsm12["DATE_TIME"].ToString());
			cmd_inq.Parameters.Set("compose_list_no_new", compose_list_no);
			cmd_inq.Parameters.Set("check_flag", tfbsm14a["CHECK_FLAG"].ToString());
			cmd_inq.Parameters.Set("check_make", tfbsm14a["CHECK_MAKE"].ToString());
			cmd_inq.Parameters.Set("check_date", tfbsm14a["CHECK_DATE"].ToString());
			cmd_inq.Parameters.Set("origin_code", tfbsm14a["ORIGIN_CODE"].ToString());
			cmd_inq.Parameters.Set("singlecost", tfbsm14a["SINGLECOST"].ToDecimal());
			cmd_inq.Parameters.Set("cost_dg", tfbsm14a["COST_DG"].ToDecimal());
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
			bcls_ret->Tables[0].Rows[0]["COMPOSE_LIST_NO"] = compose_list_no;
			bcls_ret->Tables[0].Rows[0]["BACKLOG_EA"] = tfbsm12["BACKLOG_EA"].ToString();
			bcls_ret->Tables[0].Rows[0]["ST_NO"] = tfbsm12["ST_NO"].ToString();
			bcls_ret->Tables[0].Rows[0]["DATE_TIME"] = tfbsm12["DATE_TIME"].ToString();
		}

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
