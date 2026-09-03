 /*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/11/05
Description: 原料总库存查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm13_inq)

int f_fbsm13_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd_inq(conn);
	CString mat_code = " ";
	CString mat_name = " ";
	CString stock_name = " ";

	try
	{
		/* ***** 获取输入参数 ***** */
		// 安全获取物料代码（支持模糊查询）
		if (bcls_rec->Tables[0].Columns.Contains("MAT_CODE"))
		{
			mat_code = bcls_rec->Tables[0].Rows[0]["MAT_CODE"].ToString().Trim();
		}
		// 安全获取库存名称（STOCK_NAME）过滤条件（支持模糊查询）
		if (bcls_rec->Tables[0].Columns.Contains("STOCK_NAME"))
		{
			stock_name = bcls_rec->Tables[0].Rows[0]["STOCK_NAME"].ToString().Trim();
		}
		if (bcls_rec->Tables[0].Columns.Contains("MAT_NAME"))
		{
			mat_name = bcls_rec->Tables[0].Rows[0]["MAT_NAME"].ToString().Trim();
		}
		//1.先获取镍生铁 镍板库的批次成分

		/* ***** 程序处理 ***** */
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:						// 所有数据库适用，通用SQL语句
			sqlstr = " select "
				" A.LOT_NO, "
				" A.MAT_CODE, "
				" A.BUNKER_NO_LIST, "
				" A.STOCK_STATUS, "
				" B.MAT_NAME, "
				"CASE "
				" WHEN A.STOCK_NAME IN('1', '2', '3', '4', '5') THEN A.STOCK_NAME "
				" WHEN STOCK_NAME='7' THEN '1' "
				"ELSE NVL(D.MAT_NAME, A.STOCK_NAME) "
				"END AS STOCK_NAME, "
				" case when a.STOCK_NAME!='5' then a.WEIGHT/1000 else a.WEIGHT END AS STOCK_WT, "
				//" C.MAT_TYPE_PL1, "
				" NVL(A.C, 0)   C_VALUE, "
				" NVL(A.Si, 0)  SI_VALUE, "
				" NVL(A.Mn, 0)  MN_VALUE, "
				" NVL(A.P, 0)   P_VALUE, "
				" NVL(A.S, 0)   S_VALUE, "
				" NVL(A.Cr, 0)  CR_VALUE, "
				" NVL(A.Ni, 0)  NI_VALUE, "
				" NVL(A.Mo, 0)  MO_VALUE, "
				" NVL(A.Co, 0)  CO_VALUE, "
				" NVL(A.Cu, 0)  Cu_VALUE "
				" from FBSM13_AI A LEFT JOIN TMMSM50 B ON A.MAT_CODE=B.MAT_CODE "
				//" LEFT JOIN (SELECT DISTINCT MAT_CODE,MAT_TYPE_PL1 FROM TMMSM50 where MAT_TYPE_PL1!=' ' ) C ON A.MAT_CODE=C.MAT_CODE "
				" LEFT JOIN TMMSM60 D ON A.MAT_CODE = D.MAT_CODE AND A.STOCK_NAME = D.BUNKER_NO WHERE 1=1 and A.STOCK_WT!='1'   ";
			if (!mat_code.IsEmpty())
			{
				// 改为包含匹配（输入内容在任意位置）
				sqlstr += " AND A.MAT_CODE LIKE '%" + mat_code + "%'";
			}

			if (!mat_name.IsEmpty())
			{
				// 改为包含匹配（输入内容在任意位置）
				sqlstr += " AND B.mat_name LIKE '%" + mat_name + "%'";
			}
			// 条件2：库存名称（STOCK_NAME）模糊过滤（匹配计算结果）
			if (!stock_name.IsEmpty())
			{
				if (stock_name == "1")
				{
					// 传入1时：筛选STOCK_NAME=1 或 不在2/3/4/5中的记录
					sqlstr += " AND (A.STOCK_NAME = '1' OR A.STOCK_NAME NOT IN ('2','3','4','5'))";
				}
				else
				{
					// 传入2/3/4/5时：保留原模糊匹配逻辑
					sqlstr += " AND A.STOCK_NAME = '" + stock_name + "'";
				}
			}
			break;
		
		}
		sqlstr += "  ORDER BY  A.STOCK_NAME,A.MAT_CODE,NI_VALUE, STOCK_WT DESC ";
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr ={0}", sqlstr);
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
