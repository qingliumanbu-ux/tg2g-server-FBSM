/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: TFBSM10批量复制
**************************************************/
#include "stdafx.h"

BM2_FUNCTION_EXPORT

int f_fbsm10_copy_insert(CDataTable *data_table, EIClass *bcls_ret, CDbConnection *conn, CString st_no)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd(conn);
	CModel tfbsm10("TFBSM10");
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString g_ctxInsertInfo = "";

	try
	{
		int count = data_table->Rows.get_Count();
		for (int i = 0; i < count; i++)
		{
			CString src_mat_code = data_table->Rows[i]["MAT_CODE"].ToString().TrimOrBlank();

			tfbsm10.Reset();
			tfbsm10.MergeFrom(data_table->Rows[i]);

			tfbsm10["ST_NO"] = st_no;
			tfbsm10["REC_CREATOR"] = s.userid;
			tfbsm10["REC_CREATE_TIME"] = datetime;
			tfbsm10["REC_REVISOR"] = " ";
			tfbsm10["REC_REVISE_TIME"] = " ";
			tfbsm10["FACTORY_DIV"] = "LG1";

			g_ctxInsertInfo = "ST_NO=[";
			g_ctxInsertInfo += st_no;
			g_ctxInsertInfo += "], MAT_CODE=[";
			g_ctxInsertInfo += src_mat_code;
			g_ctxInsertInfo += "]";

			if (tfbsm10.QueryCount("MAT_CODE,ST_NO") > 0)
			{
				Log::Trace("", "", "记录已存在，跳过: {0}", g_ctxInsertInfo);
			}
			else
			{
				Log::Trace("", "", "插入数据: {0}", g_ctxInsertInfo);
				tfbsm10.Insert();
			}
		}
	}
	catch (CDbException &ex)
	{
		CFormattable arguments[] = {ex.GetCode(), ex.GetMsg()};
		CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);
		CString fullMsg;
		if (ex.GetCode() == 1)
		{
			fullMsg = g_ctxInsertInfo + " 已存在，无法重复插入";
		}
		else
		{
			fullMsg = g_ctxInsertInfo + " 时发生错误" + s.msg;
		}
		strncpy(s.msg, fullMsg, sizeof(s.msg) - 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char *)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException &ex)
	{
		s.flag = ex.GetCode();
		strncpy(s.msg, ex.GetMsg(), sizeof(s.msg) - 1);
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

int f_fbsm10_copy(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	try
	{
		CDataTable *pSrcTable = NULL;
		CDataTable *pTargetTable = NULL;

		for (size_t i = 0; i < bcls_rec->Tables.get_Count(); i++)
		{
			CString table_name = bcls_rec->Tables[i].get_TableName();
			if (table_name == "Table1")
			{
				pSrcTable = &bcls_rec->Tables[i];
			}
			else if (table_name == "Table2")
			{
				pTargetTable = &bcls_rec->Tables[i];
			}
		}

		if (pSrcTable == NULL || pTargetTable == NULL)
		{
			throw CApplicationException(-1, "缺少源物料表(Table1)或目标钢种表(Table2)", log.Location);
		}

		int targetCount = pTargetTable->Rows.get_Count();
		for (int t = 0; t < targetCount; t++)
		{
			CString st_no = "";
			if (pTargetTable->Columns.Contains("ST_NO"))
				st_no = pTargetTable->Rows[t]["ST_NO"].ToString().TrimOrBlank().ToUpper();

			if (st_no.IsEmpty())
			{
				strcpy(s.msg, "第" + CConvert::ToString(t + 1) + "行目标钢种的ST_NO为空，无法复制");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			doFlag = f_fbsm10_copy_insert(pSrcTable, bcls_ret, conn, st_no);
			if (doFlag < 0)
			{
				CString errMsg;
				errMsg.Format("复制到第%d个目标钢种(%s)时失败", t + 1, st_no);
				throw CApplicationException(-1, errMsg + "：" + s.msg, log.Location);
			}
		}
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
		strncpy(s.msg, ex.GetMsg(), sizeof(s.msg) - 1);
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
