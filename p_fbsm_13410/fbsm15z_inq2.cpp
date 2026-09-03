/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2026/3/5
Description: 库存信息
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15z_inq2)

int f_fbsm15z_inq2(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int i = 1;	
	CString sqlstr = " ";
	int v_count = 0;
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_q(conn);
	CDbCommand cmd(conn);
	CString station_id = "";
	CString st_no = "";
	CString backlog_ea = "";
	CModel tfbsm01("TFBSM01");
	try
	{
		st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString();
		backlog_ea = bcls_rec->Tables[0].Rows[0]["BACKLOG_EA"].ToString();

		Log::Trace("", __FUNCTION__, "st_no = [{0}],station_id = [{1}]", st_no, station_id);
		
		//物料库区的转换
		/*bcls_ret->Tables.SetTableName(0, "STOCK");
		sqlstr = "select CODE, CODE_DESC_1_CONTENT as code_desc"
			" from tep0002 "
			" where code_class = 'FBSM13'"
			" union "
			" select BUNKER_NO as CODE, MAT_NAME as code_desc"
			" from tmmsm60 "
			" where  BUNKER_NO like 'VS%'"
			;
		cmd_inq_q.SetCommandText(sqlstr);
		cmd_inq_q.ExecuteQuery(bcls_ret->Tables["STOCK"]);
		cmd_inq_q.Close();*/

		//物料类型
		bcls_ret->Tables.Add("MATTYPE");
		sqlstr = "select CODE, CODE_DESC_1_CONTENT as code_desc"
			" from tep0002 "
			" where code_class = 'FBSM01'"
			;
		cmd_inq_q.SetCommandText(sqlstr);
		cmd_inq_q.ExecuteQuery(bcls_ret->Tables["MATTYPE"]);
		cmd_inq_q.Close();


		//工艺路径
		bcls_ret->Tables.Add("BACKLOGEA");
		sqlstr = "select CODE, DESCRIP as code_desc"
			" from TFBSM02 "
			;
		cmd_inq_q.SetCommandText(sqlstr);
		cmd_inq_q.ExecuteQuery(bcls_ret->Tables["BACKLOGEA"]);
		cmd_inq_q.Close();

		//中类
		bcls_ret->Tables.Add("MIDDLECATEGORY");
		sqlstr = "select CODE, CODE_DESC_1_CONTENT as code_desc"
			" from tep0002 "
			" where code_class = 'FBSM09'"
			;
		cmd_inq_q.SetCommandText(sqlstr);
		cmd_inq_q.ExecuteQuery(bcls_ret->Tables["MIDDLECATEGORY"]);
		cmd_inq_q.Close();

		//取工序的金属收得率,先判断是否根据出钢计划维护，如果没有就根据钢种大类来维护
		//重量收得率, C%收得率, Si%收得率, Mn%收得率, P%收得率, S%收得率, Cr%收得率, Ni%收得率, Mo%收得率, Cu%收得率, Co%收得率, Al%收得率, Nb%收得率, V%收得率, Ti%收得率, B%收得率, N%收得率, Ca%收得率
		bcls_ret->Tables.Add("EAFYIELDRATE");
		bcls_ret->Tables["EAFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_CONS");
		bcls_ret->Tables["EAFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_CR");
		bcls_ret->Tables["EAFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_NI");
		bcls_ret->Tables["EAFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_MO");
		bcls_ret->Tables["EAFYIELDRATE"].Rows.Add();
		bcls_ret->Tables["EAFYIELDRATE"].Rows[0]["YIELD_CONS"] = 1;
		bcls_ret->Tables["EAFYIELDRATE"].Rows[0]["YIELD_CR"] = 1;
		bcls_ret->Tables["EAFYIELDRATE"].Rows[0]["YIELD_NI"] = 1;
		bcls_ret->Tables["EAFYIELDRATE"].Rows[0]["YIELD_MO"] = 1;
		sqlstr = "select * from"
			" ( select METAL_CONS_PERT,YIELD_CR,YIELD_NI,YIELD_MO,1 as seq_no"
			" from tfbsm04"
			" where 1=1"
			" and STATION_ID = 'E'"
			" and BACKLOG_EA = @backlog_ea"
			" and st_no = @st_no"
			" union all"//中类一致
			" select METAL_CONS_PERT, YIELD_CR,YIELD_NI, YIELD_MO, 2 as seq_no"
			" from tfbsm04 t1"
			" where 1=1"
			" and exists( select 1 from tfbsm11 t2 where t1.comm_fmly_code=t2.comm_fmly_code and t1.material_code=t2.material_code and t2.st_no= @st_no)  "
			" and MARK_POS_CODE != ' '"
			" and STATION_ID = 'E'"
			" and BACKLOG_EA = @backlog_ea"
			" union all" //大类一致
			" select METAL_CONS_PERT, YIELD_CR,YIELD_NI, YIELD_MO, 3 as seq_no"
			" from tfbsm04 t1"
			" where 1=1"
			" and exists( select 1 from tfbsm11 t2 where t1.comm_fmly_code=t2.comm_fmly_code and t2.st_no= @st_no)  "
			" and MARK_POS_CODE != ' '"
			" and STATION_ID = 'E'"
			" and BACKLOG_EA = @backlog_ea"
			" )"
			" order by seq_no"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("backlog_ea", backlog_ea);
		cmd_inq.Parameters.Set("st_no", st_no);
		cmd_inq.ExecuteReader();	
		if (cmd_inq.Read())
		{
			if (cmd_inq.GetDecimal(1) != 0)
			{
				bcls_ret->Tables["EAFYIELDRATE"].Rows[0]["YIELD_CONS"] = (1000 / cmd_inq.GetDecimal(1)).Round(6);
			}
			if (cmd_inq.GetDecimal(2) != 0)
			{
				bcls_ret->Tables["EAFYIELDRATE"].Rows[0]["YIELD_CR"] = cmd_inq.GetDecimal(2) / 100;
			}
			if (cmd_inq.GetDecimal(3) != 0)
			{
				bcls_ret->Tables["EAFYIELDRATE"].Rows[0]["YIELD_NI"] = cmd_inq.GetDecimal(3) / 100;
			}
			if (cmd_inq.GetDecimal(4) != 0)
			{
				bcls_ret->Tables["EAFYIELDRATE"].Rows[0]["YIELD_MO"] = cmd_inq.GetDecimal(4) / 100;
			}			
		}
		cmd_inq.Read();

		bcls_ret->Tables.Add("AODYIELDRATE");
		bcls_ret->Tables["AODYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_CONS");
		bcls_ret->Tables["AODYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_CR");
		bcls_ret->Tables["AODYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_NI");
		bcls_ret->Tables["AODYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_MO");
		bcls_ret->Tables["AODYIELDRATE"].Rows.Add();
		bcls_ret->Tables["AODYIELDRATE"].Rows[0]["YIELD_CONS"] = 1;
		bcls_ret->Tables["AODYIELDRATE"].Rows[0]["YIELD_CR"] = 1;
		bcls_ret->Tables["AODYIELDRATE"].Rows[0]["YIELD_NI"] = 1;
		bcls_ret->Tables["AODYIELDRATE"].Rows[0]["YIELD_MO"] = 1;
		sqlstr = "select * from"
			" ( select METAL_CONS_PERT,YIELD_CR,YIELD_NI,YIELD_MO,1 as seq_no"
			" from tfbsm04"
			" where 1=1"
			" and STATION_ID = 'A'"
			" and BACKLOG_EA = @backlog_ea"
			" and st_no = @st_no"
			" union all"//中类一致
			" select METAL_CONS_PERT, YIELD_CR,YIELD_NI, YIELD_MO, 2 as seq_no"
			" from tfbsm04 t1"
			" where 1=1"
			" and exists( select 1 from tfbsm11 t2 where t1.comm_fmly_code=t2.comm_fmly_code and t1.material_code=t2.material_code and t2.st_no= @st_no)  "
			" and MARK_POS_CODE != ' '"
			" and STATION_ID = 'A'"
			" and BACKLOG_EA = @backlog_ea"
			" union all" //大类一致
			" select METAL_CONS_PERT, YIELD_CR,YIELD_NI, YIELD_MO, 3 as seq_no"
			" from tfbsm04 t1"
			" where 1=1"
			" and exists( select 1 from tfbsm11 t2 where t1.comm_fmly_code=t2.comm_fmly_code and t2.st_no= @st_no)  "
			" and MARK_POS_CODE != ' '"
			" and STATION_ID = 'A'"
			" and BACKLOG_EA = @backlog_ea"
			" )"
			" order by seq_no"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("backlog_ea", backlog_ea);
		cmd_inq.Parameters.Set("st_no", st_no);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			if (cmd_inq.GetDecimal(1) != 0)
			{
				bcls_ret->Tables["AODYIELDRATE"].Rows[0]["YIELD_CONS"] = (1000 / cmd_inq.GetDecimal(1)).Round(6);
			}
			if (cmd_inq.GetDecimal(2) != 0)
			{
				bcls_ret->Tables["AODYIELDRATE"].Rows[0]["YIELD_CR"] = cmd_inq.GetDecimal(2) / 100;
			}
			if (cmd_inq.GetDecimal(3) != 0)
			{
				bcls_ret->Tables["AODYIELDRATE"].Rows[0]["YIELD_NI"] = cmd_inq.GetDecimal(3) / 100;
			}
			if (cmd_inq.GetDecimal(4) != 0)
			{
				bcls_ret->Tables["AODYIELDRATE"].Rows[0]["YIELD_MO"] = cmd_inq.GetDecimal(4) / 100;
			}
		}
		cmd_inq.Read();

		bcls_ret->Tables.Add("BOFYIELDRATE");
		bcls_ret->Tables["BOFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_CONS");
		bcls_ret->Tables["BOFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_CR");
		bcls_ret->Tables["BOFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_NI");
		bcls_ret->Tables["BOFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_MO");
		bcls_ret->Tables["BOFYIELDRATE"].Rows.Add();
		bcls_ret->Tables["BOFYIELDRATE"].Rows[0]["YIELD_CONS"] = 1;
		bcls_ret->Tables["BOFYIELDRATE"].Rows[0]["YIELD_CR"] = 1;
		bcls_ret->Tables["BOFYIELDRATE"].Rows[0]["YIELD_NI"] = 1;
		bcls_ret->Tables["BOFYIELDRATE"].Rows[0]["YIELD_MO"] = 1;
		sqlstr = "select * from"
			" ( select METAL_CONS_PERT,YIELD_CR,YIELD_NI,YIELD_MO,1 as seq_no"
			" from tfbsm04"
			" where 1=1"
			" and STATION_ID = 'B'"
			" and BACKLOG_EA = @backlog_ea"
			" and st_no = @st_no"
			" union all"//中类一致
			" select METAL_CONS_PERT, YIELD_CR,YIELD_NI, YIELD_MO, 2 as seq_no"
			" from tfbsm04 t1"
			" where 1=1"
			" and exists( select 1 from tfbsm11 t2 where t1.comm_fmly_code=t2.comm_fmly_code and t1.material_code=t2.material_code and t2.st_no= @st_no)  "
			" and MARK_POS_CODE != ' '"
			" and STATION_ID = 'B'"
			" and BACKLOG_EA = @backlog_ea"
			" union all" //大类一致
			" select METAL_CONS_PERT, YIELD_CR,YIELD_NI, YIELD_MO, 3 as seq_no"
			" from tfbsm04 t1"
			" where 1=1"
			" and exists( select 1 from tfbsm11 t2 where t1.comm_fmly_code=t2.comm_fmly_code and t2.st_no= @st_no)  "
			" and MARK_POS_CODE != ' '"
			" and STATION_ID = 'B'"
			" and BACKLOG_EA = @backlog_ea"
			" )"
			" order by seq_no"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("backlog_ea", backlog_ea);
		cmd_inq.Parameters.Set("st_no", st_no);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			if (cmd_inq.GetDecimal(1) != 0)
			{
				bcls_ret->Tables["BOFYIELDRATE"].Rows[0]["YIELD_CONS"] = (1000 / cmd_inq.GetDecimal(1)).Round(6);
			}
			if (cmd_inq.GetDecimal(2) != 0)
			{
				bcls_ret->Tables["BOFYIELDRATE"].Rows[0]["YIELD_CR"] = cmd_inq.GetDecimal(2)/100;
			}
			if (cmd_inq.GetDecimal(3) != 0)
			{
				bcls_ret->Tables["BOFYIELDRATE"].Rows[0]["YIELD_NI"] = cmd_inq.GetDecimal(3) / 100;
			}
			if (cmd_inq.GetDecimal(4) != 0)
			{
				bcls_ret->Tables["BOFYIELDRATE"].Rows[0]["YIELD_MO"] = cmd_inq.GetDecimal(4) / 100;
			}
		}
		cmd_inq.Read();

		bcls_ret->Tables.Add("IFYIELDRATE");
		bcls_ret->Tables["IFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_CONS");
		bcls_ret->Tables["IFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_CR");
		bcls_ret->Tables["IFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_NI");
		bcls_ret->Tables["IFYIELDRATE"].Columns.Add(DT_DECIMAL, "YIELD_MO");
		bcls_ret->Tables["IFYIELDRATE"].Rows.Add();
		bcls_ret->Tables["IFYIELDRATE"].Rows[0]["YIELD_CONS"] = 1;
		bcls_ret->Tables["IFYIELDRATE"].Rows[0]["YIELD_CR"] = 1;
		bcls_ret->Tables["IFYIELDRATE"].Rows[0]["YIELD_NI"] = 1;
		bcls_ret->Tables["IFYIELDRATE"].Rows[0]["YIELD_MO"] = 1;
		sqlstr = "select * from"
			" ( select METAL_CONS_PERT,YIELD_CR,YIELD_NI,YIELD_MO,1 as seq_no"
			" from tfbsm04"
			" where 1=1"
			" and STATION_ID = 'Z'"
			" and BACKLOG_EA = @backlog_ea"
			" and st_no = @st_no"
			" union all"//中类一致
			" select METAL_CONS_PERT, YIELD_CR,YIELD_NI, YIELD_MO, 2 as seq_no"
			" from tfbsm04 t1"
			" where 1=1"
			" and exists( select 1 from tfbsm11 t2 where t1.comm_fmly_code=t2.comm_fmly_code and t1.material_code=t2.material_code and t2.st_no= @st_no)  "
			" and MARK_POS_CODE != ' '"
			" and STATION_ID = 'Z'"
			" and BACKLOG_EA = @backlog_ea"
			" union all" //大类一致
			" select METAL_CONS_PERT, YIELD_CR,YIELD_NI, YIELD_MO, 3 as seq_no"
			" from tfbsm04 t1"
			" where 1=1"
			" and exists( select 1 from tfbsm11 t2 where t1.comm_fmly_code=t2.comm_fmly_code and t2.st_no= @st_no)  "
			" and MARK_POS_CODE != ' '"
			" and STATION_ID = 'Z'"
			" and BACKLOG_EA = @backlog_ea"
			" )"
			" order by seq_no"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("backlog_ea", backlog_ea);
		cmd_inq.Parameters.Set("st_no", st_no);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			if (cmd_inq.GetDecimal(1) != 0)
			{
				bcls_ret->Tables["IFYIELDRATE"].Rows[0]["YIELD_CONS"] = (1000 / cmd_inq.GetDecimal(1)).Round(6);
			}
			if (cmd_inq.GetDecimal(2) != 0)
			{
				bcls_ret->Tables["IFYIELDRATE"].Rows[0]["YIELD_CR"] = cmd_inq.GetDecimal(2) / 100;
			}
			if (cmd_inq.GetDecimal(3) != 0)
			{
				bcls_ret->Tables["IFYIELDRATE"].Rows[0]["YIELD_NI"] = cmd_inq.GetDecimal(3) / 100;
			}
			if (cmd_inq.GetDecimal(4) != 0)
			{
				bcls_ret->Tables["IFYIELDRATE"].Rows[0]["YIELD_MO"] = cmd_inq.GetDecimal(4) / 100;
			}
		}
		cmd_inq.Read();

		Log::Trace("", __FUNCTION__, "YIELD_CONS = [{0}],YIELD_CR = [{1}]", bcls_ret->Tables["IFYIELDRATE"].Rows[0]["YIELD_CONS"].ToDecimal(), bcls_ret->Tables["IFYIELDRATE"].Rows[0]["YIELD_CR"].ToDecimal());
			

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
