/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 库存信息批处理
**************************************************/
/***** C++ 的标准头文件部分 *****/
#include "stdafx.h"
BM2_FUNCTION_IMPORT

// service入口
BM2F_ENTERACE(fbsm13a_save)

int f_fbsm13a_save(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
	CModel tfbsm13A("TFBSM13A");
	try
	{
		// 先清空TFBSM13A表（可选，根据业务需求，不需要可删除）
		sqlstr = "DELETE FROM TFBSM13A";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

		// 写入库存信息到TFBSM13A表（插入查询结果）
		sqlstr = "INSERT INTO TFBSM13A (MAT_CODE, STOCK_WT, STOCK_NAME, QUALITY_BATCH_NO) "
			"SELECT tmmsm85.MAT_CODE, "
			"SUM(STOCK_WT) AS STOCK_WT, "
			"'3' AS STOCK_NAME, "
			"A.QUALITY_BATCH_NO "
			"FROM tmmsm85 "
			"JOIN((SELECT t60.MAT_CODE, t60.BUNKER_NO "
			"FROM tmmsm60 t60 "
			"WHERE t60.mat_simple_ename IN ('jinkouFeCr'，'HCFeCr', 'jinliaoHT') "
			"AND ((SUBSTR(t60.BUNKER_NO, 1, 1) = 'E' AND SUBSTR(t60.BUNKER_NO, 2) BETWEEN '01' AND '18') OR "
			"(SUBSTR(t60.BUNKER_NO, 1, 1) = 'F' AND SUBSTR(t60.BUNKER_NO, 2) BETWEEN '01' AND '18') OR "
			"(SUBSTR(t60.BUNKER_NO, 1, 1) = 'C' AND SUBSTR(t60.BUNKER_NO, 2) BETWEEN '01' AND '22') OR "
			"(SUBSTR(t60.BUNKER_NO, 1, 1) = 'D' AND SUBSTR(t60.BUNKER_NO, 2) BETWEEN '01' AND '22'))AND t60.BUNKER_FLAG NOT IN ('14') "
			"OR t60.BUNKER_FLAG = '13')) cfg "
			"ON tmmsm85.MAT_CODE = cfg.MAT_CODE AND tmmsm85.BUNKER_NO = cfg.BUNKER_NO "
			"LEFT JOIN (SELECT MAT_CODE, QUALITY_BATCH_NO "
			"FROM (SELECT MAT_CODE, QUALITY_BATCH_NO, REC_CREATE_TIME, "
			"ROW_NUMBER() OVER(PARTITION BY MAT_CODE ORDER BY REC_CREATE_TIME DESC) AS rn "
			"FROM TMMSM2A_YL "
			"WHERE STK_NO IN ( SELECT BUNKER_NO FROM tmmsm60 WHERE FLAG1 = 'H' )) "
			"WHERE rn = 1) A "
			"ON tmmsm85.MAT_CODE = A.MAT_CODE "
			"GROUP BY tmmsm85.MAT_CODE, A.QUALITY_BATCH_NO";
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
