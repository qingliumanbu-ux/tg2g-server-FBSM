/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/11/10
Description: 原料库存预警
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm19_inq)

int f_fbsm19_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd_inq(conn);
	CString mat_code = " ";
	CString mat_name = " ";

	try
	{
		/* ***** 获取输入参数 ***** */
		// 安全获取物料代码（支持模糊查询）
		if (bcls_rec->Tables[0].Columns.Contains("MAT_CODE"))
		{
			mat_code = bcls_rec->Tables[0].Rows[0]["MAT_CODE"].ToString().Trim();
		}
		
		/* ***** 程序处理 ***** */
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:						// 所有数据库适用，通用SQL语句
			sqlstr = " SELECT "
				" a.MAT_CODE AS MAT_CODE, "					// 物料代码
				" a.MAT_NAME AS MAT_NAME, "					// 物料名称
				" a.LOWER_LIMIT_VALUE AS LOWER_LIMIT_VALUE, "	// 下限值
				" a.UPPER_LIMIT_VALUE AS UPPER_LIMIT_VALUE, "	// 上限值
				" SUM(b.STOCK_WT) AS STOCK_WT, "			// 合并相同物料的在库重量
				// 计算预警标记（0=正常，-1=低于下限，1=高于上限）
				" CASE "
				"   WHEN SUM(b.STOCK_WT) < a.LOWER_LIMIT_VALUE THEN -1 "
				"   WHEN SUM(b.STOCK_WT) > a.UPPER_LIMIT_VALUE THEN 1 "
				"   ELSE 0 "
				" END AS ALARM_FLAG "
				" FROM TMMSM50 a "							// 预警信息表（含上下限）
				// 关联库存表，只处理有预警设置且有库存的物料
				" INNER JOIN TMMSM85 b ON a.MAT_CODE = b.MAT_CODE  "
				" WHERE 1=1 ";								// 条件拼接基础

				
			// 条件1：物料代码模糊过滤（可选）
			if (!mat_code.IsEmpty())
			{
				sqlstr += " AND a.MAT_CODE LIKE '" + mat_code + "%' ";
			}

			
			
			// 分组逻辑：按物料+上下限分组（确保合并后预警判断准确）
			sqlstr += " GROUP BY "
				" a.MAT_CODE, "
				" a.MAT_NAME, "
				" a.LOWER_LIMIT_VALUE, "		// 下限值（预警判断依赖）
				" a.UPPER_LIMIT_VALUE "		// 上限值（预警判断依赖）
				" ORDER BY a.MAT_CODE ";	// 按物料代码排序
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
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
