/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/11/26
Description: 废钢基准成分
**************************************************/
#include "stdafx.h"
int f_tableObjectCheck9999(ITableObject2& obj);	//字段超长检测
BM2F_ENTERACE(fbsm_scrap_rcv)


int f_fbsm_scrap_rcv(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CModel tfbsm08("TFBSM08");
	CModel tfbsm08_copy("TFBSM08");
	try
	{
		Log::Trace("", "", "linke = {0}", __LINE__);
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			//把字段相同的值塞到表里面
			tfbsm08.Reset();
			tfbsm08.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			tfbsm08.TrimOrBlank();
			Log::Trace("", "", "linke = {0}", __LINE__);
			f_tableObjectCheck9999(tfbsm08);
			if (tfbsm08.Query("MAT_CODE")>0){
				//查询是否存在相同的数据
				//存在先删后加
				tfbsm08["FACTORY_DIV"] = "LG1";
				tfbsm08_copy.CopyFrom(tfbsm08);
				tfbsm08.Delete("MAT_CODE");
				tfbsm08_copy["REC_REVISE_TIME"] = datetime;
				tfbsm08_copy["REC_REVISOR"] = s.userid;
				Log::Trace("", "", "linke = {0}", __LINE__);
				tfbsm08_copy.Insert();
			}
			else{
				//没有相同的数据直接添加
				tfbsm08["REC_CREATE_TIME"] = datetime;
				tfbsm08["REC_CREATOR"] = s.userid;
				Log::Trace("", "", "linke = {0}", __LINE__);
				tfbsm08.Insert();
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
