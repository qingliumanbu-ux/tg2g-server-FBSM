/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 碳钢配料单匹配对应
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm31z_plan_map)

int f_fbsm31z_plan_map(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
    CTracer log(__FUNCTION__);
    int doFlag = 0;
    CString sqlstr = " ";
    CDbCommand cmd_check(conn); // 校验查询
    CDbCommand cmd_upd(conn);   // TFBSM31更新

    CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

    CString source_compose_list_no = ""; // 复用配料单号
    CString source_st_no = "";           // 复用配料单钢种
    CString source_date_c = "";          // 复用配料单日期
    CString source_singlecost = "";      // 源配料单单炉成本
    CString source_cost_dg = "";         // 源配料单吨钢成本

    CDecimal count31 = 0;
    CDecimal count32 = 0;
    CString check_flag = "";

    CString date_c = "";
    CString plan_st_no = "";

    try
    {
        // ==================== 1. 校验输入表数量 ====================
        if (bcls_rec->Tables.get_Count() < 2)
        {
            strcpy(s.msg, "输入参数错误：缺少计划块或配料单信息块!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }

        // ==================== 2. 校验复用配料单信息（Tables[1]） ====================
        if (bcls_rec->Tables[1].Rows.get_Count() <= 0)
        {
            strcpy(s.msg, "复用配料单信息块不能为空!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }
        source_compose_list_no = bcls_rec->Tables[1].Rows[0]["COMPOSE_LIST_NO"].ToString().Trim();
        source_st_no = bcls_rec->Tables[1].Rows[0]["ST_NO"].ToString().Trim();
        source_date_c = bcls_rec->Tables[1].Rows[0]["DATE_C"].ToString().Trim();

        if (source_compose_list_no == "" || source_st_no == "" || source_date_c == "")
        {
            strcpy(s.msg, "复用配料单信息错误：COMPOSE_LIST_NO、ST_NO、DATE_C都不能为空!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }

        // 校验TFBSM31中该配料单是否存在且已审核
        sqlstr = " SELECT CHECK_FLAG, SINGLECOST, COST_DG FROM TFBSM31 "
                 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                 " AND ST_NO = @ST_NO "
                 " AND DATE_C = @DATE_C "
                 " AND ROWNUM = 1 ";
        cmd_check.Parameters.Set("COMPOSE_LIST_NO", source_compose_list_no);
        cmd_check.Parameters.Set("ST_NO", source_st_no);
        cmd_check.Parameters.Set("DATE_C", source_date_c);
        cmd_check.SetCommandText(sqlstr);
        cmd_check.ExecuteReader();
        check_flag = "";
        source_singlecost = "";
        source_cost_dg = "";
        if (cmd_check.Read())
        {
            check_flag = cmd_check.GetString(1);
            source_singlecost = cmd_check.GetString(2);
            source_cost_dg = cmd_check.GetString(3);

            check_flag = check_flag.Trim();
            source_singlecost = source_singlecost.Trim();
            source_cost_dg = source_cost_dg.Trim();
            count31 = 1;
        }
        cmd_check.Close();
        if (count31 <= 0)
        {
            strcpy(s.msg, "该配料单在TFBSM31中不存在!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }
        if (check_flag != "1")
        {
            strcpy(s.msg, "要复用的配料单未经审核!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }

        // 校验TFBSM32中该配料单是否有物料明细（排除PL0001、PL0002统计行）
        sqlstr = " SELECT COUNT(1) FROM TFBSM32 "
                 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                 " AND MAT_CODE NOT IN ('PL0001', 'PL0002') ";
        cmd_check.Parameters.Set("COMPOSE_LIST_NO", source_compose_list_no);
        cmd_check.SetCommandText(sqlstr);
        cmd_check.ExecuteReader();
        count32 = 0;
        if (cmd_check.Read())
            count32 = cmd_check.GetDecimal(1);
        cmd_check.Close();
        if (count32 <= 0)
        {
            strcpy(s.msg, "该配料单无物料明细数据!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }

        // ==================== 3. 校验计划块（Tables[0]） ====================
        if (bcls_rec->Tables[0].Rows.get_Count() <= 0)
        {
            strcpy(s.msg, "计划块不能为空!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }
        for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
        {
            date_c = bcls_rec->Tables[0].Rows[i]["DATE_C"].ToString().Trim();
            plan_st_no = bcls_rec->Tables[0].Rows[i]["ST_NO"].ToString().Trim();

            if (date_c == "" || plan_st_no == "")
            {
                strcpy(s.msg, "计划信息错误：DATE_C、ST_NO都不能为空!!!");
                throw CApplicationException(-1, s.msg, log.Location);
            }

            // 查询TFBSM31中是否存在该计划，且未经审核
            sqlstr = " SELECT CHECK_FLAG FROM TFBSM31 "
                     " WHERE DATE_C = @DATE_C "
                     " AND ST_NO = @ST_NO "
                     " AND ROWNUM = 1 ";
            cmd_check.Parameters.Set("DATE_C", date_c);
            cmd_check.Parameters.Set("ST_NO", plan_st_no);
            cmd_check.SetCommandText(sqlstr);
            cmd_check.ExecuteReader();
            CString plan_check_flag = "";
            if (cmd_check.Read())
            {
                plan_check_flag = cmd_check.GetString(1);
                plan_check_flag = plan_check_flag.Trim();
            }
            else
            {
                strcpy(s.msg, "未找到该计划：ST_NO=[" + plan_st_no + "], DATE_C=[" + date_c + "]!!!");
                throw CApplicationException(-1, s.msg, log.Location);
            }
            cmd_check.Close();
            if (plan_check_flag == "1")
            {
                strcpy(s.msg, "该计划已审核，不能复用：ST_NO=[" + plan_st_no + "], DATE_C=[" + date_c + "]!!!");
                throw CApplicationException(-1, s.msg, log.Location);
            }
        }

        // ==================== 4. 循环处理每个计划 ====================
        for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
        {
            date_c = bcls_rec->Tables[0].Rows[i]["DATE_C"].ToString().Trim();
            plan_st_no = bcls_rec->Tables[0].Rows[i]["ST_NO"].ToString().Trim();

            // 更新TFBSM31，按ST_NO+DATE_C更新该组所有炉次
            sqlstr = " UPDATE TFBSM31 "
                     " SET COMPOSE_LIST_NO = @COMPOSE_LIST_NO, "
                     "     SINGLECOST = @SINGLECOST, "
                     "     COST_DG = @COST_DG, "
                     "     REC_REVISOR = @REC_REVISOR, "
                     "     REC_REVISE_TIME = @REC_REVISE_TIME "
                     " WHERE DATE_C = @DATE_C "
                     " AND ST_NO = @ST_NO ";
            cmd_upd.Parameters.Set("COMPOSE_LIST_NO", source_compose_list_no);
            cmd_upd.Parameters.Set("SINGLECOST", source_singlecost);
            cmd_upd.Parameters.Set("COST_DG", source_cost_dg);
            cmd_upd.Parameters.Set("REC_REVISOR", s.userid);
            cmd_upd.Parameters.Set("REC_REVISE_TIME", datetime);
            cmd_upd.Parameters.Set("DATE_C", date_c);
            cmd_upd.Parameters.Set("ST_NO", plan_st_no);
            cmd_upd.SetCommandText(sqlstr);
            cmd_upd.ExecuteNonQuery();
            cmd_upd.Close();

            Log::Trace("", __FUNCTION__, "更新TFBSM31：DATE_C=[{0}]，ST_NO=[{1}]，COMPOSE_LIST_NO=[{2}]",
                       date_c, plan_st_no, source_compose_list_no);
        }

        Log::Trace("", __FUNCTION__, "计划复用完成，共处理[{0}]个计划", bcls_rec->Tables[0].Rows.get_Count());
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
        strncpy(s.msg, ex.GetMsg(), sizeof(s.msg) - 1);
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
