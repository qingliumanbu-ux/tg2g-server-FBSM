/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15z_inq)

int f_fbsm15z_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int i = 0;
	CString s_id_01 = " ";
	CString s_id_02 = " ";
	CString s_id_03 = " ";
	CString sqlstr = " ";
	
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd(conn);
	CString compose_list_no = "";
	CString st_no = " ";
	CString flag_co = "0"; //低估
	CString flag_vod = "0"; //过vod
	CString st_no_1 = "";
	CString st_no_2 = "";
	CString backlog_ea = " ";//工艺路线
	CString material_code = " ";//中类代码
	CString comm_fmly_code = " ";//大类代码 	
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
			compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("BACKLOG_EA"))
			backlog_ea = bcls_rec->Tables[0].Rows[0]["BACKLOG_EA"].ToString();
		

		if (compose_list_no.Trim() != "")
		{

			sqlstr = "    SELECT t1.compose_list_no,t1.STATION_ID,"
				"        t1.DATE_C,"
				"        t1.ST_NO,"
				"        t1.BACKLOG_EA,"
				"        t1.MAT_CODE,"
				"        t1.MAT_NAME,"
				"        t1.COMPOSE_LIST_NO,"
				"        t1.WEIGHT,"
				"        t1.BUNKER_NO,"
				"        t1.LOT_NO,"
				"        t1.C_VALUE,"
				"        t1.SI_VALUE,"
				"        t1.MN_VALUE,"
				"        t1.P_VALUE,"
				"        t1.S_VALUE,"
				"        t1.CR_VALUE,"
				"        t1.NI_VALUE,"
				"        t1.MO_VALUE,"
				"        t1.CU_VALUE,"
				"        t1.CO_VALUE,"
				"        t1.TI_VALUE," // 2026.3.6 报表需求新增元素
				"        t1.NB_VALUE,"
				"        t1.AL_VALUE,"
				"        t1.B_VALUE,"
				"        t1.V_VALUE,"
				"        t1.CA_VALUE,"
				"        t1.N_VALUE,"
				"        t1.COST,"
				"        nvl(t3.code_desc,t1.STOCK_NAME) STOCK_NAME,"
				"       t1.BACK_C2"
				" ,case when t2.YEILD_MINUS = 0 then 100 else t2.YEILD_MINUS end mat_yeild"
				" from TFBSM14 t1 left join tmmsm50 t2 on t1.mat_code = t2.mat_code"
				" left join (select CODE, CODE_DESC_1_CONTENT as code_desc from tep0002 where code_class = 'FBSM13' union select BUNKER_NO as CODE, MAT_NAME as code_desc  from tmmsm60  where  BUNKER_NO like 'VS%') t3 on t1.STOCK_NAME = t3.code"
				" where 1=1 "
				//" and st_no='" + st_no + "' "
				" and t1.mat_code not in ('PL0001','PL0002','PL0003')"
				" and COMPOSE_LIST_NO='" + compose_list_no + "'"
				" ORDER BY  STATION_ID,"
				"    CASE  WHEN t1.STOCK_NAME = '2' THEN 1 WHEN t1.STOCK_NAME = '3' THEN 2 WHEN t1.STOCK_NAME = '4' THEN 3 WHEN t1.STOCK_NAME = '1' THEN 4 WHEN t1.STOCK_NAME LIKE 'VS%' THEN 4 WHEN t1.STOCK_NAME = '5' THEN 6 ELSE 7 END "
				",MAT_CODE,BACK_C2"
				;
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
			cmd_inq.Close();


			//备注信息
			bcls_ret->Tables.Add("REMARK");
			sqlstr = "select BACK_AOD,BACK_BOF,BACK_IF,BACK_EAF,BACK_DS,' ' BACK_AOD_LL"
				" from tfbsm14a"
				" where 1=1"				
				" and  COMPOSE_LIST_NO='" + compose_list_no + "'"
				;
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", "", "sqlstr = {0}", sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables["REMARK"]);
			cmd_inq.Close();
			//将料篮料槽个数显示到备注里
			sqlstr = " select MAT_CODE,SUM(WEIGHT) WEIGHT,BACKLOG_EA"
				" from tfbsm14"
				" where 1=1"
				"  AND (BACK_C2 like '1%' or BACK_C2 like '5%' OR BACK_C2 = '26')"
				" and mat_code in (select mat_code from tfbsm25)"
				" and STATION_ID = 'A'"
				" and COMPOSE_LIST_NO = @compose_list_no"
				" group by MAT_CODE, BACKLOG_EA"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("compose_list_no", compose_list_no);
			cmd_inq.ExecuteReader();
			CDecimal count_lc_big = 0;
			CDecimal count_lc_small = 0;
			CDecimal wt = 0;
			while (cmd_inq.Read())
			{
				wt = cmd_inq.GetDecimal(2);
				sqlstr = " SELECT   BIG_LOAD_WT_MAX, BIG_LOAD_WT_MIN, SMALL_LOAD_WT_MAX, SMALL_LOAD_WT_MIN"
					" from "
					" (select MAT_CODE, BIG_LOAD_WT_MAX, BIG_LOAD_WT_MIN, SMALL_LOAD_WT_MAX, SMALL_LOAD_WT_MIN, 1 seq_no from tfbsm25 t2 where t2.BACKLOG_EA = @backlog_ea and mat_code = @mat_code"
					" union all"
					" select MAT_CODE, BIG_LOAD_WT_MAX, BIG_LOAD_WT_MIN, SMALL_LOAD_WT_MAX, SMALL_LOAD_WT_MIN, 2 seq_no from tfbsm25 t2  where t2.BACKLOG_EA = ' '  and mat_code = @mat_code"
					" )"
					" order by seq_no"
					;
				cmd.SetCommandText(sqlstr);
				cmd.Parameters.Set("mat_code", cmd_inq.GetString(1));
				cmd.Parameters.Set("backlog_ea", cmd_inq.GetString(3));
				cmd.ExecuteReader();
				if (cmd.Read())
				{
					if (wt>0 && cmd.GetDecimal(1) > 0)
					{
						count_lc_big = count_lc_big + (wt / cmd.GetDecimal(1)).Floor();
						wt = wt - cmd.GetDecimal(1) * (wt / cmd.GetDecimal(1)).Floor();
					}
					if (wt>0 && wt >= cmd.GetDecimal(2) && cmd.GetDecimal(2) > 0)
					{
						count_lc_big = count_lc_big + 1;
						wt = wt - cmd.GetDecimal(1) * ((wt / cmd.GetDecimal(2)).Floor()+1);
					}
					if (wt>0 && wt >= cmd.GetDecimal(3) && cmd.GetDecimal(3) > 0) // 小料槽
					{
						//Log::Trace("", __FUNCTION__, "jisuan  = [{0}],wt = [{1}],槽最大值=[{2}]", (wt / cmd.GetDecimal(3)).Floor(), wt, cmd.GetDecimal(3));

						count_lc_small = count_lc_small + (wt / cmd.GetDecimal(3)).Floor();
						wt = wt - cmd.GetDecimal(3) * (wt / cmd.GetDecimal(3)).Floor();
					}
					if (wt>0 && wt >= cmd.GetDecimal(4) && cmd.GetDecimal(4) > 0) // 小料槽
					{
						count_lc_small = count_lc_small + 1;
						wt = wt - cmd.GetDecimal(4) * ((wt / cmd.GetDecimal(4)).Floor()+1);
					}
					if (wt > 0)
					{
						count_lc_small = count_lc_small + 1;
					}
				}
				cmd.Close();
			}
			cmd_inq.Close();
			
				if (count_lc_big > 0 && count_lc_small>0)
				{
					bcls_ret->Tables["REMARK"].Rows[0]["BACK_AOD_LL"] = count_lc_big.ToString() + "大" + count_lc_small.ToString() + "小";
				}
				if (count_lc_big > 0 && count_lc_small == 0)
				{
					bcls_ret->Tables["REMARK"].Rows[0]["BACK_AOD_LL"] = count_lc_big.ToString() + "大" ;

				}
				if (count_lc_big == 0 && count_lc_small > 0)
				{
					bcls_ret->Tables["REMARK"].Rows[0]["BACK_AOD_LL"] = count_lc_small.ToString() + "小";
				}

			bcls_ret->Tables.Add("Y");
			sqlstr = "    SELECT t1.compose_list_no,t1.STATION_ID,"
				"        t1.DATE_C,"
				"        t1.ST_NO,"
				"        t1.BACKLOG_EA,"
				"        t1.MAT_CODE,"
				"        t1.MAT_NAME,"
				"        t1.COMPOSE_LIST_NO,"
				"        t1.WEIGHT,"
				"        t1.BUNKER_NO,"
				"        t1.LOT_NO,"
				"        t1.C_VALUE,"
				"        t1.SI_VALUE,"
				"        t1.MN_VALUE,"
				"        t1.P_VALUE,"
				"        t1.S_VALUE,"
				"        t1.CR_VALUE,"
				"        t1.NI_VALUE,"
				"        t1.MO_VALUE,"
				"        t1.CU_VALUE,"
				"        t1.CO_VALUE,"
				"        t1.TI_VALUE," // 2026.3.6 报表需求新增元素
				"        t1.NB_VALUE,"
				"        t1.AL_VALUE,"
				"        t1.B_VALUE,"
				"        t1.V_VALUE,"
				"        t1.CA_VALUE,"
				"        t1.N_VALUE,"
				"        t1.COST,"
				"        t1.STOCK_NAME,"
				"        t1.BACK_C2"
				" ,100 mat_yeild"
				" from TFBSM14 t1 "
				" where 1=1"
				" and t1.mat_code  = 'PL0003'"
				" and st_no = '" + st_no + "' "
				" and COMPOSE_LIST_NO='" + compose_list_no + "'"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteQuery(bcls_ret->Tables["Y"]);
			cmd_inq.Close();
		}

		bcls_ret->Tables.Add("CONTROL");
		//C%收得率, Si%收得率, Mn%收得率, P%收得率, S%收得率, Cr%收得率, Ni%收得率, Mo%收得率, Cu%收得率, Co%收得率, Al%收得率, Nb%收得率, V%收得率, Ti%收得率, B%收得率, N%收得率, Ca%收得率]
		sqlstr = " SELECT nvl(MAX(CASE WHEN ELM_NAME = 'C' THEN MAIN_AIM ELSE 0 END),0) C_AIM,nvl(MAX(CASE WHEN ELM_NAME = 'C' THEN SPE_MIN ELSE 0 END),0) C_MIN,nvl(MAX(CASE WHEN ELM_NAME = 'C' THEN SPE_MAX ELSE 0 END),0) C_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'Si' THEN MAIN_AIM ELSE 0 END),0) SI_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'Si' THEN SPE_MIN ELSE 0 END),0) SI_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'Si' THEN SPE_MAX ELSE 0 END),0) SI_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'Mn' THEN MAIN_AIM ELSE 0 END),0) MN_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'Mn' THEN SPE_MIN ELSE 0 END),0) MN_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'Mn' THEN SPE_MAX ELSE 0 END),0) MN_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'P' THEN MAIN_AIM ELSE 0 END),0) P_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'P' THEN SPE_MIN ELSE 0 END),0) P_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'P' THEN SPE_MAX ELSE 0 END),0) P_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'S' THEN MAIN_AIM ELSE 0 END),0) S_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'S' THEN SPE_MIN ELSE 0 END),0) S_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'S' THEN SPE_MAX ELSE 0 END),0) S_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'Cr' THEN MAIN_AIM ELSE 0 END),0) CR_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'Cr' THEN SPE_MIN ELSE 0 END),0) CR_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'Cr' THEN SPE_MAX ELSE 0 END),0) CR_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'Ni' THEN MAIN_AIM ELSE 0 END),0) NI_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'Ni' THEN SPE_MIN ELSE 0 END),0) NI_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'Ni' THEN SPE_MAX ELSE 0 END),0) NI_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'Mo' THEN MAIN_AIM ELSE 0 END),0) MO_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'Mo' THEN SPE_MIN ELSE 0 END),0) MO_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'Mo' THEN SPE_MAX ELSE 0 END),0) MO_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'Cu' THEN MAIN_AIM ELSE 0 END),0) CU_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'Cu' THEN SPE_MIN ELSE 0 END),0) CU_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'Cu' THEN SPE_MAX ELSE 0 END),0) CU_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'Co' THEN MAIN_AIM ELSE 0 END),0) CO_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'Co' THEN SPE_MIN ELSE 0 END),0) CO_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'Co' THEN SPE_MAX ELSE 0 END),0) CO_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'Al' THEN MAIN_AIM ELSE 0 END),0) AL_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'Al' THEN SPE_MIN ELSE 0 END),0) AL_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'Al' THEN SPE_MAX ELSE 0 END),0) AL_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'Nb' THEN MAIN_AIM ELSE 0 END),0) NB_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'Nb' THEN SPE_MIN ELSE 0 END),0) NB_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'Nb' THEN SPE_MAX ELSE 0 END),0) NB_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'V' THEN MAIN_AIM ELSE 0 END),0) V_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'V' THEN SPE_MIN ELSE 0 END),0) V_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'V' THEN SPE_MAX ELSE 0 END),0) V_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'Ti' THEN MAIN_AIM ELSE 0 END),0) TI_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'Ti' THEN SPE_MIN ELSE 0 END),0) TI_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'Ti' THEN SPE_MAX ELSE 0 END),0) TI_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'B' THEN MAIN_AIM ELSE 0 END),0) B_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'B' THEN SPE_MIN ELSE 0 END),0) B_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'B' THEN SPE_MAX ELSE 0 END),0) B_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'N' THEN MAIN_AIM ELSE 0 END),0) N_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'N' THEN SPE_MIN ELSE 0 END),0) N_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'N' THEN SPE_MAX ELSE 0 END),0) N_MAX"
			",nvl(MAX(CASE WHEN ELM_NAME = 'Ca' THEN MAIN_AIM ELSE 0 END),0) CA_AIM, nvl(MAX(CASE WHEN ELM_NAME = 'Ca' THEN SPE_MIN ELSE 0 END),0) CA_MIN, nvl(MAX(CASE WHEN ELM_NAME = 'Ca' THEN SPE_MAX ELSE 0 END),0) CA_MAX"
			",0 WEIGHT_MAX,0 WEIGHT_MIN"
			" FROM TQMTS02 "
			" WHERE IDX_NO = (SELECT ELM_STD_IDX_A FROM TQMTS0X WHERE ST_NO = @st_no)"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("st_no", st_no);
		cmd_inq.ExecuteQuery(bcls_ret->Tables["CONTROL"]);
		cmd_inq.Close();

		//获取出钢重量
		//取大类和中类
		sqlstr = " select COMM_FMLY_CODE,MATERIAL_CODE,CO_ST_NO,VOD_USE_FLAG "
			"  ,nvl((select st_no from tfbsm11 t1 where  MARK_POS_CODE='1' and exists (select 1 from tfbsm11 t2 where  t1.material_code = t2.material_code and st_no=@st_no )),' ') st_no_1 "
			", nvl((select st_no from tfbsm11 t1 where  MARK_POS_CODE = '2' and exists(select 1 from tfbsm11 t2 where  t1.comm_fmly_code = t2.comm_fmly_code and st_no = @st_no)), ' ') st_no_2"
			" from TFBSM11 where ST_NO=@st_no"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("st_no", st_no);
		flag_co = "0";
		flag_vod = "0";
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			comm_fmly_code = cmd_inq.GetString(1);
			material_code = cmd_inq.GetString(2);
			st_no_1 = cmd_inq.GetString(5);
			st_no_2 = cmd_inq.GetString(6);
			if (st_no.SubstringNE(1, 1) == "A" || st_no.SubstringNE(1, 1) == "D" || ((st_no.SubstringNE(1, 1) == "M" || st_no.SubstringNE(1, 1) == "F") && comm_fmly_code == 'V'))
			{
				flag_co = cmd_inq.GetString(3); //低Co标记
			}
			flag_vod = cmd_inq.GetString(4);
		}

		if (!bcls_ret->Tables["CONTROL"].Columns.Contains("WEIGHT_MAX"))
		{ 
			bcls_ret->Tables["CONTROL"].Columns.Add(DT_DECIMAL,"WEIGHT_MAX");
			bcls_ret->Tables["CONTROL"].Rows.Add();
		}
		if (!bcls_ret->Tables["CONTROL"].Columns.Contains("WEIGHT_MIN"))
		{
			bcls_ret->Tables["CONTROL"].Columns.Add(DT_DECIMAL, "WEIGHT_MIN");
		}

		sqlstr = " select case when @flag_vod='1'  then VOD_MIN else TAPPING_WT_MIN end "
			", case when @flag_vod='1' then VOD_MAX else TAPPING_WT end "
			" from("
			"    SELECT t.*,ROW_NUMBER() OVER (ORDER BY seq_rn ) as rn"
			"      from ("
			"         select t.*,1 seq_rn  from tfbsm09 t where   BACKLOG_EA ='" + backlog_ea + "' and ST_NO = '" + st_no + "'"
			"		  union "
			"         select t.*,2 seq_rn  from tfbsm09 t where   BACKLOG_EA ='" + backlog_ea + "' and ST_NO = '" + st_no_1 + "'"
			"		  union "
			"         select t.*,3 seq_rn  from tfbsm09 t where   BACKLOG_EA ='" + backlog_ea + "' and ST_NO = '" + st_no_2 + "'"
			"      ) t"
			" ) "
			"  where rn = 1 "
			;
		Log::Trace("", __FUNCTION__, "sqlstr = [{0} ]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("st_no", st_no);
		cmd_inq.Parameters.Set("backlog_ea", backlog_ea);
		cmd_inq.Parameters.Set("st_no_1", st_no_1);
		cmd_inq.Parameters.Set("st_no_2", st_no_2);
		cmd_inq.Parameters.Set("flag_vod", flag_vod);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			bcls_ret->Tables["CONTROL"].Rows[0]["WEIGHT_MIN"] = cmd_inq.GetDecimal(1);
			bcls_ret->Tables["CONTROL"].Rows[0]["WEIGHT_MAX"] = cmd_inq.GetDecimal(2);
		}
		cmd_inq.Close();

		Log::Trace("", __FUNCTION__, "WEIGHT_MIN = [{0} ]", bcls_ret->Tables["CONTROL"].Rows[0]["WEIGHT_MIN"].ToDecimal());
		Log::Trace("", __FUNCTION__, "WEIGHT_MAX = [{0} ]", bcls_ret->Tables["CONTROL"].Rows[0]["WEIGHT_MAX"].ToDecimal());

		if (!bcls_ret->Tables["CONTROL"].Columns.Contains("P_FLAG")) //脱磷标记
		{
			bcls_ret->Tables["CONTROL"].Columns.Add(DT_STRING, "P_FLAG");			
		}
		sqlstr = " select P_FLAG "
			" from tfbsm06"
			" where BACKLOG_EA ='" + backlog_ea + "' and ST_NO = '" + st_no + "'"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			bcls_ret->Tables["CONTROL"].Rows[0]["P_FLAG"] = cmd_inq.GetString(1);
		}
		cmd_inq.Close();

		
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
