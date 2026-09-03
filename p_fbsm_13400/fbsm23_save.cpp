/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 每日配料规则维护保存
**************************************************/
#include "stdafx.h"

BM2F_ENTERACE(fbsm23_save)

int f_fbsm23_ins(CDataTable *data_table, EIClass *bcls_ret, CDbConnection *conn) // 新增
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd(conn);
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	try
	{
		/*获取前台传过来的数据条数*/
		int count = data_table->Rows.get_Count();
		/*循环压入数据库操作*/
		for (int i = 0; i < count; i++)
		{
			CString st_no = data_table->Rows[i]["ST_NO"].ToString().Trim();
			CString back_c2 = data_table->Rows[i]["BACK_C2"].ToString().Trim();
			CString upper_limit_value = data_table->Rows[i]["UPPER_LIMIT_VALUE"].ToString().Trim();
			CString lower_limit_value = data_table->Rows[i]["LOWER_LIMIT_VALUE"].ToString().Trim();
			CString must_do_flag = data_table->Rows[i]["MUST_DO_FLAG"].ToString().Trim();
			CString value_max = data_table->Rows[i]["VALUE_MAX"].ToString().Trim();
			CString value_min = data_table->Rows[i]["VALUE_MIN"].ToString().Trim();
			CString material_code = data_table->Rows[i]["MATERIAL_CODE"].ToString().Trim();

			if (st_no == "")
			{
				strcpy(s.msg, "ST_NO不能为空!!!");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/*单引号转义，防止SQL注入*/
			back_c2 = back_c2.Replace("'", "''");
			upper_limit_value = upper_limit_value.Replace("'", "''");
			lower_limit_value = lower_limit_value.Replace("'", "''");
			must_do_flag = must_do_flag.Replace("'", "''");
			value_max = value_max.Replace("'", "''");
			value_min = value_min.Replace("'", "''");
			material_code = material_code.Replace("'", "''");

			/* 先删除已存在的同主键记录，避免主键冲突 */
			sqlstr = "DELETE FROM TFBSM23 WHERE ST_NO='" + st_no + "' AND BACK_C2='" + back_c2 + "' AND UPPER_LIMIT_VALUE='" + upper_limit_value + "' AND LOWER_LIMIT_VALUE='" + lower_limit_value + "' ";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();

			sqlstr = "INSERT INTO TFBSM23 (ST_NO, BACK_C2, UPPER_LIMIT_VALUE, LOWER_LIMIT_VALUE, MUST_DO_FLAG, VALUE_MAX, VALUE_MIN, MATERIAL_CODE, REC_CREATOR, REC_CREATE_TIME) "
				"VALUES ('" + st_no + "', '" + back_c2 + "', '" + upper_limit_value + "', '" + lower_limit_value + "', '" + must_do_flag + "', '" + value_max + "', '" + value_min + "', '" + material_code + "', @rec_creator, @rec_create_time)";
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("rec_creator", s.userid);
			cmd.Parameters.Set("rec_create_time", datetime);
			cmd.ExecuteNonQuery();
			cmd.Close();
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

int f_fbsm23_upd(CDataTable *data_table, EIClass *bcls_ret, CDbConnection *conn) // 修改
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd(conn);
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	try
	{
		Log::Trace("", "", "update start");
		/*获取要更新的数据条数*/
		int count = data_table->Rows.get_Count();
		/*循环更新数据*/
		for (int i = 0; i < count; i++)
		{
			CString st_no = data_table->Rows[i]["ST_NO"].ToString().Trim();
			CString back_c2 = data_table->Rows[i]["BACK_C2"].ToString().Trim();
			CString upper_limit_value = data_table->Rows[i]["UPPER_LIMIT_VALUE"].ToString().Trim();
			CString lower_limit_value = data_table->Rows[i]["LOWER_LIMIT_VALUE"].ToString().Trim();
			CString must_do_flag = data_table->Rows[i]["MUST_DO_FLAG"].ToString().Trim();
			CString value_max = data_table->Rows[i]["VALUE_MAX"].ToString().Trim();
			CString value_min = data_table->Rows[i]["VALUE_MIN"].ToString().Trim();
			CString material_code = data_table->Rows[i]["MATERIAL_CODE"].ToString().Trim();

			if (st_no == "")
			{
				strcpy(s.msg, "ST_NO不能为空!!!");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/*单引号转义，防止SQL注入*/
			back_c2 = back_c2.Replace("'", "''");
			upper_limit_value = upper_limit_value.Replace("'", "''");
			lower_limit_value = lower_limit_value.Replace("'", "''");
			must_do_flag = must_do_flag.Replace("'", "''");
			value_max = value_max.Replace("'", "''");
			value_min = value_min.Replace("'", "''");
			material_code = material_code.Replace("'", "''");

			sqlstr = "UPDATE TFBSM23 SET UPPER_LIMIT_VALUE='" + upper_limit_value + "', LOWER_LIMIT_VALUE='" + lower_limit_value + "', MUST_DO_FLAG='" + must_do_flag + "', VALUE_MAX='" + value_max + "', VALUE_MIN='" + value_min + "', MATERIAL_CODE='" + material_code + "', REC_REVISOR=@rec_revisor, REC_REVISE_TIME=@rec_revise_time "
				"WHERE ST_NO='" + st_no + "' AND BACK_C2='" + back_c2 + "'";
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("rec_revisor", s.userid);
			cmd.Parameters.Set("rec_revise_time", datetime);
			cmd.ExecuteNonQuery();
			cmd.Close();
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

int f_fbsm23_del(CDataTable *data_table, EIClass *bcls_ret, CDbConnection *conn) // 删除
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd(conn);

	try
	{
		/*获取要删除的数据条数*/
		int count = data_table->Rows.get_Count();
		/*循环删除数据*/
		for (int i = 0; i < count; i++)
		{
			CString st_no = data_table->Rows[i]["ST_NO"].ToString().Trim();
			CString back_c2 = data_table->Rows[i]["BACK_C2"].ToString().Trim();

			if (st_no == "")
			{
				strcpy(s.msg, "ST_NO不能为空!!!");
				throw CApplicationException(-1, s.msg, log.Location);
			}

			/*单引号转义，防止SQL注入*/
			back_c2 = back_c2.Replace("'", "''");

			sqlstr = "DELETE FROM TFBSM23 WHERE ST_NO='" + st_no + "' AND BACK_C2='" + back_c2 + "'";
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteNonQuery();
			cmd.Close();
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

int f_fbsm23_save(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	try
	{
		for (size_t i = 0; i < bcls_rec->Tables.get_Count(); i++)
		{
			CString table_name = bcls_rec->Tables[i].get_TableName();
			int position = table_name.Find("_") + 1;
			if (position > 1 && table_name.Substring(position, table_name.GetLength() - position) == "ADD")
			{
				CString original_name = bcls_rec->Tables[i].get_TableName();
				bcls_rec->Tables[i].set_TableName(table_name.Substring(0, position - 1));
				doFlag = f_fbsm23_ins(&bcls_rec->Tables[i], bcls_ret, conn);
				bcls_rec->Tables[i].set_TableName(original_name);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else if (position > 1 && table_name.Substring(position, table_name.GetLength() - position) == "MODIFY")
			{
				CString original_name = bcls_rec->Tables[i].get_TableName();
				bcls_rec->Tables[i].set_TableName(table_name.Substring(0, position - 1));
				doFlag = f_fbsm23_upd(&bcls_rec->Tables[i], bcls_ret, conn);
				bcls_rec->Tables[i].set_TableName(original_name);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else if (position > 1 && table_name.Substring(position, table_name.GetLength() - position) == "DELETE")
			{
				CString original_name = bcls_rec->Tables[i].get_TableName();
				bcls_rec->Tables[i].set_TableName(table_name.Substring(0, position - 1));
				doFlag = f_fbsm23_del(&bcls_rec->Tables[i], bcls_ret, conn);
				bcls_rec->Tables[i].set_TableName(original_name);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
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
