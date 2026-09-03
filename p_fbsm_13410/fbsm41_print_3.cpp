/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 电炉料篮配料导出/打印，篮记录公式
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm41_print_3)

int f_fbsm41_print_3(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
    CString eaf_heat_no = " ";
    try
    {
        // CString str1 = "";
        // bcls_rec->WriteHTML(str1);
        // Log::Trace("", __FUNCTION__, "str=[{0}]", str1);
        // 1. 参数解析（不变）
        if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
            compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
        if (bcls_rec->Tables[0].Columns.Contains("EAF_HEAT_NO"))
            eaf_heat_no = bcls_rec->Tables[0].Rows[0]["EAF_HEAT_NO"].ToString().TrimOrBlank().ToUpper();

        Log::Trace("", "", "compose_list_no = {0}", compose_list_no);
        Log::Trace("", "", "eaf_heat_no = {0}", eaf_heat_no);

        // 2. 主查询
        sqlstr = " SELECT DISTINCT T41.BUNKER_SEQ,T41.BUNKER_NO FROM TFBSM41 T41 "
                 " WHERE 1=1 "
                 " AND T41.COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                 " AND T41.EAF_HEAT_NO = @EAF_HEAT_NO "
                 " ORDER BY T41.BUNKER_SEQ "
                 ;

        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
        cmd_inq.Parameters.Set("EAF_HEAT_NO", eaf_heat_no);
        cmd_inq.SetCommandText(sqlstr);
        Log::Trace("", "", "sqlstr = {0}", sqlstr);
        cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
        cmd_inq.Close();


        // BUNKER_SEQ 中文转换（1→第一篮，2→第二篮，...，10→第十篮）
        for (int r = 0; r < bcls_ret->Tables[0].Rows.get_Count(); r++)
        {
            CString bunkerSeq = bcls_ret->Tables[0].Rows[r]["BUNKER_SEQ"].ToString().Trim();
            if (bunkerSeq == "1")      bcls_ret->Tables[0].Rows[r]["BUNKER_SEQ"] = "第一篮";
            else if (bunkerSeq == "2") bcls_ret->Tables[0].Rows[r]["BUNKER_SEQ"] = "第二篮";
            else if (bunkerSeq == "3") bcls_ret->Tables[0].Rows[r]["BUNKER_SEQ"] = "第三篮";
            else if (bunkerSeq == "4") bcls_ret->Tables[0].Rows[r]["BUNKER_SEQ"] = "第四篮";
            else if (bunkerSeq == "5") bcls_ret->Tables[0].Rows[r]["BUNKER_SEQ"] = "第五篮";
            else if (bunkerSeq == "6") bcls_ret->Tables[0].Rows[r]["BUNKER_SEQ"] = "第六篮";
            else if (bunkerSeq == "7") bcls_ret->Tables[0].Rows[r]["BUNKER_SEQ"] = "第七篮";
            else if (bunkerSeq == "8") bcls_ret->Tables[0].Rows[r]["BUNKER_SEQ"] = "第八篮";
            else if (bunkerSeq == "9") bcls_ret->Tables[0].Rows[r]["BUNKER_SEQ"] = "第九篮";
            else if (bunkerSeq == "10") bcls_ret->Tables[0].Rows[r]["BUNKER_SEQ"] = "第十篮";
        }


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
