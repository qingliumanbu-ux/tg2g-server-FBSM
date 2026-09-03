/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 审核操作
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm17_sh)
int f_fbsm41_mx_ins(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_fbsm17_sh(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
		EIClass inEAF;	//电炉配料单
		EIClass outEAF;	
		inEAF.Tables[0].Columns.Add(DT_STRING, "COMPOSE_LIST_NO");
		inEAF.Tables[0].Columns.Add(DT_STRING, "ST_NO");
		inEAF.Tables[0].Columns.Add(DT_STRING, "COMM_FMLY_CODE");
		inEAF.Tables[0].Columns.Add(DT_STRING, "FURNACE_COUNT");

		//获取是那个配料单
		compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString();
		sqlstr = " select t1.CHECK_FLAG,t1.ST_NO,t2.COMM_FMLY_CODE,count(1),t1.BACKLOG_EA,nvl(t3.DESCRIPTION,' ') DESCRIPTION"
			" from tfbsm12 t1"			
			" left join TFBSM11 t2 on t1.st_no = t2.st_no"
			" left join TFBSM02 t3 on t1.BACKLOG_EA = t3.CODE"
			" where compose_list_no = @compose_list_no"
			" group by t1.CHECK_FLAG,t1.ST_NO,t2.COMM_FMLY_CODE,t1.BACKLOG_EA,nvl(t3.DESCRIPTION,' ') "
			;
		
		cmd_inq.SetCommandText(sqlstr); 
		cmd_inq.Parameters.Set("compose_list_no", compose_list_no);
		cmd_inq.ExecuteReader();
		int i = 0;
		while (cmd_inq.Read())
		{
			if (cmd_inq.GetString(1) == "1")
			{
				tfbsm12["CHECK_FLAG"] = "0";
			}
			else
			{
				tfbsm12["CHECK_FLAG"] = "1";
				if (cmd_inq.GetString(6).Find("E", 0) > 0)
				{
					inEAF.Tables[0].Rows.Add();
					inEAF.Tables[0].Rows[i]["COMPOSE_LIST_NO"] = compose_list_no;
					inEAF.Tables[0].Rows[i]["ST_NO"] = cmd_inq.GetString(2);
					inEAF.Tables[0].Rows[i]["COMM_FMLY_CODE"] = cmd_inq.GetString(3);
					inEAF.Tables[0].Rows[i]["FURNACE_COUNT"] = cmd_inq.GetString(4);
					i++;
				}
			}
			tfbsm12["CHECK_MAKE"] = s.username;
			tfbsm12["CHECK_DATE"] = datetime;
			tfbsm12["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm12.Update("CHECK_MAKE,CHECK_DATE,CHECK_FLAG", "COMPOSE_LIST_NO");

			tfbsm14a["CHECK_FLAG"] = tfbsm12["CHECK_FLAG"].ToString();
			tfbsm14a["CHECK_MAKE"] = s.username;
			tfbsm14a["CHECK_DATE"] = datetime;
			tfbsm14a["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14a.Update("CHECK_MAKE,CHECK_DATE,CHECK_FLAG", "COMPOSE_LIST_NO");
			if (cmd_inq.GetString(1) == "1")
			{
				tfbsm12["MB_MARK"] = "0";
			}
			else
			{
				tfbsm12["MB_MARK"] = "1";
			}
			tfbsm12["MB_CREATOR"] = s.username;
			tfbsm12["MB_CREATE_TIME"] = datetime;
			tfbsm12["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm12.Update("MB_CREATOR,MB_CREATE_TIME,MB_MARK","COMPOSE_LIST_NO");

			tfbsm14a["MB_MARK"] = tfbsm12["MB_MARK"].ToString();
			tfbsm14a["MB_CREATOR"] = s.username;
			tfbsm14a["MB_CREATE_TIME"] = datetime;
			tfbsm14a["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14a.Update("MB_CREATOR,MB_CREATE_TIME,MB_MARK", "COMPOSE_LIST_NO");
		}
		cmd_inq.Close();

		if (inEAF.Tables[0].Rows.get_Count() > 0)
		{
			doFlag = f_fbsm41_mx_ins(&inEAF, &outEAF, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
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
