/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 电炉料篮配料单查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm41_inq_dtl)

int f_fbsm41_inq_dtl(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
    CString year_str = CDateTime::Now().ToString("yyyy");
    CString compose_list_no = "";
    CString compose_list_no_eaf = "";
    try
    {
        if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
            compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
        if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO_EAF"))
            compose_list_no_eaf = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO_EAF"].ToString().TrimOrBlank().ToUpper();

        Log::Trace("", "", "compose_list_no = {0}", compose_list_no);
        Log::Trace("", "", "compose_list_no_eaf = {0}", compose_list_no_eaf);


        sqlstr = " SELECT T41.*,T14.* FROM TFBSM41 T41 "
                 " LEFT JOIN TFBSM14 T14 "
                 " ON T41.COMPOSE_LIST_NO = T14.COMPOSE_LIST_NO "
                 " AND T41.MAT_CODE = T14.MAT_CODE "
                 " AND T41.LOT_NO = T14.LOT_NO "
                 " AND T41.STOCK_NAME = T14.STOCK_NAME "
                 " AND T14.STATION_ID = 'E' "
                 " WHERE 1=1 "
                 " AND T41.COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                 " AND T41.COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF "

            ;
        cmd_inq.SetCommandText(sqlstr);
        Log::Trace("", "", "sqlstr = {0}", sqlstr);
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
        cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
        cmd_inq.Close();

        bcls_ret->Tables.Add();
        sqlstr = " SELECT "
                 " NVL(TRIM(T43.COMPOSE_LIST_NO), '') AS COMPOSE_LIST_NO,"
                 " NVL(TRIM(T43.ST_NO), '') AS ST_NO,"
                 " NVL(TRIM(T43.COMPOSE_LIST_NO_EAF), '') AS COMPOSE_LIST_NO_EAF,"
                 " NVL(TRIM(T43.DEV_CODE), '') AS DEV_CODE,"
                 " NVL(TRIM(T43.EAF_PROC_NO), '') AS EAF_PROC_NO,"
                 " NVL(TRIM(T43.REMARK), '') AS REMARK,"
                 " NVL(TRIM(T43.RESP), '') AS RESP,"
                 " NVL(TRIM(T43.SHIFT_GROUP), '') AS SHIFT_GROUP,"
                 " NVL(TRIM(T43.ERROR_REMARK), '') AS ERROR_REMARK,"
                 " NVL((SELECT MAX(TO_NUMBER(SUBSTR(T41.BUNKER_NO, INSTR(T41.BUNKER_NO, '#') + 1))) "
                 "      FROM TFBSM41 T41 "
                 "      INNER JOIN TFBSM43 T43 ON T41.COMPOSE_LIST_NO = T43.COMPOSE_LIST_NO AND T41.COMPOSE_LIST_NO_EAF = T43.COMPOSE_LIST_NO_EAF "
                 "      WHERE REGEXP_LIKE(T41.BUNKER_NO, '#[0-9]+') "
                 "        AND T43.CHECK_DATE LIKE @YEAR_PREFIX), 0) AS BUNKER_SEQ_MAX "
                 " FROM TFBSM43 T43 "
                 " WHERE 1=1 "
                 " AND T43.COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                 " AND T43.COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF "

            ;
        cmd_inq.SetCommandText(sqlstr);
        Log::Trace("", "", "sqlstr = {0}", sqlstr);
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
        cmd_inq.Parameters.Set("YEAR_PREFIX", year_str + "%");
        cmd_inq.ExecuteQuery(bcls_ret->Tables[1]);
        cmd_inq.Close();


        bcls_ret->Tables.Add();
        sqlstr = " SELECT "
                 " 'E1' || SUBSTR(TO_CHAR(SYSDATE, 'YYYY'), 4, 1) || "
                 " NVL( LPAD( TO_NUMBER( (SELECT MAX(SUBSTR(EAF_PROC_NO, 4, 5)) FROM TFBSM43 WHERE DEV_CODE = 'E1' AND SUBSTR(EAF_PROC_NO, 3, 1) = SUBSTR(TO_CHAR(SYSDATE, 'YYYY'), 4, 1)) ) + 1, 5, '0' ), '00001') AS E1,"
                 " 'E2' || SUBSTR(TO_CHAR(SYSDATE, 'YYYY'), 4, 1) || "
                 " NVL( LPAD( TO_NUMBER( (SELECT MAX(SUBSTR(EAF_PROC_NO, 4, 5)) FROM TFBSM43 WHERE DEV_CODE = 'E2' AND SUBSTR(EAF_PROC_NO, 3, 1) = SUBSTR(TO_CHAR(SYSDATE, 'YYYY'), 4, 1)) ) + 1, 5, '0' ), '00001') AS E2"
                 " FROM DUAL "
            ;
        cmd_inq.SetCommandText(sqlstr);
        Log::Trace("", "", "sqlstr = {0}", sqlstr);
        cmd_inq.ExecuteQuery(bcls_ret->Tables[2]);
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
