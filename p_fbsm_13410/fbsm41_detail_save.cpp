/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 电炉料篮配料单明细保存
**************************************************/
#include "stdafx.h"
#include <set>
BM2F_ENTERACE(fbsm41_detail_save)

int f_fbsm41_detail_save(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
    CTracer log(__FUNCTION__);
    int doFlag = 0;
    CString sqlstr = " ";
    /* 数据库操作类定义：统一放在Service或函数前段 */
    CDbCommand cmd_inq(conn);
    CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
    CString compose_list_no = "";
    CString compose_list_no_eaf = "";
    CString key = "";
    CString check_flag = "";

    CModel tfbsm41("TFBSM41");
    CModel tfbsm43("TFBSM43");
    set<CString> processedSet; // 记录已删除旧数据的COMPOSE_LIST_NO+COMPOSE_LIST_NO_EAF组合

    try
    {
        for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
        {
            compose_list_no = bcls_rec->Tables[0].Rows[i]["COMPOSE_LIST_NO"].ToString().Trim();
            compose_list_no_eaf = bcls_rec->Tables[0].Rows[i]["COMPOSE_LIST_NO_EAF"].ToString().Trim();
            key = compose_list_no + "#" + compose_list_no_eaf;

            // 如果是新出现的组合，先删除旧数据
            if (processedSet.find(key) == processedSet.end())
            {
                processedSet.insert(key);
                tfbsm41.Reset();
                tfbsm41["COMPOSE_LIST_NO"] = compose_list_no;
                tfbsm41["COMPOSE_LIST_NO_EAF"] = compose_list_no_eaf;
                tfbsm41.Delete("COMPOSE_LIST_NO,COMPOSE_LIST_NO_EAF");
                Log::Trace("", __FUNCTION__, "删除TFBSM41旧数据：COMPOSE_LIST_NO=[{0}]，COMPOSE_LIST_NO_EAF=[{1}]",
                           compose_list_no, compose_list_no_eaf);
            }

            // 插入新数据
            tfbsm41.Reset();
            tfbsm41.MergeFrom(bcls_rec->Tables[0].Rows[i]);
            tfbsm41["REC_CREATOR"] = s.userid;
            tfbsm41["REC_CREATE_TIME"] = datetime;
            tfbsm41.Insert();
        }

        for (int i = 0; i < bcls_rec->Tables[1].Rows.get_Count(); i++)
        {
            compose_list_no = bcls_rec->Tables[1].Rows[i]["COMPOSE_LIST_NO"].ToString().Trim();
            compose_list_no_eaf = bcls_rec->Tables[1].Rows[i]["COMPOSE_LIST_NO_EAF"].ToString().Trim();

            // 校验该电炉料篮配料单是否已审核
            sqlstr = " SELECT CHECK_FLAG FROM TFBSM43 "
                     " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                     " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF ";
            cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
            cmd_inq.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
            cmd_inq.SetCommandText(sqlstr);
            cmd_inq.ExecuteReader();
            check_flag = "";
            if (cmd_inq.Read())
                check_flag = cmd_inq.GetString(1);
            cmd_inq.Close();
            if (check_flag.Trim() == "1")
            {
                strcpy(s.msg, "该电炉料篮配料单已被审核，无法修改!!!");
                throw CApplicationException(-1, s.msg, log.Location);
            }

            // 首次保存时记录 FIRST_SAVE_TIME
            sqlstr = " UPDATE TFBSM43 "
                     " SET FIRST_SAVE_TIME = @FIRST_SAVE_TIME "
                     " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
                     " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF "
                     " AND TRIM(FIRST_SAVE_TIME) IS NULL ";
            cmd_inq.SetCommandText(sqlstr);
            cmd_inq.Parameters.Set("FIRST_SAVE_TIME", datetime);
            cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
            cmd_inq.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
            cmd_inq.ExecuteNonQuery();
            cmd_inq.Close();

            tfbsm43.Reset();
            tfbsm43.MergeFrom(bcls_rec->Tables[1].Rows[i]);
            tfbsm43["REC_REVISOR"] = s.userid;
            tfbsm43["REC_REVISE_TIME"] = datetime;
            
            if (tfbsm43["EAF_PROC_NO"].ToString().Trim() != "" && tfbsm43["DEV_CODE"].ToString().Trim() == "")
                tfbsm43["DEV_CODE"] = tfbsm43["EAF_PROC_NO"].ToString().Trim().SubstringNE(0, 2);

            tfbsm43.Update("DEV_CODE,RESP,SHIFT_GROUP,REMARK,REC_REVISOR,REC_REVISE_TIME,EAF_PROC_NO", "COMPOSE_LIST_NO,COMPOSE_LIST_NO_EAF");
            Log::Trace("", __FUNCTION__, "更新TFBSM43：COMPOSE_LIST_NO=[{0}]，COMPOSE_LIST_NO_EAF=[{1}]",
                       compose_list_no, compose_list_no_eaf);

            
        }

        Log::Trace("", __FUNCTION__, "全部处理完成，TFBSM41共[{0}]行，TFBSM43共[{1}]行",
                   bcls_rec->Tables[0].Rows.get_Count(), bcls_rec->Tables[1].Rows.get_Count());
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
