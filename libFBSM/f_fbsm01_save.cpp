/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 新增修改删除综合函数
**************************************************/
#include "stdafx.h"

BM2_FUNCTION_EXPORT

int f_fbsm01_ins(CDataTable *data_table, EIClass *bcls_ret, CDbConnection *conn) // 新增
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	// 程序公共CDbCommand
	CDbCommand cmd(conn);
	CModel tfbsm01("TFBSM01");
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	try
	{
		/*新增*/
		Log::Trace("", "", "insert start");
		/*获取传入行数*/
		int count = data_table->Rows.get_Count();
		/*循环压入数据表行数*/
		for (int i = 0; i < count; i++)
		{
			tfbsm01.Reset();
			tfbsm01.MergeFrom(data_table->Rows[i]);
			tfbsm01["REC_CREATOR"] = s.userid;
			tfbsm01["REC_CREATE_TIME"] = datetime;
			//tfbsm01["FACTORY_DIV"] = "LG1";
			if (data_table->Rows[i]["E_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["E_STATION_ID"].ToString().Trim() != "0")
			{
				tfbsm01["STATION_ID"] = "E";
				tfbsm01.Insert();
			}
			if (data_table->Rows[i]["Z_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["Z_STATION_ID"].ToString().Trim() != "0")
			{
				tfbsm01["STATION_ID"] = "Z";
				tfbsm01.Insert();
			}
			if (data_table->Rows[i]["A_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["A_STATION_ID"].ToString().Trim() != "0")
			{
				tfbsm01["STATION_ID"] = "A";
				tfbsm01.Insert();
			}
			if (data_table->Rows[i]["D_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["D_STATION_ID"].ToString().Trim() != "0")
			{
				tfbsm01["STATION_ID"] = "D";
				tfbsm01.Insert();
			}
			if (data_table->Rows[i]["B_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["B_STATION_ID"].ToString().Trim() != "0")
			{
				tfbsm01["STATION_ID"] = "B";
				tfbsm01.Insert();
			}
		}
		cmd.Close();
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
int f_fbsm01_upd(CDataTable *data_table, CDataTable *dt_key, EIClass *bcls_ret, CDbConnection *conn) // 修改
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CModel tfbsm01("TFBSM01");
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	// 程序公共CDbCommand
	CDbCommand cmd(conn);
	try
	{
		Log::Trace("", "", "update start");
		//-----------------------------------------------
		// 获取要操作的行前台传入的行数
		int count = data_table->Rows.get_Count();
		for (int i = 0; i < count; i++)
		{
			tfbsm01.Reset();
			tfbsm01.MergeFrom(data_table->Rows[i]);
			tfbsm01["REC_REVISOR"] = s.userid;
			tfbsm01["REC_REVISE_TIME"] = datetime;
			if (tfbsm01["BACKLOG_EA"].ToString() == "")
				tfbsm01["BACKLOG_EA"] = " ";

			Log::Trace("", "", "Z_STATION_ID{0}", data_table->Rows[i]["Z_STATION_ID"].ToString().Trim());
			if (data_table->Rows[i]["E_STATION_ID"].ToString().Trim() == "" || data_table->Rows[i]["E_STATION_ID"].ToString().Trim() == "0")
			{
				tfbsm01["STATION_ID"] = "E";
				// 判断是否有数据
				if (tfbsm01.QueryCount("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA") > 0)
				{
					tfbsm01.Delete("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE,REC_CREATOR,REC_CREATE_TIME");
				}
			}
			if (data_table->Rows[i]["Z_STATION_ID"].ToString().Trim() == "" || data_table->Rows[i]["Z_STATION_ID"].ToString().Trim() == "0")
			{
				tfbsm01["STATION_ID"] = "Z";
				// 判断是否有数据
				if (tfbsm01.QueryCount("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA") > 0)
				{
					tfbsm01.Delete("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE,REC_CREATOR,REC_CREATE_TIME");
				}
			}
			if (data_table->Rows[i]["A_STATION_ID"].ToString().Trim() == "" || data_table->Rows[i]["A_STATION_ID"].ToString().Trim() == "0")
			{
				tfbsm01["STATION_ID"] = "A";
				// 判断是否有数据
				if (tfbsm01.QueryCount("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA") > 0)
				{
					tfbsm01.Delete("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE,REC_CREATOR,REC_CREATE_TIME");
				}
			}
			if (data_table->Rows[i]["D_STATION_ID"].ToString().Trim() == "" || data_table->Rows[i]["D_STATION_ID"].ToString().Trim() == "0")
			{
				tfbsm01["STATION_ID"] = "D";
				// 判断是否有数据
				if (tfbsm01.QueryCount("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA") > 0)
				{
					tfbsm01.Delete("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE,REC_CREATOR,REC_CREATE_TIME");
				}
			}
			if (data_table->Rows[i]["B_STATION_ID"].ToString().Trim() == "" || data_table->Rows[i]["B_STATION_ID"].ToString().Trim() == "0")
			{
				tfbsm01["STATION_ID"] = "B";
				// 判断是否有数据
				if (tfbsm01.QueryCount("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA") > 0)
				{
					tfbsm01.Delete("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE,REC_CREATOR,REC_CREATE_TIME");
				}
			}
			if (data_table->Rows[i]["E_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["E_STATION_ID"].ToString().Trim() != "0")
			{
				tfbsm01["STATION_ID"] = "E";
				if (tfbsm01.QueryCount("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA") > 0)
				{
					tfbsm01.Update("MAT_FAMILY_CODE,UPPER_LIMIT_VALUE,MUST_DO_FLAG,LOWER_LIMIT_VALUE,QUALITY_FLAS,DATE_C,REC_REVISOR,REC_REVISE_TIME,BACK_C2,BACKLOG_EA,MARK_POS_CODE", "STATION_ID,MAT_CODE,ST_NO,MATERIAL_CODE,COMM_FMLY_CODE,BACKLOG_EA");
				}
				else
				{
					tfbsm01.Insert();
				}
			}
			if (data_table->Rows[i]["Z_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["Z_STATION_ID"].ToString().Trim() != "0")
			{
				tfbsm01["STATION_ID"] = "Z";
				if (tfbsm01.QueryCount("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA") > 0)
				{
					tfbsm01.Update("MAT_FAMILY_CODE,UPPER_LIMIT_VALUE,MUST_DO_FLAG,LOWER_LIMIT_VALUE,QUALITY_FLAS,DATE_C,REC_REVISOR,REC_REVISE_TIME,BACK_C2,BACKLOG_EA,MARK_POS_CODE", "STATION_ID,MAT_CODE,ST_NO,MATERIAL_CODE,COMM_FMLY_CODE,BACKLOG_EA");
				}
				else
				{
					tfbsm01.Insert();
				}
			}
			if (data_table->Rows[i]["A_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["A_STATION_ID"].ToString().Trim() != "0")
			{
				tfbsm01["STATION_ID"] = "A";
				if (tfbsm01.QueryCount("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA") > 0)
				{
					tfbsm01.Update("MAT_FAMILY_CODE,UPPER_LIMIT_VALUE,MUST_DO_FLAG,LOWER_LIMIT_VALUE,QUALITY_FLAS,DATE_C,REC_REVISOR,REC_REVISE_TIME,BACK_C2,BACKLOG_EA,MARK_POS_CODE", "STATION_ID,MAT_CODE,ST_NO,MATERIAL_CODE,COMM_FMLY_CODE,BACKLOG_EA");
				}
				else
				{
					tfbsm01.Insert();
				}
			}
			if (data_table->Rows[i]["D_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["D_STATION_ID"].ToString().Trim() != "0")
			{
				tfbsm01["STATION_ID"] = "D";
				if (tfbsm01.QueryCount("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA") > 0)
				{
					tfbsm01.Update("MAT_FAMILY_CODE,UPPER_LIMIT_VALUE,MUST_DO_FLAG,LOWER_LIMIT_VALUE,QUALITY_FLAS,DATE_C,REC_REVISOR,REC_REVISE_TIME,BACK_C2,BACKLOG_EA,MARK_POS_CODE", "STATION_ID,MAT_CODE,ST_NO,MATERIAL_CODE,COMM_FMLY_CODE,BACKLOG_EA");
				}
				else
				{
					tfbsm01.Insert();
				}
			}
			if (data_table->Rows[i]["B_STATION_ID"].ToString().Trim() != "" && data_table->Rows[i]["B_STATION_ID"].ToString().Trim() != "0")
			{
				tfbsm01["STATION_ID"] = "B";
				if (tfbsm01.QueryCount("STATION_ID,MAT_CODE,ST_NO,BACKLOG_EA") > 0)
				{
					tfbsm01.Update("MAT_FAMILY_CODE,UPPER_LIMIT_VALUE,MUST_DO_FLAG,LOWER_LIMIT_VALUE,QUALITY_FLAS,DATE_C,REC_REVISOR,REC_REVISE_TIME,BACK_C2,BACKLOG_EA,MARK_POS_CODE", "STATION_ID,MAT_CODE,ST_NO,MATERIAL_CODE,COMM_FMLY_CODE,BACKLOG_EA");
				}
				else
				{
					tfbsm01.Insert();
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
int f_fbsm01_del(CDataTable *data_table, CDataTable *dt_key, EIClass *bcls_ret, CDbConnection *conn) // 删除
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CModel tfbsm01("TFBSM01");
	try
	{
		// ----------------------------------------
		Log::Trace("", "", "delete start");
		// 定义公共CDbCommand
		CDbCommand cmd(conn);
		// 整条删除
		int count = data_table->Rows.get_Count();
		for (int i = 0; i < count; i++)
		{
			tfbsm01.Reset();
			tfbsm01.MergeFrom(data_table->Rows[i]);
			Log::Trace("", "", "MAT_CODE{0}", data_table->Rows[i]["MAT_CODE"].ToString().Trim());
			sqlstr = "delete from tfbsm01 where MAT_CODE='" + tfbsm01["MAT_CODE"].ToString() + "' and ST_NO='" + tfbsm01["ST_NO"].ToString() + "' and MATERIAL_CODE='" + tfbsm01["MATERIAL_CODE"].ToString() + "' and COMM_FMLY_CODE='" + tfbsm01["COMM_FMLY_CODE"].ToString() + "' and BACKLOG_EA='" + tfbsm01["BACKLOG_EA"].ToString() + "' and REC_CREATOR='" + tfbsm01["REC_CREATOR"].ToString() + "' and REC_CREATE_TIME='" + tfbsm01["REC_CREATE_TIME"].ToString() + "' and REC_REVISOR='" + tfbsm01["REC_REVISOR"].ToString() + "' and REC_REVISE_TIME='" + tfbsm01["REC_REVISE_TIME"].ToString() + "'";
			Log::Trace("", "", "delete sqlstr{0}", sqlstr);
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

int f_fbsm01_save(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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
				// bcls_rec->Tables[i].set_TableName(table_name.Substring(0, position - 1));
				doFlag = f_fbsm01_ins(&bcls_rec->Tables[i], bcls_ret, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else if (position > 1 && table_name.Substring(position, table_name.GetLength() - position) == "MODIFY")
			{
				// bcls_rec->Tables[i].set_TableName(table_name.Substring(0, position - 1));
				doFlag = f_fbsm01_upd(&bcls_rec->Tables[i], &bcls_rec->Tables["DT_KEY"], bcls_ret, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			else if (position > 1 && table_name.Substring(position, table_name.GetLength() - position) == "DELETE")
			{
				// bcls_rec->Tables[i].set_TableName(table_name.Substring(0, position - 1));
				doFlag = f_fbsm01_del(&bcls_rec->Tables[i], &bcls_rec->Tables["DT_KEY"], bcls_ret, conn);
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
