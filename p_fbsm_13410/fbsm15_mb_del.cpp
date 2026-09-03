/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 审核操作取消
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15_mb_del)

int f_fbsm15_mb_del(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
	CString date_c = "";
	CString backlog_ea = "";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CModel tfbsm12("TFBSM12");
	try
	{
		//获取是那个配料单为
		compose_list_no = bcls_rec->Tables["LD"].Rows[0]["COMPOSE_LIST_NO"].ToString();
		//日期
		//date_c = bcls_rec->Tables["LD"].Rows[0]["DATE_C"].ToString();
		//工艺
		//backlog_ea = bcls_rec->Tables["LD"].Rows[0]["BACKLOG_EA"].ToString();
		//修改状态
		tfbsm12["MB_CREATOR"] = s.username;
		sqlstr = " update tfbsm12 set MB_MARK='0', MB_CREATOR='" + tfbsm12["MB_CREATOR"].ToString() + "',MB_CREATE_TIME='" + datetime + "' where COMPOSE_LIST_NO='" + compose_list_no + "' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteNonQuery();
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
