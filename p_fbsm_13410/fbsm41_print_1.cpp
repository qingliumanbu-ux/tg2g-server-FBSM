/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 电炉料篮配料导出/打印单记录公式
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm41_print_1)

int f_fbsm41_print_1(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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

        sqlstr = " SELECT T43.COMPOSE_LIST_NO, T43.ST_NO, T43.REMARK, T43.RESP, "
                 "        DECODE(T43.SHIFT_GROUP, 'A', '甲', 'B', '乙', 'C', '丙', 'D', '丁', T43.SHIFT_GROUP) AS SHIFT_GROUP, T43.DEV_CODE, T43.EAF_PROC_NO, T43.COMPOSE_LIST_NO_EAF, T43.FIRST_SAVE_TIME,T43.CHECK_DATE,"
                 "        T12.DATE_TIME, "
                 "        NVL(W.ACT_WEIGHT_SUM, 0) AS ACT_WEIGHT_SUM, "
                 "        ROUND(DECODE(NVL(W.ACT_WEIGHT_SUM, 0), 0, 0, NVL(W.C_WEIGHT, 0) / W.ACT_WEIGHT_SUM), 4) AS C_VALUE, "
                 "        ROUND(DECODE(NVL(W.ACT_WEIGHT_SUM, 0), 0, 0, NVL(W.SI_WEIGHT, 0) / W.ACT_WEIGHT_SUM), 4) AS SI_VALUE, "
                 "        ROUND(DECODE(NVL(W.ACT_WEIGHT_SUM, 0), 0, 0, NVL(W.MN_WEIGHT, 0) / W.ACT_WEIGHT_SUM), 4) AS MN_VALUE, "
                 "        ROUND(DECODE(NVL(W.ACT_WEIGHT_SUM, 0), 0, 0, NVL(W.P_WEIGHT, 0) / W.ACT_WEIGHT_SUM), 4) AS P_VALUE, "
                 "        ROUND(DECODE(NVL(W.ACT_WEIGHT_SUM, 0), 0, 0, NVL(W.S_WEIGHT, 0) / W.ACT_WEIGHT_SUM), 4) AS S_VALUE, "
                 "        ROUND(DECODE(NVL(W.ACT_WEIGHT_SUM, 0), 0, 0, NVL(W.CR_WEIGHT, 0) / W.ACT_WEIGHT_SUM), 4) AS CR_VALUE, "
                 "        ROUND(DECODE(NVL(W.ACT_WEIGHT_SUM, 0), 0, 0, NVL(W.NI_WEIGHT, 0) / W.ACT_WEIGHT_SUM), 4) AS NI_VALUE, "
                 "        ROUND(DECODE(NVL(W.ACT_WEIGHT_SUM, 0), 0, 0, NVL(W.MO_WEIGHT, 0) / W.ACT_WEIGHT_SUM), 4) AS MO_VALUE, "
                 "        ROUND(DECODE(NVL(W.ACT_WEIGHT_SUM, 0), 0, 0, NVL(W.CU_WEIGHT, 0) / W.ACT_WEIGHT_SUM), 4) AS CU_VALUE, "
                 "        ROUND(DECODE(NVL(W.ACT_WEIGHT_SUM, 0), 0, 0, NVL(W.CO_WEIGHT, 0) / W.ACT_WEIGHT_SUM), 4) AS CO_VALUE "
                 " FROM TFBSM43 T43 "
                 " LEFT JOIN TFBSM12 T12 "
                 " ON T43.COMPOSE_LIST_NO_EAF = T12.COMPOSE_LIST_NO_EAF "
                 " LEFT JOIN ( "
                 "   SELECT T41.COMPOSE_LIST_NO, T41.COMPOSE_LIST_NO_EAF, "
                 "          SUM(T41.ACT_WEIGHT) AS ACT_WEIGHT_SUM, "
                 "          SUM(T41.ACT_WEIGHT * T14.C_VALUE) AS C_WEIGHT, "
                 "          SUM(T41.ACT_WEIGHT * T14.SI_VALUE) AS SI_WEIGHT, "
                 "          SUM(T41.ACT_WEIGHT * T14.MN_VALUE) AS MN_WEIGHT, "
                 "          SUM(T41.ACT_WEIGHT * T14.P_VALUE) AS P_WEIGHT, "
                 "          SUM(T41.ACT_WEIGHT * T14.S_VALUE) AS S_WEIGHT, "
                 "          SUM(T41.ACT_WEIGHT * T14.CR_VALUE) AS CR_WEIGHT, "
                 "          SUM(T41.ACT_WEIGHT * T14.NI_VALUE) AS NI_WEIGHT, "
                 "          SUM(T41.ACT_WEIGHT * T14.MO_VALUE) AS MO_WEIGHT, "
                 "          SUM(T41.ACT_WEIGHT * T14.CU_VALUE) AS CU_WEIGHT, "
                 "          SUM(T41.ACT_WEIGHT * T14.CO_VALUE) AS CO_WEIGHT "
                 "   FROM TFBSM41 T41 "
                 "   LEFT JOIN TFBSM14 T14 "
                 "   ON T41.COMPOSE_LIST_NO = T14.COMPOSE_LIST_NO "
                 "   AND T41.MAT_CODE = T14.MAT_CODE "
                 "   AND T41.STOCK_NAME = T14.STOCK_NAME "
                 "   AND T41.LOT_NO = T14.LOT_NO "
                 "   AND T14.STATION_ID = 'E' "
                 "   GROUP BY T41.COMPOSE_LIST_NO, T41.COMPOSE_LIST_NO_EAF "
                 " ) W "
                 " ON W.COMPOSE_LIST_NO = T43.COMPOSE_LIST_NO "
                 " AND W.COMPOSE_LIST_NO_EAF = T43.COMPOSE_LIST_NO_EAF "
                 " WHERE T43.COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                 "   AND T43.COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF "
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
