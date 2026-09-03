/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: H5静态表查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsma_inq)

int f_fbsma_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CString sqlstr1 = " ";
	CDbCommand cmd(conn);
	CDbCommand cmd_inq(conn);

	try
	{
		int record_count_per_page = 0; /* 每页记录数 */
		int current_page_no = 0;	   /* 需查询的页号,从0开始计数 */
		int start_row = 0;			   /* 将要压入outBlock的起始行 */
		/* 获取传入的表名 */
		CString sql_tableName = (CString)bcls_rec->Tables[1].Rows[0]["tableName"].ToString();
		Log::Trace("", "", "sql_tableName[{0}],", sql_tableName);
		//  CString str1 = "";
		//  bcls_rec->WriteHTML(str1);
		//  Log::Trace("", __FUNCTION__, "str=[{0}]", str1);
		if (sql_tableName == "tfbsm04" || sql_tableName == "tfbsm11" || sql_tableName == "tfbsm05" ||
			sql_tableName == "tfbsm09" || sql_tableName == "tfbsm24" || sql_tableName == "tfbsm25")
		{
			if (sql_tableName == "tfbsm25")
			{
				CString mat_code = bcls_rec->Tables[0].Rows[0]["MAT_CODE"].ToString().Trim();
				sqlstr = "SELECT * FROM " + sql_tableName + " where 1=1 ";
				if (mat_code != "")
				{
					sqlstr += " and MAT_CODE LIKE '%" + mat_code + "%' ";
				}
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteQuery(bcls_ret->Tables[0]);
				cmd.Close();
			}
			else
			{
				CString st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().Trim();
				CString material_code = bcls_rec->Tables[0].Rows[0]["MATERIAL_CODE"].ToString().Trim();
				CString comm_fmly_code = bcls_rec->Tables[0].Rows[0]["COMM_FMLY_CODE"].ToString().Trim();
				// 需要特殊处理
				sqlstr = "SELECT * FROM " + sql_tableName + " where 1=1 ";
				if (st_no != "")
				{
					sqlstr += " and st_no='" + st_no + "' ";
				}
				if (st_no.Trim() != "")
				{
					sqlstr1 = "SELECT MATERIAL_CODE, COMM_FMLY_CODE FROM TFBSM11 WHERE ST_NO = '" + st_no + "'";
					cmd_inq.SetCommandText(sqlstr1);
					Log::Trace("", __FUNCTION__, "根据ST_NO补全参数sqlstr = {0}", sqlstr1);
					cmd_inq.ExecuteReader();
					if (cmd_inq.Read())
					{
						material_code = cmd_inq.GetString(1);
						comm_fmly_code = cmd_inq.GetString(2);
					}
					cmd_inq.Close();
				}
				else if (st_no.Trim() == "" && material_code.Trim() != "")
				{
					sqlstr1 = "SELECT COMM_FMLY_CODE FROM TFBSM11 WHERE MATERIAL_CODE = '" + material_code + "'";
					cmd_inq.SetCommandText(sqlstr1);
					Log::Trace("", __FUNCTION__, "根据MATERIAL_CODE补全参数sqlstr = {0}", sqlstr1);
					cmd_inq.ExecuteReader();
					if (cmd_inq.Read())
					{
						comm_fmly_code = cmd_inq.GetString(1);
					}
					cmd_inq.Close();
				}

				Log::Trace("", __FUNCTION__, "补全后查询条件st_no = {0}", st_no);
				Log::Trace("", __FUNCTION__, "补全后查询条件material_code = {0}", material_code);
				Log::Trace("", __FUNCTION__, "补全后查询条件comm_fmly_code = {0}", comm_fmly_code);

				if (comm_fmly_code != "")
				{
					sqlstr += " and COMM_FMLY_CODE='" + comm_fmly_code + "' ";
				}
				if (material_code != "")
				{
					sqlstr += " and MATERIAL_CODE='" + material_code + "' ";
				}
				if (sql_tableName == "tfbsm05" || sql_tableName == "tfbsm09")
				{
					CString backlog_ea = bcls_rec->Tables[0].Rows[0]["BACKLOG_EA"].ToString().Trim();
					Log::Trace("", __FUNCTION__, "进入BACKLOG_EA=[{0}]", backlog_ea);
					if (backlog_ea != "")
					{
						sqlstr += " and BACKLOG_EA='" + backlog_ea + "' ";
					}
				}
				cmd.SetCommandText(sqlstr);
				cmd.ExecuteQuery(bcls_ret->Tables[0]);
				cmd.Close();
				Log::Trace("", __FUNCTION__, "table_count			    = [{0}]", bcls_ret->Tables[0].Rows.get_Count());
				if (bcls_ret->Tables[0].Rows.get_Count() == 0)
				{
					CString st_no_list = "";
					if (material_code != "")
					{
						sqlstr1 = "SELECT ST_NO FROM TFBSM11 WHERE MATERIAL_CODE = '" + material_code + "' AND MARK_POS_CODE = '1'";
						cmd_inq.SetCommandText(sqlstr1);
						Log::Trace("", __FUNCTION__, "Query TFBSM11 by MATERIAL_CODE = {0}", sqlstr1);
						cmd_inq.ExecuteReader();
						while (cmd_inq.Read())
						{
							if (st_no_list != "")
								st_no_list += ",";
							st_no_list += "'" + cmd_inq.GetString(1) + "'";
						}
						cmd_inq.Close();
					}

					if (st_no_list != "")
					{
						sqlstr = "SELECT * FROM " + sql_tableName + " where 1=1 ";
						sqlstr += " and ST_NO IN (" + st_no_list + ") ";
						if (sql_tableName == "tfbsm05" || sql_tableName == "tfbsm09")
						{
							CString backlog_ea = bcls_rec->Tables[0].Rows[0]["BACKLOG_EA"].ToString().Trim();
							if (backlog_ea != "")
							{
								sqlstr += " and BACKLOG_EA='" + backlog_ea + "' ";
							}
						}
						Log::Trace("", __FUNCTION__, "Query by mid-class rep = {0}", sqlstr);
						cmd.SetCommandText(sqlstr);
						cmd.ExecuteQuery(bcls_ret->Tables[0]);
						cmd.Close();
					}

					if (bcls_ret->Tables[0].Rows.get_Count() == 0)
					{
						st_no_list = "";
						if (comm_fmly_code != "")
						{
							sqlstr1 = "SELECT ST_NO FROM TFBSM11 WHERE COMM_FMLY_CODE = '" + comm_fmly_code + "' AND MARK_POS_CODE = '2'";
							cmd_inq.SetCommandText(sqlstr1);
							Log::Trace("", __FUNCTION__, "Query TFBSM11 by COMM_FMLY_CODE = {0}", sqlstr1);
							cmd_inq.ExecuteReader();
							while (cmd_inq.Read())
							{
								if (st_no_list != "")
									st_no_list += ",";
								st_no_list += "'" + cmd_inq.GetString(1) + "'";
							}
							cmd_inq.Close();
						}

						if (st_no_list != "")
						{
							sqlstr = "SELECT * FROM " + sql_tableName + " where 1=1 ";
							sqlstr += " and ST_NO IN (" + st_no_list + ") ";
							if (sql_tableName == "tfbsm05" || sql_tableName == "tfbsm09")
							{
								CString backlog_ea = bcls_rec->Tables[0].Rows[0]["BACKLOG_EA"].ToString().Trim();
								if (backlog_ea != "")
								{
									sqlstr += " and BACKLOG_EA='" + backlog_ea + "' ";
								}
							}
							Log::Trace("", __FUNCTION__, "Query by big-class rep = {0}", sqlstr);
							cmd.SetCommandText(sqlstr);
							cmd.ExecuteQuery(bcls_ret->Tables[0]);
							cmd.Close();
						}
					}
				}

				if ((sql_tableName == "tfbsm04" || sql_tableName == "tfbsm05" || sql_tableName == "tfbsm09") && bcls_ret->Tables[0].Rows.get_Count() > 0)
				{
					CString st_no_list = "";
					if (!bcls_ret->Tables[0].Columns.Contains("MARK_POS_CODE"))
						bcls_ret->Tables[0].Columns.Add(DT_STRING, "MARK_POS_CODE");
					for (int i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
					{
						CString row_st_no = bcls_ret->Tables[0].Rows[i]["ST_NO"].ToString().Trim();
						if (row_st_no != "")
						{
							if (st_no_list != "")
								st_no_list += ",";
							st_no_list += "'" + row_st_no + "'";
						}
					}

					if (st_no_list != "")
					{
						sqlstr1 = "SELECT ST_NO, MARK_POS_CODE FROM TFBSM11 WHERE ST_NO IN (" + st_no_list + ")";
						cmd_inq.SetCommandText(sqlstr1);
						cmd_inq.ExecuteReader();
						while (cmd_inq.Read())
						{
							CString q_st_no = cmd_inq.GetString(1);
							CString q_mark = cmd_inq.GetString(2);
							for (int j = 0; j < bcls_ret->Tables[0].Rows.get_Count(); j++)
							{
								if (bcls_ret->Tables[0].Rows[j]["ST_NO"].ToString().Trim() == q_st_no)
								{
									bcls_ret->Tables[0].Rows[j]["MARK_POS_CODE"] = q_mark;
								}
							}
						}
						cmd_inq.Close();
					}
				}
			}
		}
		else
		{
			/* 获取传入的列名 */
			// CString sql_colName = (CString)bcls_rec->Tables[1].Rows[0]["colName"].ToString().Trim();

			CString sql_orderBy = (CString)bcls_rec->Tables[1].Rows[0]["ORDER_BY"].ToString().Trim();
			/* 每页记录数 */
			record_count_per_page = bcls_rec->Tables[1].Rows[0]["PAGE_SIZE"];
			/* 需查询的页号 */
			current_page_no = bcls_rec->Tables[1].Rows[0]["PAGE_NUM"];

			/*拼接查询sql语句*/
			CString sql = "SELECT * FROM " + sql_tableName;

			CString sql_where = " WHERE 1=1";

			/*拼接查询sql条数语句*/
			CString sql_count = "SELECT COUNT(*) FROM " + sql_tableName;

			/*获取查询条件传入列数*/
			int count_row = bcls_rec->Tables[0].Columns.get_Count();
			CString apply_steel = "";
			CString st_to = "";
			for (int i = 0; i < count_row; i++)
			{
				Log::Trace("", "", "bcls_rec->Tables[0].Rows[0][i].ToString().Trim()= {0}", bcls_rec->Tables[0].Rows[0][i].ToString().Trim());
				/*如果查询条件的值为空则跳出*/
				if (bcls_rec->Tables[0].Rows[0][i].ToString().Trim().IsEmpty())
				{
					continue;
				}
				if (bcls_rec->Tables[0].Columns[i].get_ColumnName() == "DATE_E")
				{
					sql_where += " AND NVL(NULLIF(TRIM(REC_CREATE_TIME), ''), REC_REVISE_TIME) <= @" + bcls_rec->Tables[0].Columns[i].get_ColumnName();
					cmd.Parameters.Set(bcls_rec->Tables[0].Columns[i].get_ColumnName(), bcls_rec->Tables[0].Rows[0][i].ToString().Trim());
					continue;
				}
				if (bcls_rec->Tables[0].Columns[i].get_ColumnName() == "DATE_S")
				{
					sql_where += " AND NVL(NULLIF(TRIM(REC_CREATE_TIME), ''), REC_REVISE_TIME) >= @" + bcls_rec->Tables[0].Columns[i].get_ColumnName();
					cmd.Parameters.Set(bcls_rec->Tables[0].Columns[i].get_ColumnName(), bcls_rec->Tables[0].Rows[0][i].ToString().Trim());
					continue;
				}
				if (bcls_rec->Tables[0].Columns[i].get_ColumnName() == "DU_FLAG")
				{
					sql_where += " AND DU_FLAG =@" + bcls_rec->Tables[0].Columns[i].get_ColumnName();
					Log::Trace("", "", "DU_FLAG ={0}", bcls_rec->Tables[0].Rows[0][i].ToString().TrimOrBlank());
					if (bcls_rec->Tables[0].Rows[0][i].ToString().TrimOrBlank() == "1")
					{
						cmd.Parameters.Set(bcls_rec->Tables[0].Columns[i].get_ColumnName(), "U");
					}
					else
					{
						cmd.Parameters.Set(bcls_rec->Tables[0].Columns[i].get_ColumnName(), " ");
					}
					continue;
				}
				if (bcls_rec->Tables[0].Columns[i].get_ColumnName() == "ST_TO")
				{
					st_to = bcls_rec->Tables[0].Rows[0][i].ToString().Trim();
					sql_where += " AND SUBSTRING(ST_NO, 2, 1) =@" + bcls_rec->Tables[0].Columns[i].get_ColumnName();
					Log::Trace("", "", "st_to ={0}", st_to);
					cmd.Parameters.Set(bcls_rec->Tables[0].Columns[i].get_ColumnName(), st_to);
					continue;
				}
				Log::Trace("", "", "条件查询sql {0}++[{1}],", i, bcls_rec->Tables[0].Columns[i].get_ColumnName() + ":" + bcls_rec->Tables[1].Rows[0][0].ToString().Trim());
				if (bcls_rec->Tables[0].Columns[i].get_ColumnName() == "APPLY_STEEL")
				{
					apply_steel = bcls_rec->Tables[0].Rows[0][i].ToString().Trim();
					Log::Trace("", "", "apply_steel = {0}", apply_steel);
					if (apply_steel != "")
					{
						sql_where += " AND APPLY_STEEL like '%" + apply_steel + "%' ";
						cmd.Parameters.Set("apply_steel", apply_steel);
					}
				}
				else
				{
					sql_where += " AND " + bcls_rec->Tables[0].Columns[i].get_ColumnName() + " LIKE '%'||@" + bcls_rec->Tables[0].Columns[i].get_ColumnName() + "||'%'";
					cmd.Parameters.Set(bcls_rec->Tables[0].Columns[i].get_ColumnName(), bcls_rec->Tables[0].Rows[0][i].ToString().Trim());
				}
			}

			/*连接sql语句*/
			sqlstr = sql_count + sql_where;
			cmd.SetCommandText(sqlstr);

			/*获取条数*/
			CDecimal rc = cmd.ExecuteScalar();

			/*把值压入RC中，传出前台*/
			bcls_ret->ExtendedProperties.Add("RC", rc.ToString());

			if (sql_orderBy.Trim() != "")
			{
				sql_orderBy = " ORDER BY " + sql_orderBy;
			}
			/*完成拼接查询sql*/
			sqlstr = sql + sql_where + sql_orderBy;
			Log::Trace("", "", "sql[{0}],", sqlstr);
			cmd.SetCommandText(sqlstr);

			start_row = record_count_per_page * (current_page_no - 1);
			if (start_row > rc.ToDouble())
			{
				start_row = 0;
			}
			Log::Trace("", __FUNCTION__, "current_page_no		= [{0}]", current_page_no);
			Log::Trace("", __FUNCTION__, "record_count_per_page = [{0}]", record_count_per_page);
			Log::Trace("", __FUNCTION__, "start_row			    = [{0}]", start_row);
			int count = cmd.ExecuteQuery(bcls_ret->Tables[0]);
			bcls_ret->Tables[0].set_TableName(sql_tableName);
			Log::Trace("", __FUNCTION__, "table_count			    = [{0}]", bcls_ret->Tables[0].Rows.get_Count());

			// 返回分页总数量信息

			bcls_ret->Tables.Add("PageInfo");
			bcls_ret->Tables["PageInfo"].Columns.Add(DT_DECIMAL, "TotalRecordCount");
			bcls_ret->Tables["PageInfo"].Rows.Add();
			bcls_ret->Tables["PageInfo"].Rows[0]["TotalRecordCount"] = rc;
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
