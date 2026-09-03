/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: H5批量复制 - 配料单数据复制
**************************************************/
#include "stdafx.h"

BM2_FUNCTION_EXPORT

// ==========================================
// 位置A-1：函数签名增加4个目标参数
// ==========================================
int f_fbsm01_copy_insert(CDataTable *data_table, EIClass *bcls_ret, CDbConnection *conn,
						 CString comm_fmly_code, CString material_code, CString st_no, CString backlog_ea, CString table_id)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd(conn);
	CDbCommand cmd_inq(conn);
	CModel tfbsm01("TFBSM01");
	CModel tfbsm05("TFBSM05");
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString g_ctxInsertInfo = ""; // 记录当前正在插入的上下文，供 catch 使用

	try
	{
		Log::Trace("", "", "1 start");
		Log::Trace("", "", "进入复制函数 f_fbsm01_copy_insert");
		Log::Trace("", "", "前台传入数据行数 = {0}", data_table->Rows.get_Count());

		Log::Trace("", "", "目标COMM_FMLY_CODE=" + comm_fmly_code);
		Log::Trace("", "", "目标MATERIAL_CODE=" + material_code);
		Log::Trace("", "", "目标ST_NO=" + st_no);
		Log::Trace("", "", "目标BACKLOG_EA=" + backlog_ea);
		Log::Trace("", "", "目标TABLE_ID=" + table_id);

		// 3. 获取需要复制的源数据行数
		int count = data_table->Rows.get_Count();

		// ==========================================
		// 位置A-3：循环每一行，所有行都是源物料
		// ==========================================
		if (table_id == "tfbsm01")
		{
			for (int i = 0; i < count; i++)
			{
				tfbsm01.Reset();
				tfbsm01.MergeFrom(data_table->Rows[i]);

				// 取当前源物料的 MAT_CODE，用于报错定位
				CString src_mat_code = data_table->Rows[i]["MAT_CODE"].ToString().TrimOrBlank();

				tfbsm01["COMM_FMLY_CODE"] = comm_fmly_code;
				tfbsm01["MATERIAL_CODE"] = material_code;
				tfbsm01["ST_NO"] = st_no;
				tfbsm01["BACKLOG_EA"] = backlog_ea;

				tfbsm01["REC_CREATOR"] = s.userid;
				tfbsm01["REC_CREATE_TIME"] = datetime;
				tfbsm01["FACTORY_DIV"] = "LG1";
				tfbsm01["MARK_POS_CODE"] = " ";

				sqlstr = "SELECT MARK_POS_CODE FROM TFBSM11 WHERE ST_NO = '" + st_no + "'";
				cmd_inq.SetCommandText(sqlstr);
				Log::Trace("", "", "查询大类sqlstr = {0}", sqlstr);
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tfbsm01["MARK_POS_CODE"] = cmd_inq.GetString(1);
				}
				cmd_inq.Close();

				// 5. 循环处理 E/Z/A/D/B 工序
				// 5. E
				if (data_table->Rows[i]["E_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["E_STATION_ID"].ToString().Trim() != "0")
				{
					tfbsm01["STATION_ID"] = "E";
					g_ctxInsertInfo = "ST_NO=[";
					g_ctxInsertInfo += st_no;
					g_ctxInsertInfo += "], MAT_CODE=[";
					g_ctxInsertInfo += src_mat_code;
					g_ctxInsertInfo += "], STATION_ID=[E]";
					if (tfbsm01.QueryCount("ST_NO,STATION_ID,MAT_CODE,BACKLOG_EA") > 0)
					{
						Log::Trace("", "", "记录已存在，跳过: " + g_ctxInsertInfo);
					}
					else
					{
						tfbsm01.Insert();
					}
				}
				// 5. Z
				if (data_table->Rows[i]["Z_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["Z_STATION_ID"].ToString().Trim() != "0")
				{
					tfbsm01["STATION_ID"] = "Z";
					g_ctxInsertInfo = "ST_NO=[";
					g_ctxInsertInfo += st_no;
					g_ctxInsertInfo += "], MAT_CODE=[";
					g_ctxInsertInfo += src_mat_code;
					g_ctxInsertInfo += "], STATION_ID=[Z]";
					if (tfbsm01.QueryCount("ST_NO,STATION_ID,MAT_CODE,BACKLOG_EA") > 0)
					{
						Log::Trace("", "", "记录已存在，跳过: " + g_ctxInsertInfo);
					}
					else
					{
						tfbsm01.Insert();
					}
				}
				// 5. A
				if (data_table->Rows[i]["A_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["A_STATION_ID"].ToString().Trim() != "0")
				{
					tfbsm01["STATION_ID"] = "A";
					g_ctxInsertInfo = "ST_NO=[";
					g_ctxInsertInfo += st_no;
					g_ctxInsertInfo += "], MAT_CODE=[";
					g_ctxInsertInfo += src_mat_code;
					g_ctxInsertInfo += "], STATION_ID=[A]";
					if (tfbsm01.QueryCount("ST_NO,STATION_ID,MAT_CODE,BACKLOG_EA") > 0)
					{
						Log::Trace("", "", "记录已存在，跳过: " + g_ctxInsertInfo);
					}
					else
					{
						tfbsm01.Insert();
					}
				}
				// 5. D
				if (data_table->Rows[i]["D_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["D_STATION_ID"].ToString().Trim() != "0")
				{
					tfbsm01["STATION_ID"] = "D";
					g_ctxInsertInfo = "ST_NO=[";
					g_ctxInsertInfo += st_no;
					g_ctxInsertInfo += "], MAT_CODE=[";
					g_ctxInsertInfo += src_mat_code;
					g_ctxInsertInfo += "], STATION_ID=[D]";
					if (tfbsm01.QueryCount("ST_NO,STATION_ID,MAT_CODE,BACKLOG_EA") > 0)
					{
						Log::Trace("", "", "记录已存在，跳过: " + g_ctxInsertInfo);
					}
					else
					{
						tfbsm01.Insert();
					}
				}
				// 5. B
				if (data_table->Rows[i]["B_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["B_STATION_ID"].ToString().Trim() != "0")
				{
					tfbsm01["STATION_ID"] = "B";
					g_ctxInsertInfo = "ST_NO=[";
					g_ctxInsertInfo += st_no;
					g_ctxInsertInfo += "], MAT_CODE=[";
					g_ctxInsertInfo += src_mat_code;
					g_ctxInsertInfo += "], STATION_ID=[B]";
					if (tfbsm01.QueryCount("ST_NO,STATION_ID,MAT_CODE,BACKLOG_EA") > 0)
					{
						Log::Trace("", "", "记录已存在，跳过: " + g_ctxInsertInfo);
					}
					else
					{
						tfbsm01.Insert();
					}
				}
			}
		}
		else if (table_id == "tfbsm05")
		{
			for (int i = 0; i < count; i++)
			{
				tfbsm05.Reset();
				tfbsm05.MergeFrom(data_table->Rows[i]);

				// 取当前源物料的 MAT_CODE，用于报错定位
				CString elm_name = data_table->Rows[i]["ELM_NAME"].ToString().TrimOrBlank();

				tfbsm05["COMM_FMLY_CODE"] = comm_fmly_code;
				tfbsm05["MATERIAL_CODE"] = material_code;
				tfbsm05["ST_NO"] = st_no;
				tfbsm05["BACKLOG_EA"] = backlog_ea;

				if (backlog_ea.Trim() == "")
				{
					strcpy(s.msg, "要修改工艺路线不得为空!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				tfbsm05["REC_CREATOR"] = s.userid;
				tfbsm05["REC_CREATE_TIME"] = datetime;
				tfbsm05["FACTORY_DIV"] = "LG1";
				tfbsm05["MARK_POS_CODE"] = " ";

				sqlstr = "SELECT MARK_POS_CODE FROM TFBSM11 WHERE ST_NO = '" + st_no + "'";
				cmd_inq.SetCommandText(sqlstr);
				Log::Trace("", "", "查询大类sqlstr = {0}", sqlstr);
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read())
				{
					tfbsm05["MARK_POS_CODE"] = cmd_inq.GetString(1);
				}
				cmd_inq.Close();

				g_ctxInsertInfo = "ST_NO=[";
				g_ctxInsertInfo += st_no;
				g_ctxInsertInfo += "], ELM_NAME=[";
				g_ctxInsertInfo += elm_name;
				g_ctxInsertInfo += "], STATION_ID=[";
				g_ctxInsertInfo += tfbsm05["FACTORY_DIV"].ToString();
				g_ctxInsertInfo += "]";
				if (tfbsm05.QueryCount("ST_NO,STATION_ID,ELM_NAME,BACKLOG_EA") > 0)
				{
					Log::Trace("", "", "记录已存在，跳过: " + g_ctxInsertInfo);
				}
				else
				{
					tfbsm05.Insert();
				}
			}
		}

		cmd.Close();
	}
	catch (CDbException &ex)
	{
		CFormattable arguments[] = {ex.GetCode(), ex.GetMsg()};
		CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);

		CString fullMsg;
		if (ex.GetCode() == 1)
		{
			// ORA-00001 唯一约束冲突 → 直接报"已存在"
			fullMsg = g_ctxInsertInfo + " 已存在，无法重复插入";
		}
		else
		{
			// 其他数据库错误保持原样
			fullMsg = g_ctxInsertInfo + " 时发生错误：" + s.msg;
		}
		strncpy(s.msg, fullMsg, sizeof(s.msg) - 1);

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

// 批量复制入口函数（完全同 f_fbsm01_save 风格）
int f_fbsm01_copy(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString table_id = " ";
	try
	{
		Log::Trace("", "", "进入 f_fbsm01_copy 入口函数");
		CString tblCount;
		tblCount.Format("前台传入表数量=%d", bcls_rec->Tables.get_Count());
		Log::Trace("", "", tblCount);

		// ==========================================
		// 位置B-1：固定找前台传入的两个表（不变）
		// ==========================================
		CDataTable *pSrcTable = NULL;	 // Table1：源物料（MAT_CODE等）
		CDataTable *pTargetTable = NULL; // Table2：目标钢种（ST_NO等）

		for (size_t i = 0; i < bcls_rec->Tables.get_Count(); i++)
		{
			CString table_name = bcls_rec->Tables[i].get_TableName();
			Log::Trace("F7_COPY", "表名 = ", table_name);

			if (table_name == "Table1")
			{
				pSrcTable = &bcls_rec->Tables[i];
			}
			else if (table_name == "Table2")
			{
				pTargetTable = &bcls_rec->Tables[i];
			}
			else if (table_name == "Table3")
			{
				if (bcls_rec->Tables[i].Rows[0]["TABLE_ID"].ToString().Trim() != "")
					table_id = bcls_rec->Tables[i].Rows[0]["TABLE_ID"].ToString().Trim();
				else
				{
					strcpy(s.msg, "要修改的表名未传入!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
		}

		// 校验两个表都必须存在
		if (pSrcTable == NULL || pTargetTable == NULL)
		{
			throw CApplicationException(-1, "缺少源物料表(Table1)或目标钢种表(Table2)", log.Location);
		}

		// ==========================================
		// 位置B-2：改为循环 Table2 的每一行（每个目标钢种）
		// ==========================================
		int targetCount = pTargetTable->Rows.get_Count();
		Log::Trace("", "", "目标钢种数量=" + targetCount);

		for (int t = 0; t < targetCount; t++)
		{
			CString comm_fmly_code = "";
			CString material_code = "";
			CString st_no = "";
			CString backlog_ea = " ";

			if (pTargetTable->Columns.Contains("COMM_FMLY_CODE"))
				comm_fmly_code = pTargetTable->Rows[t]["COMM_FMLY_CODE"].ToString().TrimOrBlank().ToUpper();
			if (pTargetTable->Columns.Contains("MATERIAL_CODE"))
				material_code = pTargetTable->Rows[t]["MATERIAL_CODE"].ToString().TrimOrBlank().ToUpper();
			if (pTargetTable->Columns.Contains("ST_NO"))
				st_no = pTargetTable->Rows[t]["ST_NO"].ToString().TrimOrBlank().ToUpper();
			if (pTargetTable->Columns.Contains("BACKLOG_EA"))
				backlog_ea = pTargetTable->Rows[t]["BACKLOG_EA"].ToString().TrimOrBlank().ToUpper();

			if (st_no.IsEmpty())
			{
				strcpy(s.msg, "第" + CConvert::ToString(t + 1) + "个目标钢种的ST_NO为空，无法复制");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			if (backlog_ea == "")
			{
				backlog_ea = " ";
			}

			// ==========================================
			// 位置B-3：对每个目标钢种，调用insert复制Table1全部物料
			// ==========================================
			doFlag = f_fbsm01_copy_insert(pSrcTable, bcls_ret, conn,
										  comm_fmly_code, material_code, st_no, backlog_ea, table_id);

			if (doFlag < 0)
			{
				CString errMsg;
				errMsg.Format("复制到第%d个目标钢种(%s)时失败", t + 1, st_no);
				throw CApplicationException(-1, errMsg + "：" + s.msg, log.Location);
			}
		}

		CString successMsg;
		successMsg.Format("全部完成，共复制到%d个目标钢种", targetCount);
		Log::Trace("", "", successMsg);
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