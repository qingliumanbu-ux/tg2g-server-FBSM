/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: H5静态表保存
**************************************************/
#include "stdafx.h"

BM2F_ENTERACE(fbsmxx_save)

int f_fbsmxx_save(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString st_no = " ";//出钢记号
	int ret = 0;
	CString backlog_ea = " ";//工艺路线
	CString date = " ";//日期
	CString material_code = " ";//中类代码
	CString seq_no = " ";//序号
	CDecimal p_seq = 0;
	CString compose_list_no = " ";
	CString comm_fmly_code = "";
	CDbCommand cmd_inq(conn);
	CModel tfbsmxx("TFBSMXX");
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	try
	{
		//新增
		if (bcls_rec->Tables.Contains("TFBSMXX_ADD")){
			for (size_t i = 0; i < bcls_rec->Tables["TFBSMXX_ADD"].Rows.get_Count(); i++){
				tfbsmxx.MergeFrom(bcls_rec->Tables["TFBSMXX_ADD"].Rows[i]);
				Log::Trace("", "", "linke = {0}", __LINE__);
				st_no = bcls_rec->Tables["TFBSMXX_ADD"].Rows[0]["ST_NO"].ToString();
				backlog_ea = bcls_rec->Tables["TFBSMXX_ADD"].Rows[0]["BACKLOG_EA"].ToString();
				date = bcls_rec->Tables["TFBSMXX_ADD"].Rows[0]["DATE_C"].ToString();
				//配料单号空的则代表是一个新的配料单 COMPOSE_LIST_NO
				if (bcls_rec->Tables["TFBSMXX_ADD"].Rows[0]["COMPOSE_LIST_NO"].ToString().Trim() == "" && i == 0){
					//生成配料单
					//查询后台数据
					sqlstr = " select COMM_FMLY_CODE,MATERIAL_CODE from TFBSM11 where ST_NO='" + st_no + "' ";
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.ExecuteReader();
					if (cmd_inq.Read())
					{
						comm_fmly_code = cmd_inq.GetString(1);
						material_code = cmd_inq.GetString(2);
					}
					cmd_inq.Close();
					//默认是第一个配料单
					sqlstr = " select  max(substr(COMPOSE_LIST_NO,12,14)),max(substr(COMPOSE_LIST_NO,14,1)),max(substr(COMPOSE_LIST_NO,13, 1)),max(substr(COMPOSE_LIST_NO,13, 2)) from tfbsm12 where DATE_C='" + date + "'  and COMPOSE_LIST_NO!=' ' and MATERIAL_CODE='" + material_code + "'  and BACKLOG_EA='" + backlog_ea + "'   ";
					Log::Trace("", __FUNCTION__, "== sqlstr[{0}] ==", sqlstr);
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.ExecuteReader();
					if (cmd_inq.Read()){
						if (cmd_inq.GetDecimal(3) == 0 && cmd_inq.GetDecimal(2) != 9){
							p_seq = cmd_inq.GetDecimal(2);
							Log::Trace("", __FUNCTION__, "== p_seq1[{0}] ==", p_seq.ToString());
							p_seq = p_seq + 1;
							Log::Trace("", __FUNCTION__, "== p_seq1[{0}] ==", p_seq.ToString());
							compose_list_no = st_no.SubstringNE(1, 1) + material_code + backlog_ea + date.SubstringNE(2, 6) + "0" + p_seq.ToString();
						}
						else
						{
							p_seq = cmd_inq.GetDecimal(4);
							Log::Trace("", __FUNCTION__, "== p_seq2[{0}] ==", p_seq.ToString());
							p_seq = p_seq + 1;
							Log::Trace("", __FUNCTION__, "== p_seq2[{0}] ==", p_seq.ToString());
							compose_list_no = st_no.SubstringNE(1, 1) + material_code + backlog_ea + date.SubstringNE(2, 6) + p_seq.ToString();
						}
					}
					else{
						Log::Trace("", __FUNCTION__, "== linke[{0}] ==", __LINE__);
						compose_list_no = st_no.SubstringNE(1, 1) + material_code + backlog_ea + date.SubstringNE(2, 6) + "01";
					}
					cmd_inq.Close();
				}
				tfbsmxx["COMPOSE_LIST_NO"] = compose_list_no;
				tfbsmxx["DATE_C"] = date;
				tfbsmxx["REC_CREATOR"] = s.userid;
				tfbsmxx["REC_CREATE_TIME"] = datetime;
				tfbsmxx.Insert();
			}
		}
		//修改
		if (bcls_rec->Tables.Contains("TFBSMXX_MODIFY")){
			for (size_t i = 0; i < bcls_rec->Tables["TFBSMXX_MODIFY"].Rows.get_Count(); i++){
				tfbsmxx.MergeFrom(bcls_rec->Tables["TFBSMXX_MODIFY"].Rows[i]);
				st_no = bcls_rec->Tables["TFBSMXX_MODIFY"].Rows[0]["ST_NO"].ToString();
				backlog_ea = bcls_rec->Tables["TFBSMXX_MODIFY"].Rows[0]["BACKLOG_EA"].ToString();
				date = bcls_rec->Tables["TFBSMXX_MODIFY"].Rows[0]["DATE_C"].ToString();
				tfbsmxx.Delete();
				tfbsmxx["REC_REVISOR"] = s.userid;
				tfbsmxx["REC_REVISE_TIME"] = datetime;
				tfbsmxx.Insert();
			}
		}
		//删除
		if (bcls_rec->Tables.Contains("TFBSMXX_DELETE")){
			for (size_t i = 0; i < bcls_rec->Tables["TFBSMXX_DELETE"].Rows.get_Count(); i++){
				tfbsmxx.MergeFrom(bcls_rec->Tables["TFBSMXX_DELETE"].Rows[i]);
				tfbsmxx.Delete();
			}
		}
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
