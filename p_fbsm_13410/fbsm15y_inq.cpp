/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15y_inq)

int f_fbsm15y_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
    CTracer log(__FUNCTION__);
    int doFlag = 0;
    int i = 0;
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
		 // CString str1 = "";
         // bcls_rec->WriteHTML(str1);
         // Log::Trace("", __FUNCTION__, "str=[{0}]", str1);
        if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
			compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
            Log::Trace("", "", "compose_list_no = {0}", compose_list_no);
            Log::Trace("", "", "st_no = {0}", st_no);
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
                 "        TI_VALUE," // 2026.3.6 报表需求新增元素
                 "        NB_VALUE,"
                 "        AL_VALUE,"
                 "        B_VALUE,"
                 "        V_VALUE,"
                 "        CA_VALUE,"
                 "        N_VALUE,"
                 "        COST,"
                 "        CASE WHEN STOCK_NAME NOT IN('1','2','3','4','5') THEN ("
                 "            SELECT MAT_NAME FROM TMMSM60 "
                 "            WHERE BUNKER_NO = TFBSM14.STOCK_NAME AND MAT_CODE = TFBSM14.MAT_CODE AND ROWNUM = 1"
                 "        ) ELSE STOCK_NAME END AS STOCK_NAME "
                 "		from TFBSM14	"
                 "          WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                 "            AND st_no = @ST_NO "
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
                 "        CO_VALUE,"
                 "        TI_VALUE," // 2026.3.6 报表需求新增元素
                 "        NB_VALUE,"
                 "        AL_VALUE,"
                 "        B_VALUE,"
                 "        V_VALUE,"
                 "        CA_VALUE,"
                 "        N_VALUE"
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
                 "        COALESCE(md.TI_VALUE, 0) AS TI_VALUE,"
                 "        COALESCE(md.NB_VALUE, 0) AS NB_VALUE,"
                 "        COALESCE(md.AL_VALUE, 0) AS AL_VALUE,"
                 "        COALESCE(md.B_VALUE, 0) AS B_VALUE,"
                 "        COALESCE(md.V_VALUE, 0) AS V_VALUE,"
                 "        COALESCE(md.CA_VALUE, 0) AS CA_VALUE,"
                 "        COALESCE(md.N_VALUE, 0) AS N_VALUE,"
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
                 // " where mat_code not in ('PL0001','PL0002','PL0003')"
                 "ORDER BY "

                 "    S_XU,"
                 "    MAT_CODE,"
                 "    CASE STOCK_NAME WHEN '2' THEN 1 WHEN '3' THEN 2 WHEN '1' THEN 3 WHEN '4' THEN 4 WHEN '5' THEN 5 ELSE 6 END ";
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
        cmd_inq.Parameters.Set("ST_NO", st_no);
        cmd_inq.SetCommandText(sqlstr);
        Log::Trace("", "", "sqlstr = {0}", sqlstr);
        cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
        cmd_inq.Close();
        Log::Trace("", "", "查询结果条数 = {0}", bcls_ret->Tables[0].Rows.get_Count());

        // 备注信息
        bcls_ret->Tables.Add("REMARK");
        sqlstr = "select BACK_AOD,BACK_BOF,BACK_IF,BACK_EAF"
                 " from tfbsm14a"
                 " where 1=1"
                 " and  COMPOSE_LIST_NO= @COMPOSE_LIST_NO ";
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
        cmd_inq.SetCommandText(sqlstr);
        Log::Trace("", "", "sqlstr = {0}", sqlstr);
        cmd_inq.ExecuteQuery(bcls_ret->Tables["REMARK"]);
        cmd_inq.Close();

        bcls_ret->Tables.Add("CONTROL");
        // C%收得率, Si%收得率, Mn%收得率, P%收得率, S%收得率, Cr%收得率, Ni%收得率, Mo%收得率, Cu%收得率, Co%收得率, Al%收得率, Nb%收得率, V%收得率, Ti%收得率, B%收得率, N%收得率, Ca%收得率]
        sqlstr = " SELECT MAX(CASE WHEN ELM_NAME = 'C' THEN MAIN_AIM ELSE 0 END) C_AIM,MAX(CASE WHEN ELM_NAME = 'C' THEN MAIN_MIN ELSE 0 END) C_MIN,MAX(CASE WHEN ELM_NAME = 'C' THEN MAIN_MAX ELSE 0 END) C_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'Si' THEN MAIN_AIM ELSE 0 END) SI_AIM, MAX(CASE WHEN ELM_NAME = 'Si' THEN MAIN_MIN ELSE 0 END) SI_MIN, MAX(CASE WHEN ELM_NAME = 'Si' THEN MAIN_MAX ELSE 0 END) SI_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'Mn' THEN MAIN_AIM ELSE 0 END) MN_AIM, MAX(CASE WHEN ELM_NAME = 'Mn' THEN MAIN_MIN ELSE 0 END) MN_MIN, MAX(CASE WHEN ELM_NAME = 'Mn' THEN MAIN_MAX ELSE 0 END) MN_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'P' THEN MAIN_AIM ELSE 0 END) P_AIM, MAX(CASE WHEN ELM_NAME = 'P' THEN MAIN_MIN ELSE 0 END) P_MIN, MAX(CASE WHEN ELM_NAME = 'P' THEN MAIN_MAX ELSE 0 END) P_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'S' THEN MAIN_AIM ELSE 0 END) S_AIM, MAX(CASE WHEN ELM_NAME = 'S' THEN MAIN_MIN ELSE 0 END) S_MIN, MAX(CASE WHEN ELM_NAME = 'S' THEN MAIN_MAX ELSE 0 END) S_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'Cr' THEN MAIN_AIM ELSE 0 END) CR_AIM, MAX(CASE WHEN ELM_NAME = 'Cr' THEN MAIN_MIN ELSE 0 END) CR_MIN, MAX(CASE WHEN ELM_NAME = 'Cr' THEN MAIN_MAX ELSE 0 END) CR_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'Ni' THEN MAIN_AIM ELSE 0 END) NI_AIM, MAX(CASE WHEN ELM_NAME = 'Ni' THEN MAIN_MIN ELSE 0 END) NI_MIN, MAX(CASE WHEN ELM_NAME = 'Ni' THEN MAIN_MAX ELSE 0 END) NI_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'Mo' THEN MAIN_AIM ELSE 0 END) MO_AIM, MAX(CASE WHEN ELM_NAME = 'Mo' THEN MAIN_MIN ELSE 0 END) MO_MIN, MAX(CASE WHEN ELM_NAME = 'Mo' THEN MAIN_MAX ELSE 0 END) MO_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'Mu' THEN MAIN_AIM ELSE 0 END) CU_AIM, MAX(CASE WHEN ELM_NAME = 'Mu' THEN MAIN_MIN ELSE 0 END) CU_MIN, MAX(CASE WHEN ELM_NAME = 'Mu' THEN MAIN_MAX ELSE 0 END) CU_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'Co' THEN MAIN_AIM ELSE 0 END) CO_AIM, MAX(CASE WHEN ELM_NAME = 'Co' THEN MAIN_MIN ELSE 0 END) CO_MIN, MAX(CASE WHEN ELM_NAME = 'Co' THEN MAIN_MAX ELSE 0 END) CO_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'Al' THEN MAIN_AIM ELSE 0 END) AL_AIM, MAX(CASE WHEN ELM_NAME = 'Al' THEN MAIN_MIN ELSE 0 END) AL_MIN, MAX(CASE WHEN ELM_NAME = 'Al' THEN MAIN_MAX ELSE 0 END) AL_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'Ab' THEN MAIN_AIM ELSE 0 END) NB_AIM, MAX(CASE WHEN ELM_NAME = 'Ab' THEN MAIN_MIN ELSE 0 END) NB_MIN, MAX(CASE WHEN ELM_NAME = 'Ab' THEN MAIN_MAX ELSE 0 END) NB_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'V' THEN MAIN_AIM ELSE 0 END) V_AIM, MAX(CASE WHEN ELM_NAME = 'V' THEN MAIN_MIN ELSE 0 END) V_MIN, MAX(CASE WHEN ELM_NAME = 'V' THEN MAIN_MAX ELSE 0 END) V_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'Ti' THEN MAIN_AIM ELSE 0 END) TI_AIM, MAX(CASE WHEN ELM_NAME = 'Ti' THEN MAIN_MIN ELSE 0 END) TI_MIN, MAX(CASE WHEN ELM_NAME = 'Ti' THEN MAIN_MAX ELSE 0 END) TI_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'B' THEN MAIN_AIM ELSE 0 END) B_AIM, MAX(CASE WHEN ELM_NAME = 'B' THEN MAIN_MIN ELSE 0 END) B_MIN, MAX(CASE WHEN ELM_NAME = 'B' THEN MAIN_MAX ELSE 0 END) B_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'N' THEN MAIN_AIM ELSE 0 END) N_AIM, MAX(CASE WHEN ELM_NAME = 'N' THEN MAIN_MIN ELSE 0 END) N_MIN, MAX(CASE WHEN ELM_NAME = 'N' THEN MAIN_MAX ELSE 0 END) N_MAX"
                 ",MAX(CASE WHEN ELM_NAME = 'Ca' THEN MAIN_AIM ELSE 0 END) CA_AIM, MAX(CASE WHEN ELM_NAME = 'Ca' THEN MAIN_MIN ELSE 0 END) CA_MIN, MAX(CASE WHEN ELM_NAME = 'Ca' THEN MAIN_MAX ELSE 0 END) CA_MAX"
                 " FROM TQMTS02 "
                 " WHERE IDX_NO = (SELECT ELM_STD_IDX_A FROM TQMTS0X WHERE ST_NO = @st_no)";
        cmd_inq.SetCommandText(sqlstr);
        cmd_inq.Parameters.Set("st_no", st_no);
        cmd_inq.ExecuteQuery(bcls_ret->Tables["CONTROL"]);
        cmd_inq.Close();
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
