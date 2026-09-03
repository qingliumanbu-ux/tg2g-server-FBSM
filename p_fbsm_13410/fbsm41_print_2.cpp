/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 电炉料篮配料导出/打印多记录公式
**************************************************/
#include "stdafx.h"
#include <map>
BM2F_ENTERACE(fbsm41_print_2)

int f_fbsm41_print_2(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
    map<CString, CString> stockNameMap;
    try
    {
        // CString str1 = "";
        // bcls_rec->WriteHTML(str1);
        // Log::Trace("", __FUNCTION__, "str=[{0}]", str1);
        // 1. 参数解析（不变）
        if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
            compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
        if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO_EAF"))
            compose_list_no_eaf = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO_EAF"].ToString().TrimOrBlank().ToUpper();

        Log::Trace("", "", "compose_list_no = {0}", compose_list_no);
        Log::Trace("", "", "compose_list_no_eaf = {0}", compose_list_no_eaf);

        // 2. 主查询
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
                 " ORDER BY T41.BUNKER_SEQ,T41.LAYER_NO "
                 ;

        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
        cmd_inq.SetCommandText(sqlstr);
        Log::Trace("", "", "sqlstr = {0}", sqlstr);
        cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
        cmd_inq.Close();

        // 加载 STOCK_NAME 代码与中文对照
        sqlstr = " SELECT CODE, CODE_DESC_1_CONTENT as code_desc FROM tep0002 WHERE code_class = 'FBSM13' "
                 " UNION "
                 " SELECT BUNKER_NO as CODE, MAT_NAME as code_desc FROM tmmsm60 WHERE BUNKER_NO like 'VS%' ";
        cmd.SetCommandText(sqlstr);
        cmd.ExecuteReader();
        while (cmd.Read())
        {
            CString code = cmd.GetString(1).Trim();
            CString codeDesc = cmd.GetString(2).Trim();
            stockNameMap[code] = codeDesc;
        }
        cmd.Close();

        // BUNKER_SEQ 中文转换（1→第一篮，2→第二篮，...，10→第十篮）
        for (int r = 0; r < bcls_ret->Tables[0].Rows.get_Count(); r++)
        {
            CString stockName = bcls_ret->Tables[0].Rows[r]["STOCK_NAME"].ToString().Trim();
            if (stockNameMap.find(stockName) != stockNameMap.end())
                bcls_ret->Tables[0].Rows[r]["STOCK_NAME"] = stockNameMap[stockName];

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
