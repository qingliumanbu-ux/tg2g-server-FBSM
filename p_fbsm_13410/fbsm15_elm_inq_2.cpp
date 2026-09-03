/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料查评1�71ￄ1�77(备注信息)
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15_elm_inq_2)

int f_fbsm15_elm_inq_2(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
    CTracer log(__FUNCTION__);
    int doFlag = 0;
    int i = 1;
    CString s_id_01 = " ";
    CString s_id_02 = " ";
    CString s_id_03 = " ";
    CString sqlstr = " ";
    /* 数据库操作类定义：统丢�放在Service或函数前殄1�71ￄ1�77 */
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
        Log::Info("", __FUNCTION__, "compose_list_no  =[{0}]", compose_list_no);
        Log::Info("", __FUNCTION__, "st_no  =[{0}]", st_no);
        sqlstr = "SELECT STATION_ID, BACK "
         "FROM ( "
         "    SELECT 'IF' AS STATION_ID, BACK_IF AS BACK, 1 AS rn FROM TFBSM12 t12 "
         "    WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO AND st_no = @ST_NO "
         "      AND EXISTS ( "
         "          SELECT 1 FROM TFBSM14 "
         "          WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
         "            AND st_no = @ST_NO "
         "            AND (CASE WHEN STATION_ID = 'Y' THEN 'A' ELSE STATION_ID END) = 'Z' "
         "      ) "
         "    UNION "
         "    SELECT 'EAF' AS STATION_ID, BACK_EAF AS BACK, 2 AS rn FROM TFBSM12 t12 "
         "    WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO AND st_no = @ST_NO "
         "      AND EXISTS ( "
         "          SELECT 1 FROM TFBSM14 "
         "          WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
         "            AND st_no = @ST_NO "
         "            AND (CASE WHEN STATION_ID = 'Y' THEN 'A' ELSE STATION_ID END) = 'E' "
         "      ) "
         "    UNION "
         "    SELECT 'AOD' AS STATION_ID, BACK_AOD AS BACK, 3 AS rn FROM TFBSM12 t12 "
         "    WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO AND st_no = @ST_NO "
         "      AND EXISTS ( "
         "          SELECT 1 FROM TFBSM14 "
         "          WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
         "            AND st_no = @ST_NO "
         "            AND (CASE WHEN STATION_ID = 'Y' THEN 'A' ELSE STATION_ID END) = 'A' "
         "      ) "
         ") t "
         "ORDER BY rn ";
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
        cmd_inq.Parameters.Set("ST_NO", st_no);
        cmd_inq.SetCommandText(sqlstr);
        Log::Trace("", "", "sqlstr = {0}", sqlstr);
        cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
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
