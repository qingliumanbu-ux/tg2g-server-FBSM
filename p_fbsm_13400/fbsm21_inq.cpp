/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/12/29
Description: 模型配料库存查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm21_inq)

int f_fbsm21_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd_inq(conn);
	CString mat_code = " ";
	CString stock_name = " ";

	try
	{
		/* ***** 获取输入参数 ***** */
		// 安全获取物料代码（支持模糊查询）
		if (bcls_rec->Tables[0].Columns.Contains("MAT_CODE"))
		{
			mat_code = bcls_rec->Tables[0].Rows[0]["MAT_CODE"].ToString().Trim();
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
			sqlstr = "WITH FBSM13_DATA AS ("
				"    SELECT "
				"        A.MAT_CODE, "
				"        B.MAT_NAME, "
				"        TO_CHAR(A.STOCK_NAME) AS STOCK_NAME, "
				"        NVL(A.LOT_NO, ' ') AS LOT_NO, "
				"        CASE "
				"            WHEN TO_CHAR(A.STOCK_NAME) = '5' THEN A.WEIGHT "
				"            ELSE A.WEIGHT / 1000 "
				"        END AS STOCK_WT "
				"    FROM FBSM_13_AI A "
				"    LEFT JOIN TMMSM50 B ON A.MAT_CODE = B.MAT_CODE "
				"    WHERE A.MAT_CODE NOT LIKE 'PL%' ";

			// 添加物料代码模糊查询条件（确保条件前有空格）
			if (!mat_code.IsEmpty())
			{
				sqlstr += "AND A.MAT_CODE LIKE '%" + mat_code + "%' ";
			}

			sqlstr +="), "
				" TFBSM12_AGG AS( "
				" SELECT "
				" COMPOSE_LIST_NO, "
				" COUNT(*) AS USE_TIMES "
				" FROM TFBSM12 "
				" WHERE "
				" CHECK_FLAG = '1' "
				" AND TRIM(DATE_C) = ( "
				" SELECT MAX(TRIM(DATE_C)) "
				" FROM TFBSM12 "
				" WHERE CHECK_FLAG = '1' "
				" ) "
				" GROUP BY COMPOSE_LIST_NO "
				" ), "
				"TFBSM14_CLEAN AS ("
				"    SELECT "
				"        T14.MAT_CODE, "
				"        T50.MAT_NAME, "
				"		T14.STOCK_NAME, "
				"       NVL(T14.LOT_NO, ' ') AS LOT_NO, "
				"	 SUM(T14.WEIGHT * T12.USE_TIMES) AS T14_TOTAL_WT "
				"    FROM TFBSM14 T14 "
				"	INNER JOIN TFBSM12_AGG T12"
				"	ON T14.COMPOSE_LIST_NO = T12.COMPOSE_LIST_NO"
				"	LEFT JOIN TMMSM50 T50 ON"
				"	T14.MAT_CODE = T50.MAT_CODE"
				"    WHERE T14.MAT_CODE NOT LIKE 'PL%' ";

			// 14表添加物料代码筛选（确保条件前有空格）
			if (!mat_code.IsEmpty())
			{
				sqlstr += "AND T14.MAT_CODE LIKE '%" + mat_code + "%' ";
			}

			sqlstr +=
				"	GROUP BY"
				"	T14.MAT_CODE,T50.MAT_NAME,T14.STOCK_NAME,T14.LOT_NO) "
				"SELECT "
				"    COALESCE(F.MAT_CODE, T.MAT_CODE) AS MAT_CODE, "
				"    COALESCE(F.MAT_NAME, T.MAT_NAME) AS MAT_NAME, "
				"    COALESCE(F.STOCK_NAME, T.STOCK_NAME) AS STOCK_NAME, "
				"    COALESCE(F.LOT_NO, T.LOT_NO) AS LOT_NO, "
				"    NVL(F.STOCK_WT, 0) AS STOCK_WT, "
				"    NVL(T.T14_TOTAL_WT, 0) AS WEIGHT, "
				"    ROUND(NVL(F.STOCK_WT, 0) - NVL(T.T14_TOTAL_WT, 0), 1) AS STOCK_WT_QC "
				"FROM FBSM13_DATA F "
				"FULL JOIN TFBSM14_CLEAN T "
				"    ON F.MAT_CODE = T.MAT_CODE "
				"    AND F.STOCK_NAME = T.STOCK_NAME "
				"    AND F.LOT_NO = T.LOT_NO "
				"ORDER BY MAT_CODE, STOCK_NAME";

			break;

		}
		
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
