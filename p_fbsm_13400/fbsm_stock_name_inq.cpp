/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: ¿âÇøÃû³Æ²éÑ¯
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm_stock_name_inq)

int f_fbsm_stock_name_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
    CTracer log(__FUNCTION__);
    int doFlag = 0;
    CString sqlstr = " ";
    CDbCommand cmd_inq(conn);

    try
    {
        sqlstr = " SELECT CODE, CODE_DESC_1_CONTENT as code_desc FROM tep0002 WHERE code_class = 'FBSM13' "
                 " UNION "
                 " SELECT BUNKER_NO as CODE, MAT_NAME as code_desc FROM tmmsm60 WHERE BUNKER_NO like 'VS%' "
                 ;
        cmd_inq.SetCommandText(sqlstr);
        Log::Trace("", __FUNCTION__, "sqlstr = {0}", sqlstr);
        cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
        cmd_inq.Close();
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
