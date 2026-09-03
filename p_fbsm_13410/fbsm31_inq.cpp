/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 碳钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm31_inq)

int f_fbsm31_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
    try
    {
        Log::Trace("", "", "datetime = {0}", datetime);
        if (bcls_rec->Tables[0].Columns.Contains("DATE_C"))
            date_c = bcls_rec->Tables[0].Rows[0]["DATE_C"].ToString().TrimOrBlank().ToUpper();
        if (bcls_rec->Tables[0].Columns.Contains("DATE_C_1"))
            date_c_1 = bcls_rec->Tables[0].Rows[0]["DATE_C_1"].ToString().TrimOrBlank().ToUpper();
        if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
            st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
        sqlstr = " SELECT max(t31.REMARK_PS) as REMARK_PS,"
                 " t31.DATE_C, "
				 " t31.ORIGIN_CODE, "
                 " t31.DATE_TIME, "
                 " sum(1) FURNACE_COUNT, "
                 " t31.ST_NO, "
                 " t31.COMPOSE_LIST_NO, "
                 " t31.BACKLOG_EA, "
                 " max(1) AS SEQ_NO, "
                 " t31.COST_DG, "
                 " t31.SINGLECOST, "
                 " t31.CHECK_FLAG, "
                 " t31.CHECK_MAKE, "
                 " t31.ERROR_REMARK, "
                 " max(t31.CHECK_DATE) AS CHECK_DATE, "
                 " t0.ST_NO_DESC "
                 " FROM tfbsm31 t31 "
                 " LEFT JOIN TQMTS0X t0 "
                 " ON t31.ST_NO = t0.ST_NO "
                 " WHERE 1 = 1 ";
        if (date_c.Trim() != "" && date_c_1.Trim() == "")
        {
            sqlstr += " and t31.DATE_TIME='" + date_c + "' ";
        }
        else if (date_c_1.Trim() != "" && date_c.Trim() == "")
        {
            sqlstr += " and t31.DATE_TIME<='" + date_c_1 + "' ";
        }
        else if (date_c_1.Trim() != "" && date_c.Trim() != "")
        {
            sqlstr += " and t31.DATE_TIME>='" + date_c + "' and t31.DATE_TIME<='" + date_c_1 + "' ";
        }
        /*else{
            sqlstr += " and A.DATE_TIME='" + datetime + "' ";
        }*/
        if (st_no.Trim() != "")
        {
            sqlstr += " and t31.ST_NO like '%" + st_no + "%' ";
        }
        sqlstr += " group by t31.DATE_C, "
                  " t31.ST_NO, "
                  " t31.COMPOSE_LIST_NO, "
                  " t31.BACKLOG_EA,"
                  " t31.COST_DG,"
                  " t31.SINGLECOST,"
                  " t31.DATE_TIME,"
                  " t31.CHECK_FLAG,"
                  " t31.CHECK_MAKE,"
                  " t0.ST_NO_DESC,  "
				  " t31.ORIGIN_CODE, "
                  " t31.ERROR_REMARK "
                  " order by "
                  " t31.DATE_C DESC, "
                  " t31.ST_NO, "
                  " t31.BACKLOG_EA,SEQ_NO,t31.COMPOSE_LIST_NO ";
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
