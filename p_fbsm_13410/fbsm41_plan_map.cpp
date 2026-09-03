/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 电炉配料单匹配对应
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm41_plan_map)

int f_fbsm41_plan_map(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_check(conn);    // 校验查询
	CDbCommand cmd_inq(conn);      // 最大序号查询
	CDbCommand cmd_insert43(conn); // TFBSM43插入
	CDbCommand cmd_upd(conn);      // TFBSM12更新
	CDbCommand cmd_copy(conn);     // TFBSM41源数据查询
	CModel tfbsm41("TFBSM41");     // TFBSM41模型
	CModel tfbsm43("TFBSM43");     // TFBSM43模型

	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	CString source_compose_list_no = "";     // 复用配料单号
	CString source_compose_list_no_eaf = ""; // 复用电炉配料单号

	int count43 = 0;
	int count41 = 0;
	CString check_flag = "";

	CString date_c = "";
	CString backlog_ea = "";
	CString seq_no = "";
	CString plan_st_no = "";
	CString t12_compose_list_no = "";
	CString t12_compose_list_no_eaf = "";

	int max_seq = 0;
	int new_seq = 0;
	CString seq_str = "";
	CString new_compose_list_no_eaf = "";

	CString bunker_no = "";
	int pos = -1;

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
		source_compose_list_no_eaf = bcls_rec->Tables[1].Rows[0]["COMPOSE_LIST_NO_EAF"].ToString().Trim();

		if (source_compose_list_no == "" || source_compose_list_no_eaf == "")
		{
			strcpy(s.msg, "复用配料单信息错误：COMPOSE_LIST_NO和COMPOSE_LIST_NO_EAF不能为空!!!");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		// 校验TFBSM43中是否存在
		sqlstr = " SELECT COUNT(1) FROM TFBSM43 "
				 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
				 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF ";
		cmd_check.Parameters.Set("COMPOSE_LIST_NO", source_compose_list_no);
		cmd_check.Parameters.Set("COMPOSE_LIST_NO_EAF", source_compose_list_no_eaf);
		cmd_check.SetCommandText(sqlstr);
		cmd_check.ExecuteReader();
		count43 = 0;
		if (cmd_check.Read())
			count43 = cmd_check.GetDecimal(1).ToInt32();
		cmd_check.Close();
		if (count43 <= 0)
		{
			strcpy(s.msg, "该电炉料篮配料单不存在!!!");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		// 校验TFBSM41中是否存在明细
		sqlstr = " SELECT COUNT(1) FROM TFBSM41 "
				 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
				 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF ";
		cmd_check.Parameters.Set("COMPOSE_LIST_NO", source_compose_list_no);
		cmd_check.Parameters.Set("COMPOSE_LIST_NO_EAF", source_compose_list_no_eaf);
		cmd_check.SetCommandText(sqlstr);
		cmd_check.ExecuteReader();
		count41 = 0;
		if (cmd_check.Read())
			count41 = cmd_check.GetDecimal(1).ToInt32();
		cmd_check.Close();
		if (count41 <= 0)
		{
			strcpy(s.msg, "该电炉料篮配料单无明细数据!!!");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		// 校验TFBSM43中CHECK_FLAG是否为1
		sqlstr = " SELECT CHECK_FLAG FROM TFBSM43 "
				 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
				 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF ";
		cmd_check.Parameters.Set("COMPOSE_LIST_NO", source_compose_list_no);
		cmd_check.Parameters.Set("COMPOSE_LIST_NO_EAF", source_compose_list_no_eaf);
		cmd_check.SetCommandText(sqlstr);
		cmd_check.ExecuteReader();
		check_flag = "";
		if (cmd_check.Read())
			check_flag = cmd_check.GetString(1).Trim();
		cmd_check.Close();
		if (check_flag != "1")
		{
			strcpy(s.msg, "要复用的配料单未经审核!!!");
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
			backlog_ea = bcls_rec->Tables[0].Rows[i]["BACKLOG_EA"].ToString().Trim();
			seq_no = bcls_rec->Tables[0].Rows[i]["SEQ_NO"].ToString().Trim();

			if (date_c == "" || plan_st_no == "" || backlog_ea == "" || seq_no == "")
			{
				strcpy(s.msg, "计划信息错误：DATE_C、ST_NO、BACKLOG_EA、SEQ_NO都不能为空!!!");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			// 查询TFBSM12中是否存在该计划，且COMPOSE_LIST_NO和COMPOSE_LIST_NO_EAF均为空
			sqlstr = " SELECT COMPOSE_LIST_NO, COMPOSE_LIST_NO_EAF FROM TFBSM12 "
					 " WHERE DATE_C = @DATE_C "
					 " AND ST_NO = @ST_NO "
					 " AND BACKLOG_EA = @BACKLOG_EA "
					 " AND SEQ_NO = @SEQ_NO ";
			cmd_check.Parameters.Set("DATE_C", date_c);
			cmd_check.Parameters.Set("ST_NO", plan_st_no);
			cmd_check.Parameters.Set("BACKLOG_EA", backlog_ea);
			cmd_check.Parameters.Set("SEQ_NO", seq_no);
			cmd_check.SetCommandText(sqlstr);
			cmd_check.ExecuteReader();
			if (!cmd_check.Read())
			{
				cmd_check.Close();
				strcpy(s.msg, "未找到该计划!!!");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			t12_compose_list_no = cmd_check.GetString(1).Trim();
			t12_compose_list_no_eaf = cmd_check.GetString(2).Trim();
			cmd_check.Close();

			if (t12_compose_list_no != "" || t12_compose_list_no_eaf != "")
			{
				strcpy(s.msg, "该计划已进行配料!!!");
				throw CApplicationException(-1, s.msg, log.Location);
			}
		}

		// ==================== 4. 循环处理每个计划 ====================
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			date_c = bcls_rec->Tables[0].Rows[i]["DATE_C"].ToString().Trim();
			plan_st_no = bcls_rec->Tables[0].Rows[i]["ST_NO"].ToString().Trim();
			backlog_ea = bcls_rec->Tables[0].Rows[i]["BACKLOG_EA"].ToString().Trim();
			seq_no = bcls_rec->Tables[0].Rows[i]["SEQ_NO"].ToString().Trim();

			// 4.1 查询TFBSM43中该配料单最大流水号
			max_seq = 0;
			sqlstr = " SELECT MAX(TO_NUMBER(SUBSTR(COMPOSE_LIST_NO_EAF, -3))) FROM TFBSM43 "
					 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO ";
			cmd_inq.Parameters.Set("COMPOSE_LIST_NO", source_compose_list_no);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				CDecimal val = cmd_inq.GetDecimal(1);
				if (val > 0)
					max_seq = val.ToInt32();
			}
			cmd_inq.Close();

			// 生成新的COMPOSE_LIST_NO_EAF
			new_seq = max_seq + 1;
			if (new_seq < 10)
				seq_str = CString("00") + CConvert::ToString(new_seq);
			else if (new_seq < 100)
				seq_str = CString("0") + CConvert::ToString(new_seq);
			else
				seq_str = CConvert::ToString(new_seq);
			new_compose_list_no_eaf = source_compose_list_no + seq_str;

			Log::Trace("", __FUNCTION__, "生成新炉号：COMPOSE_LIST_NO=[{0}]，COMPOSE_LIST_NO_EAF=[{1}]",
						   source_compose_list_no, new_compose_list_no_eaf);

			// 4.2 插入TFBSM43
			sqlstr = " INSERT INTO TFBSM43 (COMPOSE_LIST_NO, COMPOSE_LIST_NO_EAF, ST_NO, REC_CREATOR, REC_CREATE_TIME) "
					 " VALUES (@COMPOSE_LIST_NO, @COMPOSE_LIST_NO_EAF, @ST_NO, @REC_CREATOR, @REC_CREATE_TIME) ";
			cmd_insert43.Parameters.Set("COMPOSE_LIST_NO", source_compose_list_no);
			cmd_insert43.Parameters.Set("COMPOSE_LIST_NO_EAF", new_compose_list_no_eaf);
			cmd_insert43.Parameters.Set("ST_NO", plan_st_no);
			cmd_insert43.Parameters.Set("REC_CREATOR", s.userid);
			cmd_insert43.Parameters.Set("REC_CREATE_TIME", datetime);
			cmd_insert43.SetCommandText(sqlstr);
			cmd_insert43.ExecuteNonQuery();
			cmd_insert43.Close();

			// 4.3 更新TFBSM12，绑定配料单号并置CHECK_FLAG为1
			sqlstr = " UPDATE TFBSM12 "
					 " SET COMPOSE_LIST_NO = @COMPOSE_LIST_NO, "
					 "     COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF, "
					 "     CHECK_FLAG = '1' "
					 " WHERE DATE_C = @DATE_C "
					 " AND ST_NO = @ST_NO "
					 " AND BACKLOG_EA = @BACKLOG_EA "
					 " AND SEQ_NO = @SEQ_NO ";
			cmd_upd.Parameters.Set("COMPOSE_LIST_NO", source_compose_list_no);
			cmd_upd.Parameters.Set("COMPOSE_LIST_NO_EAF", new_compose_list_no_eaf);
			cmd_upd.Parameters.Set("DATE_C", date_c);
			cmd_upd.Parameters.Set("ST_NO", plan_st_no);
			cmd_upd.Parameters.Set("BACKLOG_EA", backlog_ea);
			cmd_upd.Parameters.Set("SEQ_NO", seq_no);
			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.ExecuteNonQuery();
			cmd_upd.Close();

			Log::Trace("", __FUNCTION__, "更新TFBSM12：DATE_C=[{0}]，ST_NO=[{1}]，BACKLOG_EA=[{2}]，SEQ_NO=[{3}]，COMPOSE_LIST_NO_EAF=[{4}]",
						   date_c, plan_st_no, backlog_ea, seq_no, new_compose_list_no_eaf);

			// 4.4 复制TFBSM41明细
			sqlstr = " SELECT * FROM TFBSM41 "
					 " WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
					 " AND COMPOSE_LIST_NO_EAF = @COMPOSE_LIST_NO_EAF "
					 " ORDER BY BUNKER_SEQ, LAYER_NO ";
			cmd_copy.Parameters.Set("COMPOSE_LIST_NO", source_compose_list_no);
			cmd_copy.Parameters.Set("COMPOSE_LIST_NO_EAF", source_compose_list_no_eaf);
			cmd_copy.SetCommandText(sqlstr);
			cmd_copy.ExecuteReader();
			while (cmd_copy.Read())
			{
				tfbsm41.Reset();
				cmd_copy.Fetch(tfbsm41);

				// BUNKER_NO去掉#及后面内容
				bunker_no = tfbsm41["BUNKER_NO"].ToString().Trim();
				pos = bunker_no.Find("#");
				if (pos >= 0)
					bunker_no = bunker_no.SubstringNE(0, pos);
				tfbsm41["BUNKER_NO"] = bunker_no;

				// 更新创建信息和炉号
				tfbsm41["REC_CREATOR"] = s.userid;
				tfbsm41["REC_CREATE_TIME"] = datetime;
				tfbsm41["COMPOSE_LIST_NO_EAF"] = new_compose_list_no_eaf;

				tfbsm41.Insert();
			}
			cmd_copy.Close();
		}

		Log::Trace("", __FUNCTION__, "计划复用完成，共处理[{0}]个计划", bcls_rec->Tables[0].Rows.get_Count());
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
