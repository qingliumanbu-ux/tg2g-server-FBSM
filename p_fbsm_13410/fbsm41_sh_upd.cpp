/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 电炉料篮配料单审核操作
**************************************************/
#include "stdafx.h"
#include <map>
BM2F_ENTERACE(fbsm41_sh_upd)

int f_fbsm41_sh_upd(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd(conn);
	CString compose_list_no = "";
	CString compose_list_no_eaf = "";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString year_str = CDateTime::Now().ToString("yyyy");
	CString old_bunker_no = "";
	CString new_bunker_no = "";
	CString seq_str = "";
	CString check_flag = "";
	int max_seq = 0;
	int bunker_seq = 0;
	map<int, CString> bunkerSeqMap;

	try
	{
		CString sh_flag = bcls_rec->Tables[1].Rows[0]["SH_FLAG"].ToString().Trim();
		if (sh_flag == "")
			sh_flag = "UPD"; // 默认审核，兼容旧调用
		if (sh_flag != "UPD" && sh_flag != "DEL")
		{
			strcpy(s.msg, "参数错误：SH_FLAG只能为UPD或DEL!!!");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			compose_list_no = bcls_rec->Tables[0].Rows[i]["COMPOSE_LIST_NO"].ToString().Trim();
			compose_list_no_eaf = bcls_rec->Tables[0].Rows[i]["COMPOSE_LIST_NO_EAF"].ToString().Trim();

			if (compose_list_no == "")
			{
				strcpy(s.msg, "参数错误：COMPOSE_LIST_NO为空!!!");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (compose_list_no_eaf == "")
			{
				strcpy(s.msg, "参数错误：COMPOSE_LIST_NO_EAF为空!!!");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			if (sh_flag == "UPD")
			{
				// 1. 查询TFBSM43确认该炉未经审核
				sqlstr = " SELECT CHECK_FLAG FROM TFBSM43 "
						 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
						 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF ";
				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
				cmd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
				cmd.ExecuteReader();
				check_flag = "";
				if (cmd.Read())
					check_flag = cmd.GetString(1);
				cmd.Close();
				if (check_flag.Trim() == "1")
				{
					strcpy(s.msg, "该炉已经审核，无法重复审核!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				// 2. 审核TFBSM43
				sqlstr = " UPDATE TFBSM43 "
						 " SET CHECK_FLAG = '1', CHECK_MAKE = @CHECK_MAKE, CHECK_DATE = @CHECK_DATE "
						 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
						 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF ";
				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("CHECK_MAKE", s.userid);
				cmd.Parameters.Set("CHECK_DATE", datetime);
				cmd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
				cmd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
				cmd.ExecuteNonQuery();
				cmd.Close();
				Log::Trace("", __FUNCTION__, "审核TFBSM43：COMPOSE_LIST_NO=[{0}]，COMPOSE_LIST_NO_EAF=[{1}]",
						   compose_list_no, compose_list_no_eaf);

				// 3. 更新TFBSM12的ACT_CHECK_FLAG，表示该炉已在TFBSM43中审核
				sqlstr = " UPDATE TFBSM12 "
						 " SET ACT_CHECK_FLAG = '1' "
						 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
						 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF ";
				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
				cmd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
				cmd.ExecuteNonQuery();
				cmd.Close();
				Log::Trace("", __FUNCTION__, "更新TFBSM12 ACT_CHECK_FLAG：COMPOSE_LIST_NO=[{0}]，COMPOSE_LIST_NO_EAF=[{1}]",
						   compose_list_no, compose_list_no_eaf);

				// 4. 查询全局今年最大流水号（通过TFBSM41.BUNKER_NO中的#后数字）
				max_seq = 0;
				sqlstr = " SELECT MAX(TO_NUMBER(SUBSTR(T41.BUNKER_NO, INSTR(T41.BUNKER_NO, '#') + 1))) "
						 " FROM TFBSM41 T41 "
						 " INNER JOIN TFBSM43 T43 ON T41.COMPOSE_LIST_NO = T43.COMPOSE_LIST_NO AND T41.COMPOSE_LIST_NO_EAF = T43.COMPOSE_LIST_NO_EAF "
						 " WHERE T41.BUNKER_NO LIKE '%#%' "
						 " AND T43.CHECK_DATE LIKE @YEAR_PREFIX ";
				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("YEAR_PREFIX", year_str + "%");
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					CDecimal val = cmd.GetDecimal(1);
					if (val > 0)
						max_seq = val.ToInt32();
				}
				cmd.Close();
				Log::Trace("", __FUNCTION__, "今年最大流水号=[{0}]", max_seq);

				// 比较前端传入的BUNKER_SEQ_MAX
				if (bcls_rec->Tables[0].Columns.Contains("BUNKER_SEQ_MAX"))
				{
					CString bunker_seq_max_str = bcls_rec->Tables[0].Rows[i]["BUNKER_SEQ_MAX"].ToString().Trim();
					if (bunker_seq_max_str != "")
					{
						int bunker_seq_max = bcls_rec->Tables[0].Rows[i]["BUNKER_SEQ_MAX"].ToDecimal().ToInt32();
						if (bunker_seq_max < max_seq)
						{
							CString err_msg = "修改的最大流水号不得小于已生产的料篮流水号，当前已生产最大流水号为 ";
							err_msg += CConvert::ToString(max_seq);
							strcpy(s.msg, err_msg);
							throw CApplicationException(-1, s.msg, log.Location);
						}
						else if (bunker_seq_max > max_seq)
						{
							max_seq = bunker_seq_max;
							Log::Trace("", __FUNCTION__, "用户指定最大流水号=[{0}]，覆盖查询值", max_seq);
						}
					}
				}

				// 5. 查询当前炉的所有料篮顺序号（按BUNKER_SEQ去重）
				bunkerSeqMap.clear();
				sqlstr = " SELECT DISTINCT BUNKER_SEQ, BUNKER_NO FROM TFBSM41 "
						 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
						 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF "
						 " ORDER BY BUNKER_SEQ ";
				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
				cmd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
				cmd.ExecuteReader();
				while (cmd.Read())
				{
					bunker_seq = cmd.GetDecimal(1).ToInt32();
					old_bunker_no = cmd.GetString(2);
					bunkerSeqMap[bunker_seq] = old_bunker_no;
				}
				cmd.Close();

				// 6. 为当前炉的每一篮分配流水号并更新TFBSM41.BUNKER_NO
				int seq_index = 1;
				for (map<int, CString>::iterator it = bunkerSeqMap.begin(); it != bunkerSeqMap.end(); ++it)
				{
					bunker_seq = it->first;
					old_bunker_no = it->second;
					int current_seq = max_seq + seq_index;

					if (current_seq < 10)
						seq_str = CString("00") + CConvert::ToString(current_seq);
					else if (current_seq < 100)
						seq_str = CString("0") + CConvert::ToString(current_seq);
					else
						seq_str = CConvert::ToString(current_seq);

					new_bunker_no = old_bunker_no + "#" + seq_str;

					sqlstr = " UPDATE TFBSM41 "
							 " SET BUNKER_NO = @NEW_BUNKER_NO "
							 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
							 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF "
							 " AND BUNKER_SEQ = @BUNKER_SEQ ";
					cmd.SetCommandText(sqlstr);
					cmd.Parameters.Set("NEW_BUNKER_NO", new_bunker_no);
					cmd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
					cmd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
					cmd.Parameters.Set("BUNKER_SEQ", CConvert::ToString(bunker_seq));
					cmd.ExecuteNonQuery();
					cmd.Close();

					Log::Trace("", __FUNCTION__, "更新TFBSM41料篮号：COMPOSE_LIST_NO=[{0}]，COMPOSE_LIST_NO_EAF=[{1}]，BUNKER_SEQ=[{2}]，BUNKER_NO=[{3}]",
							   compose_list_no, compose_list_no_eaf, CConvert::ToString(bunker_seq), new_bunker_no);

					seq_index++;
				}
			}
			else if (sh_flag == "DEL")
			{
				// 1. 查询TFBSM43确认该炉已审核
				sqlstr = " SELECT CHECK_FLAG FROM TFBSM43 "
						 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
						 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF ";
				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
				cmd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
				cmd.ExecuteReader();
				check_flag = "";
				if (cmd.Read())
					check_flag = cmd.GetString(1);
				cmd.Close();
				if (check_flag.Trim() != "1")
				{
					strcpy(s.msg, "该炉未经审核，无法取消审核!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				// 2. 取消审核TFBSM43
				sqlstr = " UPDATE TFBSM43 "
						 " SET CHECK_FLAG = ' ', CHECK_MAKE = ' ', CHECK_DATE = ' ', DEV_CODE = ' ', EAF_PROC_NO = ' '  "
						 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
						 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF ";
				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
				cmd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
				cmd.ExecuteNonQuery();
				cmd.Close();
				Log::Trace("", __FUNCTION__, "取消审核TFBSM43：COMPOSE_LIST_NO=[{0}]，COMPOSE_LIST_NO_EAF=[{1}]",
						   compose_list_no, compose_list_no_eaf);

				// 3. 更新TFBSM12的ACT_CHECK_FLAG为空格
				sqlstr = " UPDATE TFBSM12 "
						 " SET ACT_CHECK_FLAG = ' ' "
						 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
						 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF ";
				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
				cmd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
				cmd.ExecuteNonQuery();
				cmd.Close();
				Log::Trace("", __FUNCTION__, "更新TFBSM12 ACT_CHECK_FLAG为空格：COMPOSE_LIST_NO=[{0}]，COMPOSE_LIST_NO_EAF=[{1}]",
						   compose_list_no, compose_list_no_eaf);

				// 4. 查询当前炉的所有料篮顺序号（按BUNKER_SEQ去重）
				bunkerSeqMap.clear();
				sqlstr = " SELECT DISTINCT BUNKER_SEQ, BUNKER_NO FROM TFBSM41 "
						 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
						 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF "
						 " ORDER BY BUNKER_SEQ ";
				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
				cmd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
				cmd.ExecuteReader();
				while (cmd.Read())
				{
					bunker_seq = cmd.GetDecimal(1).ToInt32();
					old_bunker_no = cmd.GetString(2);
					bunkerSeqMap[bunker_seq] = old_bunker_no;
				}
				cmd.Close();

				// 5. 去掉TFBSM41中BUNKER_NO的流水号（#及后面部分）
				for (map<int, CString>::iterator it = bunkerSeqMap.begin(); it != bunkerSeqMap.end(); ++it)
				{
					bunker_seq = it->first;
					old_bunker_no = it->second;
					int pos = old_bunker_no.Find("#");
					if (pos >= 0)
						new_bunker_no = old_bunker_no.SubstringNE(0, pos);
					else
						new_bunker_no = old_bunker_no;

					sqlstr = " UPDATE TFBSM41 "
							 " SET BUNKER_NO = @NEW_BUNKER_NO "
							 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
							 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF "
							 " AND BUNKER_SEQ = @BUNKER_SEQ ";
					cmd.SetCommandText(sqlstr);
					cmd.Parameters.Set("NEW_BUNKER_NO", new_bunker_no);
					cmd.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
					cmd.Parameters.Set("COMPOSE_LIST_NO_EAF", compose_list_no_eaf);
					cmd.Parameters.Set("BUNKER_SEQ", CConvert::ToString(bunker_seq));
					cmd.ExecuteNonQuery();
					cmd.Close();

					Log::Trace("", __FUNCTION__, "去掉TFBSM41料篮号流水号：COMPOSE_LIST_NO=[{0}]，COMPOSE_LIST_NO_EAF=[{1}]，BUNKER_SEQ=[{2}]，BUNKER_NO=[{3}]",
							   compose_list_no, compose_list_no_eaf, CConvert::ToString(bunker_seq), new_bunker_no);
				}
			}
		}

		Log::Trace("", __FUNCTION__, "审核完成，共[{0}]行", bcls_rec->Tables[0].Rows.get_Count());
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
