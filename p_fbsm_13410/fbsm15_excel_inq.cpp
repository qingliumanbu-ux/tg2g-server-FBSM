/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15_excel_inq)

int f_fbsm15_excel_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int i = 1;
	CString s_id_01 = " ";
	CString s_id_02 = " ";
	CString s_id_03 = " ";
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd(conn);
	CString compose_list_no = "";
	CString st_no = " ";
	try
	{
		for (size_t i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++){
			if (bcls_rec->Tables[0].Rows.get_Count() != 1){
				if (i + 1 == bcls_rec->Tables[0].Rows.get_Count()){
					compose_list_no = compose_list_no+"'"+ bcls_rec->Tables[0].Rows[i]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper()+"'";
					st_no = st_no + "'" + bcls_rec->Tables[0].Rows[i]["ST_NO"].ToString().TrimOrBlank().ToUpper() + "'";
				}
				else{
					compose_list_no = compose_list_no + "'" + bcls_rec->Tables[0].Rows[i]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper() + "'" + ",";
					st_no = st_no + "'" + bcls_rec->Tables[0].Rows[i]["ST_NO"].ToString().TrimOrBlank().ToUpper() + "'" + ",";
				}

			}
			else{
				compose_list_no = "'"+ bcls_rec->Tables[0].Rows[i]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper() + "'";
				st_no = "'" + bcls_rec->Tables[0].Rows[i]["ST_NO"].ToString().TrimOrBlank().ToUpper() + "'";
			}
		}
		/*if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
			compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();*/
		sqlstr = "select COMPOSE_LIST_NO as 配料单号,"
			 "DATE_C as 时间,"
			 "ST_NO as 出钢记号,"
			"BACKLOG_EA as 工艺路径,"
			 "STATION_ID as 工序,"
			 "MAT_CODE as 物料代码,"
			 "MAT_NAME as 物料名称,"
			 "WEIGHT as 重量,"
			 "LOT_NO as 试批号,"
			 "STOCK_NAME as 库区,"
			 "c_value as C,"
			 "si_value as Si,"
			 "mn_value as Mn,"
			 "p_value as P,"
			 "s_value as S,"
			 "cr_value as Cr,"
			 "ni_value as Ni,"
			 "mo_value as Mo,"
			 "cu_value as Cu,"
			 "co_value as Co,"
			 "ti_value as Ti,"
			 "nb_value as Nb,"
			 "al_value as Al "
			 "from TFBSM14 "
			 "WHERE COMPOSE_LIST_NO  in ( " + compose_list_no + ") and st_no in ("+st_no+") ";;
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
