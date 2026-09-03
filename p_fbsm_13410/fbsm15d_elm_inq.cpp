/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15d_elm_inq)

int f_fbsm15d_elm_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
	CString heat_no = "";
	//CString st_no = " ";
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("HEAT_NO"))
			heat_no = bcls_rec->Tables[0].Rows[0]["HEAT_NO"].ToString().TrimOrBlank().ToUpper();
		//先查询一下有几个工序
		sqlstr = " SELECT  STATION_ID FROM ( select "
			"  distinct case when STATION_ID='Y' THEN 'A' ELSE  STATION_ID END STATION_ID, "
			" CASE WHEN STATION_ID = 'A' OR STATION_ID = 'Y' THEN 1 "
			" WHEN STATION_ID = 'Z' THEN 3 "
			" WHEN STATION_ID = 'E' THEN 2 "
			" WHEN STATION_ID = 'B' THEN 4 "
			" WHEN STATION_ID = 'D' THEN 5 ELSE 0 END XU "
			" from TMMSM2A_YL where HEAT_NO= @HEAT_NO AND STATION_ID IN ('A','Y','Z','E','B','D')) ORDER BY XU  DESC ";
		cmd.SetCommandText(sqlstr);
		cmd.Parameters.Set("HEAT_NO", heat_no);
		Log::Trace("", "", "sqlstr = {0}", sqlstr);
		Log::Trace("", "", "heat_no = {0}", heat_no);
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
		if (s_id_01.Trim() != ""){
			sqlstr = "SELECT "
"    t1.STATION_ID,"
"    t1.MAT_CODE,"
"    t1.MAT_NAME,"
"    t1.LOT_NO,"
"    SUM(t1.DEVO_WT) AS WEIGHT,"
"    t2.C_VALUE,"
"    t2.SI_VALUE,"
"    t2.MN_VALUE,"
"    t2.P_VALUE,"
"    t2.S_VALUE,"
"    t2.CR_VALUE,"
"    t2.NI_VALUE,"
"    t2.MO_VALUE,"
"    t2.CU_VALUE "
"FROM TMMSM2A_YL t1 "
"INNER JOIN TMMSM50 t0 ON t1.MAT_CODE = t0.MAT_CODE "
"LEFT JOIN ( "
"    SELECT "
"        t85.MAT_CODE,"
"        t85.LOT_NO,"
"        t85.QUALITY_BATCH_NO "
"    FROM ( "
"        SELECT "
"            t85.MAT_CODE,"
"            t85.LOT_NO,"
"            t85.QUALITY_BATCH_NO,"
"            t85.SEQ_NO,"
"            ROW_NUMBER() OVER ( "
"                PARTITION BY t85.MAT_CODE, t85.LOT_NO "
"                ORDER BY "
"                    CASE WHEN EXISTS ( "
"                        SELECT 1 "
"                        FROM TMMSM81AL t81 "
"                        WHERE t81.MAT_CODE = t85.MAT_CODE "
"                            AND t81.LOT_NO = t85.LOT_NO "
"                            AND t81.QUALITY_BATCH_NO = t85.QUALITY_BATCH_NO "
"                    ) THEN 1 ELSE 2 END,"
"                    t85.SEQ_NO DESC "
"            ) AS rn "
"        FROM TMMSM85 t85 "
"    ) t85 "
"    WHERE t85.rn = 1 "
") t85_max ON t1.MAT_CODE = t85_max.MAT_CODE AND t1.LOT_NO = t85_max.LOT_NO "
"LEFT JOIN ( "
"    SELECT "
"        MAT_CODE,"
"        LOT_NO,"
"        QUALITY_BATCH_NO,"
"        C_VALUE,"
"        SI_VALUE,"
"        MN_VALUE,"
"        P_VALUE,"
"        S_VALUE,"
"        CR_VALUE,"
"        NI_VALUE,"
"        MO_VALUE,"
"        CU_VALUE "
"    FROM ( "
"        SELECT "
"            MAT_CODE,"
"            LOT_NO,"
"            QUALITY_BATCH_NO,"
"            ANALYSE_DATA_TYPE,"
"            MAX(CASE WHEN ELM_NAME = 'C' THEN ELM_VALUE END) AS C_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Si' THEN ELM_VALUE END) AS SI_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Mn' THEN ELM_VALUE END) AS MN_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'P' THEN ELM_VALUE END) AS P_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'S' THEN ELM_VALUE END) AS S_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Cr' THEN ELM_VALUE END) AS CR_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Ni' THEN ELM_VALUE END) AS NI_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Mo' THEN ELM_VALUE END) AS MO_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Cu' THEN ELM_VALUE END) AS CU_VALUE,"
"            ROW_NUMBER() OVER ( "
"                PARTITION BY MAT_CODE, LOT_NO, QUALITY_BATCH_NO "
"                ORDER BY CASE "
"                    WHEN ANALYSE_DATA_TYPE = 'N' THEN 1 "
"                    WHEN ANALYSE_DATA_TYPE = 'Y' THEN 2 "
"                    WHEN ANALYSE_DATA_TYPE IS NULL OR ANALYSE_DATA_TYPE = ' ' THEN 3 "
"                    ELSE 4 "
"                END "
"            ) AS rn "
"        FROM TMMSM81AL "
"        GROUP BY MAT_CODE, LOT_NO, QUALITY_BATCH_NO, ANALYSE_DATA_TYPE "
"    ) t "
"    WHERE t.rn = 1 "
") t2 ON t85_max.MAT_CODE = t2.MAT_CODE "
"    AND t85_max.LOT_NO = t2.LOT_NO "
"    AND t85_max.QUALITY_BATCH_NO = t2.QUALITY_BATCH_NO "
"WHERE t1.HEAT_NO = @HEAT_NO "
"  AND t1.STATION_ID = @STATION_ID "
"  AND (t0.MAT_TYPE IS NULL OR t0.MAT_TYPE <> '3') "
"GROUP BY "
"    t1.STATION_ID,"
"    t1.MAT_CODE,"
"    t1.MAT_NAME,"
"    t1.LOT_NO,"
"    t2.C_VALUE,"
"    t2.SI_VALUE,"
"    t2.MN_VALUE,"
"    t2.P_VALUE,"
"    t2.S_VALUE,"
"    t2.CR_VALUE,"
"    t2.NI_VALUE,"
"    t2.MO_VALUE,"
"    t2.CU_VALUE "
"ORDER BY t1.STATION_ID, t1.MAT_CODE";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("HEAT_NO", heat_no);
			cmd_inq.Parameters.Set("STATION_ID", s_id_01);
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();
		}

		bcls_ret->Tables.Add();
		if (s_id_02.Trim() != ""){
			sqlstr = "SELECT "
"    t1.STATION_ID,"
"    t1.MAT_CODE,"
"    t1.MAT_NAME,"
"    t1.LOT_NO,"
"    SUM(t1.DEVO_WT) AS WEIGHT,"
"    t2.C_VALUE,"
"    t2.SI_VALUE,"
"    t2.MN_VALUE,"
"    t2.P_VALUE,"
"    t2.S_VALUE,"
"    t2.CR_VALUE,"
"    t2.NI_VALUE,"
"    t2.MO_VALUE,"
"    t2.CU_VALUE "
"FROM TMMSM2A_YL t1 "
"INNER JOIN TMMSM50 t0 ON t1.MAT_CODE = t0.MAT_CODE "
"LEFT JOIN ( "
"    SELECT "
"        t85.MAT_CODE,"
"        t85.LOT_NO,"
"        t85.QUALITY_BATCH_NO "
"    FROM ( "
"        SELECT "
"            t85.MAT_CODE,"
"            t85.LOT_NO,"
"            t85.QUALITY_BATCH_NO,"
"            t85.SEQ_NO,"
"            ROW_NUMBER() OVER ( "
"                PARTITION BY t85.MAT_CODE, t85.LOT_NO "
"                ORDER BY "
"                    CASE WHEN EXISTS ( "
"                        SELECT 1 "
"                        FROM TMMSM81AL t81 "
"                        WHERE t81.MAT_CODE = t85.MAT_CODE "
"                            AND t81.LOT_NO = t85.LOT_NO "
"                            AND t81.QUALITY_BATCH_NO = t85.QUALITY_BATCH_NO "
"                    ) THEN 1 ELSE 2 END,"
"                    t85.SEQ_NO DESC "
"            ) AS rn "
"        FROM TMMSM85 t85 "
"    ) t85 "
"    WHERE t85.rn = 1 "
") t85_max ON t1.MAT_CODE = t85_max.MAT_CODE AND t1.LOT_NO = t85_max.LOT_NO "
"LEFT JOIN ( "
"    SELECT "
"        MAT_CODE,"
"        LOT_NO,"
"        QUALITY_BATCH_NO,"
"        C_VALUE,"
"        SI_VALUE,"
"        MN_VALUE,"
"        P_VALUE,"
"        S_VALUE,"
"        CR_VALUE,"
"        NI_VALUE,"
"        MO_VALUE,"
"        CU_VALUE "
"    FROM ( "
"        SELECT "
"            MAT_CODE,"
"            LOT_NO,"
"            QUALITY_BATCH_NO,"
"            ANALYSE_DATA_TYPE,"
"            MAX(CASE WHEN ELM_NAME = 'C' THEN ELM_VALUE END) AS C_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Si' THEN ELM_VALUE END) AS SI_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Mn' THEN ELM_VALUE END) AS MN_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'P' THEN ELM_VALUE END) AS P_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'S' THEN ELM_VALUE END) AS S_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Cr' THEN ELM_VALUE END) AS CR_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Ni' THEN ELM_VALUE END) AS NI_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Mo' THEN ELM_VALUE END) AS MO_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Cu' THEN ELM_VALUE END) AS CU_VALUE,"
"            ROW_NUMBER() OVER ( "
"                PARTITION BY MAT_CODE, LOT_NO, QUALITY_BATCH_NO "
"                ORDER BY CASE "
"                    WHEN ANALYSE_DATA_TYPE = 'N' THEN 1 "
"                    WHEN ANALYSE_DATA_TYPE = 'Y' THEN 2 "
"                    WHEN ANALYSE_DATA_TYPE IS NULL OR ANALYSE_DATA_TYPE = ' ' THEN 3 "
"                    ELSE 4 "
"                END "
"            ) AS rn "
"        FROM TMMSM81AL "
"        GROUP BY MAT_CODE, LOT_NO, QUALITY_BATCH_NO, ANALYSE_DATA_TYPE "
"    ) t "
"    WHERE t.rn = 1 "
") t2 ON t85_max.MAT_CODE = t2.MAT_CODE "
"    AND t85_max.LOT_NO = t2.LOT_NO "
"    AND t85_max.QUALITY_BATCH_NO = t2.QUALITY_BATCH_NO "
"WHERE t1.HEAT_NO = @HEAT_NO "
"  AND t1.STATION_ID = @STATION_ID "
"  AND (t0.MAT_TYPE IS NULL OR t0.MAT_TYPE <> '3') "
"GROUP BY "
"    t1.STATION_ID,"
"    t1.MAT_CODE,"
"    t1.MAT_NAME,"
"    t1.LOT_NO,"
"    t2.C_VALUE,"
"    t2.SI_VALUE,"
"    t2.MN_VALUE,"
"    t2.P_VALUE,"
"    t2.S_VALUE,"
"    t2.CR_VALUE,"
"    t2.NI_VALUE,"
"    t2.MO_VALUE,"
"    t2.CU_VALUE "
"ORDER BY t1.STATION_ID, t1.MAT_CODE";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("HEAT_NO", heat_no);
			cmd_inq.Parameters.Set("STATION_ID", s_id_02);
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[1]);
			cmd_inq.Close();
		}
		bcls_ret->Tables.Add();
		if (s_id_03.Trim() != ""){
			sqlstr = "SELECT "
"    t1.STATION_ID,"
"    t1.MAT_CODE,"
"    t1.MAT_NAME,"
"    t1.LOT_NO,"
"    SUM(t1.DEVO_WT) AS WEIGHT,"
"    t2.C_VALUE,"
"    t2.SI_VALUE,"
"    t2.MN_VALUE,"
"    t2.P_VALUE,"
"    t2.S_VALUE,"
"    t2.CR_VALUE,"
"    t2.NI_VALUE,"
"    t2.MO_VALUE,"
"    t2.CU_VALUE "
"FROM TMMSM2A_YL t1 "
"INNER JOIN TMMSM50 t0 ON t1.MAT_CODE = t0.MAT_CODE "
"LEFT JOIN ( "
"    SELECT "
"        t85.MAT_CODE,"
"        t85.LOT_NO,"
"        t85.QUALITY_BATCH_NO "
"    FROM ( "
"        SELECT "
"            t85.MAT_CODE,"
"            t85.LOT_NO,"
"            t85.QUALITY_BATCH_NO,"
"            t85.SEQ_NO,"
"            ROW_NUMBER() OVER ( "
"                PARTITION BY t85.MAT_CODE, t85.LOT_NO "
"                ORDER BY "
"                    CASE WHEN EXISTS ( "
"                        SELECT 1 "
"                        FROM TMMSM81AL t81 "
"                        WHERE t81.MAT_CODE = t85.MAT_CODE "
"                            AND t81.LOT_NO = t85.LOT_NO "
"                            AND t81.QUALITY_BATCH_NO = t85.QUALITY_BATCH_NO "
"                    ) THEN 1 ELSE 2 END,"
"                    t85.SEQ_NO DESC "
"            ) AS rn "
"        FROM TMMSM85 t85 "
"    ) t85 "
"    WHERE t85.rn = 1 "
") t85_max ON t1.MAT_CODE = t85_max.MAT_CODE AND t1.LOT_NO = t85_max.LOT_NO "
"LEFT JOIN ( "
"    SELECT "
"        MAT_CODE,"
"        LOT_NO,"
"        QUALITY_BATCH_NO,"
"        C_VALUE,"
"        SI_VALUE,"
"        MN_VALUE,"
"        P_VALUE,"
"        S_VALUE,"
"        CR_VALUE,"
"        NI_VALUE,"
"        MO_VALUE,"
"        CU_VALUE "
"    FROM ( "
"        SELECT "
"            MAT_CODE,"
"            LOT_NO,"
"            QUALITY_BATCH_NO,"
"            ANALYSE_DATA_TYPE,"
"            MAX(CASE WHEN ELM_NAME = 'C' THEN ELM_VALUE END) AS C_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Si' THEN ELM_VALUE END) AS SI_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Mn' THEN ELM_VALUE END) AS MN_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'P' THEN ELM_VALUE END) AS P_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'S' THEN ELM_VALUE END) AS S_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Cr' THEN ELM_VALUE END) AS CR_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Ni' THEN ELM_VALUE END) AS NI_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Mo' THEN ELM_VALUE END) AS MO_VALUE,"
"            MAX(CASE WHEN ELM_NAME = 'Cu' THEN ELM_VALUE END) AS CU_VALUE,"
"            ROW_NUMBER() OVER ( "
"                PARTITION BY MAT_CODE, LOT_NO, QUALITY_BATCH_NO "
"                ORDER BY CASE "
"                    WHEN ANALYSE_DATA_TYPE = 'N' THEN 1 "
"                    WHEN ANALYSE_DATA_TYPE = 'Y' THEN 2 "
"                    WHEN ANALYSE_DATA_TYPE IS NULL OR ANALYSE_DATA_TYPE = ' ' THEN 3 "
"                    ELSE 4 "
"                END "
"            ) AS rn "
"        FROM TMMSM81AL "
"        GROUP BY MAT_CODE, LOT_NO, QUALITY_BATCH_NO, ANALYSE_DATA_TYPE "
"    ) t "
"    WHERE t.rn = 1 "
") t2 ON t85_max.MAT_CODE = t2.MAT_CODE "
"    AND t85_max.LOT_NO = t2.LOT_NO "
"    AND t85_max.QUALITY_BATCH_NO = t2.QUALITY_BATCH_NO "
"WHERE t1.HEAT_NO = @HEAT_NO "
"  AND t1.STATION_ID = @STATION_ID "
"  AND (t0.MAT_TYPE IS NULL OR t0.MAT_TYPE <> '3') "
"GROUP BY "
"    t1.STATION_ID,"
"    t1.MAT_CODE,"
"    t1.MAT_NAME,"
"    t1.LOT_NO,"
"    t2.C_VALUE,"
"    t2.SI_VALUE,"
"    t2.MN_VALUE,"
"    t2.P_VALUE,"
"    t2.S_VALUE,"
"    t2.CR_VALUE,"
"    t2.NI_VALUE,"
"    t2.MO_VALUE,"
"    t2.CU_VALUE "
"ORDER BY t1.STATION_ID, t1.MAT_CODE";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("HEAT_NO", heat_no);
			cmd_inq.Parameters.Set("STATION_ID", s_id_03);
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
