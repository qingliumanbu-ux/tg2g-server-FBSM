/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 新增修改删除综合函数
**************************************************/
#include "stdafx.h"

BM2_FUNCTION_EXPORT

int f_fbsm12_remark_refresh(CDbConnection *conn);

int f_fbsm_ins(CDataTable *data_table, EIClass *bcls_ret, CDbConnection *conn) // 新增
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	try
	{
		/*新增*/
		// 程序公共CDbCommand
		CDbCommand cmd(conn);
		CDbCommand cmd_clear(conn);
		/* 申明新增语句 */
		CString sql_ins = "INSERT INTO T";
		/* 获取前台传入的窗体名称 */
		CString form_name = (CString)data_table->get_TableName();
		form_name = form_name.Substring(1,form_name.GetLength() - 1);
		/* 拼接sql语句 */
		sql_ins += form_name;
		Log::Trace("", "", "form_name = {0}", (const char *)form_name);
		/* 打印表名 */
		Log::Trace("", "", "table_name{0}", (const char *)form_name);
		sql_ins += " ( ";
		/* 获取压入表名称 */
		for (int j = 0; j < data_table->Columns.get_Count(); j++)
		{
			/* 拼接列名 */
			sql_ins += data_table->Columns[j].get_ColumnName();
			/* 判断拼接,若相等的的时候，则拼接此列 */
			if (j == data_table->Columns.get_Count() - 1)
			{
				sql_ins += " ) VALUES ( ";
			}
			else
			{
				sql_ins += ",";
			}
		}
		/* 打印SQL_INS语句 */
		Log::Trace("", "", "sql_ins{0}", (const char *)sql_ins);
		/*获取传入行数*/
		int count = data_table->Rows.get_Count();
		/*循环压入数据表行数*/
		for (int i = 0; i < count; i++)
		{
			/*申明sql值*/
			CString sql_value;
			/*循环列数*/
			for (int j = 0; j < data_table->Columns.get_Count(); j++)
			{
				if (data_table->Columns[j].get_ColumnName() == "REC_CREATOR")
				{
					sql_value += "@rec_creator";
				}
				else if (data_table->Columns[j].get_ColumnName() == "REC_REVISOR")
				{
					sql_value += "@rec_revisor";
				}
				else if (data_table->Columns[j].get_ColumnName() == "REC_CREATE_TIME")
				{
					sql_value += "@rec_create_time";
				}
				else if (data_table->Columns[j].get_ColumnName() == "REC_REVISE_TIME")
				{
					sql_value += "@rec_revise_time";
				}
				else if (data_table->Columns[j].get_ColumnName() == "VERSION")
				{
					sql_value += "0";
				}
				else if (data_table->Columns[j].get_ColumnName() == "SEND_TIME")
				{
					sql_value += "' '";
				}
				else if (data_table->Columns[j].get_ColumnName() == "SEND_MAKER")
				{
					sql_value += "' '";
				}
				else if (data_table->Columns[j].get_ColumnName() == "VALID_FLAG")
				{
					sql_value += "' '";
				}
				else if (data_table->Columns[j].get_ColumnName() == "FACTORY_DIV")
				{
					sql_value += "'LG1'";
				}
				else
				{
					if (((CString)data_table->Rows[i][j]).IsEmpty())
					{
						data_table->Rows[i][j] = " ";
					}
					sql_value += "'" + ((CString)data_table->Rows[i][j]).Replace("'", "''") + "'";
				}
				if (j == data_table->Columns.get_Count() - 1)
				{
					sql_value += " ) ";
				}
				else
				{
					sql_value += ",";
				}
			}
			/*拼接成完整的的sql语句*/
			CString sql = sql_ins + sql_value;
			Log::Trace("", "", "sql{0}", (const char *)sql);
			/* 打印sql语句 */
			/* 连接数据库，对在线表进行操作 */
			CDbCommand comm2(sql, conn);
			comm2.Parameters.Set("rec_creator", s.userid);
			comm2.Parameters.Set("rec_revisor", s.userid);
			comm2.Parameters.Set("rec_create_time", datetime);
			comm2.Parameters.Set("rec_revise_time", datetime);
			comm2.ExecuteNonQuery();

			// FBSM11修改：若设置了代表标记，需清空同组其他记录的代表标记
			if (form_name == "FBSM11" && data_table->Columns.Contains("MARK_POS_CODE"))
			{
				CString mark_pos_code = data_table->Rows[i]["MARK_POS_CODE"].ToString().TrimOrBlank();
				CString st_no = data_table->Rows[i]["ST_NO"].ToString().TrimOrBlank();
				if (mark_pos_code == "1")
				{
					CString material_code = data_table->Rows[i]["MATERIAL_CODE"].ToString().TrimOrBlank();
					CString sql_clear = "UPDATE T" + form_name + " SET MARK_POS_CODE = ' ' WHERE MATERIAL_CODE = '" + material_code + "' AND MARK_POS_CODE = '1' AND ST_NO != '" + st_no + "'";
					cmd_clear.SetCommandText(sql_clear);
					cmd_clear.ExecuteNonQuery();
					cmd_clear.Close();
				}
				else if (mark_pos_code == "2")
				{
					CString comm_fmly_code = data_table->Rows[i]["COMM_FMLY_CODE"].ToString().TrimOrBlank();
					CString sql_clear = "UPDATE T" + form_name + " SET MARK_POS_CODE = ' ' WHERE COMM_FMLY_CODE = '" + comm_fmly_code + "' AND MARK_POS_CODE = '2' AND ST_NO != '" + st_no + "'";
					cmd_clear.SetCommandText(sql_clear);
					cmd_clear.ExecuteNonQuery();
					cmd_clear.Close();
				}
			}
		}
		/*新增结束*/
		if (form_name == "FBSM05")
		{
			f_fbsm12_remark_refresh(conn);
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
int f_fbsm_upd(CDataTable *data_table, CDataTable *dt_key, EIClass *bcls_ret, CDbConnection *conn) // 修改
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	// 程序公共CDbCommand
	CDbCommand cmd(conn);
	CDbCommand cmd_clear(conn);
	try
	{
		Log::Trace("", "", "update start");
		//-----------------------------------------------
		// 获取要操作的行前台传入的行数
		int count = data_table->Rows.get_Count();
		CString form_name = (CString)data_table->get_TableName();
		form_name = form_name.Substring(1,form_name.GetLength() - 1);
		//-----------------------------------------------
		// 循环获取列名和列值，插入历史表
		for (int i = 0; i < count; i++)
		{
			CString sql_ins = "INSERT INTO H" + form_name + " ( ";
			// 要插入历史表的值
			CString ins_value = "";
			CString sql_where = " ";
			CString column_name = " ";
			// 循环获取列名
			for (int j = 0; j < data_table->Columns.get_Count(); j++)
			{
				if (data_table->Columns[j].get_ColumnName() == "DU_MAKER")
				{
					column_name += "@du_maker";
				}
				else if (data_table->Columns[j].get_ColumnName() == "DU_TIME")
				{
					column_name += "@du_time";
				}
				else if (data_table->Columns[j].get_ColumnName() == "DU_FLAG")
				{
					column_name += "'U'";
				}
				else
				{
					column_name += data_table->Columns[j].get_ColumnName();
				}
				if (j == data_table->Columns.get_Count() - 1)
				{
					column_name += " FROM T" + form_name;
				}
				else
				{
					column_name += ",";
				}

				sql_ins += data_table->Columns[j].get_ColumnName();
				if (j == data_table->Columns.get_Count() - 1)
				{
					sql_ins += " ) SELECT ";
				}
				else
				{
					sql_ins += ",";
				}
			}
			for (int k = 0; k < dt_key->Rows.get_Count(); k++)
			{
				if (0 == k)
				{
					sql_where += " WHERE " + dt_key->Rows[k]["COLNAME"].ToString().Trim() + "=@colName_" + CConvert::ToString(k);
				}
				else
				{
					sql_where += " AND " + dt_key->Rows[k]["COLNAME"].ToString().Trim() + "=@colName_" + CConvert::ToString(k);
				}
				cmd.Parameters.Set("colName_" + CConvert::ToString(k), (CString)data_table->Rows[i][dt_key->Rows[k]["COLNAME"].ToString().Trim()].ToString().Trim());
			}

			sqlstr = sql_ins + column_name + sql_where;
			Log::Trace("", "", "HIS_INSERT sqlstr{0}", sqlstr);
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("du_maker", s.userid);
			cmd.Parameters.Set("du_time", datetime);
			try
			{
				cmd.ExecuteNonQuery();
			}
			catch (CDbException &ex)
			{
				// 无历史表错误忽略，让开发者可以任意配置表，可以有历史表也可以没有历史表
				if (ex.GetCode() != -204 && ex.GetCode() != 942)
				{
					CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
					CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);
					CString str = sqlstr + "\r\n" + ex.GetMsg();
					strncpy(s.sysmsg, (const char *)str, sizeof(s.sysmsg) - 1);
					s.flag = -1;
					doFlag = -1;
				}
			}
			cmd.Close();
		}
		//-----------------------------------------------
		// 循环更新在线表
		for (int i = 0; i < count; i++)
		{
			// 插入历史表的SQL语句
			CString sql_upd = "UPDATE T" + form_name + " SET ";
			CString sql_where = " ";
			for (int j = 0; j < data_table->Columns.get_Count(); j++)
			{
				// 在线表数据修改sql拼接
				if (data_table->Columns[j].get_ColumnName() == "REC_REVISOR")
				{
					sql_upd += data_table->Columns[j].get_ColumnName() + " = @rec_revisor,";
				}
				else if (data_table->Columns[j].get_ColumnName() == "REC_REVISE_TIME")
				{
					sql_upd += data_table->Columns[j].get_ColumnName() + " = @rec_revise_time,";
				}
				else if (data_table->Columns[j].get_ColumnName() == "VERSION")
				{
					sql_upd += data_table->Columns[j].get_ColumnName() + " = version + 1,";
				}
				else if (data_table->Columns[j].get_ColumnName() == "SEND_TIME") // 更新SEND_TIME,SEND_MAKER,VALID_FLAG为空
				{
					sql_upd += data_table->Columns[j].get_ColumnName() + " = ' ',";
				}
				else if (data_table->Columns[j].get_ColumnName() == "SEND_MAKER") // 更新SEND_TIME,SEND_MAKER,VALID_FLAG为空
				{
					sql_upd += data_table->Columns[j].get_ColumnName() + " = ' ',";
				}
				else if (data_table->Columns[j].get_ColumnName() == "VALID_FLAG") // 更新SEND_TIME,SEND_MAKER,VALID_FLAG为空
				{
					sql_upd += data_table->Columns[j].get_ColumnName() + " = ' ',";
				}
				else
				{
					sql_upd += data_table->Columns[j].get_ColumnName() + " = '" + ((CString)data_table->Rows[i][j]).TrimOrBlank().Replace("'", "''") + "',";
				}
				// sql更新的条件根据主键更新
				if (j == data_table->Columns.get_Count() - 1)
				{
					// 截取字符串的最后一位的逗号
					sql_upd = sql_upd.Substring(0, sql_upd.GetLength() - 1);
				}
			}
			// 拼接where
			for (int k = 0; k < dt_key->Rows.get_Count(); k++)
			{
				// 第一步：初始化sql_where为空
				// sql_where = "";

				// 第二步：按表单分支处理
				if (form_name == "FBSM01") {
					Log::Trace("", "", "跳出去1{0}");
					sql_where = " where COMM_FMLY_CODE='" + data_table->Rows[i]["COMM_FMLY_CODE"].ToString() + "' and ST_NO='" + data_table->Rows[i]["ST_NO"].ToString() + "' and STATION_ID='" + data_table->Rows[i]["STATION_ID"].ToString() + "' and MAT_CODE='" + data_table->Rows[i]["MAT_CODE"].ToString() + "' and MATERIAL_CODE='" + data_table->Rows[i]["MATERIAL_CODE"].ToString() + "' ";
				}
				else if (form_name == "FBSM05") {
					Log::Trace("", "", "跳出去5{0}");
					sql_where = " where COMM_FMLY_CODE='" + data_table->Rows[i]["COMM_FMLY_CODE"].ToString() + "' and ST_NO='" + data_table->Rows[i]["ST_NO"].ToString() + "' and STATION_ID='" + data_table->Rows[i]["STATION_ID"].ToString() + "' and ELM_CODE='" + data_table->Rows[i]["ELM_CODE"].ToString() + "' and MATERIAL_CODE='" + data_table->Rows[i]["MATERIAL_CODE"].ToString() + "' and BACKLOG_EA='" + data_table->Rows[i]["BACKLOG_EA"].ToString() + "' ";
				}
				else if (form_name == "FBSM04") {
					Log::Trace("", "", "跳出去4{0}");
					sql_where = " where COMM_FMLY_CODE='" + data_table->Rows[i]["COMM_FMLY_CODE"].ToString() + "' and ST_NO='" + data_table->Rows[i]["ST_NO"].ToString() + "' and STATION_ID='" + data_table->Rows[i]["STATION_ID"].ToString() + "' and MATERIAL_CODE='" + data_table->Rows[i]["MATERIAL_CODE"].ToString() + "' and BACKLOG_EA='" + data_table->Rows[i]["BACKLOG_EA"].ToString() + "' ";
				}
				else if (form_name == "FBSM24") {
					Log::Trace("", "", "跳出去24{0}");
					sql_where = " where COMM_FMLY_CODE='" + data_table->Rows[i]["COMM_FMLY_CODE"].ToString() + "' and MARK_POS_CODE='" + data_table->Rows[i]["MARK_POS_CODE"].ToString() +  "' ";
				}
				else if (form_name == "FBSM09") {
					Log::Trace("", "", "跳出去9{0}");
					sql_where = " where COMM_FMLY_CODE='" + data_table->Rows[i]["COMM_FMLY_CODE"].ToString() + "' and ST_NO='" + data_table->Rows[i]["ST_NO"].ToString() + "' and MATERIAL_CODE='" + data_table->Rows[i]["MATERIAL_CODE"].ToString() + "' and BACKLOG_EA='" + data_table->Rows[i]["BACKLOG_EA"].ToString() + "' ";
				}
				else if (form_name == "FBSM20") {
					Log::Trace("", "", "跳出去20{0}");
					sql_where = " where COMM_FMLY_CODE='" + data_table->Rows[i]["COMM_FMLY_CODE"].ToString() +  "' and BACKLOG_EA='" + data_table->Rows[i]["BACKLOG_EA"].ToString() + "' ";
				}
				else if (form_name == "FBSM25") {
					Log::Trace("", "", "跳出去25{0}");
					sql_where = " where MAT_CODE='" + data_table->Rows[i]["MAT_CODE"].ToString() + "' and BACKLOG_EA='" + data_table->Rows[i]["BACKLOG_EA"].ToString().TrimOrBlank() + "' and IF_WT_CODE='" + data_table->Rows[i]["IF_WT_CODE"].ToString() + "' ";
				}
				else if (form_name == "FBSM06") {
					Log::Trace("", "", "跳出去06{0}");
					sql_where = " where ST_NO='" + data_table->Rows[i]["ST_NO"].ToString() + "' and BACKLOG_EA='" + data_table->Rows[i]["BACKLOG_EA"].ToString().TrimOrBlank() + "' and P_FLAG='" + data_table->Rows[i]["P_FLAG"].ToString() + "' ";
				}
				else {
					// 通用表单：动态拼接参数化where
					if (0 == k) {
						sql_where += " WHERE " + dt_key->Rows[k]["COLNAME"].ToString().Trim() + "=@colName_" + CConvert::ToString(k);
					}
					else {
						sql_where += " AND " + dt_key->Rows[k]["COLNAME"].ToString().Trim() + "=@colName_" + CConvert::ToString(k);
					}
					cmd.Parameters.Set("colName_" + CConvert::ToString(k), (CString)data_table->Rows[i][dt_key->Rows[k]["COLNAME"].ToString().Trim()].ToString().Trim());
				}
			}
			// 更新在线表
			sqlstr = sql_upd + sql_where;
			Log::Trace("", "", "UPDATE sql_upd{0}", sqlstr);
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("rec_revisor", s.userid);
			cmd.Parameters.Set("rec_revise_time", datetime);
			cmd.ExecuteNonQuery();
			cmd.Close();

			// FBSM11修改：若设置了代表标记，需清空同组其他记录的代表标记
			if (form_name == "FBSM11" && data_table->Columns.Contains("MARK_POS_CODE"))
			{
				CString mark_pos_code = data_table->Rows[i]["MARK_POS_CODE"].ToString().TrimOrBlank();
				CString st_no = data_table->Rows[i]["ST_NO"].ToString().TrimOrBlank();
				if (mark_pos_code == "1")
				{
					CString material_code = data_table->Rows[i]["MATERIAL_CODE"].ToString().TrimOrBlank();
					CString sql_clear = "UPDATE T" + form_name + " SET MARK_POS_CODE = ' ' WHERE MATERIAL_CODE = '" + material_code + "' AND MARK_POS_CODE = '1' AND ST_NO != '" + st_no + "'";
					cmd_clear.SetCommandText(sql_clear);
					cmd_clear.ExecuteNonQuery();
					cmd_clear.Close();
				}
				else if (mark_pos_code == "2")
				{
					CString comm_fmly_code = data_table->Rows[i]["COMM_FMLY_CODE"].ToString().TrimOrBlank();
					CString sql_clear = "UPDATE T" + form_name + " SET MARK_POS_CODE = ' ' WHERE COMM_FMLY_CODE = '" + comm_fmly_code + "' AND MARK_POS_CODE = '2' AND ST_NO != '" + st_no + "'";
					cmd_clear.SetCommandText(sql_clear);
					cmd_clear.ExecuteNonQuery();
					cmd_clear.Close();
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
int f_fbsm_del(CDataTable *data_table, CDataTable *dt_key, EIClass *bcls_ret, CDbConnection *conn) // 删除
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	try
	{
		// ----------------------------------------
		// 定义公共CDbCommand
		CDbCommand cmd(conn);
		// 获取要前台传入的值
		CString form_name = (CString)data_table->get_TableName();
		form_name = form_name.Substring(1,form_name.GetLength() - 1);
		// 新增历史表记录
		CString sql_ins = "INSERT INTO H" + form_name + " (";
		// 取得单行传入信息
		for (int j = 0; j < data_table->Columns.get_Count(); j++)
		{
			sql_ins += data_table->Columns[j].get_ColumnName();

			if (j == data_table->Columns.get_Count() - 1)
			{
				sql_ins += " ) VALUES ( ";
			}
			else
			{
				sql_ins += " , ";
			}
		}
		// 获取要删除的行数，逐行进行处理
		int count = data_table->Rows.get_Count();
		for (int i = 0; i < count; i++)
		{
			CString sql_value;
			// 删除SQL语句
			CString sql_del = "DELETE FROM T" + form_name + " WHERE 1 = 1 ";

			// 新增：FBSM01按指定主键精准拼接删除条件
			if (form_name == "FBSM05")
			{
				// 清空原有WHERE 1=1，按固定主键拼接精准删除条件
				sql_del = "DELETE FROM T" + form_name +
					" WHERE ELM_CODE='" + data_table->Rows[i]["ELM_CODE"].ToString().TrimOrBlank() + "'" +
					" AND STATION_ID='" + data_table->Rows[i]["STATION_ID"].ToString().TrimOrBlank() + "'" +
					" AND ST_NO='" + data_table->Rows[i]["ST_NO"].ToString().TrimOrBlank() + "'" +
					" AND BACKLOG_EA='" + data_table->Rows[i]["BACKLOG_EA"].ToString().TrimOrBlank() + "'" +
					" AND MATERIAL_CODE='" + data_table->Rows[i]["MATERIAL_CODE"].ToString().TrimOrBlank() + "'" +
					" AND COMM_FMLY_CODE='" + data_table->Rows[i]["COMM_FMLY_CODE"].ToString().TrimOrBlank() + "'";
			}
			else if (form_name == "FBSM04")
			{
				// 清空原有WHERE 1=1，按固定主键拼接精准删除条件
				sql_del = "DELETE FROM T" + form_name +
					" WHERE STATION_ID='" + data_table->Rows[i]["STATION_ID"].ToString().TrimOrBlank() + "'" +
					" AND ST_NO='" + data_table->Rows[i]["ST_NO"].ToString().TrimOrBlank() + "'" +
					" AND BACKLOG_EA='" + data_table->Rows[i]["BACKLOG_EA"].ToString().TrimOrBlank() + "'" +
					" AND MATERIAL_CODE='" + data_table->Rows[i]["MATERIAL_CODE"].ToString().TrimOrBlank() + "'" +
					" AND COMM_FMLY_CODE='" + data_table->Rows[i]["COMM_FMLY_CODE"].ToString().TrimOrBlank() + "'";
			}
			else if(form_name == "FBSM09")
			{
				// 清空原有WHERE 1=1，按固定主键拼接精准删除条件
				sql_del = "DELETE FROM T" + form_name +
					" WHERE ST_NO='" + data_table->Rows[i]["ST_NO"].ToString().TrimOrBlank() + "'" +
					" AND BACKLOG_EA='" + data_table->Rows[i]["BACKLOG_EA"].ToString().TrimOrBlank() + "'" +
					" AND MATERIAL_CODE='" + data_table->Rows[i]["MATERIAL_CODE"].ToString().TrimOrBlank() + "'" +
					" AND COMM_FMLY_CODE='" + data_table->Rows[i]["COMM_FMLY_CODE"].ToString().TrimOrBlank() + "'";
			}
			else if (form_name == "FBSM25")
			{
				// 清空原有WHERE 1=1，按固定主键拼接精准删除条件
				sql_del = "DELETE FROM T" + form_name +
					" WHERE MAT_CODE='" + data_table->Rows[i]["MAT_CODE"].ToString().TrimOrBlank() + "'" +
					" AND BACKLOG_EA='" + data_table->Rows[i]["BACKLOG_EA"].ToString().TrimOrBlank() + "'" +
					" AND IF_WT_CODE='" + data_table->Rows[i]["IF_WT_CODE"].ToString().TrimOrBlank() + "'";
			}
			else if (form_name == "FBSM24")
			{
				// 清空原有WHERE 1=1，按固定主键拼接精准删除条件
				sql_del = "DELETE FROM T" + form_name +
					" where COMM_FMLY_CODE='" + data_table->Rows[i]["COMM_FMLY_CODE"].ToString() + 
					"' and ST_NO='" + data_table->Rows[i]["ST_NO"].ToString() + 
					"' and MARK_POS_CODE='" + data_table->Rows[i]["MARK_POS_CODE"].ToString() + 
					"' and MATERIAL_CODE='" + data_table->Rows[i]["MATERIAL_CODE"].ToString() + 
					"' and UPPER_LIMIT_VALUE='" + data_table->Rows[i]["UPPER_LIMIT_VALUE"].ToString() + 
					"' and LOWER_LIMIT_VALUE='" + data_table->Rows[i]["LOWER_LIMIT_VALUE"].ToString() + 
					"' and MUST_DO_FLAG='" + data_table->Rows[i]["MUST_DO_FLAG"].ToString() + "' ";
			}
			else

			{
				// 其他表保留原有动态主键逻辑（原有代码）
				// 根据前台传入主键删除
				for (int k = 0; k < dt_key->Rows.get_Count(); k++)
				{
					if (data_table->Rows[i][dt_key->Rows[k]["COLNAME"].ToString().Trim()].ToString().Trim() != "")
					{
						sql_del += " AND " + dt_key->Rows[k]["COLNAME"].ToString().Trim() + "=@colName_" + CConvert::ToString(k);
						cmd.Parameters.Set("colName_" + CConvert::ToString(k), (CString)data_table->Rows[i][dt_key->Rows[k]["COLNAME"].ToString().Trim()].ToString().Trim());
					}
				}
			}

			// 先逐列新增入历史表
			for (int j = 0; j < data_table->Columns.get_Count(); j++)
			{
				if (data_table->Columns[j].get_ColumnName() == "DU_MAKER")
				{
					sql_value += "@du_maker";
				}
				else if (data_table->Columns[j].get_ColumnName() == "DU_TIME")
				{
					sql_value += "@du_time";
				}
				else if (data_table->Columns[j].get_ColumnName() == "DU_FLAG")
				{
					sql_value += "'D'";
				}
				else
				{
					sql_value += " @" + data_table->Columns[j].get_ColumnName();
					cmd.Parameters.Set(data_table->Columns[j].get_ColumnName(), (CString)data_table->Rows[i][j].ToString().TrimOrBlank());
				}

				if (j == data_table->Columns.get_Count() - 1)
				{
					sql_value += " ) ";
				}
				else
				{
					sql_value += " , ";
				}
			}
			sqlstr = sql_ins + sql_value;
			// 执行历史表新增操作
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("du_maker", s.userid);
			cmd.Parameters.Set("du_time", datetime);
			try
			{
				cmd.ExecuteNonQuery();
			}
			catch (CDbException &ex)
			{
				// 无历史表错误忽略，让开发者可以任意配置表，可以有历史表也可以没有历史表
				if (ex.GetCode() != -204 && ex.GetCode() != 942)
				{
					CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
					CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);
					CString str = sqlstr + "\r\n" + ex.GetMsg();
					strncpy(s.sysmsg, (const char *)str, sizeof(s.sysmsg) - 1);
					s.flag = -1;
					doFlag = -1;
				}
			}
			cmd.Close();
			// 根据前台传入主键删除
			for (int k = 0; k < dt_key->Rows.get_Count(); k++)
			{
				if (data_table->Rows[i][dt_key->Rows[k]["COLNAME"].ToString().Trim()].ToString().Trim() != "")
				{
					sql_del += " AND " + dt_key->Rows[k]["COLNAME"].ToString().Trim() + "=@colName_" + CConvert::ToString(k);
					cmd.Parameters.Set("colName_" + CConvert::ToString(k), (CString)data_table->Rows[i][dt_key->Rows[k]["COLNAME"].ToString().Trim()].ToString().Trim());
				}
			}
			// 执行在线表删除操作
			sqlstr = sql_del;
			cmd.SetCommandText(sqlstr);
			Log::Trace("", "", "sql[{0}][{1}]", sqlstr, data_table->Rows[i][dt_key->Rows[0]["COLNAME"].ToString().Trim()].ToString().Trim());
			cmd.ExecuteNonQuery();
		}
		if (form_name == "FBSM05")
		{
			f_fbsm12_remark_refresh(conn);
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

int f_fbsm12_remark_refresh(CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString date_c = CDateTime::Now().ToString("yyyyMMdd").SubstringNE(2, 6);
	CString userid = s.userid;

	CDbCommand cmd(conn);
	CDbCommand cmd_check(conn);
	CDbCommand cmd_upd(conn);

	try
	{
		sqlstr = "SELECT DATE_C, FURNACE_COUNT, ST_NO, CC_MACH_NO, BACKLOG_EA, REMARK_PS FROM TFBSM12 WHERE DATE_C = '" + date_c + "'";
		cmd.SetCommandText(sqlstr);
		cmd.ExecuteReader();

		while (cmd.Read())
		{
			CString row_date_c = cmd.GetString(1);
			CString row_furnace_count = cmd.GetString(2);
			CString row_st_no = cmd.GetString(3);
			CString row_cc_mach_no = cmd.GetString(4);
			CString row_backlog_ea = cmd.GetString(5);
			CString row_remark_ps = cmd.GetString(6);

			// 清除REMARK_PS中旧的工艺路线提示信息
			CString prefix = "【出钢记号:";
			CString suffix = "工艺路线!!!】";
			int pos_start = row_remark_ps.Find(prefix);
			while (pos_start >= 0)
			{
				int pos_end = row_remark_ps.Find(suffix, pos_start);
				if (pos_end >= 0)
				{
					pos_end += suffix.GetLength();
					row_remark_ps = row_remark_ps.SubstringNE(0, pos_start) + row_remark_ps.SubstringNE(pos_end, row_remark_ps.GetLength() - pos_end);
					pos_start = row_remark_ps.Find(prefix);
				}
				else
				{
					break;
				}
			}

			if (row_backlog_ea != "06" && row_backlog_ea != "07" && row_backlog_ea != "08")
			{
				// 校验：检查tfbsm05是否存在该钢种+工艺路线
				CString comm_fmly_code = "";
				CString material_code = "";

			sqlstr = " SELECT COMM_FMLY_CODE,MATERIAL_CODE FROM TFBSM11 WHERE ST_NO='" + row_st_no + "' ";
			cmd_check.SetCommandText(sqlstr);
			cmd_check.ExecuteReader();
			if (cmd_check.Read())
			{
				comm_fmly_code = cmd_check.GetString(1);
				material_code = cmd_check.GetString(2);
			}
			cmd_check.Close();

			// 确定t05查询使用的ST_NO：先查当前，没有则中类代表，再没有则大类代表
			CString t05_st_no = row_st_no;
			sqlstr = "SELECT COUNT(*) FROM TFBSM05 WHERE ST_NO = '" + t05_st_no + "'";
			cmd_check.SetCommandText(sqlstr);
			cmd_check.ExecuteReader();
			if (cmd_check.Read())
			{
				if (cmd_check.GetString(1) == "0")
				{
					cmd_check.Close();
					// 找中类代表
					sqlstr = "SELECT ST_NO FROM TFBSM11 WHERE MATERIAL_CODE = '" + material_code + "' AND MARK_POS_CODE = '1'";
					cmd_check.SetCommandText(sqlstr);
					cmd_check.ExecuteReader();
					if (cmd_check.Read())
					{
						t05_st_no = cmd_check.GetString(1);
					}
					else
					{
						cmd_check.Close();
						// 找大类代表
						sqlstr = "SELECT ST_NO FROM TFBSM11 WHERE COMM_FMLY_CODE = '" + comm_fmly_code + "' AND MARK_POS_CODE = '2'";
						cmd_check.SetCommandText(sqlstr);
						cmd_check.ExecuteReader();
						if (cmd_check.Read())
						{
							t05_st_no = cmd_check.GetString(1);
						}
					}
				}
			}
			cmd_check.Close();

			sqlstr = "SELECT COUNT(*) FROM tfbsm05 WHERE ST_NO='" + t05_st_no + "' AND BACKLOG_EA='" + row_backlog_ea + "'";
			cmd_check.SetCommandText(sqlstr);
			cmd_check.ExecuteReader();
			if (cmd_check.Read())
			{
				if (cmd_check.GetDecimal(1) == 0)
				{
					row_remark_ps = row_remark_ps + "【出钢记号:" + row_st_no + "没有[" + row_backlog_ea + "]工艺路线!!!】";
				}
			}
			cmd_check.Close();
			}

			// 更新tfbsm12
			sqlstr = " UPDATE TFBSM12 SET REC_REVISOR='" + userid + "',REC_REVISE_TIME='" + datetime + "',REMARK_PS='" + row_remark_ps + "' WHERE DATE_C='" + row_date_c + "' AND FURNACE_COUNT='" + row_furnace_count + "' AND ST_NO='" + row_st_no + "' AND CC_MACH_NO='" + row_cc_mach_no + "' ";
			cmd_upd.SetCommandText(sqlstr);
			cmd_upd.ExecuteNonQuery();
			cmd_upd.Close();
		}
		cmd.Close();
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

int f_fbsm_save(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
			if (position > 1 && table_name.Substring(position, table_name.GetLength() - position) == "ADD")
			{
				CString original_name = bcls_rec->Tables[i].get_TableName();
				bcls_rec->Tables[i].set_TableName(table_name.Substring(0, position - 1));
				doFlag = f_fbsm_ins(&bcls_rec->Tables[i], bcls_ret, conn);
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
				doFlag = f_fbsm_upd(&bcls_rec->Tables[i], &bcls_rec->Tables["DT_KEY"], bcls_ret, conn);
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
				doFlag = f_fbsm_del(&bcls_rec->Tables[i], &bcls_rec->Tables["DT_KEY"], bcls_ret, conn);
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
