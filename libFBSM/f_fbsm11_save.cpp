/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 钢种新增函数
**************************************************/
#include "stdafx.h"

BM2_FUNCTION_EXPORT

int f_fbsm11_save(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
    CTracer log(__FUNCTION__);
    int doFlag = 0;
    CString sqlstr = " ";
    CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CModel tfbsm11("TFBSM11");
    CDbCommand cmd(conn);
    try
    {
        if (bcls_rec->Tables.get_Count() <= 0)
        {
            strcpy(s.msg, "传入块数据异常!!!");
            throw CApplicationException(-1, s.msg, log.Location);
        }
		if (bcls_rec->Tables[0].Rows.get_Count() <= 0)
		{
			strcpy(s.msg, "传入块数据行数异常!!!");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
        {
            if (bcls_rec->Tables[0].Rows[i]["ST_NO"].ToString().Trim() == "")
            {
                Log::Trace("", __FUNCTION__, "行号=[{0}],第[{1}]行ST_NO为空，跳过处理", __LINE__, i);
                continue;
            }

			tfbsm11.Reset();
			tfbsm11.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			// tfbsm11["REC_REVISOR"] = s.userid;
			// tfbsm11["REC_REVISE_TIME"] = datetime;

			if (tfbsm11.QueryCount("ST_NO") > 0)
			{
				Log::Trace("", __FUNCTION__, "行号=[{0}],第[{1}]行ST_NO为已存在，跳过处理", __LINE__, i);
				continue;
			}


            sqlstr = "INSERT INTO TFBSM11 (ST_NO, REC_CREATOR, REC_CREATE_TIME) "
								 "VALUES (@ST_NO, @REC_CREATOR, @REC_CREATE_TIME)";

			cmd.Parameters.Set("ST_NO", tfbsm11["ST_NO"].ToString());
			cmd.Parameters.Set("REC_CREATOR", s.userid);
			cmd.Parameters.Set("REC_CREATE_TIME", datetime);
            cmd.SetCommandText(sqlstr);
            cmd.ExecuteNonQuery();
            cmd.Close();
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
