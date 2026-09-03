/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15_elm_inq)

int f_fbsm15_elm_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
	CString compose_list_no = "";
	CString st_no = " ";
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
			compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		//先查询一下有几个工序
		sqlstr = " SELECT  STATION_ID FROM ( select "
			"  distinct case when STATION_ID='Y' THEN 'A' ELSE  STATION_ID END STATION_ID, "
			" CASE WHEN STATION_ID = 'A' OR STATION_ID = 'Y' THEN 1 "
			" WHEN STATION_ID = 'Z' THEN 3 "
			" WHEN STATION_ID = 'E' THEN 2 "
			" WHEN STATION_ID = 'B' THEN 4 "
			" WHEN STATION_ID = 'D' THEN 5 ELSE 999 END XU "
			" from TFBSM14 where COMPOSE_LIST_NO='" + compose_list_no + "' ) ORDER BY XU  DESC ";
		cmd.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr = {0}", sqlstr);
		cmd.ExecuteReader();
		while (cmd.Read())
		{
			if (i == 1)
			{
				s_id_01 = cmd.GetString(1);
			}
			if (i == 2)
			{
				s_id_02 = cmd.GetString(1);
			}
			if (i == 3)
			{
				s_id_03 = cmd.GetString(1);
			}
			++i;
		}
		cmd.Close();


		if (i == 1)
		{
			// 14表没查到记录，说明是碳钢配料单，转查32表
			sqlstr = " SELECT "
				"   STATION_ID, "
				"   MAT_CODE, "
				"   MAT_NAME, "
				"   COMPOSE_LIST_NO, "
				"   WEIGHT, "
				"   STOCK_NAME, "
				"   ST_NO, "
				"   BACKLOG_EA, "
				"   DATE_C, "
				"   COST, "
				"   REC_CREATOR, "
				"   REC_CREATE_TIME, "
				"   REC_REVISOR, "
				"   REC_REVISE_TIME, "
				"   PLAN_ID, "
				"   ORIGIN_CODE, "
				"   FACTORY_DIV, "
				"   COMPANY_NAME, "
				"   COMPANY_CODE, "
				"   ARCHIVE_STAMP_NO, "
				"   ARCHIVE_FLAG "
				" FROM TFBSM32 "
				" WHERE COMPOSE_LIST_NO = '" + compose_list_no + "' "
				" ORDER BY MAT_CODE ";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();

			// 碳钢只有1个工序，后面两个Table必须Add空表，否则前台框架报错
			bcls_ret->Tables.Add();  // Tables[1] 空
			bcls_ret->Tables.Add();  // Tables[2] 空

			return doFlag;  // 直接返回，不再执行下方不锈钢逻辑
		}


		if (s_id_01 == "A"){
			sqlstr = " WITH original_data AS ("
"    SELECT"
"        CASE WHEN STATION_ID = 'Y' THEN 'A' ELSE STATION_ID END STATION_ID,"
"        CASE WHEN STATION_ID = 'Y' THEN 1 WHEN (MAT_CODE = 'PL0001' OR MAT_CODE = 'PL0002') THEN 3 ELSE 2 END S_XU,"
"        DATE_C,"
"        ST_NO,"
"        BACKLOG_EA,"
"        MAT_CODE,"
"        MAT_NAME,"
"        COMPOSE_LIST_NO,"
"        WEIGHT,"
"        BUNKER_NO,"
"        LOT_NO,"
"        C_VALUE,"
"        SI_VALUE,"
"        MN_VALUE,"
"        P_VALUE,"
"        S_VALUE,"
"        CR_VALUE,"
"        NI_VALUE,"
"        MO_VALUE,"
"        CU_VALUE,"
"        CO_VALUE,"
"        COST,"
"        CASE WHEN STOCK_NAME NOT IN('1','2','3','4','5') THEN ("
"            SELECT MAT_NAME FROM TMMSM60 "
"            WHERE BUNKER_NO = TFBSM14.STOCK_NAME AND MAT_CODE = TFBSM14.MAT_CODE AND ROWNUM = 1"
"        ) ELSE STOCK_NAME END AS STOCK_NAME "
"from TFBSM14 where COMPOSE_LIST_NO='" + compose_list_no +  "' and (STATION_ID='" + s_id_01 + "' OR STATION_ID='Y') "
"),"
" max_date_at000258 AS ("
"    SELECT"
"        C_VALUE,"
"        SI_VALUE,"
"        MN_VALUE,"
"        P_VALUE,"
"        S_VALUE,"
"        CR_VALUE,"
"        NI_VALUE,"
"        MO_VALUE,"
"        CU_VALUE,"
"        CO_VALUE"
"    FROM TFBSM14"
"    WHERE MAT_CODE = 'AT000258'"
"      AND DATE_C = (SELECT MAX(DATE_C) FROM TFBSM14 WHERE MAT_CODE = 'AT000258')"
"    AND ROWNUM = 1 "
"), "
"supplement_data AS ("
"    SELECT"
"        'A' AS STATION_ID,"
"        3 AS S_XU,"
"        od.DATE_C,"
"        od.ST_NO,"
"        od.BACKLOG_EA,"
"        'AT000258' AS MAT_CODE,"
"        '硅铁 FeSi72Al2.0' AS MAT_NAME,"
"        od.COMPOSE_LIST_NO,"
"        COALESCE(t20.SI_PDI, 0) AS WEIGHT,"
"        od.BUNKER_NO,"
"        od.LOT_NO,"
"        COALESCE(md.C_VALUE, 0) AS C_VALUE,"
"        COALESCE(md.SI_VALUE, 0) AS SI_VALUE,"
"        COALESCE(md.MN_VALUE, 0) AS MN_VALUE,"
"        COALESCE(md.P_VALUE, 0) AS P_VALUE,"
"        COALESCE(md.S_VALUE, 0) AS S_VALUE,"
"        COALESCE(md.CR_VALUE, 0) AS CR_VALUE,"
"        COALESCE(md.NI_VALUE, 0) AS NI_VALUE,"
"        COALESCE(md.MO_VALUE, 0) AS MO_VALUE,"
"        COALESCE(md.CU_VALUE, 0) AS CU_VALUE,"
"        COALESCE(md.CO_VALUE, 0) AS CO_VALUE,"
"        0 AS COST,  "
"        '3' AS STOCK_NAME"
"    FROM original_data od"
"    LEFT JOIN TFBSM11 t11 ON od.ST_NO = t11.ST_NO"
"    LEFT JOIN TFBSM20 t20 ON t11.COMM_FMLY_CODE = t20.COMM_FMLY_CODE AND od.BACKLOG_EA = t20.BACKLOG_EA"
"    CROSS JOIN max_date_at000258 md"
"    WHERE NOT EXISTS (SELECT 1 FROM original_data WHERE MAT_CODE = 'AT000258')"
"    AND ROWNUM = 1"
") "
"SELECT * FROM ("
"    SELECT * FROM original_data"
"    UNION ALL"
"    SELECT * FROM supplement_data"
") temp_data "
"ORDER BY "
"    S_XU,"
"    MAT_CODE,"
"    CASE STOCK_NAME WHEN '2' THEN 1 WHEN '3' THEN 2 WHEN '1' THEN 3 WHEN '4' THEN 4 WHEN '5' THEN 5 ELSE 6 END ";			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();
		}
		else
		{
			sqlstr = " select case when STATION_ID='Y' THEN 'A' ELSE  STATION_ID END STATION_ID,case when STATION_ID='Y' THEN 1  when (MAT_CODE='PL0001' or MAT_CODE='PL0002')   THEN 3 ELSE 2 END S_XU,DATE_C,ST_NO,BACKLOG_EA,MAT_CODE,MAT_NAME,COMPOSE_LIST_NO,WEIGHT,BUNKER_NO,LOT_NO,C_VALUE,SI_VALUE,MN_VALUE,P_VALUE,S_VALUE,CR_VALUE,NI_VALUE,MO_VALUE,CU_VALUE,CO_VALUE,COST, "
				"CASE "
				"WHEN STOCK_NAME NOT IN('1', '2', '3', '4', '5') THEN(SELECT MAT_NAME FROM TMMSM60 WHERE BUNKER_NO = TFBSM14.STOCK_NAME AND MAT_CODE = TFBSM14.MAT_CODE) "
				"ELSE STOCK_NAME "
				"END AS STOCK_NAME "
				"from TFBSM14 where COMPOSE_LIST_NO='" + compose_list_no +  "' and STATION_ID='" + s_id_01 + "' order by S_XU,MAT_CODE, CASE STOCK_NAME WHEN '2' THEN 1 WHEN '3' THEN 2 WHEN '1' THEN 3 WHEN '4' THEN 4 WHEN '5' THEN 5 ELSE 6 END ";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();
		}
		bcls_ret->Tables.Add();
		if (s_id_02 == "A"){
			sqlstr = " WITH original_data AS ("
"    SELECT"
"        CASE WHEN STATION_ID = 'Y' THEN 'A' ELSE STATION_ID END STATION_ID,"
"        CASE WHEN STATION_ID = 'Y' THEN 1 WHEN (MAT_CODE = 'PL0001' OR MAT_CODE = 'PL0002') THEN 3 ELSE 2 END S_XU,"
"        DATE_C,"
"        ST_NO,"
"        BACKLOG_EA,"
"        MAT_CODE,"
"        MAT_NAME,"
"        COMPOSE_LIST_NO,"
"        WEIGHT,"
"        BUNKER_NO,"
"        LOT_NO,"
"        C_VALUE,"
"        SI_VALUE,"
"        MN_VALUE,"
"        P_VALUE,"
"        S_VALUE,"
"        CR_VALUE,"
"        NI_VALUE,"
"        MO_VALUE,"
"        CU_VALUE,"
"        CO_VALUE,"
"        COST,"
"        CASE WHEN STOCK_NAME NOT IN('1','2','3','4','5') THEN ("
"            SELECT MAT_NAME FROM TMMSM60 "
"            WHERE BUNKER_NO = TFBSM14.STOCK_NAME AND MAT_CODE = TFBSM14.MAT_CODE AND ROWNUM = 1"
"        ) ELSE STOCK_NAME END AS STOCK_NAME "
"from TFBSM14 where COMPOSE_LIST_NO='" + compose_list_no +  "' and (STATION_ID='" + s_id_02 + "' OR STATION_ID='Y') "
"),"
" max_date_at000258 AS ("
"    SELECT"
"        C_VALUE,"
"        SI_VALUE,"
"        MN_VALUE,"
"        P_VALUE,"
"        S_VALUE,"
"        CR_VALUE,"
"        NI_VALUE,"
"        MO_VALUE,"
"        CU_VALUE,"
"        CO_VALUE"
"    FROM TFBSM14"
"    WHERE MAT_CODE = 'AT000258'"
"      AND DATE_C = (SELECT MAX(DATE_C) FROM TFBSM14 WHERE MAT_CODE = 'AT000258')"
"    AND ROWNUM = 1 "
"), "
"supplement_data AS ("
"    SELECT"
"        'A' AS STATION_ID,"
"        3 AS S_XU,"
"        od.DATE_C,"
"        od.ST_NO,"
"        od.BACKLOG_EA,"
"        'AT000258' AS MAT_CODE,"
"        '硅铁 FeSi72Al2.0' AS MAT_NAME,"
"        od.COMPOSE_LIST_NO,"
"        COALESCE(t20.SI_PDI, 0) AS WEIGHT,"
"        od.BUNKER_NO,"
"        od.LOT_NO,"
"        COALESCE(md.C_VALUE, 0) AS C_VALUE,"
"        COALESCE(md.SI_VALUE, 0) AS SI_VALUE,"
"        COALESCE(md.MN_VALUE, 0) AS MN_VALUE,"
"        COALESCE(md.P_VALUE, 0) AS P_VALUE,"
"        COALESCE(md.S_VALUE, 0) AS S_VALUE,"
"        COALESCE(md.CR_VALUE, 0) AS CR_VALUE,"
"        COALESCE(md.NI_VALUE, 0) AS NI_VALUE,"
"        COALESCE(md.MO_VALUE, 0) AS MO_VALUE,"
"        COALESCE(md.CU_VALUE, 0) AS CU_VALUE,"
"        COALESCE(md.CO_VALUE, 0) AS CO_VALUE,"
"        0 AS COST,  "
"        '3' AS STOCK_NAME"
"    FROM original_data od"
"    LEFT JOIN TFBSM11 t11 ON od.ST_NO = t11.ST_NO"
"    LEFT JOIN TFBSM20 t20 ON t11.COMM_FMLY_CODE = t20.COMM_FMLY_CODE AND od.BACKLOG_EA = t20.BACKLOG_EA"
"    CROSS JOIN max_date_at000258 md"
"    WHERE NOT EXISTS (SELECT 1 FROM original_data WHERE MAT_CODE = 'AT000258')"
"    AND ROWNUM = 1"
") "
"SELECT * FROM ("
"    SELECT * FROM original_data"
"    UNION ALL"
"    SELECT * FROM supplement_data"
") temp_data "
"ORDER BY "
"    S_XU,"
"    MAT_CODE,"
"    CASE STOCK_NAME WHEN '2' THEN 1 WHEN '3' THEN 2 WHEN '1' THEN 3 WHEN '4' THEN 4 WHEN '5' THEN 5 ELSE 6 END ";			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[1]);
			cmd_inq.Close();
		}
		else
		{
			sqlstr = " select case when STATION_ID='Y' THEN 'A' ELSE  STATION_ID END STATION_ID,case when STATION_ID='Y' THEN 1  when (MAT_CODE='PL0001' or MAT_CODE='PL0002')   THEN 3 ELSE 2 END S_XU,DATE_C,ST_NO,BACKLOG_EA,MAT_CODE,MAT_NAME,COMPOSE_LIST_NO,WEIGHT,BUNKER_NO,LOT_NO,C_VALUE,SI_VALUE,MN_VALUE,P_VALUE,S_VALUE,CR_VALUE,NI_VALUE,MO_VALUE,CU_VALUE,CO_VALUE,COST, "
				"CASE "
				"WHEN STOCK_NAME NOT IN('1', '2', '3', '4', '5') THEN(SELECT MAT_NAME FROM TMMSM60 WHERE BUNKER_NO = TFBSM14.STOCK_NAME AND MAT_CODE = TFBSM14.MAT_CODE) "
				"ELSE STOCK_NAME "
				"END AS STOCK_NAME "
				"from TFBSM14 where COMPOSE_LIST_NO='" + compose_list_no +  "' and STATION_ID='" + s_id_02 + "' order by S_XU,MAT_CODE, CASE STOCK_NAME WHEN '2' THEN 1 WHEN '3' THEN 2 WHEN '1' THEN 3 WHEN '4' THEN 4 WHEN '5' THEN 5 ELSE 6 END ";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[1]);
			cmd_inq.Close();
		}
		bcls_ret->Tables.Add();
		if (s_id_03 == "A"){
			sqlstr = " WITH original_data AS ("
"    SELECT"
"        CASE WHEN STATION_ID = 'Y' THEN 'A' ELSE STATION_ID END STATION_ID,"
"        CASE WHEN STATION_ID = 'Y' THEN 1 WHEN (MAT_CODE = 'PL0001' OR MAT_CODE = 'PL0002') THEN 3 ELSE 2 END S_XU,"
"        DATE_C,"
"        ST_NO,"
"        BACKLOG_EA,"
"        MAT_CODE,"
"        MAT_NAME,"
"        COMPOSE_LIST_NO,"
"        WEIGHT,"
"        BUNKER_NO,"
"        LOT_NO,"
"        C_VALUE,"
"        SI_VALUE,"
"        MN_VALUE,"
"        P_VALUE,"
"        S_VALUE,"
"        CR_VALUE,"
"        NI_VALUE,"
"        MO_VALUE,"
"        CU_VALUE,"
"        CO_VALUE,"
"        COST,"
"        CASE WHEN STOCK_NAME NOT IN('1','2','3','4','5') THEN ("
"            SELECT MAT_NAME FROM TMMSM60 "
"            WHERE BUNKER_NO = TFBSM14.STOCK_NAME AND MAT_CODE = TFBSM14.MAT_CODE AND ROWNUM = 1"
"        ) ELSE STOCK_NAME END AS STOCK_NAME "
"from TFBSM14 where COMPOSE_LIST_NO='" + compose_list_no +  "' and (STATION_ID='" + s_id_03 + "' OR STATION_ID='Y') "
"),"
" max_date_at000258 AS ("
"    SELECT"
"        C_VALUE,"
"        SI_VALUE,"
"        MN_VALUE,"
"        P_VALUE,"
"        S_VALUE,"
"        CR_VALUE,"
"        NI_VALUE,"
"        MO_VALUE,"
"        CU_VALUE,"
"        CO_VALUE"
"    FROM TFBSM14"
"    WHERE MAT_CODE = 'AT000258'"
"      AND DATE_C = (SELECT MAX(DATE_C) FROM TFBSM14 WHERE MAT_CODE = 'AT000258')"
"    AND ROWNUM = 1 "
"), "
"supplement_data AS ("
"    SELECT"
"        'A' AS STATION_ID,"
"        3 AS S_XU,"
"        od.DATE_C,"
"        od.ST_NO,"
"        od.BACKLOG_EA,"
"        'AT000258' AS MAT_CODE,"
"        '硅铁 FeSi72Al2.0' AS MAT_NAME,"
"        od.COMPOSE_LIST_NO,"
"        COALESCE(t20.SI_PDI, 0) AS WEIGHT,"
"        od.BUNKER_NO,"
"        od.LOT_NO,"
"        COALESCE(md.C_VALUE, 0) AS C_VALUE,"
"        COALESCE(md.SI_VALUE, 0) AS SI_VALUE,"
"        COALESCE(md.MN_VALUE, 0) AS MN_VALUE,"
"        COALESCE(md.P_VALUE, 0) AS P_VALUE,"
"        COALESCE(md.S_VALUE, 0) AS S_VALUE,"
"        COALESCE(md.CR_VALUE, 0) AS CR_VALUE,"
"        COALESCE(md.NI_VALUE, 0) AS NI_VALUE,"
"        COALESCE(md.MO_VALUE, 0) AS MO_VALUE,"
"        COALESCE(md.CU_VALUE, 0) AS CU_VALUE,"
"        COALESCE(md.CO_VALUE, 0) AS CO_VALUE,"
"        0 AS COST, "
"        '3' AS STOCK_NAME"
"    FROM original_data od"
"    LEFT JOIN TFBSM11 t11 ON od.ST_NO = t11.ST_NO"
"    LEFT JOIN TFBSM20 t20 ON t11.COMM_FMLY_CODE = t20.COMM_FMLY_CODE AND od.BACKLOG_EA = t20.BACKLOG_EA"
"    CROSS JOIN max_date_at000258 md"
"    WHERE NOT EXISTS (SELECT 1 FROM original_data WHERE MAT_CODE = 'AT000258')"
"    AND ROWNUM = 1"
") "
"SELECT * FROM ("
"    SELECT * FROM original_data"
"    UNION ALL"
"    SELECT * FROM supplement_data"
") temp_data "
"ORDER BY "
"    S_XU,"
"    MAT_CODE,"
"    CASE STOCK_NAME WHEN '2' THEN 1 WHEN '3' THEN 2 WHEN '1' THEN 3 WHEN '4' THEN 4 WHEN '5' THEN 5 ELSE 6 END ";			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[2]);
			cmd_inq.Close();
		}
		else{
			sqlstr = " select case when STATION_ID='Y' THEN 'A' ELSE  STATION_ID END STATION_ID,case when STATION_ID='Y' THEN 1  when (MAT_CODE='PL0001' or MAT_CODE='PL0002')   THEN 3  ELSE 2 END S_XU,DATE_C,ST_NO,BACKLOG_EA,MAT_CODE,MAT_NAME,COMPOSE_LIST_NO,WEIGHT,BUNKER_NO,LOT_NO,C_VALUE,SI_VALUE,MN_VALUE,P_VALUE,S_VALUE,CR_VALUE,NI_VALUE,MO_VALUE,CU_VALUE,CO_VALUE,COST, "
				"CASE "
				"WHEN STOCK_NAME NOT IN('1', '2', '3', '4', '5') THEN(SELECT MAT_NAME FROM TMMSM60 WHERE BUNKER_NO = TFBSM14.STOCK_NAME AND MAT_CODE = TFBSM14.MAT_CODE) "
				"ELSE STOCK_NAME "
				"END AS STOCK_NAME "
				"from TFBSM14 where COMPOSE_LIST_NO='" + compose_list_no +  "' and STATION_ID='" + s_id_03 + "' order by S_XU,MAT_CODE, CASE STOCK_NAME WHEN '2' THEN 1 WHEN '3' THEN 2 WHEN '1' THEN 3 WHEN '4' THEN 4 WHEN '5' THEN 5 ELSE 6 END ";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[2]);
			cmd_inq.Close();
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
