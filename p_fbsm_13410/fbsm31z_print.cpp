/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 碳钢钢种配料导出/打印
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm31z_print)

int f_fbsm31z_print(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
    CTracer log(__FUNCTION__);
    int doFlag = 0;
    int i = 0;
    CString sqlstr = " ";
    CString remark = " ";
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
        // 1. 参数解析（不变）
        if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
            compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
        if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
            st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();

        Log::Trace("", "", "compose_list_no = {0}", compose_list_no);
        Log::Trace("", "", "st_no = {0}", st_no);

        // 2. 主查询
        sqlstr = " SELECT T1.STATION_ID, T1.DATE_C, T1.ST_NO, T1.BACKLOG_EA, T1.MAT_CODE, "
                 " T1.MAT_NAME, T1.COMPOSE_LIST_NO, T1.WEIGHT, "
                 " CASE WHEN T1.STOCK_NAME NOT IN('1','2','3','4','5') THEN ("
                 "     SELECT MAT_NAME FROM TMMSM60 "
                 "     WHERE BUNKER_NO = T1.STOCK_NAME AND MAT_CODE = T1.MAT_CODE AND ROWNUM = 1"
                 " ) ELSE T1.STOCK_NAME END AS STOCK_NAME, "
                 " NVL(T5.YEILD_MINUS, 100) AS MAT_YEILD " // 收得率：从TMMSM50取，默认为100
                 " FROM TFBSM32 T1 "
                 " LEFT JOIN TMMSM50 T5 ON T1.MAT_CODE = T5.MAT_CODE " 
                 " WHERE T1.COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                 "   AND T1.ST_NO = @ST_NO "
                 " ORDER BY T1.MAT_CODE, "
                 " CASE T1.STOCK_NAME WHEN '2' THEN 1 WHEN '3' THEN 2 WHEN '1' THEN 3 "
                 " WHEN '4' THEN 4 WHEN '5' THEN 5 ELSE 6 END ";

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
