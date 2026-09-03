/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/21
Description: 铁水温度及成分预测查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm12_inq)

int f_fbsm12_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd_inq(conn);
	CString mat_code = " ";

	try
	{
		/* ***** 获取输入参数 ***** */
		//mat_code = bcls_rec->Tables[0].Rows[0]["MAT_CODE"].ToString();
		/* ***** 程序处理 ***** */
		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:				// DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:				// MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default:						// 所有数据库适用，通用SQL语句
			sqlstr = " SELECT numbers.rn          AS ROW_NUM, "
				" b0_iron.IRON_SI     AS B0_IRON_SI, "
				" b0_steel.STEEL_TEMP AS B0_STEEL_TEMP, "
				" b1_iron.IRON_SI     AS B1_IRON_SI, "
				" b1_steel.STEEL_TEMP AS B1_STEEL_TEMP, "
				" b2_iron.IRON_SI     AS B2_IRON_SI, "
				" b2_steel.STEEL_TEMP AS B2_STEEL_TEMP "
				" FROM( "
				" SELECT LEVEL as rn "
				" FROM DUAL "
				" CONNECT BY LEVEL <= 10 "
				" ) numbers "
				" LEFT JOIN( "
				" SELECT IRON_SI, ROW_NUMBER() OVER(ORDER BY REC_CREATE_TIME DESC) as rn "
				" FROM TMMSM21 "
				" WHERE DEV_CODE = 'B0' "
				" ) b0_iron ON numbers.rn = b0_iron.rn "
				" LEFT JOIN( "
				" SELECT STEEL_TEMP, ROW_NUMBER() OVER(ORDER BY REC_CREATE_TIME DESC) as rn "
				" FROM TMMSM2B "
				" WHERE DEV_CODE = 'B0' "
				" ) b0_steel ON numbers.rn = b0_steel.rn "
				" LEFT JOIN( "
				" SELECT IRON_SI, ROW_NUMBER() OVER(ORDER BY REC_CREATE_TIME DESC) as rn "
				" FROM TMMSM21 "
				" WHERE DEV_CODE = 'B1' "
				" ) b1_iron ON numbers.rn = b1_iron.rn "
				" LEFT JOIN( "
				" SELECT STEEL_TEMP, ROW_NUMBER() OVER(ORDER BY REC_CREATE_TIME DESC) as rn "
				" FROM TMMSM2B "
				" WHERE DEV_CODE = 'B1' "
				" ) b1_steel ON numbers.rn = b1_steel.rn "
				" LEFT JOIN( "
				" SELECT IRON_SI, ROW_NUMBER() OVER(ORDER BY REC_CREATE_TIME DESC) as rn "
				" FROM TMMSM21 "
				" WHERE DEV_CODE = 'B2' "
				" ) b2_iron ON numbers.rn = b2_iron.rn "
				" LEFT JOIN( "
				" SELECT STEEL_TEMP, ROW_NUMBER() OVER(ORDER BY REC_CREATE_TIME DESC) as rn "
				" FROM TMMSM2B "
				" WHERE DEV_CODE = 'B2' "
				" ) b2_steel ON numbers.rn = b2_steel.rn "
				" ORDER BY numbers.rn ";
			//if (mat_code.Trim() != "")
				//sqlstr = sqlstr + " AND A.mat_code LIKE '" + mat_code + "%'";
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
