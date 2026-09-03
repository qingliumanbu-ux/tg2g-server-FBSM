/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: H5静态表查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm01_inq)

int f_fbsm01_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString mat_code = "";
	CString comm_fmly_code = "";
	CString material_code = "";
	CString st_no = "";
	CString backlog_ea = "";
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("MAT_CODE"))
			mat_code = bcls_rec->Tables[0].Rows[0]["MAT_CODE"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("COMM_FMLY_CODE"))
			comm_fmly_code = bcls_rec->Tables[0].Rows[0]["COMM_FMLY_CODE"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("MATERIAL_CODE"))
			material_code = bcls_rec->Tables[0].Rows[0]["MATERIAL_CODE"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("BACKLOG_EA"))
			backlog_ea = bcls_rec->Tables[0].Rows[0]["BACKLOG_EA"].ToString().TrimOrBlank().ToUpper();

		if (st_no.Trim() != "")
		{
			sqlstr = "SELECT MATERIAL_CODE, COMM_FMLY_CODE FROM TFBSM11 WHERE ST_NO = '" + st_no + "'";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", __FUNCTION__, "根据ST_NO补全参数sqlstr = {0}", sqlstr);
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
			sqlstr = "SELECT COMM_FMLY_CODE FROM TFBSM11 WHERE MATERIAL_CODE = '" + material_code + "'";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", __FUNCTION__, "根据MATERIAL_CODE补全参数sqlstr = {0}", sqlstr);
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

		sqlstr = " SELECT t.BACKLOG_EA,t.QUALITY_FLAS,t.MAT_CODE,t.MAT_NAME,t.MUST_DO_FLAG,t.UPPER_LIMIT_VALUE,t.LOWER_LIMIT_VALUE,m50.MAT_TYPE_PL1 AS BACK_C2, "
				 " t.E_STATION_ID,t.Z_STATION_ID,t.A_STATION_ID,t.D_STATION_ID,t.B_STATION_ID,t.MATERIAL_CODE,t.COMM_FMLY_CODE, "
				 " t.MAT_DIVISION_DESC,t.BIG_CLASS_NAME,t.ST_NO,t.MARK_POS_CODE, "
				 " t.REC_CREATOR,t.REC_CREATE_TIME,t.REC_REVISOR,t.REC_REVISE_TIME, "
				 " v.WEIGHT AS WEIGHT,v.CR,v.NI,v.MO "
				 " FROM ( "
				 " SELECT BACKLOG_EA,QUALITY_FLAS,MAT_CODE,MAT_NAME,MUST_DO_FLAG,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE,REC_CREATOR,REC_CREATE_TIME,REC_REVISOR,REC_REVISE_TIME, "
				 " MAX(DECODE(STATION_ID,'E','1')) AS E_STATION_ID, "
				 " MAX(DECODE(STATION_ID,'Z','1')) AS Z_STATION_ID, "
				 " MAX(DECODE(STATION_ID,'A','1')) AS A_STATION_ID, "
				 " MAX(DECODE(STATION_ID,'D','1')) AS D_STATION_ID, "
				 " MAX(DECODE(STATION_ID,'B','1')) AS B_STATION_ID, "
				 " MATERIAL_CODE,COMM_FMLY_CODE,MAT_DIVISION_DESC,BIG_CLASS_NAME,ST_NO,MARK_POS_CODE "
				 " FROM tfbsm01 WHERE 1=1 ";
		if (comm_fmly_code.Trim() != "")
		{
			sqlstr += " and comm_fmly_code like'%" + comm_fmly_code + "%' ";
		}
		if (mat_code.Trim() != "")
		{
			sqlstr += " and mat_code like'%" + mat_code + "%' ";
		}
		if (material_code.Trim() != "")
		{
			sqlstr += " and material_code like'%" + material_code + "%' ";
		}
		if (st_no.Trim() != "")
		{
			sqlstr += " and st_no like'%" + st_no + "%' ";
		}
		if (backlog_ea.Trim() != "")
		{
			sqlstr += " and backlog_ea like'%" + backlog_ea + "%' ";
		}
		sqlstr += " GROUP BY BACKLOG_EA,QUALITY_FLAS,MAT_CODE,MUST_DO_FLAG,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE,REC_CREATOR,REC_CREATE_TIME,REC_REVISOR,REC_REVISE_TIME, "
				  " COMM_FMLY_CODE,MATERIAL_CODE,MAT_NAME,MAT_DIVISION_DESC,BIG_CLASS_NAME,ST_NO,MARK_POS_CODE "
				  " ) t "
				  " LEFT JOIN TMMSM50 m50 ON t.MAT_CODE = m50.MAT_CODE "
				  " LEFT JOIN ( "
				  " SELECT MAT_CODE,SUM( CASE WHEN STOCK_NAME != '5' THEN WEIGHT / 1000 ELSE WEIGHT END ) AS WEIGHT, "
				  " MAX(CR) KEEP(DENSE_RANK FIRST ORDER BY DBMS_RANDOM.VALUE) AS CR, "
				  " MAX(NI) KEEP(DENSE_RANK FIRST ORDER BY DBMS_RANDOM.VALUE) AS NI, "
				  " MAX(MO) KEEP(DENSE_RANK FIRST ORDER BY DBMS_RANDOM.VALUE) AS MO "
				  " FROM FBSM_13_AI GROUP BY MAT_CODE "
				  " ) v ON t.MAT_CODE = v.MAT_CODE "
				  " ORDER BY m50.MAT_TYPE_PL1 ";
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "st_no查询sqlstr = {0}", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		if (bcls_ret->Tables[0].Rows.get_Count() == 0)
		{
			Log::Trace("", __FUNCTION__, "ST_NO 未找到, 尝试寻找中类代表 MATERIAL_CODE + MARK_POS_CODE=1");

			CString st_no_list = "";
			if (material_code.Trim() != "")
			{
				sqlstr = "SELECT ST_NO FROM TFBSM11 WHERE MATERIAL_CODE = '" + material_code + "' AND MARK_POS_CODE = '1'";
				cmd_inq.SetCommandText(sqlstr);
				Log::Trace("", __FUNCTION__, "Query TFBSM11 sqlstr = {0}", sqlstr);
				cmd_inq.ExecuteReader();
				while (cmd_inq.Read())
				{
					if (st_no_list != "")
						st_no_list += ",";
					st_no_list += "'" + cmd_inq.GetString(1) + "'";
				}
				cmd_inq.Close();
			}

			Log::Trace("", __FUNCTION__, "中类代表钢种st_no_list = {0}", st_no_list);

			if (st_no_list != "")
			{
				sqlstr = " SELECT t.BACKLOG_EA,t.QUALITY_FLAS,t.MAT_CODE,t.MAT_NAME,t.MUST_DO_FLAG,t.UPPER_LIMIT_VALUE,t.LOWER_LIMIT_VALUE,m50.MAT_TYPE_PL1 AS BACK_C2, "
						 " t.E_STATION_ID,t.Z_STATION_ID,t.A_STATION_ID,t.D_STATION_ID,t.B_STATION_ID,t.MATERIAL_CODE,t.COMM_FMLY_CODE, "
						 " t.MAT_DIVISION_DESC,t.BIG_CLASS_NAME,t.ST_NO,t.MARK_POS_CODE, "
							 " t.REC_CREATOR,t.REC_CREATE_TIME,t.REC_REVISOR,t.REC_REVISE_TIME, "
						 " v.WEIGHT AS WEIGHT,v.CR,v.NI,v.MO "
						 " FROM ( "
						 " SELECT BACKLOG_EA,QUALITY_FLAS,MAT_CODE,MAT_NAME,MUST_DO_FLAG,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE,REC_CREATOR,REC_CREATE_TIME,REC_REVISOR,REC_REVISE_TIME, "
						 " MAX(DECODE(STATION_ID,'E','1')) AS E_STATION_ID, "
						 " MAX(DECODE(STATION_ID,'Z','1')) AS Z_STATION_ID, "
						 " MAX(DECODE(STATION_ID,'A','1')) AS A_STATION_ID, "
						 " MAX(DECODE(STATION_ID,'D','1')) AS D_STATION_ID, "
						 " MAX(DECODE(STATION_ID,'B','1')) AS B_STATION_ID, "
						 " MATERIAL_CODE,COMM_FMLY_CODE,MAT_DIVISION_DESC,BIG_CLASS_NAME,ST_NO,MARK_POS_CODE "
						 " FROM tfbsm01 WHERE 1=1 ";

				sqlstr += " and st_no in (" + st_no_list + ") ";

				if (mat_code.Trim() != "")
				{
					sqlstr += " and mat_code like'" + mat_code + "%' ";
				}
				if (backlog_ea.Trim() != "")
				{
					sqlstr += " and backlog_ea = '" + backlog_ea + "' ";
				}

				sqlstr += " GROUP BY BACKLOG_EA,QUALITY_FLAS,MAT_CODE,MUST_DO_FLAG,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE,REC_CREATOR,REC_CREATE_TIME,REC_REVISOR,REC_REVISE_TIME, "
						  " COMM_FMLY_CODE,MATERIAL_CODE,MAT_NAME,MAT_DIVISION_DESC,BIG_CLASS_NAME,ST_NO,MARK_POS_CODE "
						  " ) t "
						  " LEFT JOIN TMMSM50 m50 ON t.MAT_CODE = m50.MAT_CODE "
						  " LEFT JOIN ( "
						  " SELECT MAT_CODE,SUM( CASE WHEN STOCK_NAME != '5' THEN WEIGHT / 1000 ELSE WEIGHT END ) AS WEIGHT, "
						  " MAX(CR) KEEP(DENSE_RANK FIRST ORDER BY DBMS_RANDOM.VALUE) AS CR, "
						  " MAX(NI) KEEP(DENSE_RANK FIRST ORDER BY DBMS_RANDOM.VALUE) AS NI, "
						  " MAX(MO) KEEP(DENSE_RANK FIRST ORDER BY DBMS_RANDOM.VALUE) AS MO "
						  " FROM FBSM_13_AI GROUP BY MAT_CODE "
						  " ) v ON t.MAT_CODE = v.MAT_CODE "
						  " ORDER BY m50.MAT_TYPE_PL1 ";

				cmd_inq.SetCommandText(sqlstr);
				Log::Trace("", "", "Query tfbsm01 by ST_NO from TFBSM11(MATERIAL_CODE) = {0}", sqlstr);
				cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
				cmd_inq.Close();
			}

			if (bcls_ret->Tables[0].Rows.get_Count() == 0)
			{
				Log::Trace("", __FUNCTION__, "MATERIAL_CODE 未找到, 尝试寻找大类代表 COMM_FMLY_CODE + MARK_POS_CODE=2");

				st_no_list = "";
				if (comm_fmly_code.Trim() != "")
				{
					sqlstr = "SELECT ST_NO FROM TFBSM11 WHERE COMM_FMLY_CODE = '" + comm_fmly_code + "' AND MARK_POS_CODE = '2'";
					cmd_inq.SetCommandText(sqlstr);
					Log::Trace("", __FUNCTION__, "Query TFBSM11 sqlstr = {0}", sqlstr);
					cmd_inq.ExecuteReader();
					while (cmd_inq.Read())
					{
						if (st_no_list != "")
							st_no_list += ",";
						st_no_list += "'" + cmd_inq.GetString(1) + "'";
					}
					cmd_inq.Close();
				}

				Log::Trace("", __FUNCTION__, "大类代表钢种st_no_list = {0}", st_no_list);

				if (st_no_list != "")
				{
					sqlstr = " SELECT t.BACKLOG_EA,t.QUALITY_FLAS,t.MAT_CODE,t.MAT_NAME,t.MUST_DO_FLAG,t.UPPER_LIMIT_VALUE,t.LOWER_LIMIT_VALUE,m50.MAT_TYPE_PL1 AS BACK_C2, "
							 " t.E_STATION_ID,t.Z_STATION_ID,t.A_STATION_ID,t.D_STATION_ID,t.B_STATION_ID,t.MATERIAL_CODE,t.COMM_FMLY_CODE, "
							 " t.MAT_DIVISION_DESC,t.BIG_CLASS_NAME,t.ST_NO,t.MARK_POS_CODE, "
							 " t.REC_CREATOR,t.REC_CREATE_TIME,t.REC_REVISOR,t.REC_REVISE_TIME, "
							 " v.WEIGHT AS WEIGHT,v.CR,v.NI,v.MO "
							 " FROM ( "
							 " SELECT BACKLOG_EA,QUALITY_FLAS,MAT_CODE,MAT_NAME,MUST_DO_FLAG,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE,REC_CREATOR,REC_CREATE_TIME,REC_REVISOR,REC_REVISE_TIME, "
							 " MAX(DECODE(STATION_ID,'E','1')) AS E_STATION_ID, "
							 " MAX(DECODE(STATION_ID,'Z','1')) AS Z_STATION_ID, "
							 " MAX(DECODE(STATION_ID,'A','1')) AS A_STATION_ID, "
							 " MAX(DECODE(STATION_ID,'D','1')) AS D_STATION_ID, "
							 " MAX(DECODE(STATION_ID,'B','1')) AS B_STATION_ID, "
							 " MATERIAL_CODE,COMM_FMLY_CODE,MAT_DIVISION_DESC,BIG_CLASS_NAME,ST_NO,MARK_POS_CODE "
							 " FROM tfbsm01 WHERE 1=1 ";

					sqlstr += " and st_no in (" + st_no_list + ") ";

					if (mat_code.Trim() != "")
					{
						sqlstr += " and mat_code like'" + mat_code + "%' ";
					}
					if (backlog_ea.Trim() != "")
					{
						sqlstr += " and backlog_ea = '" + backlog_ea + "' ";
					}

					sqlstr += " GROUP BY BACKLOG_EA,QUALITY_FLAS,MAT_CODE,MUST_DO_FLAG,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE,REC_CREATOR,REC_CREATE_TIME,REC_REVISOR,REC_REVISE_TIME, "
							  " COMM_FMLY_CODE,MATERIAL_CODE,MAT_NAME,MAT_DIVISION_DESC,BIG_CLASS_NAME,ST_NO,MARK_POS_CODE "
							  " ) t "
							  " LEFT JOIN TMMSM50 m50 ON t.MAT_CODE = m50.MAT_CODE "
							  " LEFT JOIN ( "
							  " SELECT MAT_CODE,SUM( CASE WHEN STOCK_NAME != '5' THEN WEIGHT / 1000 ELSE WEIGHT END ) AS WEIGHT, "
							  " MAX(CR) KEEP(DENSE_RANK FIRST ORDER BY DBMS_RANDOM.VALUE) AS CR, "
							  " MAX(NI) KEEP(DENSE_RANK FIRST ORDER BY DBMS_RANDOM.VALUE) AS NI, "
							  " MAX(MO) KEEP(DENSE_RANK FIRST ORDER BY DBMS_RANDOM.VALUE) AS MO "
							  " FROM FBSM_13_AI GROUP BY MAT_CODE "
							  " ) v ON t.MAT_CODE = v.MAT_CODE "
							  " ORDER BY m50.MAT_TYPE_PL1 ";

					cmd_inq.SetCommandText(sqlstr);
					Log::Trace("", "", "Query tfbsm01 by ST_NO from TFBSM11(COMM_FMLY_CODE) = {0}", sqlstr);
					cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
					cmd_inq.Close();
				}
			}
		}

		if (bcls_ret->Tables[0].Rows.get_Count() > 0)
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
				//sqlstr = "SELECT ST_NO, MARK_POS_CODE FROM TFBSM11 WHERE ST_NO IN (" + st_no_list + ")";
				// 开始拆分 st_no_list，解决 ORA-01795
				CString strWhere;
				int nCount = 0;
				CString strTemp = st_no_list;

				while (!strTemp.IsEmpty())
				{
					int nPos = -1;
					for (int k = 0; k < strTemp.GetLength(); k++)
					{
						if (strTemp[k] == ',')
						{
							nPos = k;
							break;
						}
					}

					CString strItem;
					if (nPos >= 0)
					{
						for (int k = 0; k < nPos; k++)
						{
							strItem += strTemp[k];
						}
						CString newTemp;
						for (int k = nPos + 1; k < strTemp.GetLength(); k++)
						{
							newTemp += strTemp[k];
						}
						strTemp = newTemp;
					}
					else
					{
						strItem = strTemp;
						strTemp.Empty();
					}

					if (nCount % 900 == 0)
					{
						if (!strWhere.IsEmpty())
							strWhere += " OR ";
						strWhere += "ST_NO IN (";
					}
					else
					{
						strWhere += ",";
					}
					strWhere += strItem;
					nCount++;

					if (nCount % 900 == 0)
					{
						strWhere += ")";
					}
				}

				if (!strWhere.IsEmpty() && (nCount % 900 != 0))
				{
					strWhere += ")";
				}

				sqlstr = "SELECT ST_NO, MARK_POS_CODE FROM TFBSM11 WHERE 1=1 ";
				if (!strWhere.IsEmpty())
				{
					sqlstr += " AND (" + strWhere + ") ";
				}
				cmd_inq.SetCommandText(sqlstr);
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
