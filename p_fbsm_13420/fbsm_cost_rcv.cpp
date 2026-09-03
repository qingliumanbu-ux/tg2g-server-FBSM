/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/11/26
Description: 标准成本
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm_cost_rcv)

int f_fbsm_cost_rcv(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CModel tfbsm07("TFBSM07");
	CModel tfbsm07_copy("TFBSM07");
	CString st_no = "";
	try
	{
		Log::Trace("", "", "linke = {0}", __LINE__);
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			//把字段相同的值塞到表里面
			tfbsm07.Reset();
			tfbsm07.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			//强制设置厂别为LG1（不允许前端传入其他厂别）
			tfbsm07["FACTORY_DIV"] = "LG1";
			Log::Trace("", "", "linke = {0}", __LINE__);
			if (tfbsm07.Query("ST_NO") > 0){
				//查询已存在的旧数据
				tfbsm07.Delete("ST_NO");
				Log::Trace("", "", "linke = {0}", __LINE__);
				tfbsm07_copy.CopyFrom(tfbsm07);
				tfbsm07_copy["REC_REVISE_TIME"] = datetime;
				tfbsm07_copy["REC_REVISOR"] = s.userid;
				tfbsm07_copy.Insert();
				Log::Trace("", "", "linke = {0}", __LINE__);
			}
			
			else{
				//没有相同的数据直接添加
				tfbsm07["REC_CREATE_TIME"] = datetime;
				tfbsm07["REC_CREATOR"] = s.userid;
				tfbsm07.Insert();
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
