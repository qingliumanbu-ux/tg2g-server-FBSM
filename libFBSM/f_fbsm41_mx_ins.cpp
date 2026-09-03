/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2026
Author:      XXX
Version:     1.0
Date:        2026/04/02
Description: 电炉料篮模型调用程序
**************************************************/
#include "stdafx.h"
#include <map>
#include <vector>
#include <set>

BM2_FUNCTION_EXPORT

/* ***** 外部函数声明 ***** */
// 模型计算
void f_epex_call_rest_svc(CDbConnection *conn, const CString &system_code, const CString &svc_name, EIClass *blks_in, EIClass *blks_out);

int f_fbsm41_mx_ins(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
    CTracer log(__FUNCTION__);
    int doFlag = 0;

    // ==================== 1. 变量声明区 ====================
    EIClass inDMST02;  // 模型传入块
    EIClass outDMST02; // 模型传出块

    CString sqlstr = " ";      // 储存访问数据库的SQL的字符串
    CString compose_list_no_eaf = " "; // 电炉流水号

    CString compose_list_no = " ";   // 当前配料单号
    CString st_no = " ";             // 出钢记号
    CString comm_fmly_code = " ";         // 大类代码
    CString st_no_second = "";       // ST_NO第二位字符
    CString query_type = "";         // 钢类型（铬钢/镍钢）
    CString seq_str = " ";           // COMPOSE_LIST_NO_EAF流水号
    CString basket_status_msg = "";  // BASKET_STATUS报错信息
    CString basket_is_success = "";  // BASKET_STATUS成功标记

    CDecimal bunker_weight_max = 0;       // 单篮最大重量
    CDecimal bunker_num = 0;              // 篮数
    CDecimal furnace_count = 0;             // 总炉数
    CDecimal input_bunker_weight_max = 0; // 前台传入单篮最大重量
    CDecimal input_bunker_num = 0;        // 前台传入篮数

    int checked_furnace_count = 0;   // 已审核炉数
    int unchecked_furnace_count = 0; // 未审核炉数
    int max_eaf_seq = 0;             // 当前最大COMPOSE_LIST_NO_EAF序号

    CDbCommand cmd_inq(conn);   // 环境参数查询
    CDbCommand cmd_mat(conn);   // MatTable构建
    CDbCommand cmd_range(conn); // CHARGE_RANGE构建
    CDbCommand cmd_check(conn); // 已审核炉数查询
    CDbCommand cmd_seq(conn);   // 最大流水号/未审核SEQ_NO查询
    CDbCommand cmd_del(conn);   // 删除未审核数据
    CDbCommand cmd_upd(conn);   // 更新TFBSM12的COMPOSE_LIST_NO_EAF

    CModel tfbsm41("TFBSM41");
    CModel tfbsm43("TFBSM43");

    CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

    try
    {
        // ==================== 2. 初始化输入Block结构 ====================

        // 【Block 1: MatTable】装入配料单物料信息
        inDMST02.Tables.Add("MatTable");
        inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "COMPOSE_LIST_NO");
        inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MAT_NAME");
        inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MAT_CODE");
        inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "LOT_NO");
        inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "STOCK_NAME");
        inDMST02.Tables["MatTable"].Columns.Add(DT_DECIMAL, "WEIGHT");
        inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "BACK_C2");

        // 【Block 2: DLLINFO】模型运行所需必要参数
        inDMST02.Tables.Add("DLLINFO");
        inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "DLLNAME");
        inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "CLASSNAME");
        inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "METHODNAME");
        inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "ENVIRONMENT");
        CDataRow &DLLINFOrow = inDMST02.Tables["DLLINFO"].Rows.Add();
        DLLINFOrow["DLLNAME"] = "TGLLModel.dll";
        DLLINFOrow["CLASSNAME"] = "TGLLModel.BasketSplitter";
        DLLINFOrow["METHODNAME"] = "RunFromDataSet";

        sqlstr = " SELECT CODE FROM TEP0002 WHERE CODE_CLASS = 'FBSM23' ";
        cmd_inq.SetCommandText(sqlstr);
        Log::Trace("", "", "sql_ENVIRONMENT = {0}", sqlstr);
        cmd_inq.ExecuteReader();
        if (cmd_inq.Read())
        {
            DLLINFOrow["ENVIRONMENT"] = cmd_inq.GetString(1);
        }
        cmd_inq.Close();

        // 【Block 3: CHARGE_RANGE】装入每个配料单的料篮和重量配置信息
        inDMST02.Tables.Add("CHARGE_RANGE");
        inDMST02.Tables["CHARGE_RANGE"].Columns.Add(DT_STRING, "COMPOSE_LIST_NO");
        inDMST02.Tables["CHARGE_RANGE"].Columns.Add(DT_DECIMAL, "BUNKER_WEIGHT_MAX");
        inDMST02.Tables["CHARGE_RANGE"].Columns.Add(DT_DECIMAL, "BUNKER_NUM");
        inDMST02.Tables["CHARGE_RANGE"].Columns.Add(DT_DECIMAL, "FURNACE_COUNT");

        // ==================== 3. 获取前台传入的去重配料单号列表 ====================
        // 从 Tables[0] 获取所有 COMPOSE_LIST_NO，并去重
        set<CString> composeSet;
        map<CString, CString> composeToStNoMap;     // COMPOSE_LIST_NO -> ST_NO
        map<CString, CString> composeToCommFmlyMap; // COMPOSE_LIST_NO -> COMM_FMLY_CODE
        map<CString, int> composeToFurnaceCountMap; // COMPOSE_LIST_NO -> FURNACE_COUNT（总炉数）
        map<CString, int> composeToCheckedCountMap; // COMPOSE_LIST_NO -> 已审核炉数

        for (size_t i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
        {
            compose_list_no = bcls_rec->Tables[0].Rows[i]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
            if (compose_list_no == "")
                continue;

            if (composeSet.find(compose_list_no) == composeSet.end())
            {
                composeSet.insert(compose_list_no);
                composeToStNoMap[compose_list_no] = bcls_rec->Tables[0].Rows[i]["ST_NO"].ToString().TrimOrBlank().ToUpper();
                composeToCommFmlyMap[compose_list_no] = bcls_rec->Tables[0].Rows[i]["COMM_FMLY_CODE"].ToString().TrimOrBlank().ToUpper();
                composeToFurnaceCountMap[compose_list_no] = bcls_rec->Tables[0].Rows[i]["FURNACE_COUNT"].ToDecimal().ToInt32();
            }
        }

        if (composeSet.empty())
        {
            strcpy(s.msg, "前台传入的配料单号为空，无法处理!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }

        // ==================== 4. 构建模型传入块MatTable ====================
        // 对每个唯一的 COMPOSE_LIST_NO，查询物料信息并压入MatTable
        for (set<CString>::iterator it = composeSet.begin(); it != composeSet.end(); ++it)
        {
            compose_list_no = *it;

            sqlstr = " SELECT DISTINCT * FROM ( "
                     " SELECT T14.COMPOSE_LIST_NO, T14.MAT_NAME, T14.MAT_CODE, T14.LOT_NO, T14.STOCK_NAME, T14.WEIGHT, T14.BACK_C2 FROM TFBSM14 T14 "
                     " LEFT JOIN TFBSM12 T12 ON T14.COMPOSE_LIST_NO = T12.COMPOSE_LIST_NO "
                     " WHERE 1=1 "
                     " AND T12.CHECK_FLAG = '1' "
                     " AND T14.STATION_ID = 'E' "
                     " AND T14.MAT_CODE NOT LIKE 'PL%' "
                     " AND T14.COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                     " ) ";

            cmd_mat.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
            cmd_mat.SetCommandText(sqlstr);
            Log::Trace("", "", "查询MatTable SQL = {0}", sqlstr);
            cmd_mat.ExecuteReader();

            while (cmd_mat.Read())
            {
                CDataRow &row = inDMST02.Tables["MatTable"].Rows.Add();
                row["COMPOSE_LIST_NO"] = compose_list_no;
                row["MAT_NAME"] = cmd_mat.GetString(2);
                row["MAT_CODE"] = cmd_mat.GetString(3);
                row["LOT_NO"] = cmd_mat.GetString(4);
                row["STOCK_NAME"] = cmd_mat.GetString(5);
                row["WEIGHT"] = cmd_mat.GetDecimal(6);
                row["BACK_C2"] = cmd_mat.GetString(7);
            }
            cmd_mat.Close();
        }

        // 校验MatTable行数
        if (inDMST02.Tables["MatTable"].Rows.get_Count() <= 0)
        {
            strcpy(s.msg, "MatTable构建失败，未查询到任何物料数据!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }
        Log::Trace("", __FUNCTION__, "MatTable构建完成，共[{0}]行", inDMST02.Tables["MatTable"].Rows.get_Count());

        // ==================== 5. 构建模型传入块CHARGE_RANGE ====================
        // 获取前台传入的BUNKER_WEIGHT_MAX和BUNKER_NUM（Tables[1]）
        input_bunker_weight_max = 0;
        input_bunker_num = 0;
        if (bcls_rec->Tables.get_Count() > 1)
        {
            if (bcls_rec->Tables[1].Columns.Contains("BUNKER_WEIGHT_MAX"))
                input_bunker_weight_max = bcls_rec->Tables[1].Rows[0]["BUNKER_WEIGHT_MAX"].ToDecimal();
            if (bcls_rec->Tables[1].Columns.Contains("BUNKER_NUM"))
                input_bunker_num = bcls_rec->Tables[1].Rows[0]["BUNKER_NUM"].ToDecimal();
        }

        for (set<CString>::iterator it = composeSet.begin(); it != composeSet.end(); ++it)
        {
            compose_list_no = *it;
            st_no = composeToStNoMap[compose_list_no];
            comm_fmly_code = composeToCommFmlyMap[compose_list_no];
            furnace_count = composeToFurnaceCountMap[compose_list_no];

            // 查询已审核炉数
            checked_furnace_count = 0;
            sqlstr = " SELECT COUNT(1) FROM TFBSM43 WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO AND CHECK_FLAG = '1' ";
            cmd_check.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
            cmd_check.SetCommandText(sqlstr);
            Log::Trace("", "", "查询已审核炉数SQL = {0}", sqlstr);
            cmd_check.ExecuteReader();
            if (cmd_check.Read())
            {
                checked_furnace_count = cmd_check.GetDecimal(1).ToInt32();
            }
            cmd_check.Close();
            composeToCheckedCountMap[compose_list_no] = checked_furnace_count;

            // 未审核炉数 = 总炉数 - 已审核炉数
            unchecked_furnace_count = furnace_count.ToInt32() - checked_furnace_count;
            if (unchecked_furnace_count < 0)
                unchecked_furnace_count = 0;

            bunker_weight_max = input_bunker_weight_max;
            bunker_num = input_bunker_num;

            // 如果前台传入为空，则查TFBSM42
            if (bunker_weight_max == 0 || bunker_num == 0)
            {
                st_no_second = "";
                if (st_no.GetLength() >= 2)
                    st_no_second = st_no.SubstringNE(1, 1);

                query_type = "镍钢"; // 默认
                if ((st_no_second == "F" || st_no_second == "M") && comm_fmly_code != "V")
                {
                    query_type = "铬钢";
                }

                sqlstr = "SELECT BUNKER_NUM, BUNKER_WEIGHT_MAX FROM TFBSM42 WHERE ST_GRD_DIS_BEF = @TYPE";
                cmd_range.Parameters.Set("TYPE", query_type);
                cmd_range.SetCommandText(sqlstr);
                cmd_range.ExecuteReader();

                if (cmd_range.Read())
                {
                    if (bunker_num == 0)
                        bunker_num = cmd_range.GetDecimal(1);
                    if (bunker_weight_max == 0)
                        bunker_weight_max = cmd_range.GetDecimal(2);
                }
                cmd_range.Close();
            }

            CDataRow &row = inDMST02.Tables["CHARGE_RANGE"].Rows.Add();
            row["COMPOSE_LIST_NO"] = compose_list_no;
            row["BUNKER_WEIGHT_MAX"] = bunker_weight_max;
            row["BUNKER_NUM"] = bunker_num;
            row["FURNACE_COUNT"] = unchecked_furnace_count;
        }

        // 校验CHARGE_RANGE行数
        if (inDMST02.Tables["CHARGE_RANGE"].Rows.get_Count() <= 0)
        {
            strcpy(s.msg, "CHARGE_RANGE构建失败，未生成任何配置数据!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }
        Log::Trace("", __FUNCTION__, "CHARGE_RANGE构建完成，共[{0}]行", inDMST02.Tables["CHARGE_RANGE"].Rows.get_Count());

        // ==================== 6. 调用模型 ====================
        Log::Trace("", __FUNCTION__, "调用模型：MatTable共[{0}]行，CHARGE_RANGE共[{1}]行",
                   inDMST02.Tables["MatTable"].Rows.get_Count(),
                   inDMST02.Tables["CHARGE_RANGE"].Rows.get_Count());

        Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc start ==");
        f_epex_call_rest_svc(conn, "FBSM_SSMODEL", "RUN_BASKET_MODEL", &inDMST02, &outDMST02);
        Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc end ==");

        // 校验RESULT_TABLE
        // if (!outDMST02.Tables.Contains("RESULT_TABLE") || outDMST02.Tables["RESULT_TABLE"].Rows.get_Count() <= 0)
        // {
        //     strcpy(s.msg, "模型计算失败，RESULT_TABLE返回为空!!!");
        //     throw CApplicationException(-1, s.msg, log.Location);
        // }
        Log::Trace("", __FUNCTION__, "RESULT_TABLE返回[{0}]行", outDMST02.Tables["RESULT_TABLE"].Rows.get_Count());

        // ==================== 7. 传出块处理 ====================
        // 按RESULT_TABLE遍历，遇到新配料单时删除未审核旧数据，生成所有炉号后再复制物料
        set<CString> processedComposeSet; // 记录RESULT_TABLE中已处理的配料单号
        map<CString, vector<CString>> composeToEafList; // 记录每个配料单生成的COMPOSE_LIST_NO_EAF列表

        // 第一轮：按配料单删除旧数据、生成所有电炉流水号、插入TFBSM43、更新TFBSM12
        for (size_t r = 0; r < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); r++)
        {
            compose_list_no = outDMST02.Tables["RESULT_TABLE"].Rows[r]["COMPOSE_LIST_NO"].ToString().Trim().ToUpper();
            furnace_count = outDMST02.Tables["RESULT_TABLE"].Rows[r]["FURNACE_COUNT"].ToDecimal();

            // 如果是新出现的配料单号，处理一次
            if (processedComposeSet.find(compose_list_no) == processedComposeSet.end())
            {
                processedComposeSet.insert(compose_list_no);

                // 先删除TFBSM41中对应未审核炉的数据
                sqlstr = " DELETE FROM TFBSM41 "
                         " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                         " AND COMPOSE_LIST_NO_EAF IN ( "
                         "   SELECT COMPOSE_LIST_NO_EAF FROM TFBSM43 "
                         "   WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO AND CHECK_FLAG != '1' ) ";
                cmd_del.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
                cmd_del.SetCommandText(sqlstr);
                cmd_del.ExecuteNonQuery();
                cmd_del.Close();

                // 再删除TFBSM43中未审核的数据
                sqlstr = " DELETE FROM TFBSM43 "
                         " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO AND CHECK_FLAG != '1' ";
                cmd_del.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
                cmd_del.SetCommandText(sqlstr);
                cmd_del.ExecuteNonQuery();
                cmd_del.Close();

                Log::Trace("", __FUNCTION__, "删除未审核旧数据：COMPOSE_LIST_NO=[{0}]", compose_list_no);

                // 查询当前配料单最大COMPOSE_LIST_NO_EAF序号
                max_eaf_seq = 0;
                sqlstr = " SELECT MAX(TO_NUMBER(SUBSTR(COMPOSE_LIST_NO_EAF, -3))) FROM TFBSM43 WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO ";
                cmd_seq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
                cmd_seq.SetCommandText(sqlstr);
                cmd_seq.ExecuteReader();
                if (cmd_seq.Read())
                {
                    CDecimal val = cmd_seq.GetDecimal(1);
                    if (val > 0)
                        max_eaf_seq = val.ToInt32();
                }
                cmd_seq.Close();

                // 查询当前配料单下未审核的SEQ_NO列表，按顺序分配给新炉号
                vector<int> uncheckedSeqNoList;
                sqlstr = " SELECT SEQ_NO FROM TFBSM12 "
                         " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                         " AND (ACT_CHECK_FLAG IS NULL OR ACT_CHECK_FLAG != '1') "
                         " ORDER BY TO_NUMBER(SEQ_NO) ";
                cmd_seq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
                cmd_seq.SetCommandText(sqlstr);
                cmd_seq.ExecuteReader();
                while (cmd_seq.Read())
                {
                    uncheckedSeqNoList.push_back(cmd_seq.GetDecimal(1).ToInt32());
                }
                cmd_seq.Close();

                // 生成所有电炉流水号并插入TFBSM43、更新TFBSM12
                vector<CString> eafList;
                for (int f = 1; f <= furnace_count.ToInt32(); f++)
                {
                    int current_seq = max_eaf_seq + f;

                    // 格式化流水号（3位，不足补零）
                    if (current_seq < 10)
                        seq_str = CString("00") + CConvert::ToString(current_seq);
                    else if (current_seq < 100)
                        seq_str = CString("0") + CConvert::ToString(current_seq);
                    else
                        seq_str = CConvert::ToString(current_seq);

                    compose_list_no_eaf = compose_list_no + seq_str;
                    eafList.push_back(compose_list_no_eaf);

                    // 取对应未审核的SEQ_NO
                    int seq_no = 0;
                    if (f <= (int)uncheckedSeqNoList.size())
                        seq_no = uncheckedSeqNoList[f - 1];
                    else
                    {
                        Log::Trace("", __FUNCTION__, "警告：配料单[{0}]未审核SEQ_NO数量不足，无法分配第[{1}]个新炉号", compose_list_no, f);
                        continue;
                    }

                    // 插入TFBSM43（到炉记录）
                    tfbsm43.Reset();
                    tfbsm43["COMPOSE_LIST_NO"] = compose_list_no;
                    tfbsm43["ST_NO"] = composeToStNoMap[compose_list_no];
                    tfbsm43["COMPOSE_LIST_NO_EAF"] = compose_list_no_eaf;
                    tfbsm43["REC_CREATOR"] = s.userid;
                    tfbsm43["REC_CREATE_TIME"] = datetime;
                    tfbsm43.Insert();

                    // 更新TFBSM12的COMPOSE_LIST_NO_EAF
                    sqlstr = " UPDATE TFBSM12 "
                             " SET COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF "
                             " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                             " AND TO_NUMBER(SEQ_NO) = @SEQ_NO ";
                    cmd_upd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
                    cmd_upd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
                    cmd_upd.Parameters.Set("SEQ_NO", CConvert::ToString(seq_no));
                    cmd_upd.SetCommandText(sqlstr);
                    cmd_upd.ExecuteNonQuery();
                    cmd_upd.Close();

                    Log::Trace("", __FUNCTION__, "生成新炉号：COMPOSE_LIST_NO=[{0}]，COMPOSE_LIST_NO_EAF=[{1}]，SEQ_NO=[{2}]",
                               compose_list_no, compose_list_no_eaf, seq_no);
                }

                composeToEafList[compose_list_no] = eafList;
            }
        }

        // 第二轮：把RESULT_TABLE中每个配料单的所有物料复制到该配料单的每一炉
        for (size_t r = 0; r < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); r++)
        {
            compose_list_no = outDMST02.Tables["RESULT_TABLE"].Rows[r]["COMPOSE_LIST_NO"].ToString().Trim().ToUpper();

            if (composeToEafList.find(compose_list_no) == composeToEafList.end())
                continue;

            vector<CString> &eafList = composeToEafList[compose_list_no];
            for (size_t e = 0; e < eafList.size(); e++)
            {
                compose_list_no_eaf = eafList[e];

                tfbsm41.Reset();
                tfbsm41["COMPOSE_LIST_NO"] = compose_list_no;
                tfbsm41["BUNKER_SEQ"] = outDMST02.Tables["RESULT_TABLE"].Rows[r]["BUNKER_SEQ"].ToString();
                tfbsm41["LAYER_NO"] = outDMST02.Tables["RESULT_TABLE"].Rows[r]["LAYER_NO"].ToString();
                tfbsm41["MAT_NAME"] = outDMST02.Tables["RESULT_TABLE"].Rows[r]["MAT_NAME"].ToString();
                tfbsm41["MAT_CODE"] = outDMST02.Tables["RESULT_TABLE"].Rows[r]["MAT_CODE"].ToString();
                tfbsm41["LOT_NO"] = outDMST02.Tables["RESULT_TABLE"].Rows[r]["LOT_NO"].ToString();
                tfbsm41["CAL_WEIGHT"] = outDMST02.Tables["RESULT_TABLE"].Rows[r]["CAL_WEIGHT"].ToDecimal();
                tfbsm41["STOCK_NAME"] = outDMST02.Tables["RESULT_TABLE"].Rows[r]["STOCK_NAME"].ToString();
                tfbsm41["BACK_C2"] = outDMST02.Tables["RESULT_TABLE"].Rows[r]["BACK_C2"].ToString();
                tfbsm41["COMPOSE_LIST_NO_EAF"] = compose_list_no_eaf;
                tfbsm41["REC_CREATOR"] = s.userid;
                tfbsm41["REC_CREATE_TIME"] = datetime;
                tfbsm41.Insert();
            }
        }

        // 处理报错的配料单（BASKET_STATUS）
        if (outDMST02.Tables.Contains("BASKET_STATUS"))
        {
            for (size_t b = 0; b < outDMST02.Tables["BASKET_STATUS"].Rows.get_Count(); b++)
            {
                compose_list_no = outDMST02.Tables["BASKET_STATUS"].Rows[b]["COMPOSE_LIST_NO"].ToString().Trim().ToUpper();
                basket_is_success = outDMST02.Tables["BASKET_STATUS"].Rows[b]["IS_SUCCESS"].ToString().Trim();
                basket_status_msg = outDMST02.Tables["BASKET_STATUS"].Rows[b]["WARNINGS"].ToString().Trim();

                // 0是失败的,这里只处理为0的情况
                if (basket_is_success != "0")
                    continue;

                checked_furnace_count = composeToCheckedCountMap[compose_list_no];
                furnace_count = composeToFurnaceCountMap[compose_list_no] - checked_furnace_count;
                if (furnace_count < 0)
                    furnace_count = 0;

                // 先删除TFBSM41中对应未审核炉的数据
                sqlstr = " DELETE FROM TFBSM41 "
                         " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                         " AND COMPOSE_LIST_NO_EAF IN ( "
                         "   SELECT COMPOSE_LIST_NO_EAF FROM TFBSM43 "
                         "   WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO AND CHECK_FLAG != '1' ) ";
                cmd_del.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
                cmd_del.SetCommandText(sqlstr);
                cmd_del.ExecuteNonQuery();
                cmd_del.Close();

                // 再删除TFBSM43中未审核的数据
                sqlstr = " DELETE FROM TFBSM43 "
                         " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO AND CHECK_FLAG != '1' ";
                cmd_del.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
                cmd_del.SetCommandText(sqlstr);
                cmd_del.ExecuteNonQuery();
                cmd_del.Close();
                Log::Trace("", __FUNCTION__, "删除未审核旧数据（报错配料单）：COMPOSE_LIST_NO=[{0}]", compose_list_no);

                // 查询当前配料单最大COMPOSE_LIST_NO_EAF序号
                max_eaf_seq = 0;
                sqlstr = " SELECT MAX(TO_NUMBER(SUBSTR(COMPOSE_LIST_NO_EAF, -3))) FROM TFBSM43 WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO ";
                cmd_seq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
                cmd_seq.SetCommandText(sqlstr);
                cmd_seq.ExecuteReader();
                if (cmd_seq.Read())
                {
                    CDecimal val = cmd_seq.GetDecimal(1);
                    if (val > 0)
                        max_eaf_seq = val.ToInt32();
                }
                cmd_seq.Close();

                // 查询当前配料单下未审核的SEQ_NO列表，按顺序分配给新炉号
                vector<int> uncheckedSeqNoList;
                sqlstr = " SELECT SEQ_NO FROM TFBSM12 "
                         " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                         " AND (ACT_CHECK_FLAG IS NULL OR ACT_CHECK_FLAG != '1') "
                         " ORDER BY TO_NUMBER(SEQ_NO) ";
                cmd_seq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
                cmd_seq.SetCommandText(sqlstr);
                cmd_seq.ExecuteReader();
                while (cmd_seq.Read())
                {
                    uncheckedSeqNoList.push_back(cmd_seq.GetDecimal(1).ToInt32());
                }
                cmd_seq.Close();

                // 分炉生成COMPOSE_LIST_NO_EAF并插入TFBSM43，ERROR_REMARK写入报错信息
                for (int f = 1; f <= furnace_count.ToInt32(); f++)
                {
                    int current_seq = max_eaf_seq + f;

                    if (current_seq < 10)
                        seq_str = CString("00") + CConvert::ToString(current_seq);
                    else if (current_seq < 100)
                        seq_str = CString("0") + CConvert::ToString(current_seq);
                    else
                        seq_str = CConvert::ToString(current_seq);

                    compose_list_no_eaf = compose_list_no + seq_str;

                    // 取对应未审核的SEQ_NO
                    int seq_no = 0;
                    if (f <= (int)uncheckedSeqNoList.size())
                        seq_no = uncheckedSeqNoList[f - 1];
                    else
                    {
                        Log::Trace("", __FUNCTION__, "警告：配料单[{0}]未审核SEQ_NO数量不足，无法分配第[{1}]个新炉号", compose_list_no, f);
                        continue;
                    }

                    tfbsm43.Reset();
                    tfbsm43["COMPOSE_LIST_NO"] = compose_list_no;
                    tfbsm43["ST_NO"] = composeToStNoMap[compose_list_no];
                    tfbsm43["COMPOSE_LIST_NO_EAF"] = compose_list_no_eaf;
                    tfbsm43["ERROR_REMARK"] = basket_status_msg;
                    tfbsm43["REC_CREATOR"] = s.userid;
                    tfbsm43["REC_CREATE_TIME"] = datetime;
                    tfbsm43.Insert();

                    // 更新TFBSM12的COMPOSE_LIST_NO_EAF
                    sqlstr = " UPDATE TFBSM12 "
                             " SET COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF "
                             " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                             " AND TO_NUMBER(SEQ_NO) = @SEQ_NO ";
                    cmd_upd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
                    cmd_upd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
                    cmd_upd.Parameters.Set("SEQ_NO", CConvert::ToString(seq_no));
                    cmd_upd.SetCommandText(sqlstr);
                    cmd_upd.ExecuteNonQuery();
                    cmd_upd.Close();
                }
            }
        }

        // 检查是否有传入的配料单未在模型返回中出现
        for (set<CString>::iterator it = composeSet.begin(); it != composeSet.end(); ++it)
        {
            compose_list_no = *it;
            if (processedComposeSet.find(compose_list_no) == processedComposeSet.end())
            {
                Log::Trace("", __FUNCTION__, "警告：配料单[{0}]在模型返回结果中未出现,请检查报错信息", compose_list_no);
            }
        }

        Log::Trace("", __FUNCTION__, "全部处理完成");
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
