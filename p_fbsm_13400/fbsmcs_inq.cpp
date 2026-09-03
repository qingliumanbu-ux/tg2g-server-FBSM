/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: H5静态表查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsmcs_inq)

int f_mmsm_getprice(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);
int f_fbsmcs_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CModel tfbsmcs("TFBSMCS");
	EIClass bcls_rec_rep;
	EIClass bcls_ret_rep;
	bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_CODE");
	bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_NAME");
	bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "CR_VALUE");
	bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "NI_VALUE");
	bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MO_VALUE");
	bcls_rec_rep.Tables[0].Rows.Add();
	if (!bcls_ret->Tables[0].Columns.Contains("MAT_CODE"))
		Log::Trace("", "", "ss ={0}",__LINE__ );
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_CODE");
	if (!bcls_ret->Tables[0].Columns.Contains("MAT_NAME"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MAT_NAME");
	if (!bcls_ret->Tables[0].Columns.Contains("C_VALUE"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "C_VALUE");
	if (!bcls_ret->Tables[0].Columns.Contains("SI_VALUE"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "SI_VALUE");
	if (!bcls_ret->Tables[0].Columns.Contains("MN_VALUE")) 
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MN_VALUE");
	if (!bcls_ret->Tables[0].Columns.Contains("P_VALUE"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "P_VALUE");
	if (!bcls_ret->Tables[0].Columns.Contains("S_VALUE"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "S_VALUE");
	if (!bcls_ret->Tables[0].Columns.Contains("CR_VALUE"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CR_VALUE");
	if (!bcls_ret->Tables[0].Columns.Contains("NI_VALUE"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "NI_VALUE");
	if (!bcls_ret->Tables[0].Columns.Contains("MO_VALUE"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "MO_VALUE");
	if (!bcls_ret->Tables[0].Columns.Contains("CU_VALUE"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "CU_VALUE");
	if (!bcls_ret->Tables[0].Columns.Contains("UNIT_PRICE"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "UNIT_PRICE");
	if (!bcls_ret->Tables[0].Columns.Contains("STATION_ID"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STATION_ID");
	if (!bcls_ret->Tables[0].Columns.Contains("ARCHIVE_FLAG"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ARCHIVE_FLAG");
	if (!bcls_ret->Tables[0].Columns.Contains("BACK_C2"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "BACK_C2");
	if (!bcls_ret->Tables[0].Columns.Contains("ST_NO"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "ST_NO");
	if (!bcls_ret->Tables[0].Columns.Contains("STOCK_NAME"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STOCK_NAME");
	if (!bcls_ret->Tables[0].Columns.Contains("STOCK_WT"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STOCK_WT");
	if (!bcls_ret->Tables[0].Columns.Contains("LOT_NO"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "LOT_NO");
	if (!bcls_ret->Tables[0].Columns.Contains("STOCK_WT_QC"))
		bcls_ret->Tables[0].Columns.Add(DT_STRING, "STOCK_WT_QC");
	try
	{
		sqlstr = " select case when MAT_NAME like '%硅铁%' then '1' ELSE ' ' END as ARCHIVE_FLAG1,a.* from tfbsmcs a  order by REC_CREATE_TIME  asc ";
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr = {0}", sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			cmd_inq.Fetch(tfbsmcs);
			bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = tfbsmcs["MAT_CODE"];
			bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = tfbsmcs["MAT_NAME"];
			bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = tfbsmcs["CR_VALUE"];
			bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = tfbsmcs["NI_VALUE"];
			bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = tfbsmcs["MO_VALUE"];
			CDataRow &row = bcls_ret->Tables[0].Rows.Add();
			doFlag = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
			if (doFlag < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			row["MAT_CODE"] = tfbsmcs["MAT_CODE"];
			row["STOCK_WT_QC"] = tfbsmcs["STOCK_WT_QC"];
			row["MAT_NAME"] = tfbsmcs["MAT_NAME"];
			row["C_VALUE"] = tfbsmcs["C_VALUE"];
			row["SI_VALUE"] = tfbsmcs["SI_VALUE"];
			row["MN_VALUE"] = tfbsmcs["MN_VALUE"];
			row["P_VALUE"] = tfbsmcs["P_VALUE"];
			row["S_VALUE"] = tfbsmcs["S_VALUE"];
			row["STATION_ID"] = tfbsmcs["STATION_ID"];
			row["NI_VALUE"] = tfbsmcs["NI_VALUE"];
			row["CR_VALUE"] = tfbsmcs["CR_VALUE"];
			row["MO_VALUE"] = tfbsmcs["MO_VALUE"];
			row["CU_VALUE"] = tfbsmcs["CU_VALUE"];
			row["LOT_NO"] = tfbsmcs["LOT_NO"];
			row["STOCK_WT"] = tfbsmcs["STOCK_WT"];
			row["BACK_C2"] = tfbsmcs["BACK_C2"];
			row["STOCK_NAME"] = tfbsmcs["STOCK_NAME"];
			row["ST_NO"] = tfbsmcs["ST_NO"];
			row["ARCHIVE_FLAG"] = cmd_inq.GetString(1);
			row["UNIT_PRICE"] = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"];
			
		}
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
