/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 电炉模板配料单明细查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm41m_dtl_inq)

int f_fbsm41m_dtl_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString compose_list_no_eaf = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMdd");
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO_EAF"))
			compose_list_no_eaf = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO_EAF"].ToString();

        Log::Trace("", __FUNCTION__, "compose_list_no_eaf = [{0}]]", compose_list_no_eaf);

		sqlstr = " SELECT * FROM TFBSM41 T41 "
				 " WHERE 1=1"
                 " AND T41.COMPOSE_LIST_NO_EAF=@COMPOSE_LIST_NO_EAF";

		sqlstr = sqlstr + " ORDER BY BUNKER_SEQ, LAYER_NO ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
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
