/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 新增修改删除综合函数
**************************************************/
#include "stdafx.h"

BM2_FUNCTION_EXPORT

int f_fbsm12_ins(CDataTable *data_table, EIClass *bcls_ret, CDbConnection *conn) // 新增
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString stNo = " ";
	CString yesterday_start = " ";
	CString yesterday_end = " ";
	CString tapping_wt_value = " ";
	CString prev_tapping_mode = " ";
	CString comm_fmly_code = " ";
	CString backlog_ea = " ";
	CString material_code = " ";

	CDecimal si_content = 0;
	CDecimal si_percent = 0;
	CDecimal prev_tapping_avg = 0;

	CModel tfbsm12("TFBSM12");
	CModel tfbsm31("TFBSM31");
	CModel tqmts0x("TQMTS0X");
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	// 程序公共CDbCommand
	CDbCommand cmd(conn);
	CDbCommand cmd_check(conn);
	try
	{
		/*新增*/
		/*获取传入行数*/
		int count = data_table->Rows.get_Count();
		/*循环压入数据表行数*/
		for (int i = 0; i < count; i++)
		{
			stNo = data_table->Rows[i]["ST_NO"].ToString().Trim();
			if (stNo.Trim() == "")
			{
				strcpy(s.msg, "有数据出钢记号为空!!!");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			Log::Trace("", __FUNCTION__, "开始新增");
			if (stNo.Substring(0, 1) == "2" || stNo.Substring(0, 1) == "3") // 碳钢
			{
				Log::Trace("", __FUNCTION__, "新增，碳钢");
				tfbsm31.Reset();
				tfbsm31.MergeFrom(data_table->Rows[i]);
				tfbsm31["REC_CREATOR"] = s.userid;
				tfbsm31["REC_CREATE_TIME"] = datetime;
				Log::Trace("", "", "linke = {0}", __LINE__);
				tfbsm31["FACTORY_DIV"] = "LG1";
				tqmts0x["ST_NO"] = tfbsm31["ST_NO"].ToString();
				if (tqmts0x.QueryCount("ST_NO") == 0)
				{
					strcpy(s.msg, "出钢记号:" + tqmts0x["ST_NO"].ToString() + "输入错误!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tfbsm31["DATE_C"] = tfbsm31["DATE_TIME"].ToString().SubstringNE(2, 6);
				Log::Trace("", "", "linke = {0}", __LINE__);
				tfbsm31["DATE_TIME"] = tfbsm31["DATE_TIME"].ToString().SubstringNE(0, 8);

				// 1. 查询倒罐站前一天铁水Si含量（TQMTS24表，HEAT_NO以H开头）
				// 计算前一天的起止时间
				yesterday_start = CDateTime::Now().AddDays(-1).ToString("yyyyMMdd") + "000000";
				yesterday_end = CDateTime::Now().ToString("yyyyMMdd") + "000000";

				sqlstr = "SELECT AVG(ELM_002) FROM TQMTS24 "
						 "WHERE HEAT_NO LIKE 'H%' "
						 "AND REC_CREATE_TIME >= @START_TIME "
						 "AND REC_CREATE_TIME < @END_TIME";

				cmd_check.Parameters.Set("START_TIME", yesterday_start);
				cmd_check.Parameters.Set("END_TIME", yesterday_end);
				cmd_check.SetCommandText(sqlstr);
				cmd_check.ExecuteReader();

				if (cmd_check.Read())
				{
					si_content = cmd_check.GetDecimal(1); // 获取原始值，如0.004（表示0.4%）
				}
				cmd_check.Close();

				si_percent = si_content * 100;

				// 3. 根据Si含量判断TAPPING_WT
				// 规则：Si<=40（0.4%）→25t；40<Si<=60（0.4%-0.6%）→30t；Si>60（0.6%）→35t
				if (si_percent <= 0)
				{
					Log::Trace("", __FUNCTION__, "未查询到前一天铁水Si含量，使用默认值25t");
					tapping_wt_value = "25"; // 查询失败时的默认值
				}
				else if (si_percent <= 40)
				{
					tapping_wt_value = "25";
				}
				else if (si_percent <= 60)
				{
					tapping_wt_value = "30";
				}
				else
				{
					tapping_wt_value = "35";
				}

				// 4. 前一天配比模式比对逻辑
				sqlstr = " SELECT AVG(TOTAL_WEIGHT) as AVG_TOTAL_WEIGHT "
						 " FROM ( "
						 "   SELECT "
						 "     COALESCE(SUM(m2a.DEVO_WT), 0) as TOTAL_WEIGHT "
						 " FROM TMMSM21 m21 "
						 "  LEFT JOIN TMMSM2A_YL m2a "
						 "     ON m21.HEAT_NO = m2a.HEAT_NO "
						 " LEFT JOIN TMMSM50 m50 "
						 "     ON m2a.MAT_CODE = m50.MAT_CODE "
						 " WHERE m21.ST_NO LIKE '2%' "
						 "     AND m21.START_TIME >= @START_TIME "
						 "      AND m21.START_TIME <= @END_TIME "
						 "      AND (m50.MAT_TYPE IS NULL OR m50.MAT_TYPE <> '3') "
						 "    GROUP BY m21.HEAT_NO "
						 " ) t ";

				cmd_check.Parameters.Set("START_TIME", yesterday_start);
				cmd_check.Parameters.Set("END_TIME", yesterday_end);
				cmd_check.SetCommandText(sqlstr);
				cmd_check.ExecuteReader();

				if (cmd_check.Read())
				{
					prev_tapping_avg = cmd_check.GetDecimal(1);
				}

				cmd_check.Close();

				// 3. 根据前一天的配料模式判断TAPPING_WT
				// 规则：S25、30、35，正负2.5归类。
				if (prev_tapping_avg <= 0)
				{
					Log::Trace("", __FUNCTION__, "未查询到前一天平均配料值，使用默认值30t");
					prev_tapping_mode = "30"; // 查询失败时的默认值
				}
				else if (prev_tapping_avg <= 27.5)
				{
					prev_tapping_mode = "25";
				}
				else if (prev_tapping_avg <= 32.5)
				{
					prev_tapping_mode = "30";
				}
				else if (prev_tapping_avg <= 37.5)
				{
					prev_tapping_mode = "35";
				}
				else
				{
					Log::Trace("", __FUNCTION__, "前一天平均配料值为[{0}],不在可处理数据范围内，使用默认值30t", prev_tapping_avg.ToString());
					prev_tapping_mode = "30"; // 查询失败时的默认值
				}

				if (prev_tapping_mode.Trim() != "" && tapping_wt_value != prev_tapping_mode)
				{
					Log::Trace("", __FUNCTION__, "Si含量对应模式[{0}]与前一天模式[{1}]不符，采用前一天模式",
							   tapping_wt_value, prev_tapping_mode);
					tapping_wt_value = prev_tapping_mode; // 以前一天为准
				}

				Log::Trace("", __FUNCTION__, "最终TAPPING_WT=[{0}]，Si含量=[{1}]",
						   tapping_wt_value, si_percent.ToString());

				tfbsm31["TAPPING_WT"] = tapping_wt_value;

				int ls = tfbsm31["FURNACE_COUNT"].ToDecimal().ToInt32();

				if (ls == 0)
				{
					ls = 1;
				}
				for (int j = 1; j <= ls; j++)
				{
					sqlstr = " select COUNT(*)+1  from tfbsm31 where ST_NO='" + tfbsm31["ST_NO"].ToString() + "' and DATE_C='" + tfbsm31["DATE_C"].ToString() + "' ";
					cmd.SetCommandText(sqlstr);
					cmd.ExecuteReader();
					if (cmd.Read())
					{
						tfbsm31["SEQ_NO"] = cmd.GetString(1);
					}
					else
					{
						tfbsm31["SEQ_NO"] = "1";
					}
					cmd.Close();
					tfbsm31["SEQ_NOW"] = j;
					tfbsm31.Insert();
				}
			}
			else
			{
				Log::Trace("", __FUNCTION__, "新增，不锈钢");
				tfbsm12.Reset();
				tfbsm12.MergeFrom(data_table->Rows[i]);
				tfbsm12["REC_CREATOR"] = s.userid;
				tfbsm12["REC_CREATE_TIME"] = datetime;
				Log::Trace("", "", "linke = {0}", __LINE__);
				tfbsm12["FACTORY_DIV"] = "LG1";
				tqmts0x["ST_NO"] = tfbsm12["ST_NO"].ToString();
				if (tqmts0x.QueryCount("ST_NO") == 0)
				{
					strcpy(s.msg, "出钢记号:" + tqmts0x["ST_NO"].ToString() + "输入错误!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				if (tfbsm12["BACKLOG_EA"].ToString().GetLength() != 2)
				{
					strcpy(s.msg, "输入的工艺路径：" + tfbsm12["BACKLOG_EA"].ToString() + "错误，格式应该是两位!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				CString backlog_ea_ins = tfbsm12["BACKLOG_EA"].ToString();
				if (backlog_ea_ins != "06" && backlog_ea_ins != "07" && backlog_ea_ins != "08")
				{
					// 校验：检查tfbsm05是否存在该钢种+工艺路线

					sqlstr = " select COMM_FMLY_CODE,MATERIAL_CODE from TFBSM11 where ST_NO='" + tfbsm12["ST_NO"].ToString() + "' ";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					comm_fmly_code = cmd.GetString(1);
					material_code = cmd.GetString(2);
				}
				cmd.Close();

				// 确定t05查询使用的ST_NO：先查当前，没有则中类代表，再没有则大类代表
				CString t05_st_no = tfbsm12["ST_NO"].ToString();
				sqlstr = "SELECT COUNT(*) FROM TFBSM05 WHERE ST_NO = '" + t05_st_no + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					if (cmd.GetString(1) == "0")
					{
						cmd.Close();
						// 找中类代表
						sqlstr = "SELECT ST_NO FROM TFBSM11 WHERE MATERIAL_CODE = '" + material_code + "' AND MARK_POS_CODE = '1'";
						cmd.SetCommandText(sqlstr);
						cmd.ExecuteReader();
						if (cmd.Read())
						{
							t05_st_no = cmd.GetString(1);
						}
						else
						{
							cmd.Close();
							// 找大类代表
							sqlstr = "SELECT ST_NO FROM TFBSM11 WHERE COMM_FMLY_CODE = '" + comm_fmly_code + "' AND MARK_POS_CODE = '2'";
							cmd.SetCommandText(sqlstr);
							cmd.ExecuteReader();
							if (cmd.Read())
							{
								t05_st_no = cmd.GetString(1);
							}
						}
					}
				}
				cmd.Close();

				sqlstr = "SELECT COUNT(*) FROM tfbsm05 WHERE ST_NO='" + t05_st_no + "' AND BACKLOG_EA='" + tfbsm12["BACKLOG_EA"].ToString() + "'";
				Log::Trace("", "", "校验工艺路线sqlstr = [{0}]", sqlstr);
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					if (cmd.GetDecimal(1) == 0)
					{
						tfbsm12["REMARK_PS"] = tfbsm12["REMARK_PS"].ToString() + "【出钢记号:" + tfbsm12["ST_NO"].ToString() + "没有[" + tfbsm12["BACKLOG_EA"].ToString() + "]工艺路线!!!】";
					}
				}
				cmd.Close();
				}

				if (tfbsm12["REMARK_PS"].ToString() == "")
				{
					tfbsm12["REMARK_PS"] = " ";
				}

				tfbsm12["DATE_C"] = tfbsm12["DATE_TIME"].ToString().SubstringNE(2, 6);
				Log::Trace("", "", "linke = {0}", __LINE__);
				tfbsm12["DATE_TIME"] = tfbsm12["DATE_TIME"].ToString().SubstringNE(0, 8);
				int ls = tfbsm12["FURNACE_COUNT"].ToDecimal().ToInt32();
				sqlstr = "select MATERIAL_CODE from tfbsm11 where ST_NO='" + tfbsm12["ST_NO"].ToString() + "'";
				Log::Trace("", "", "linke = {0}", __LINE__);
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					tfbsm12["MATERIAL_CODE"] = cmd.GetString(1);
				}
				cmd.Close();

				if (ls == 0)
				{
					ls = 1;
				}
				for (int j = 1; j <= ls; j++)
				{
					sqlstr = " select COUNT(*)+1  from tfbsm12 where ST_NO='" + tfbsm12["ST_NO"].ToString() + "' and DATE_C='" + tfbsm12["DATE_C"].ToString() + "' and BACKLOG_EA='" + tfbsm12["BACKLOG_EA"].ToString() + "' ";
					cmd.SetCommandText(sqlstr);
					cmd.ExecuteReader();
					if (cmd.Read())
					{
						tfbsm12["SEQ_NO"] = cmd.GetString(1);
					}
					else
					{
						tfbsm12["SEQ_NO"] = "1";
					}
					cmd.Close();
					tfbsm12["SEQ_NOW"] = j;
					tfbsm12.TrimOrBlank();
					tfbsm12.Insert();
				}
			}
		}
		/*新增结束*/
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
int f_fbsm12_upd(CDataTable *data_table, EIClass *bcls_ret, CDbConnection *conn) // 修改
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString stNo = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString comm_fmly_code = "";
	CString material_code = "";
	// 程序公共CDbCommand
	CDbCommand cmd(conn);
	CModel tfbsm12("TFBSM12");
	CModel tfbsm31("TFBSM31");
	try
	{
		Log::Trace("", "", "update start");
		//-----------------------------------------------
		// 获取要操作的行前台传入的行数
		int count = data_table->Rows.get_Count();
		/*循环压入数据表行数*/
		for (int i = 0; i < count; i++)
		{
			stNo = data_table->Rows[i]["ST_NO"].ToString().Trim();
			if (stNo.Trim() == "")
			{
				strcpy(s.msg, "有数据出钢记号为空!!!");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (stNo.Substring(0, 1) == "2" || stNo.Substring(0, 1) == "3") // 碳钢
			{

				tfbsm31.Reset();
				tfbsm31.MergeFrom(data_table->Rows[i]);
				tfbsm31["REC_REVISOR"] = s.userid;
				sqlstr = " update tfbsm31 set REC_REVISOR='" + tfbsm31["REC_REVISOR"].ToString() + "',REC_REVISE_TIME='" + datetime + "',BACKLOG_EA='" + tfbsm31["BACKLOG_EA"].ToString() + "',REMARK_PS='" + tfbsm31["REMARK_PS"].ToString() + "' where DATE_C='" + tfbsm31["DATE_C"].ToString() + "' and FURNACE_COUNT='" + tfbsm31["FURNACE_COUNT"].ToString() + "' and ST_NO='" + tfbsm31["ST_NO"].ToString() + "' ";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
			}
			else
			{
				tfbsm12.Reset();
				tfbsm12.MergeFrom(data_table->Rows[i]);
				tfbsm12["REC_REVISOR"] = s.userid;
				tfbsm12["DATE_C"] = tfbsm12["DATE_TIME"].ToString().SubstringNE(2, 6);
				tfbsm12["DATE_TIME"] = tfbsm12["DATE_TIME"].ToString().SubstringNE(0, 8);

				// 清除REMARK_PS中旧的工艺路线提示信息
				CString remark_ps = tfbsm12["REMARK_PS"].ToString();
				CString prefix = "【出钢记号:";
				CString suffix = "工艺路线!!!】";
				int pos_start = remark_ps.Find(prefix);
				while (pos_start >= 0)
				{
					int pos_end = remark_ps.Find(suffix, pos_start);
					if (pos_end >= 0)
					{
						pos_end += suffix.GetLength();
						remark_ps = remark_ps.SubstringNE(0, pos_start) + remark_ps.SubstringNE(pos_end, remark_ps.GetLength() - pos_end);
						pos_start = remark_ps.Find(prefix);
					}
					else
					{
						break;
					}
				}

				CString backlog_ea_upd = tfbsm12["BACKLOG_EA"].ToString();
				if (backlog_ea_upd != "06" && backlog_ea_upd != "07" && backlog_ea_upd != "08")
				{
				// 校验：检查tfbsm05是否存在该钢种+工艺路线

				sqlstr = " select COMM_FMLY_CODE,MATERIAL_CODE from TFBSM11 where ST_NO='" + tfbsm12["ST_NO"].ToString() + "' ";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					comm_fmly_code = cmd.GetString(1);
					material_code = cmd.GetString(2);
				}
				cmd.Close();

				// 确定t05查询使用的ST_NO：先查当前，没有则中类代表，再没有则大类代表
				CString t05_st_no = tfbsm12["ST_NO"].ToString();
				sqlstr = "SELECT COUNT(*) FROM TFBSM05 WHERE ST_NO = '" + t05_st_no + "'";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					if (cmd.GetString(1) == "0")
					{
						cmd.Close();
						// 找中类代表
						sqlstr = "SELECT ST_NO FROM TFBSM11 WHERE MATERIAL_CODE = '" + material_code + "' AND MARK_POS_CODE = '1'";
						cmd.SetCommandText(sqlstr);
						cmd.ExecuteReader();
						if (cmd.Read())
						{
							t05_st_no = cmd.GetString(1);
						}
						else
						{
							cmd.Close();
							// 找大类代表
							sqlstr = "SELECT ST_NO FROM TFBSM11 WHERE COMM_FMLY_CODE = '" + comm_fmly_code + "' AND MARK_POS_CODE = '2'";
							cmd.SetCommandText(sqlstr);
							cmd.ExecuteReader();
							if (cmd.Read())
							{
								t05_st_no = cmd.GetString(1);
							}
						}
					}
				}
				cmd.Close();

				sqlstr = "SELECT COUNT(*) FROM tfbsm05 WHERE ST_NO='" + t05_st_no + "' AND BACKLOG_EA='" + tfbsm12["BACKLOG_EA"].ToString() + "'";
				Log::Trace("", "", "校验工艺路线sqlstr = [{0}]", sqlstr);
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					if (cmd.GetDecimal(1) == 0)
					{
						remark_ps = remark_ps + "【出钢记号:" + tfbsm12["ST_NO"].ToString() + "没有[" + tfbsm12["BACKLOG_EA"].ToString() + "]工艺路线!!!】";
					}
				}
				cmd.Close();
				}
				tfbsm12["REMARK_PS"] = remark_ps;

				if (tfbsm12["REMARK_PS"].ToString() == "")
				{
					tfbsm12["REMARK_PS"] = " ";
				}

				sqlstr = " update tfbsm12 set REC_REVISOR='" + tfbsm12["REC_REVISOR"].ToString() + "',REC_REVISE_TIME='" + datetime + "',BACKLOG_EA='" + tfbsm12["BACKLOG_EA"].ToString() + "',REMARK_PS='" + tfbsm12["REMARK_PS"].ToString() + "' where DATE_C='" + tfbsm12["DATE_C"].ToString() + "' and FURNACE_COUNT='" + tfbsm12["FURNACE_COUNT"].ToString() + "' and ST_NO='" + tfbsm12["ST_NO"].ToString() + "' and CC_MACH_NO='" + tfbsm12["CC_MACH_NO"].ToString() + "' ";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
			}
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
int f_fbsm12_del(CDataTable *data_table, EIClass *bcls_ret, CDbConnection *conn) // 删除
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString stNo = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	// 程序公共CDbCommand
	CDbCommand cmd(conn);
	CModel tfbsm12("TFBSM12");
	CModel tfbsm31("TFBSM31");
	try
	{
		// 获取要操作的行前台传入的行数
		int count = data_table->Rows.get_Count();
		/*循环压入数据表行数*/
		for (int i = 0; i < count; i++)
		{

			Log::Trace("", "", "delte start");
			stNo = data_table->Rows[i]["ST_NO"].ToString().Trim();
			if (stNo.Trim() == "")
			{
				strcpy(s.msg, "有数据出钢记号为空!!!");
				throw CApplicationException(-1, s.msg, log.Location);
			}
			if (stNo.Substring(0, 1) == "2" || stNo.Substring(0, 1) == "3") // 碳钢
			{
				tfbsm31.Reset();
				tfbsm31.MergeFrom(data_table->Rows[i]);
				sqlstr = " delete from tfbsm31   where DATE_C='" + tfbsm31["DATE_C"].ToString() + "' and FURNACE_COUNT='" + tfbsm31["FURNACE_COUNT"].ToString() + "' and ST_NO='" + tfbsm31["ST_NO"].ToString() + "' ";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();

				// 新增：删除tfbsm32表数据，匹配ST_NO、DATE_C两个字段
				/*Log::Trace("", "", "delete tfbsm32 start");
				sqlstr = " delete from tfbsm32 where ST_NO='" + tfbsm31["ST_NO"].ToString() + "' and DATE_C='" + tfbsm31["DATE_C"].ToString() + "' ";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();*/

				/*if (data_table->Rows[i][""].ToString().Trim() != ""){

				}*/
			}
			else
			{
				tfbsm12.Reset();
				tfbsm12.MergeFrom(data_table->Rows[i]);
				tfbsm12["DATE_C"] = tfbsm12["DATE_TIME"].ToString().SubstringNE(2, 6);
				tfbsm12["DATE_TIME"] = tfbsm12["DATE_TIME"].ToString().SubstringNE(0, 8);
				Log::Trace("", "", "DATE_C = {0}", tfbsm12["DATE_C"].ToString());
				Log::Trace("", "", "DATE_TIME = {0}", tfbsm12["DATE_TIME"].ToString());
				sqlstr = " delete from tfbsm12   where DATE_C='" + tfbsm12["DATE_C"].ToString() + "' and FURNACE_COUNT='" + tfbsm12["FURNACE_COUNT"].ToString() + "' and ST_NO='" + tfbsm12["ST_NO"].ToString() + "' and CC_MACH_NO='" + tfbsm12["CC_MACH_NO"].ToString() + "' ";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();
				Log::Trace("", __FUNCTION__, "delete.sqlstr = [{0}]", sqlstr);

				// 新增：删除tfbsm14表数据，匹配ST_NO、BACKLOG_EA、DATE_C三个字段
				/*Log::Trace("", "", "delete tfbsm14 start");
				sqlstr = " delete from tfbsm14 where ST_NO='" + tfbsm12["ST_NO"].ToString() + "' and BACKLOG_EA='" + tfbsm12["BACKLOG_EA"].ToString() + "' and DATE_C='" + tfbsm12["DATE_C"].ToString() + "' ";
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteNonQuery();
				cmd.Close();*/

				/*if (data_table->Rows[i][""].ToString().Trim() != ""){

				}*/
			}
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

int f_fbsm12_save(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	try
	{
		for (size_t i = 0; i < bcls_rec->Tables.get_Count(); i++)
		{
			CString table_name = bcls_rec->Tables[i].get_TableName();
			int position = table_name.Find("_") + 1;
			Log::Trace("", "", "position = {0}", position);
			Log::Trace("", "", "table_name = {0}", table_name);
			Log::Trace("", "", "s = {0}", table_name.Substring(position, table_name.GetLength() - position));
			if (position > 1 && table_name.Substring(position, table_name.GetLength() - position) == "ADD")
			{
				// bcls_rec->Tables[i].set_TableName(table_name.Substring(0, position - 1));
				doFlag = f_fbsm12_ins(&bcls_rec->Tables[i], bcls_ret, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else if (position > 1 && table_name.Substring(position, table_name.GetLength() - position) == "MODIFY")
			{
				// bcls_rec->Tables[i].set_TableName(table_name.Substring(0, position - 1));
				doFlag = f_fbsm12_upd(&bcls_rec->Tables[i], bcls_ret, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else if (position > 1 && table_name.Substring(position, table_name.GetLength() - position) == "DELETE")
			{
				// bcls_rec->Tables[i].set_TableName(table_name.Substring(0, position - 1));
				doFlag = f_fbsm12_del(&bcls_rec->Tables[i], bcls_ret, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
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
