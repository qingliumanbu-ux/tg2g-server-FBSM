/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15d_inq)

int f_fbsm15d_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString date_c = "";
	CString date_c_1 = "";
	CString st_no = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMdd");
	try
	{
		Log::Trace("", "", "datetime = {0}", datetime);
		if (bcls_rec->Tables[0].Columns.Contains("DATE_C"))
			date_c = bcls_rec->Tables[0].Rows[0]["DATE_C"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("DATE_C_1"))
			date_c_1 = bcls_rec->Tables[0].Rows[0]["DATE_C_1"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		sqlstr = " SELECT"
"    heat_no,"
"    st_no,"
"    BACKLOG_EA,"
"    DATE_C "
"FROM (  "
"    SELECT"
"        heat_no,"
"        st_no,"
"        BACKLOG_EA,"
"        DATE_C"
"    FROM ("
"        SELECT"
"            heat_no,"
"            st_no,"
"            CASE"
"                WHEN concatenated_chars = 'D' THEN '06'"
"                WHEN concatenated_chars = 'DH' THEN '06'"
"                WHEN concatenated_chars = 'HD' THEN '06'"
"                WHEN concatenated_chars = 'EF' THEN '02'"
"                WHEN concatenated_chars = 'FE' THEN '02'"
"                WHEN concatenated_chars = 'FF' THEN '01'"
"                WHEN concatenated_chars = 'E' THEN '03'"
"                WHEN concatenated_chars = 'FFF' THEN '04'"
"                WHEN concatenated_chars = 'FFE' THEN '05'"
"                WHEN concatenated_chars = 'FEF' THEN '05'"
"                WHEN concatenated_chars = 'EFF' THEN '05'"
"                WHEN concatenated_chars = 'B' THEN '08'"
"                WHEN concatenated_chars = 'FB' THEN '07'"
"                WHEN concatenated_chars = 'BF' THEN '07'"
"                WHEN concatenated_chars = 'FFB' THEN '07'"
"                WHEN concatenated_chars = 'BFF' THEN '07'"
"                WHEN concatenated_chars = 'FBF' THEN '07'"
"                ELSE NULL"
"            END AS BACKLOG_EA,"
"            SUBSTR(START_TIME, 1, 8) AS DATE_C"
"        FROM ("
"            SELECT"
"                heat_no,"
"                st_no,"
"                TRIM(NVL(SUBSTR(HEATNO_PREMELT1, 1, 1), '') || NVL(SUBSTR(HEATNO_PREMELT2, 1, 1), '') || NVL(SUBSTR(HEATNO_PREMELT3, 1, 1), '')) AS concatenated_chars,"
"                START_TIME"
"            FROM tmmsm27"
"            WHERE"
"                HEATNO_PREMELT1 IS NOT NULL"
"                AND HEATNO_PREMELT2 IS NOT NULL"
"                AND HEATNO_PREMELT3 IS NOT NULL"
"        )"
"    )"
"    WHERE BACKLOG_EA IS NOT NULL"
"    UNION ALL"
"    SELECT"
"        heat_no,"
"        st_no,"
"        '09' AS BACKLOG_EA,"
"        SUBSTR(START_TIME, 1, 8) AS DATE_C"
"    FROM TMMSM21"
"    WHERE"
"        st_no LIKE '2%'"
"        AND heat_no IS NOT NULL"
"        AND START_TIME IS NOT NULL"
") t  "
"WHERE"
"    1 = 1";

		if (date_c.Trim() != ""  && date_c_1.Trim() == ""){
			sqlstr += " and DATE_C ='" + date_c + "' ";
		}
		else if (date_c_1.Trim() != "" && date_c.Trim() == ""){
			sqlstr += " and DATE_C <='" + date_c_1 + "' ";
		}
		else if (date_c_1.Trim() != "" && date_c.Trim() != ""){
			sqlstr += " and DATE_C >='" + date_c + "' and DATE_C <='" + date_c_1 + "' ";
		}
		/*else{
		sqlstr += " and A.DATE_TIME='" + datetime + "' ";
		}*/
		if (st_no.Trim() != ""){
			sqlstr += " and st_no like '%" + st_no + "%' ";
		}
		//sqlstr += "group by t12.DATE_C, t12.ST_NO,t12.FURNACE_COUNT, t12.COMPOSE_LIST_NO, t12.BACKLOG_EA,t12.COST_DG,t12.SINGLECOST,t12.DATE_TIME,t12.MATERIAL_CODE,t12.CHECK_FLAG,t12.CHECK_MAKE,t11.BIG_CLASS_NAME, t11.MAT_DIVISION_DESC,t0.ST_NO_DESC  order by t12.DATE_C DESC, t12.ST_NO, t12.BACKLOG_EA,SEQ_NO ";
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr = {0}", sqlstr);
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
