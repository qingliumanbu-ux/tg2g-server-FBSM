/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 电炉料篮配料中频炉多记录公式
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm41_print_4)

int f_fbsm41_print_4(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
    CString compose_list_no_eaf = " ";
    try
    {
        // CString str1 = "";
        // bcls_rec->WriteHTML(str1);
        // Log::Trace("", __FUNCTION__, "str=[{0}]", str1);
        if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
            compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
        if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO_EAF"))
            compose_list_no_eaf = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO_EAF"].ToString().TrimOrBlank().ToUpper();

        Log::Trace("", "", "compose_list_no = {0}", compose_list_no);
        Log::Trace("", "", "compose_list_no_eaf = {0}", compose_list_no_eaf);

        sqlstr = " SELECT ROWNUM AS ROW_NO, T14.* FROM TFBSM14 T14 "
                 " WHERE 1=1 "
                 " AND T14.COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                 " AND T14.STATION_ID = 'Z' "
                 " AND T14.MAT_CODE NOT IN ('PL0001', 'PL0002') "
                 ;

        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
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
