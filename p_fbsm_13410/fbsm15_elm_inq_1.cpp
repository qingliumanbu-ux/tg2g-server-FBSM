/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料查询
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15_elm_inq_1)

int f_fbsm15_elm_inq_1(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int i = 0;
	CString s_id_01 = " ";
	CString s_id_02 = " ";
	CString s_id_03 = " ";
	CString sqlstr = " ";
	CString sql_control = " ";
	CDecimal c_max = 3.5, c_aim = 3.0, c_min = 2.5;
	CDecimal si_max = 0, si_aim = 0, si_min = 0;
	CDecimal mn_max = 0, mn_aim = 0, mn_min = 0;
	CDecimal p_max = 0, p_aim = 0, p_min = 0;
	CDecimal s_max = 0, s_aim = 0, s_min = 0;
	CDecimal cr_max = 0, cr_aim = 0, cr_min = 0;
	CDecimal ni_max = 0, ni_aim = 0, ni_min = 0;
	CDecimal mo_max = 0, mo_aim = 0, mo_min = 0;
	CDecimal cu_max = 0, cu_aim = 0, cu_min = 0;
	CDecimal co_max = 0, co_aim = 0, co_min = 0;
	CDecimal al_max = 0, al_aim = 0, al_min = 0;
	CDecimal nb_max = 0, nb_aim = 0, nb_min = 0;
	CDecimal v_max = 0, v_aim = 0, v_min = 0;
	CDecimal ti_max = 0, ti_aim = 0, ti_min = 0;
	CDecimal b_max = 0, b_aim = 0, b_min = 0;
	CDecimal n_max = 0, n_aim = 0, n_min = 0;
	CDecimal ca_max = 0, ca_aim = 0, ca_min = 0;
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd(conn);
	CString compose_list_no = "";
	CString st_no = " ";
	try
	{
		if (bcls_rec->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
			compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank().ToUpper();
		if (bcls_rec->Tables[0].Columns.Contains("ST_NO"))
			st_no = bcls_rec->Tables[0].Rows[0]["ST_NO"].ToString().TrimOrBlank().ToUpper();
		Log::Info("", __FUNCTION__, "compose_list_no  =[{0}]", compose_list_no);
		Log::Info("", __FUNCTION__, "st_no  =[{0}]", st_no);
		sqlstr = " WITH original_data AS ("
				 "    SELECT"
				 "        CASE WHEN STATION_ID = 'Y' THEN 'A' ELSE STATION_ID END STATION_ID,"
				 "        CASE WHEN STATION_ID = 'Y' THEN 1 WHEN (MAT_CODE = 'PL0001' OR MAT_CODE = 'PL0002') THEN 3 ELSE 2 END S_XU,"
				 "        DATE_C,"
				 "        ST_NO,"
				 "        BACKLOG_EA,"
				 "        MAT_CODE,"
				 "        MAT_NAME,"
				 "        COMPOSE_LIST_NO,"
				 "        WEIGHT,"
				 "        BUNKER_NO,"
				 "        LOT_NO,"
				 "        C_VALUE,"
				 "        SI_VALUE,"
				 "        MN_VALUE,"
				 "        P_VALUE,"
				 "        S_VALUE,"
				 "        CR_VALUE,"
				 "        NI_VALUE,"
				 "        MO_VALUE,"
				 "        CU_VALUE,"
				 "        CO_VALUE,"
				 "        TI_VALUE," // 2026.3.6 报表需求新增元素
				 "        NB_VALUE,"
				 "        AL_VALUE,"
				 "        B_VALUE,"
				 "        V_VALUE,"
				 "        CA_VALUE,"
				 "        N_VALUE,"
				 "        COST,"
				 "        CASE WHEN STOCK_NAME NOT IN('1','2','3','4','5') THEN ("
				 "            SELECT MAT_NAME FROM TMMSM60 "
				 "            WHERE BUNKER_NO = TFBSM14.STOCK_NAME AND MAT_CODE = TFBSM14.MAT_CODE AND ROWNUM = 1"
				 "        ) ELSE STOCK_NAME END AS STOCK_NAME "
				 "		from TFBSM14	"
				 "          WHERE COMPOSE_LIST_NO = @COMPOSE_LIST_NO "
				 "            AND st_no = @ST_NO "
				 "),"
				 " max_date_at000258 AS ("
				 "    SELECT"
				 "        C_VALUE,"
				 "        SI_VALUE,"
				 "        MN_VALUE,"
				 "        P_VALUE,"
				 "        S_VALUE,"
				 "        CR_VALUE,"
				 "        NI_VALUE,"
				 "        MO_VALUE,"
				 "        CU_VALUE,"
				 "        CO_VALUE,"
				 "        TI_VALUE," // 2026.3.6 报表需求新增元素
				 "        NB_VALUE,"
				 "        AL_VALUE,"
				 "        B_VALUE,"
				 "        V_VALUE,"
				 "        CA_VALUE,"
				 "        N_VALUE"
				 "    FROM TFBSM14"
				 "    WHERE MAT_CODE = 'AT000258'"
				 "      AND DATE_C = (SELECT MAX(DATE_C) FROM TFBSM14 WHERE MAT_CODE = 'AT000258')"
				 "    AND ROWNUM = 1 "
				 "), "
				 "supplement_data AS ("
				 "    SELECT"
				 "        'A' AS STATION_ID,"
				 "        3 AS S_XU,"
				 "        od.DATE_C,"
				 "        od.ST_NO,"
				 "        od.BACKLOG_EA,"
				 "        'AT000258' AS MAT_CODE,"
				 "        '硅铁 FeSi72Al2.0' AS MAT_NAME,"
				 "        od.COMPOSE_LIST_NO,"
				 "        COALESCE(t20.SI_PDI, 0) AS WEIGHT,"
				 "        od.BUNKER_NO,"
				 "        od.LOT_NO,"
				 "        COALESCE(md.C_VALUE, 0) AS C_VALUE,"
				 "        COALESCE(md.SI_VALUE, 0) AS SI_VALUE,"
				 "        COALESCE(md.MN_VALUE, 0) AS MN_VALUE,"
				 "        COALESCE(md.P_VALUE, 0) AS P_VALUE,"
				 "        COALESCE(md.S_VALUE, 0) AS S_VALUE,"
				 "        COALESCE(md.CR_VALUE, 0) AS CR_VALUE,"
				 "        COALESCE(md.NI_VALUE, 0) AS NI_VALUE,"
				 "        COALESCE(md.MO_VALUE, 0) AS MO_VALUE,"
				 "        COALESCE(md.CU_VALUE, 0) AS CU_VALUE,"
				 "        COALESCE(md.CO_VALUE, 0) AS CO_VALUE,"
				 "        COALESCE(md.TI_VALUE, 0) AS TI_VALUE,"
				 "        COALESCE(md.NB_VALUE, 0) AS NB_VALUE,"
				 "        COALESCE(md.AL_VALUE, 0) AS AL_VALUE,"
				 "        COALESCE(md.B_VALUE, 0) AS B_VALUE,"
				 "        COALESCE(md.V_VALUE, 0) AS V_VALUE,"
				 "        COALESCE(md.CA_VALUE, 0) AS CA_VALUE,"
				 "        COALESCE(md.N_VALUE, 0) AS N_VALUE,"
				 "        0 AS COST, "
				 "        '3' AS STOCK_NAME"
				 "    FROM original_data od"
				 "    LEFT JOIN TFBSM11 t11 ON od.ST_NO = t11.ST_NO"
				 "    LEFT JOIN TFBSM20 t20 ON t11.COMM_FMLY_CODE = t20.COMM_FMLY_CODE AND od.BACKLOG_EA = t20.BACKLOG_EA"
				 "    CROSS JOIN max_date_at000258 md"
				 "    WHERE NOT EXISTS (SELECT 1 FROM original_data WHERE MAT_CODE = 'AT000258')"
				 "    AND ROWNUM = 1"
				 ") "
				 "SELECT * FROM ("
				 "    SELECT * FROM original_data"
				 "    UNION ALL"
				 "    SELECT * FROM supplement_data"
				 ") temp_data "
				 // " where mat_code not in ('PL0001','PL0002','PL0003')"
				 "ORDER BY "

				 "    S_XU,"
				 "    MAT_CODE,"
				 "    CASE STOCK_NAME WHEN '2' THEN 1 WHEN '3' THEN 2 WHEN '1' THEN 3 WHEN '4' THEN 4 WHEN '5' THEN 5 ELSE 6 END ";
		cmd_inq.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
		cmd_inq.Parameters.Set("ST_NO", st_no);
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", "", "sqlstr = {0}", sqlstr);
		cmd_inq.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq.Close();

		for (i = 0; i < bcls_ret->Tables[0].Rows.get_Count(); i++)
		{
			if (bcls_ret->Tables[0].Rows[i]["STATION_ID"].ToString() == 'Z')
				bcls_ret->Tables[0].Rows[i]["STATION_ID"] = "IF";
			if (bcls_ret->Tables[0].Rows[i]["STATION_ID"].ToString() == 'E')
				bcls_ret->Tables[0].Rows[i]["STATION_ID"] = "EAF";
			if (bcls_ret->Tables[0].Rows[i]["STATION_ID"].ToString() == 'A')
				bcls_ret->Tables[0].Rows[i]["STATION_ID"] = "AOD";
			if (bcls_ret->Tables[0].Rows[i]["STOCK_NAME"].ToString() == '1')
				bcls_ret->Tables[0].Rows[i]["STOCK_NAME"] = "废钢料场";
			if (bcls_ret->Tables[0].Rows[i]["STOCK_NAME"].ToString() == '2')
				bcls_ret->Tables[0].Rows[i]["STOCK_NAME"] = "高位料仓";
			if (bcls_ret->Tables[0].Rows[i]["STOCK_NAME"].ToString() == '3')
				bcls_ret->Tables[0].Rows[i]["STOCK_NAME"] = "低位料仓";
			if (bcls_ret->Tables[0].Rows[i]["STOCK_NAME"].ToString() == '4')
				bcls_ret->Tables[0].Rows[i]["STOCK_NAME"] = "镍板库";
			if (bcls_ret->Tables[0].Rows[i]["STOCK_NAME"].ToString() == '5')
				bcls_ret->Tables[0].Rows[i]["STOCK_NAME"] = "一级库";
		}

		sql_control = " SELECT "
					  " MAX(CASE WHEN ELM_NAME = 'C' THEN MAIN_MAX ELSE 0 END) C_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'C' THEN MAIN_AIM ELSE 0 END) C_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'C' THEN MAIN_MIN ELSE 0 END) C_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'Si' THEN MAIN_MAX ELSE 0 END) SI_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'Si' THEN MAIN_AIM ELSE 0 END) SI_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'Si' THEN MAIN_MIN ELSE 0 END) SI_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'Mn' THEN MAIN_MAX ELSE 0 END) MN_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'Mn' THEN MAIN_AIM ELSE 0 END) MN_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'Mn' THEN MAIN_MIN ELSE 0 END) MN_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'P' THEN MAIN_MAX ELSE 0 END) P_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'P' THEN MAIN_AIM ELSE 0 END) P_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'P' THEN MAIN_MIN ELSE 0 END) P_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'S' THEN MAIN_MAX ELSE 0 END) S_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'S' THEN MAIN_AIM ELSE 0 END) S_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'S' THEN MAIN_MIN ELSE 0 END) S_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'Cr' THEN MAIN_MAX ELSE 0 END) CR_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'Cr' THEN MAIN_AIM ELSE 0 END) CR_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'Cr' THEN MAIN_MIN ELSE 0 END) CR_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'Ni' THEN MAIN_MAX ELSE 0 END) NI_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'Ni' THEN MAIN_AIM ELSE 0 END) NI_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'Ni' THEN MAIN_MIN ELSE 0 END) NI_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'Mo' THEN MAIN_MAX ELSE 0 END) MO_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'Mo' THEN MAIN_AIM ELSE 0 END) MO_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'Mo' THEN MAIN_MIN ELSE 0 END) MO_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'Cu' THEN MAIN_MAX ELSE 0 END) CU_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'Cu' THEN MAIN_AIM ELSE 0 END) CU_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'Cu' THEN MAIN_MIN ELSE 0 END) CU_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'Co' THEN MAIN_MAX ELSE 0 END) CO_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'Co' THEN MAIN_AIM ELSE 0 END) CO_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'Co' THEN MAIN_MIN ELSE 0 END) CO_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'Al' THEN MAIN_MAX ELSE 0 END) AL_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'Al' THEN MAIN_AIM ELSE 0 END) AL_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'Al' THEN MAIN_MIN ELSE 0 END) AL_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'Nb' THEN MAIN_MAX ELSE 0 END) NB_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'Nb' THEN MAIN_AIM ELSE 0 END) NB_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'Nb' THEN MAIN_MIN ELSE 0 END) NB_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'V' THEN MAIN_MAX ELSE 0 END) V_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'V' THEN MAIN_AIM ELSE 0 END) V_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'V' THEN MAIN_MIN ELSE 0 END) V_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'Ti' THEN MAIN_MAX ELSE 0 END) TI_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'Ti' THEN MAIN_AIM ELSE 0 END) TI_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'Ti' THEN MAIN_MIN ELSE 0 END) TI_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'B' THEN MAIN_MAX ELSE 0 END) B_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'B' THEN MAIN_AIM ELSE 0 END) B_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'B' THEN MAIN_MIN ELSE 0 END) B_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'N' THEN MAIN_MAX ELSE 0 END) N_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'N' THEN MAIN_AIM ELSE 0 END) N_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'N' THEN MAIN_MIN ELSE 0 END) N_MIN,"
					  " MAX(CASE WHEN ELM_NAME = 'Ca' THEN MAIN_MAX ELSE 0 END) CA_MAX,"
					  " MAX(CASE WHEN ELM_NAME = 'Ca' THEN MAIN_AIM ELSE 0 END) CA_AIM,"
					  " MAX(CASE WHEN ELM_NAME = 'Ca' THEN MAIN_MIN ELSE 0 END) CA_MIN "
					  " FROM TQMTS02 "
					  " WHERE IDX_NO = (SELECT ELM_STD_IDX_A FROM TQMTS0X WHERE ST_NO=@st_no)";
		cmd_inq.Parameters.Set("st_no", st_no);
		cmd_inq.SetCommandText(sql_control);
		Log::Trace("", "", "sql_control = {0}", sql_control);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			c_max = cmd_inq.GetDecimal(1);
			c_aim = cmd_inq.GetDecimal(2);
			c_min = cmd_inq.GetDecimal(3);
			si_max = cmd_inq.GetDecimal(4);
			si_aim = cmd_inq.GetDecimal(5);
			si_min = cmd_inq.GetDecimal(6);
			mn_max = cmd_inq.GetDecimal(7);
			mn_aim = cmd_inq.GetDecimal(8);
			mn_min = cmd_inq.GetDecimal(9);
			p_max = cmd_inq.GetDecimal(10);
			p_aim = cmd_inq.GetDecimal(11);
			p_min = cmd_inq.GetDecimal(12);
			s_max = cmd_inq.GetDecimal(13);
			s_aim = cmd_inq.GetDecimal(14);
			s_min = cmd_inq.GetDecimal(15);
			cr_max = cmd_inq.GetDecimal(16);
			cr_aim = cmd_inq.GetDecimal(17);
			cr_min = cmd_inq.GetDecimal(18);
			ni_max = cmd_inq.GetDecimal(19);
			ni_aim = cmd_inq.GetDecimal(20);
			ni_min = cmd_inq.GetDecimal(21);
			mo_max = cmd_inq.GetDecimal(22);
			mo_aim = cmd_inq.GetDecimal(23);
			mo_min = cmd_inq.GetDecimal(24);
			cu_max = cmd_inq.GetDecimal(25);
			cu_aim = cmd_inq.GetDecimal(26);
			cu_min = cmd_inq.GetDecimal(27);
			co_max = cmd_inq.GetDecimal(28);
			co_aim = cmd_inq.GetDecimal(29);
			co_min = cmd_inq.GetDecimal(30);
			al_max = cmd_inq.GetDecimal(31);
			al_aim = cmd_inq.GetDecimal(32);
			al_min = cmd_inq.GetDecimal(33);
			nb_max = cmd_inq.GetDecimal(34);
			nb_aim = cmd_inq.GetDecimal(35);
			nb_min = cmd_inq.GetDecimal(36);
			v_max = cmd_inq.GetDecimal(37);
			v_aim = cmd_inq.GetDecimal(38);
			v_min = cmd_inq.GetDecimal(39);
			ti_max = cmd_inq.GetDecimal(40);
			ti_aim = cmd_inq.GetDecimal(41);
			ti_min = cmd_inq.GetDecimal(42);
			b_max = cmd_inq.GetDecimal(43);
			b_aim = cmd_inq.GetDecimal(44);
			b_min = cmd_inq.GetDecimal(45);
			n_max = cmd_inq.GetDecimal(46);
			n_aim = cmd_inq.GetDecimal(47);
			n_min = cmd_inq.GetDecimal(48);
			ca_max = cmd_inq.GetDecimal(49);
			ca_aim = cmd_inq.GetDecimal(50);
			ca_min = cmd_inq.GetDecimal(51);
		}
		cmd_inq.Close();

		// 内控相关数据前台是写死的，暂时这样搞一下
		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows[i]["STATION_ID"] = "AOD";
		bcls_ret->Tables[0].Rows[i]["MAT_NAME"] = "内控上限";
		bcls_ret->Tables[0].Rows[i]["C_VALUE"] = c_max;
		bcls_ret->Tables[0].Rows[i]["SI_VALUE"] = si_max;
		bcls_ret->Tables[0].Rows[i]["MN_VALUE"] = mn_max;
		bcls_ret->Tables[0].Rows[i]["P_VALUE"] = p_max;
		bcls_ret->Tables[0].Rows[i]["S_VALUE"] = s_max;
		bcls_ret->Tables[0].Rows[i]["CR_VALUE"] = cr_max;
		bcls_ret->Tables[0].Rows[i]["NI_VALUE"] = ni_max;
		bcls_ret->Tables[0].Rows[i]["MO_VALUE"] = mo_max;
		bcls_ret->Tables[0].Rows[i]["CU_VALUE"] = cu_max;
		bcls_ret->Tables[0].Rows[i]["CO_VALUE"] = co_max;
		bcls_ret->Tables[0].Rows[i]["TI_VALUE"] = ti_max; // 2026.3.6 新增钛元素
		bcls_ret->Tables[0].Rows[i]["NB_VALUE"] = nb_max;
		bcls_ret->Tables[0].Rows[i]["AL_VALUE"] = al_max;
		bcls_ret->Tables[0].Rows[i]["B_VALUE"] = b_max;
		bcls_ret->Tables[0].Rows[i]["V_VALUE"] = v_max;
		bcls_ret->Tables[0].Rows[i]["CA_VALUE"] = ca_max;
		bcls_ret->Tables[0].Rows[i]["N_VALUE"] = n_max;

		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows[i + 1]["STATION_ID"] = "AOD";
		bcls_ret->Tables[0].Rows[i + 1]["MAT_NAME"] = "内控目标";
		bcls_ret->Tables[0].Rows[i + 1]["C_VALUE"] = c_aim;
		bcls_ret->Tables[0].Rows[i + 1]["SI_VALUE"] = si_aim;
		bcls_ret->Tables[0].Rows[i + 1]["MN_VALUE"] = mn_aim;
		bcls_ret->Tables[0].Rows[i + 1]["P_VALUE"] = p_aim;
		bcls_ret->Tables[0].Rows[i + 1]["S_VALUE"] = s_aim;
		bcls_ret->Tables[0].Rows[i + 1]["CR_VALUE"] = cr_aim;
		bcls_ret->Tables[0].Rows[i + 1]["NI_VALUE"] = ni_aim;
		bcls_ret->Tables[0].Rows[i + 1]["MO_VALUE"] = mo_aim;
		bcls_ret->Tables[0].Rows[i + 1]["CU_VALUE"] = cu_aim;
		bcls_ret->Tables[0].Rows[i + 1]["CO_VALUE"] = co_aim;
		bcls_ret->Tables[0].Rows[i + 1]["TI_VALUE"] = ti_aim;
		bcls_ret->Tables[0].Rows[i + 1]["NB_VALUE"] = nb_aim;
		bcls_ret->Tables[0].Rows[i + 1]["AL_VALUE"] = al_aim;
		bcls_ret->Tables[0].Rows[i + 1]["B_VALUE"] = b_aim;
		bcls_ret->Tables[0].Rows[i + 1]["V_VALUE"] = v_aim;
		bcls_ret->Tables[0].Rows[i + 1]["CA_VALUE"] = ca_aim;
		bcls_ret->Tables[0].Rows[i + 1]["N_VALUE"] = n_aim;

		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows[i + 2]["STATION_ID"] = "AOD";
		bcls_ret->Tables[0].Rows[i + 2]["MAT_NAME"] = "内控下限";
		bcls_ret->Tables[0].Rows[i + 2]["C_VALUE"] = c_min;
		bcls_ret->Tables[0].Rows[i + 2]["SI_VALUE"] = si_min;
		bcls_ret->Tables[0].Rows[i + 2]["MN_VALUE"] = mn_min;
		bcls_ret->Tables[0].Rows[i + 2]["P_VALUE"] = p_min;
		bcls_ret->Tables[0].Rows[i + 2]["S_VALUE"] = s_min;
		bcls_ret->Tables[0].Rows[i + 2]["CR_VALUE"] = cr_min;
		bcls_ret->Tables[0].Rows[i + 2]["NI_VALUE"] = ni_min;
		bcls_ret->Tables[0].Rows[i + 2]["MO_VALUE"] = mo_min;
		bcls_ret->Tables[0].Rows[i + 2]["CU_VALUE"] = cu_min;
		bcls_ret->Tables[0].Rows[i + 2]["CO_VALUE"] = co_min;
		bcls_ret->Tables[0].Rows[i + 2]["TI_VALUE"] = ti_min;
		bcls_ret->Tables[0].Rows[i + 2]["NB_VALUE"] = nb_min;
		bcls_ret->Tables[0].Rows[i + 2]["AL_VALUE"] = al_min;
		bcls_ret->Tables[0].Rows[i + 2]["B_VALUE"] = b_min;
		bcls_ret->Tables[0].Rows[i + 2]["V_VALUE"] = v_min;
		bcls_ret->Tables[0].Rows[i + 2]["CA_VALUE"] = ca_min;
		bcls_ret->Tables[0].Rows[i + 2]["N_VALUE"] = n_min;
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
