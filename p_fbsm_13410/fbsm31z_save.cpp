/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2026/3/5
Description: 配料单的信息新增
配料单生成规则：钢种第二位+六位日期+三位流水
**************************************************/
#include "stdafx.h"

// 外部函数声明
int f_mmsm_getprice(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);

// Service 入口
BM2F_ENTERACE(fbsm31z_save)
int f_fbsm31z_save(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
    CTracer log(__FUNCTION__);

    int doFlag = 0;
    CString sqlstr = " ";
    CString dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
    CModel tfbsm32("TFBSM32");
    CModel tfbsm31("TFBSM31");
    CModel tfbsm14a("TFBSM14A");

    CString new_compose_list_no = " ";

    CDecimal unit_price = 0;
    CDecimal unit_cost = 0;
    CDecimal out_wt = 0;
    CDecimal total_weight = 0;
    EIClass bcls_rec_rep;
    EIClass bcls_ret_rep;

    CDbCommand cmd_inq(conn);
    CDbCommand cmd_inq_1(conn);
    CDbCommand cmd_inq_2(conn);
    try
    {
        // ==================== 1. 读取query表基础信息 ====================
        tfbsm31.MergeFrom(bcls_rec->Tables["query"].Rows[0]);
        CString st_no = tfbsm31["ST_NO"].ToString().Trim();
        CString date_c = tfbsm31["DATE_C"].ToString().Trim();
        // COMPOSE_LIST_NO保留原值（空格也是一种有效值），仅用于判断场景时trim
        CString compose_list_no = tfbsm31["COMPOSE_LIST_NO"].ToString();

        if (compose_list_no.Trim() == "")
        {
            compose_list_no = " ";
        }

        bool is_compose_list_no_full = (compose_list_no.Trim() == "");

        if (st_no == "" || date_c == "")
        {
            strcpy(s.msg, "参数错误：ST_NO和DATE_C不能为空!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }

        // ==================== 2. 校验计划组是否存在且未经审核 ====================
        sqlstr = " SELECT COUNT(1) FROM TFBSM31 "
                 " WHERE ST_NO=@ST_NO "
                 " AND DATE_C=@DATE_C "
                 " AND COMPOSE_LIST_NO=@COMPOSE_LIST_NO "
                 " AND CHECK_FLAG='1' ";
        cmd_inq.SetCommandText(sqlstr);
        cmd_inq.Parameters.Set("ST_NO", st_no);
        cmd_inq.Parameters.Set("DATE_C", date_c);
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
        cmd_inq.ExecuteReader();
        CDecimal checked_count = 0;
        if (cmd_inq.Read())
            checked_count = cmd_inq.GetDecimal(1);
        cmd_inq.Close();
        if (checked_count > 0)
        {
            strcpy(s.msg, "该配料单已经审核，不能修改保存。");
            throw CApplicationException(-1, s.msg, log.Location);
        }

        // 确认计划组存在
        sqlstr = " SELECT COUNT(1) FROM TFBSM31 "
                 " WHERE ST_NO=@ST_NO "
                 " AND DATE_C=@DATE_C "
                 " AND COMPOSE_LIST_NO=@COMPOSE_LIST_NO ";
        cmd_inq.SetCommandText(sqlstr);
        cmd_inq.Parameters.Set("ST_NO", st_no);
        cmd_inq.Parameters.Set("DATE_C", date_c);
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
        cmd_inq.ExecuteReader();
        CDecimal plan_count = 0;
        if (cmd_inq.Read())
            plan_count = cmd_inq.GetDecimal(1);
        cmd_inq.Close();
        if (plan_count <= 0)
        {
            strcpy(s.msg, "未找到该计划组：ST_NO=[" + st_no + "], DATE_C=[" + date_c + "], COMPOSE_LIST_NO=[" + compose_list_no + "]!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }

        // ==================== 3. 确定最终配料单号 ====================
        bool need_update_31 = false; // 是否需要更新TFBSM31的COMPOSE_LIST_NO
        bool need_insert_14a = false; // 是否需要插入TFBSM14A
        CString origin_code = " ";

        if (!is_compose_list_no_full)
        {
            // 普通场景：先判断是否为人工配料单
            sqlstr = " SELECT ORIGIN_CODE FROM TFBSM14A "
                     " WHERE COMPOSE_LIST_NO=@COMPOSE_LIST_NO ";
            cmd_inq.SetCommandText(sqlstr);
            cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
            cmd_inq.ExecuteReader();
            if (cmd_inq.Read())
            {
                origin_code = cmd_inq.GetString(1);
                origin_code = origin_code.Trim();
            }
            cmd_inq.Close();
        }

        if (is_compose_list_no_full || origin_code != "1")
        {
            // 导入保存 或 非人工配料单：生成新单号
            sqlstr = " SELECT MAX(TO_NUMBER(SUBSTR(COMPOSE_LIST_NO, 8, 3))) "
                     " FROM TFBSM32 "
                     " WHERE DATE_C=@DATE_C ";
            cmd_inq_1.SetCommandText(sqlstr);
            cmd_inq_1.Parameters.Set("DATE_C", date_c);
            cmd_inq_1.ExecuteReader();
            CDecimal max_seq = 0;
            if (cmd_inq_1.Read())
                max_seq = cmd_inq_1.GetDecimal(1);
            cmd_inq_1.Close();

            CDecimal next_seq = max_seq + 1;
            if (next_seq > 999)
                next_seq = 1;

            CString seq_str;
            if (next_seq < 10)
                seq_str = "00" + next_seq.ToString();
            else if (next_seq < 100)
                seq_str = "0" + next_seq.ToString();
            else
                seq_str = next_seq.ToString();

            CString st_no_second = st_no.SubstringNE(1, 1);
            new_compose_list_no = st_no_second + date_c + seq_str;

            need_update_31 = true;
            need_insert_14a = true;
        }
        else
        {
            // 人工配料单：单号不变
            new_compose_list_no = compose_list_no;
        }

        Log::Trace("", __FUNCTION__, "最终配料单号：COMPOSE_LIST_NO=[{0}]，是否导入保存=[{1}]，是否更新31=[{2}]",
                   new_compose_list_no, is_compose_list_no_full ? "是" : "否", need_update_31 ? "是" : "否");

        // ==================== 4. 更新TFBSM31的COMPOSE_LIST_NO和ORIGIN_CODE ====================
        if (need_update_31)
        {
            sqlstr = " UPDATE TFBSM31 "
                     " SET COMPOSE_LIST_NO=@NEW_COMPOSE_LIST_NO, ORIGIN_CODE='1' "
                     " WHERE ST_NO=@ST_NO "
                     " AND DATE_C=@DATE_C "
                     " AND COMPOSE_LIST_NO=@OLD_COMPOSE_LIST_NO ";
            cmd_inq.SetCommandText(sqlstr);
            cmd_inq.Parameters.Set("NEW_COMPOSE_LIST_NO", new_compose_list_no);
            cmd_inq.Parameters.Set("ST_NO", st_no);
            cmd_inq.Parameters.Set("DATE_C", date_c);
            cmd_inq.Parameters.Set("OLD_COMPOSE_LIST_NO", compose_list_no);
            cmd_inq.ExecuteNonQuery();
            cmd_inq.Close();
        }

        // ==================== 5. 插入TFBSM14A归档 ====================
        if (need_insert_14a)
        {
            tfbsm14a.MergeFrom(bcls_rec->Tables["query"].Rows[0]);
            tfbsm14a["COMPOSE_LIST_NO"] = new_compose_list_no;
            tfbsm14a["ORIGIN_CODE"] = "1";
            tfbsm14a["DATE_C"] = date_c;
            tfbsm14a["REC_CREATOR"] = s.userid;
            tfbsm14a["REC_CREATE_TIME"] = dateNow;
            tfbsm14a.TrimOrBlank();
            tfbsm14a.Insert();
        }

        // ==================== 6. 保存TFBSM32物料明细 ====================
        tfbsm32["ORIGIN_CODE"] = "1";
        tfbsm32["COMPOSE_LIST_NO"] = new_compose_list_no;
        tfbsm32["ST_NO"] = st_no;
        tfbsm32.Delete("COMPOSE_LIST_NO,ST_NO,ORIGIN_CODE");

        if (bcls_rec->Tables.Contains("BOF")) // 消耗维护
        {
            for (int i = 0; i < bcls_rec->Tables["BOF"].Rows.get_Count(); i++)
            {
                tfbsm32.Reset();
                tfbsm32.MergeFrom(bcls_rec->Tables["BOF"].Rows[i]);
                tfbsm32["ORIGIN_CODE"] = "1";
                tfbsm32["COMPOSE_LIST_NO"] = new_compose_list_no;
                tfbsm32["ST_NO"] = st_no;
                tfbsm32["STATION_ID"] = "B";
                tfbsm32["DATE_C"] = date_c;
                tfbsm32["REC_CREATOR"] = s.userid;
                tfbsm32["REC_CREATE_TIME"] = dateNow;
                tfbsm32.TrimOrBlank();

                CString mat_name = tfbsm32["MAT_NAME"].ToString();
                CString mat_code = tfbsm32["MAT_CODE"].ToString().Trim();
                CDecimal mat_weight = tfbsm32["WEIGHT"].ToDecimal();

                if (mat_name == "配料重量")
                {
                    tfbsm32["MAT_CODE"] = "PL0001";
                    total_weight = mat_weight;
                }
                if (mat_name == "出钢重量")
                {
                    tfbsm32["MAT_CODE"] = "PL0002";
                    out_wt = mat_weight;
                }

                Log::Trace("", __FUNCTION__, "BOF mat_code=[{0}], ST_NO=[{1}], MAT_NAME=[{2}], WEIGHT=[{3}]",
                           tfbsm32["MAT_CODE"].ToString(), tfbsm32["ST_NO"].ToString(), mat_name, mat_weight);

                if (mat_name == "备注")
                {
                    sqlstr = " UPDATE TFBSM31 SET BACK_BOF=@BEIZHU "
                             " WHERE COMPOSE_LIST_NO=@COMPOSE_LIST_NO AND ST_NO=@ST_NO ";
                    cmd_inq.SetCommandText(sqlstr);
                    cmd_inq.Parameters.Set("BEIZHU", tfbsm32["STOCK_NAME"].ToString());
                    cmd_inq.Parameters.Set("COMPOSE_LIST_NO", new_compose_list_no);
                    cmd_inq.Parameters.Set("ST_NO", st_no);
                    cmd_inq.ExecuteNonQuery();
                    cmd_inq.Close();

                    sqlstr = " UPDATE TFBSM14A SET BACK_BOF=@BEIZHU "
                             " WHERE COMPOSE_LIST_NO=@COMPOSE_LIST_NO AND ST_NO=@ST_NO ";
                    cmd_inq.SetCommandText(sqlstr);
                    cmd_inq.Parameters.Set("BEIZHU", tfbsm32["STOCK_NAME"].ToString());
                    cmd_inq.Parameters.Set("COMPOSE_LIST_NO", new_compose_list_no);
                    cmd_inq.Parameters.Set("ST_NO", st_no);
                    cmd_inq.ExecuteNonQuery();
                    cmd_inq.Close();
                }
                else if (mat_name != "内控上限" && mat_name != "内控下限" && mat_name != "内控目标")
                {
                    if (mat_name != "配料重量" && mat_name != "出钢重量")
                    {
                        CString back_c2 = bcls_rec->Tables["BOF"].Rows[i]["BACK_C2"].ToString().Trim();
                        CString stock_name = bcls_rec->Tables["BOF"].Rows[i]["STOCK_NAME"].ToString().Trim();

                        sqlstr = " SELECT NVL(Cr, 0) AS CR_VALUE, NVL(Ni, 0) AS NI_VALUE, NVL(Mo, 0) AS MO_VALUE "
                                 " FROM FBSM_13_AI "
                                 " WHERE MAT_CODE=@MAT_CODE "
                                 " AND STOCK_NAME=@STOCK_NAME "
                                 " AND ROWNUM=1 ";
                        cmd_inq_2.SetCommandText(sqlstr);
                        cmd_inq_2.Parameters.Set("MAT_CODE", mat_code);
                        cmd_inq_2.Parameters.Set("STOCK_NAME", stock_name);
                        cmd_inq_2.ExecuteReader();
                        CString cr_value = "0";
                        CString ni_value = "0";
                        CString mo_value = "0";
                        if (cmd_inq_2.Read())
                        {
                            cr_value = cmd_inq_2.GetDecimal(1).ToString();
                            ni_value = cmd_inq_2.GetDecimal(2).ToString();
                            mo_value = cmd_inq_2.GetDecimal(3).ToString();
                        }
                        cmd_inq_2.Close();

                        bcls_rec_rep.Tables.Clear();
                        bcls_ret_rep.Tables.Clear();
                        bcls_rec_rep.Tables.Add();
                        bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_CODE");
                        bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_NAME");
                        bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "CR_VALUE");
                        bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "NI_VALUE");
                        bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MO_VALUE");
                        bcls_rec_rep.Tables[0].Rows.Add();
                        bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = mat_code;
                        bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = mat_name;
                        bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = cr_value;
                        bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = ni_value;
                        bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = mo_value;
                        bcls_ret_rep.Tables.Add();

                        int price_ret = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
                        if (price_ret < 0 || bcls_ret_rep.Tables[0].Rows.get_Count() <= 0)
                        {
                            unit_price = 0;
                            Log::Trace("", __FUNCTION__, "price service failed or no result, mat_code=[{0}], fallback to 0", mat_code);
                        }
                        else
                        {
                            unit_price = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"].ToDecimal();
                        }

                        unit_cost = unit_cost + unit_price * mat_weight;
                        tfbsm32["COST"] = unit_price;
                    }

                    tfbsm32.Insert();
                }
            }
        }

        // ==================== 7. 更新成本 TFBSM31 / TFBSM14A ====================
        CDecimal singlecost = unit_cost.Round(2);
        CDecimal cost_dg = 0;
        if (total_weight != 0)
        {
            cost_dg = (unit_cost / total_weight).Round(2);
        }

        sqlstr = " UPDATE TFBSM31 "
                 " SET SINGLECOST=@SINGLECOST, COST_DG=@COST_DG, REC_REVISOR=@REC_REVISOR, REC_REVISE_TIME=@REC_REVISE_TIME "
                 " WHERE ST_NO=@ST_NO AND DATE_C=@DATE_C AND COMPOSE_LIST_NO=@COMPOSE_LIST_NO ";
        cmd_inq.SetCommandText(sqlstr);
        cmd_inq.Parameters.Set("SINGLECOST", singlecost);
        cmd_inq.Parameters.Set("COST_DG", cost_dg);
        cmd_inq.Parameters.Set("REC_REVISOR", s.userid);
        cmd_inq.Parameters.Set("REC_REVISE_TIME", dateNow);
        cmd_inq.Parameters.Set("ST_NO", st_no);
        cmd_inq.Parameters.Set("DATE_C", date_c);
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", new_compose_list_no);
        cmd_inq.ExecuteNonQuery();
        cmd_inq.Close();

        sqlstr = " UPDATE TFBSM14A "
                 " SET SINGLECOST=@SINGLECOST, COST_DG=@COST_DG "
                 " WHERE COMPOSE_LIST_NO=@COMPOSE_LIST_NO AND ST_NO=@ST_NO ";
        cmd_inq.SetCommandText(sqlstr);
        cmd_inq.Parameters.Set("SINGLECOST", singlecost);
        cmd_inq.Parameters.Set("COST_DG", cost_dg);
        cmd_inq.Parameters.Set("COMPOSE_LIST_NO", new_compose_list_no);
        cmd_inq.Parameters.Set("ST_NO", st_no);
        cmd_inq.ExecuteNonQuery();
        cmd_inq.Close();

        Log::Trace("", __FUNCTION__, "cost updated: SINGLECOST=[{0}], COST_DG=[{1}], total_weight=[{2}], out_wt=[{3}]",
                   singlecost, cost_dg, total_weight, out_wt);

        // ==================== 8. 返回配料单号 ====================
        bcls_ret->Tables[0].Columns.Add(DT_STRING, "COMPOSE_LIST_NO");
        bcls_ret->Tables[0].Rows.Add();
        bcls_ret->Tables[0].Rows[0]["COMPOSE_LIST_NO"] = new_compose_list_no;
    }
    catch (CDbException &ex)
    {
        CFormattable arguments[] = {ex.GetCode()};
        CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。", arguments, 1);
        CString str = sqlstr + "\r\n" + ex.GetMsg();
        strncpy(s.sysmsg, (const char *)str, sizeof(s.sysmsg) - 1);
        s.flag = -1;
        doFlag = -1;
    }
    catch (CApplicationException &ex)
    {
        strncpy(s.msg, (const char *)ex.GetMsg(), sizeof(s.msg) - 1);
        s.flag = ex.GetCode();
        doFlag = -1;
    }
    catch (CException &ex)
    {
        strncpy(s.sysmsg, (const char *)ex.GetMsg(), sizeof(s.sysmsg) - 1);
        s.flag = ex.GetCode();
        doFlag = -1;
    }
    return doFlag;
}

