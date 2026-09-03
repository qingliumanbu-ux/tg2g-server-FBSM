/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料手工维护
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15_sg_save)
int f_mmsm_getprice(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//价格计算

int f_fbsm15_sg_save(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString compose_list_no = " ";//配料单号
	CString compose_list_no_l = " ";
	CString st_no = " ";//出钢记号
	int ret = 0;
	CString backlog_ea = " ";//工艺路线
	CString date = " ";//日期
	CString material_code = " ";//中类代码
	CString seq_no = " ";//序号
	CString plan_id = "";
	CString plan_id_1 = "";
	CString f_backlog_ea = " ";//返回工艺路线
	CDecimal p_seq = 0;
	CString f_material_code = " ";//模型返回的中类代码
	CString f_st_no = " ";//模型返回的中类代码
	CString f_date = " ";//返回日期
	CDbCommand cmd(conn);
	CModel tfbsm14("TFBSM14");
	CModel tfbsm18("TFBSM18");
	CModel tfbsm12("TFBSM12");
	CModel tfbsm13("TFBSM13");
	CString st_1 = " ";
	CString st_2 = " ";
	CString st_3 = " ";
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString comm_fmly_code = " ";//大类代码
	CDbCommand cmd_si_pdi(conn);
	CDecimal f_si_pdi = 0;
	//METAL_CONS_PERT
	CDecimal metal_cons_pert = 0;
	//YEILD_MINUS
	CString yeild_minus = " "; 
	//模型定义
	EIClass inDMST02; //传入类
	EIClass outDMST02;	//返回类
	try
	{
		//获取手动配料信息
		//价格计算
		EIClass bcls_rec_rep;
		EIClass bcls_ret_rep;
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_CODE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_NAME");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "CR_VALUE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "NI_VALUE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MO_VALUE");
		bcls_rec_rep.Tables[0].Rows.Add();
		//返回配料单号
		if (!bcls_ret->Tables[0].Columns.Contains("COMPOSE_LIST_NO"))
			bcls_ret->Tables[0].Columns.Add(DT_STRING, "COMPOSE_LIST_NO");
		bcls_ret->Tables[0].Rows.Add();
		seq_no = bcls_rec->Tables["LD"].Rows[0]["SEQ_NO"].ToString().TrimOrBlank();
		Log::Trace("", __FUNCTION__, "seq_no = [{0}]", seq_no);
		//MATERIAL_CODE
		material_code = bcls_rec->Tables["LD"].Rows[0]["MATERIAL_CODE"].ToString();
		Log::Trace("", __FUNCTION__, "material_code = [{0}]", material_code);
		//获取是那个配料单
		compose_list_no_l = bcls_rec->Tables["LD"].Rows[0]["COMPOSE_LIST_NO"].ToString();
		st_no = bcls_rec->Tables["LD"].Rows[0]["ST_NO"].ToString();
		backlog_ea = bcls_rec->Tables["LD"].Rows[0]["BACKLOG_EA"].ToString();
		date = bcls_rec->Tables["LD"].Rows[0]["DATE_C"].ToString();
		//根据出钢记号去查产品对应关系
		sqlstr = " select COMM_FMLY_CODE,MATERIAL_CODE from TFBSM11 where ST_NO='" + st_no + "' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			comm_fmly_code = cmd_inq.GetString(1);
		}
		cmd_inq.Close();
		//配料单信息
		sqlstr = " select  max(substr(COMPOSE_LIST_NO,12,14)),max(substr(COMPOSE_LIST_NO,14,1)),max(substr(COMPOSE_LIST_NO,13, 1)),max(substr(COMPOSE_LIST_NO,13, 2)) from tfbsm14 where DATE_C='" + date + "'  and COMPOSE_LIST_NO!=' ' and MATERIAL_CODE='" + material_code + "' and BACKLOG_EA='" + backlog_ea + "'   ";
		Log::Trace("", __FUNCTION__, "== sqlstr!![{0}] ==", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read()){
			if (cmd_inq.GetDecimal(3) == 0 && cmd_inq.GetDecimal(2) != 9){
				p_seq = cmd_inq.GetDecimal(2);
				Log::Trace("", __FUNCTION__, "== p_seq2[{0}] ==", p_seq.ToString());
				p_seq = p_seq + 1;
				Log::Trace("", __FUNCTION__, "== p_seq3[{0}] ==", p_seq.ToString());
				compose_list_no = st_no.SubstringNE(1, 1) + material_code + backlog_ea + date + "0" + p_seq.ToString();
			}
			else
			{
				p_seq = cmd_inq.GetDecimal(4);
				Log::Trace("", __FUNCTION__, "== p_seq2[{0}] ==", p_seq.ToString());
				p_seq = p_seq + 1;
				Log::Trace("", __FUNCTION__, "== p_seq4[{0}] ==", p_seq.ToString());
				compose_list_no = st_no.SubstringNE(1, 1) + material_code + backlog_ea + date + p_seq.ToString();
			}
		}
		else{
			Log::Trace("", __FUNCTION__, "== linke[{0}] ==", __LINE__);
			compose_list_no = st_no.SubstringNE(1, 1) + material_code + backlog_ea + date + "01";
		}
		cmd_inq.Close();
		//修改12表的标记(代表是人工修改)
		tfbsm12["ORIGIN_CODE"] = "1";
		sqlstr = " update tfbsm12 set ORIGIN_CODE='1', compose_list_no='" + compose_list_no + "' where COMPOSE_LIST_NO='" + compose_list_no_l + "' and  DATE_C='" + date + "' AND ST_NO='" + st_no + "' and BACKLOG_EA='" + backlog_ea + "'  ";
		Log::Trace("", __FUNCTION__, "== sqlstr[{0}] ==", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();
		//还原SI的值
		sqlstr = " select SI_PDI from tfbsm20 where COMM_FMLY_CODE='" + comm_fmly_code + "' and BACKLOG_EA='" + backlog_ea + "'  ";
		cmd_si_pdi.SetCommandText(sqlstr);
		cmd_si_pdi.ExecuteReader();
		Log::Trace("", __FUNCTION__, "== SI.sqlstr[{0}] ==", sqlstr);
		if (cmd_si_pdi.Read()){
			f_si_pdi = cmd_si_pdi.GetDecimal(1);
		}
		cmd_si_pdi.Close();
		//获取画面信息
		tfbsm14["REC_CREATOR"] = s.userid;
		for (size_t i = 0; i < bcls_rec->Tables["X1"].Rows.get_Count(); i++)
		{
			tfbsm14.Reset();
			tfbsm14.MergeFrom(bcls_rec->Tables["X1"].Rows[i]);
			if (bcls_rec->Tables["X1"].Rows[i]["STATION_ID"].ToString().Trim() != ""&& st_1.Trim() == ""){
				st_1 = bcls_rec->Tables["X1"].Rows[i]["STATION_ID"];
			}
			tfbsm14["STATION_ID"] = st_1;
			tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14["ST_NO"] = st_no;
			tfbsm14["BACKLOG_EA"] = backlog_ea;
			tfbsm14["DATE_C"] = date;
			//tfbsm14.Print();
			tfbsm14["REC_CREATOR"] = s.userid;
			tfbsm14["REC_CREATE_TIME"] = datetime;
			//不需要加还原SI
			/*if ((f_back_c2 == "7" || tfbsm14["MAT_CODE"].ToString() == "PL0002" || tfbsm14["MAT_CODE"].ToString() == "PL0001") && tfbsm14["STATION_ID"].ToString() == "A"){
				tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal() + f_si_pdi;
			}
			else{
				tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal();
			}*/
			tfbsm14["MATERIAL_CODE"] = material_code;
			//tfbsm14.Print();
			tfbsm14.Insert();
		}
		for (size_t i = 0; i < bcls_rec->Tables["X2"].Rows.get_Count(); i++)
		{
			tfbsm14.Reset();
			tfbsm14.MergeFrom(bcls_rec->Tables["X2"].Rows[i]);
			if (bcls_rec->Tables["X2"].Rows[i]["STATION_ID"].ToString().Trim() != ""&& st_2.Trim() == ""){
				st_2 = bcls_rec->Tables["X2"].Rows[i]["STATION_ID"];
			}
			tfbsm14["STATION_ID"] = st_2;
			tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14["ST_NO"] = st_no;
			tfbsm14["BACKLOG_EA"] = backlog_ea;
			tfbsm14["DATE_C"] = date;
			//tfbsm14.Print();
			tfbsm14["REC_CREATOR"] = s.userid;
			tfbsm14["REC_CREATE_TIME"] = datetime;
			//不需要加还原SI
			/*if ((f_back_c2 == "7" || tfbsm14["MAT_CODE"].ToString() == "PL0002" || tfbsm14["MAT_CODE"].ToString() == "PL0001") && tfbsm14["STATION_ID"].ToString() == "A"){
			tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal() + f_si_pdi;
			}
			else{
			tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal();
			}*/
			tfbsm14["MATERIAL_CODE"] = material_code;
			//tfbsm14.Print();
			tfbsm14.Insert();
		}
		for (size_t i = 0; i < bcls_rec->Tables["X3"].Rows.get_Count(); i++)
		{
			tfbsm14.Reset();
			tfbsm14.MergeFrom(bcls_rec->Tables["X3"].Rows[i]);
			if (bcls_rec->Tables["X3"].Rows[i]["STATION_ID"].ToString().Trim() != ""&& st_3.Trim()==""){
				st_3=bcls_rec->Tables["X3"].Rows[i]["STATION_ID"];
			}
			tfbsm14["STATION_ID"] = st_3;
			tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14["ST_NO"] = st_no;
			tfbsm14["BACKLOG_EA"] = backlog_ea;
			tfbsm14["DATE_C"] = date;
			//tfbsm14.Print();
			tfbsm14["REC_CREATOR"] = s.userid;
			tfbsm14["REC_CREATE_TIME"] = datetime;
			//不需要加还原SI
			/*if ((f_back_c2 == "7" || tfbsm14["MAT_CODE"].ToString() == "PL0002" || tfbsm14["MAT_CODE"].ToString() == "PL0001") && tfbsm14["STATION_ID"].ToString() == "A"){
			tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal() + f_si_pdi;
			}
			else{
			tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal();
			}*/
			tfbsm14["MATERIAL_CODE"] = material_code;
			//tfbsm14.Print();
			tfbsm14.Insert();
		}

		
		Log::Trace("", __FUNCTION__, "st_1 = [{0}]", st_1);
		if (st_1 != " " && st_1 != ""){
			sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + st_no + "' and ELM_CODE='001' and MATERIAL_CODE='" + material_code + "' and BACKLOG_EA='" + backlog_ea + "' ";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteReader();
			if (cmd.Read()){
				tfbsm14["C_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
				tfbsm14["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal() / 2;

			}
			cmd.Close();
			sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + st_no + "' and ELM_CODE='005' and MATERIAL_CODE='" + material_code + "' and BACKLOG_EA='" + backlog_ea + "' ";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteReader();
			if (cmd.Read()){
				tfbsm14["S_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
				tfbsm14["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal() / 2;

			}
			cmd.Close();
			//计算配料信息
			sqlstr = " select SUM(WEIGHT) WEIGHT, "
				" round(sum(SI_VALUE)/COUNT(*),2) SI_VALUE, "
				" round(sum(MN_VALUE)/COUNT(*),2) MN_VALUE, "
				" round(sum(P_VALUE)/COUNT(*),3) P_VALUE, "
				" round(sum(CR_VALUE)/COUNT(*),2) CR_VALUE, "
				" round(SUM(NI_VALUE)/COUNT(*),2) NI_VALUE, "
				" round(SUM(MO_VALUE)/COUNT(*),2) MO_VALUE, "
				" round(SUM(CU_VALUE)/COUNT(*),2) CU_VALUE, "
				" round(SUM(CO_VALUE)/COUNT(*),2) CO_VALUE  from tfbsm14 where COMPOSE_LIST_NO='" + compose_list_no + "' and STATION_ID='" + st_1 + "' and DATE_C='" + date + "' and ST_NO='" + st_no + "' and MAT_CODE not like 'PL00%' ";
			Log::Trace("", __FUNCTION__, "== 14.sql[{0}] ==", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				//算出来PL0001的重量及成分
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
				//cmd_inq.Fetch(tfbsm14);
				tfbsm14["WEIGHT"] = cmd_inq.GetString(1);
				tfbsm14["SI_VALUE"] = cmd_inq.GetString(2);
				tfbsm14["MN_VALUE"] = cmd_inq.GetString(3);
				tfbsm14["P_VALUE"] = cmd_inq.GetString(4);
				tfbsm14["CR_VALUE"] = cmd_inq.GetString(5);
				tfbsm14["NI_VALUE"] = cmd_inq.GetString(6);
				tfbsm14["MO_VALUE"] = cmd_inq.GetString(7);
				tfbsm14["CU_VALUE"] = cmd_inq.GetString(8);
				tfbsm14["CO_VALUE"] = cmd_inq.GetString(9);
			}
			cmd_inq.Close();
			Log::Trace("", __FUNCTION__, "== WEIGHT.[{0}] ==", tfbsm14["WEIGHT"].ToString());
			tfbsm14["MAT_CODE"] = "PL0001";
			tfbsm14["MAT_NAME"] = "配料成分";
			tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14["ST_NO"] = st_no;
			tfbsm14["BACKLOG_EA"] = backlog_ea;
			tfbsm14["DATE_C"] = date;
			tfbsm14["REC_CREATOR"] = s.userid;
			tfbsm14["REC_CREATE_TIME"] = datetime;
			tfbsm14["STATION_ID"] = st_1;
			tfbsm14["MATERIAL_CODE"] = material_code;
			if (tfbsm14.QueryCount("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID") > 0){
				//已经存在了删除后新增
				tfbsm14.Delete("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID");
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
			}
			//tfbsm14.Insert();
			sqlstr = "INSERT INTO TFBSM14 ("
				"    REC_CREATOR,"
				"    REC_CREATE_TIME,"
				"    DATE_C,"
				"    ST_NO,"
				"    BACKLOG_EA,"
				"    MAT_CODE,"
				"    MAT_NAME,"
				"    COMPOSE_LIST_NO,"
				"    WEIGHT,"
				"    C_VALUE,"
				"    SI_VALUE,"
				"    MN_VALUE,"
				"    P_VALUE,"
				"    S_VALUE,"
				"    CR_VALUE,"
				"    NI_VALUE,"
				"    MO_VALUE,"
				"    CU_VALUE,"
				"    CO_VALUE,"
				"    STATION_ID,"
				"    MATERIAL_CODE"
				") VALUES ("
				"    '" + tfbsm14["REC_CREATOR"].ToString() + "',"
				"    '" + datetime + "',"
				"    '" + tfbsm14["DATE_C"].ToString() + "',"
				"    '" + tfbsm14["ST_NO"].ToString() + "',"
				"    '" + tfbsm14["BACKLOG_EA"].ToString() + "',"
				"    '" + tfbsm14["MAT_CODE"].ToString() + "',"
				"    '" + tfbsm14["MAT_NAME"].ToString() + "',"
				"    '" + tfbsm14["COMPOSE_LIST_NO"].ToString() + "',"
				"    " + tfbsm14["WEIGHT"].ToString() + ","
				"    " + tfbsm14["C_VALUE"].ToString() + ","
				"    " + tfbsm14["SI_VALUE"].ToString() + ","
				"    " + tfbsm14["MN_VALUE"].ToString() + ","
				"    " + tfbsm14["P_VALUE"].ToString() + ","
				"    " + tfbsm14["S_VALUE"].ToString() + ","
				"    " + tfbsm14["CR_VALUE"].ToString() + ","
				"    " + tfbsm14["NI_VALUE"].ToString() + ","
				"    " + tfbsm14["MO_VALUE"].ToString() + ","
				"    " + tfbsm14["CU_VALUE"].ToString() + ","
				"    " + tfbsm14["CO_VALUE"].ToString() + ","
				"    '" + tfbsm14["STATION_ID"].ToString() + "',"
				"    '" + tfbsm14["MATERIAL_CODE"].ToString() + "' "
				" ) ";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", __FUNCTION__, "== insert .[{0}] ==", sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
			//计算PL0002的重量及成分
			//去查询04表数据
			sqlstr = " select  round(1000/(METAL_CONS_PERT),5) AS METAL_CONS_PERT from ( "
				" SELECT A.*, "
				" ROW_NUMBER() OVER(PARTITION BY A.STATION_ID ORDER BY CASE "
				" WHEN A.ST_NO = '" + st_no + "' THEN 1 "
				" WHEN A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "' "
				" THEN 2 "
				" WHEN A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' "
				" THEN 3 "
				" WHEN A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "' "
				" THEN 4 "
				" WHEN A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' "
				" THEN 5 "
				" ELSE 99 END) as rn "
				" FROM TFBSM04 A "
				" WHERE(A.ST_NO = '" + st_no + "' OR(A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "') OR "
				" (A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "') OR "
				" (A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "') OR "
				" (A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "'))) WHERE rn = 1 and  STATION_ID = '" + st_1 + "'  ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				metal_cons_pert = cmd_inq.GetDecimal(1);
			}
			cmd_inq.Close();
			//查询原料
			sqlstr = " select CASE "
				" WHEN '" + backlog_ea + "' = '03' AND '" + st_1 + "' = 'E' AND "
				" (SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND '" + comm_fmly_code + "' != 'V' THEN 0.95 "
				" WHEN G.YEILD_MINUS = 0 THEN 1 "
				" ELSE G.YEILD_MINUS/100 END       YEILD_MINUS "
				" from(select * "
				" from tfbsm14 "
				" where COMPOSE_LIST_NO = '" + compose_list_no + "' "
				" and STATION_ID = '" + st_1 + "' "
				" and DATE_C = '" + date + "' "
				" and ST_NO = '" + st_no + "' "
				" and MAT_CODE not like 'PL00%' "
				" ) S LEFT join TMMSM50 G on s.MAT_CODE = g.MAT_CODE ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read()){
				if (yeild_minus == " "){
					yeild_minus = yeild_minus + cmd_inq.GetString(1);
				}
				else{
					yeild_minus = yeild_minus + " * " + cmd_inq.GetString(1);
				}
			}
			cmd_inq.Close();
			Log::Trace("", __FUNCTION__, "== yeild_minus.[{0}] ==", yeild_minus);
			//计算PL0002出钢成分
			//tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal() * metal_cons_pert * yeild_minus;
			sqlstr = " SELECT " + tfbsm14["SI_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as SI_VALUE, "
				" " + tfbsm14["MN_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as MN_VALUE, "
				" " + tfbsm14["P_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as P_VALUE, "
				" " + tfbsm14["CR_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as CR_VALUE, "
				" " + tfbsm14["NI_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as NI_VALUE, "
				" " + tfbsm14["MO_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as MO_VALUE, "
				" " + tfbsm14["CU_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as CU_VALUE, "
				" " + tfbsm14["CO_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as CO_VALUE  "
				" FROM DUAL ";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", __FUNCTION__, "== PL0002/sqlstr.[{0}] ==", sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				//cmd_inq.Fetch(tfbsm14);
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
				tfbsm14["SI_VALUE"] = cmd_inq.GetString(1);
				tfbsm14["MN_VALUE"] = cmd_inq.GetString(2);
				tfbsm14["P_VALUE"] = cmd_inq.GetString(3);
				tfbsm14["CR_VALUE"] = cmd_inq.GetString(4);
				tfbsm14["NI_VALUE"] = cmd_inq.GetString(5);
				tfbsm14["MO_VALUE"] = cmd_inq.GetString(6);
				tfbsm14["CU_VALUE"] = cmd_inq.GetString(7);
				tfbsm14["CO_VALUE"] = cmd_inq.GetString(8);
			}
			cmd_inq.Close();
			tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal() * metal_cons_pert;
			Log::Trace("", __FUNCTION__, "=PL0002= WEIGHT.[{0}] ==", tfbsm14["WEIGHT"].ToString());
			tfbsm14["MAT_CODE"] = "PL0002";
			tfbsm14["MAT_NAME"] = "出钢成分";
			tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14["ST_NO"] = st_no;
			tfbsm14["BACKLOG_EA"] = backlog_ea;
			tfbsm14["DATE_C"] = date;
			tfbsm14["REC_CREATOR"] = s.userid;
			tfbsm14["REC_CREATE_TIME"] = datetime;
			tfbsm14["STATION_ID"] = st_1;
			tfbsm14["MATERIAL_CODE"] = material_code;
			if (tfbsm14.QueryCount("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID") > 0){
				//已经存在了删除后新增
				tfbsm14.Delete("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID");
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
			}
			//tfbsm14.Print();
			//tfbsm14.Insert();
			sqlstr = "INSERT INTO TFBSM14 ("
				"    REC_CREATOR,"
				"    REC_CREATE_TIME,"
				"    DATE_C,"
				"    ST_NO,"
				"    BACKLOG_EA,"
				"    MAT_CODE,"
				"    MAT_NAME,"
				"    COMPOSE_LIST_NO,"
				"    WEIGHT,"
				"    C_VALUE,"
				"    SI_VALUE,"
				"    MN_VALUE,"
				"    P_VALUE,"
				"    S_VALUE,"
				"    CR_VALUE,"
				"    NI_VALUE,"
				"    MO_VALUE,"
				"    CU_VALUE,"
				"    CO_VALUE,"
				"    STATION_ID,"
				"    MATERIAL_CODE"
				") VALUES ("
				"    '" + tfbsm14["REC_CREATOR"].ToString() + "',"
				"    '" + datetime + "',"
				"    '" + tfbsm14["DATE_C"].ToString() + "',"
				"    '" + tfbsm14["ST_NO"].ToString() + "',"
				"    '" + tfbsm14["BACKLOG_EA"].ToString() + "',"
				"    '" + tfbsm14["MAT_CODE"].ToString() + "',"
				"    '" + tfbsm14["MAT_NAME"].ToString() + "',"
				"    '" + tfbsm14["COMPOSE_LIST_NO"].ToString() + "',"
				"    " + tfbsm14["WEIGHT"].ToString() + ","
				"    " + tfbsm14["C_VALUE"].ToString() + ","
				"    " + tfbsm14["SI_VALUE"].ToString() + ","
				"    " + tfbsm14["MN_VALUE"].ToString() + ","
				"    " + tfbsm14["P_VALUE"].ToString() + ","
				"    " + tfbsm14["S_VALUE"].ToString() + ","
				"    " + tfbsm14["CR_VALUE"].ToString() + ","
				"    " + tfbsm14["NI_VALUE"].ToString() + ","
				"    " + tfbsm14["MO_VALUE"].ToString() + ","
				"    " + tfbsm14["CU_VALUE"].ToString() + ","
				"    " + tfbsm14["CO_VALUE"].ToString() + ","
				"    '" + tfbsm14["STATION_ID"].ToString() + "',"
				"    '" + tfbsm14["MATERIAL_CODE"].ToString() + "' "
				" ) ";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", __FUNCTION__, "== insert .[{0}] ==", sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
		}
		Log::Trace("", __FUNCTION__, "st_2 = [{0}]", st_2);
		if (st_2 != " " && st_2 != ""){
			sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + st_no + "' and ELM_CODE='001' and MATERIAL_CODE='" + material_code + "' and BACKLOG_EA='" + backlog_ea + "' ";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteReader();
			if (cmd.Read()){
				tfbsm14["C_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
				tfbsm14["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal() / 2;

			}
			cmd.Close();
			sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + st_no + "' and ELM_CODE='005' and MATERIAL_CODE='" + material_code + "' and BACKLOG_EA='" + backlog_ea + "' ";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteReader();
			if (cmd.Read()){
				tfbsm14["S_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
				tfbsm14["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal() / 2;

			}
			cmd.Close();
			//计算配料信息
			sqlstr = " select SUM(WEIGHT) WEIGHT, "
				" round(sum(SI_VALUE)/COUNT(*),2) SI_VALUE, "
				" round(sum(MN_VALUE)/COUNT(*),2) MN_VALUE, "
				" round(sum(P_VALUE)/COUNT(*),3) P_VALUE, "
				" round(sum(CR_VALUE)/COUNT(*),2) CR_VALUE, "
				" round(SUM(NI_VALUE)/COUNT(*),2) NI_VALUE, "
				" round(SUM(MO_VALUE)/COUNT(*),2) MO_VALUE, "
				" round(SUM(CU_VALUE)/COUNT(*),2) CU_VALUE, "
				" round(SUM(CO_VALUE)/COUNT(*),2) CO_VALUE  from tfbsm14 where COMPOSE_LIST_NO='" + compose_list_no + "' and STATION_ID='" + st_2 + "' and DATE_C='" + date + "' and ST_NO='" + st_no + "' and ( MAT_CODE not like 'PL00%' OR MAT_CODE = 'PL0003' ) ";
			Log::Trace("", __FUNCTION__, "== 14.sql[{0}] ==", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				//算出来PL0001的重量及成分
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
				//cmd_inq.Fetch(tfbsm14);
				tfbsm14["WEIGHT"] = cmd_inq.GetString(1);
				tfbsm14["SI_VALUE"] = cmd_inq.GetString(2);
				tfbsm14["MN_VALUE"] = cmd_inq.GetString(3);
				tfbsm14["P_VALUE"] = cmd_inq.GetString(4);
				tfbsm14["CR_VALUE"] = cmd_inq.GetString(5);
				tfbsm14["NI_VALUE"] = cmd_inq.GetString(6);
				tfbsm14["MO_VALUE"] = cmd_inq.GetString(7);
				tfbsm14["CU_VALUE"] = cmd_inq.GetString(8);
				tfbsm14["CO_VALUE"] = cmd_inq.GetString(9);
			}
			cmd_inq.Close();
			Log::Trace("", __FUNCTION__, "== WEIGHT.[{0}] ==", tfbsm14["WEIGHT"].ToString());
			tfbsm14["MAT_CODE"] = "PL0001";
			tfbsm14["MAT_NAME"] = "配料成分";
			tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14["ST_NO"] = st_no;
			tfbsm14["BACKLOG_EA"] = backlog_ea;
			tfbsm14["DATE_C"] = date;
			tfbsm14["REC_CREATOR"] = s.userid;
			tfbsm14["REC_CREATE_TIME"] = datetime;
			tfbsm14["STATION_ID"] = st_2;
			tfbsm14["MATERIAL_CODE"] = material_code;
			if (tfbsm14.QueryCount("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID") > 0){
				//已经存在了删除后新增
				tfbsm14.Delete("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID");
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
			}
			tfbsm14.Insert();
			//计算PL0002的重量及成分
			//去查询04表数据
			sqlstr = " select  round(1000/(METAL_CONS_PERT),5) AS METAL_CONS_PERT from ( "
				" SELECT A.*, "
				" ROW_NUMBER() OVER(PARTITION BY A.STATION_ID ORDER BY CASE "
				" WHEN A.ST_NO = '" + st_no + "' THEN 1 "
				" WHEN A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "' "
				" THEN 2 "
				" WHEN A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' "
				" THEN 3 "
				" WHEN A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "' "
				" THEN 4 "
				" WHEN A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' "
				" THEN 5 "
				" ELSE 99 END) as rn "
				" FROM TFBSM04 A "
				" WHERE(A.ST_NO = '" + st_no + "' OR(A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "') OR "
				" (A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "') OR "
				" (A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "') OR "
				" (A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "'))) WHERE rn = 1 and  STATION_ID = '" + st_2 + "'  ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				metal_cons_pert = cmd_inq.GetDecimal(1);
			}
			cmd_inq.Close();
			//查询原料
			sqlstr = " select CASE "
				" WHEN '" + backlog_ea + "' = '03' AND '" + st_2 + "' = 'E' AND "
				" (SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND '" + comm_fmly_code + "' != 'V' THEN 0.95 "
				" WHEN G.YEILD_MINUS = 0 THEN 1 "
				" ELSE G.YEILD_MINUS/100 END       YEILD_MINUS "
				" from(select * "
				" from tfbsm14 "
				" where COMPOSE_LIST_NO = '" + compose_list_no + "' "
				" and STATION_ID = '" + st_2 + "' "
				" and DATE_C = '" + date + "' "
				" and ST_NO = '" + st_no + "' "
				" and MAT_CODE not like 'PL00%'  "
				" ) S LEFT join TMMSM50 G on s.MAT_CODE = g.MAT_CODE ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read()){
				if (yeild_minus == " "){
					yeild_minus = yeild_minus + cmd_inq.GetString(1);
				}
				else{
					yeild_minus = yeild_minus + " * " + cmd_inq.GetString(1);
				}
			}
			cmd_inq.Close();
			Log::Trace("", __FUNCTION__, "== yeild_minus.[{0}] ==", yeild_minus);
			//计算PL0002出钢成分
			//tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal() * metal_cons_pert * yeild_minus;
			sqlstr = " SELECT " + tfbsm14["SI_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as SI_VALUE, "
				" " + tfbsm14["MN_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as MN_VALUE, "
				" " + tfbsm14["P_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as P_VALUE, "
				" " + tfbsm14["CR_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as CR_VALUE, "
				" " + tfbsm14["NI_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as NI_VALUE, "
				" " + tfbsm14["MO_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as MO_VALUE, "
				" " + tfbsm14["CU_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as CU_VALUE, "
				" " + tfbsm14["CO_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as CO_VALUE  "
				" FROM DUAL ";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", __FUNCTION__, "== PL0002/sqlstr.[{0}] ==", sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				//cmd_inq.Fetch(tfbsm14);
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
				tfbsm14["SI_VALUE"] = cmd_inq.GetString(1);
				tfbsm14["MN_VALUE"] = cmd_inq.GetString(2);
				tfbsm14["P_VALUE"] = cmd_inq.GetString(3);
				tfbsm14["CR_VALUE"] = cmd_inq.GetString(4);
				tfbsm14["NI_VALUE"] = cmd_inq.GetString(5);
				tfbsm14["MO_VALUE"] = cmd_inq.GetString(6);
				tfbsm14["CU_VALUE"] = cmd_inq.GetString(7);
				tfbsm14["CO_VALUE"] = cmd_inq.GetString(8);
			}
			cmd_inq.Close();
			tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal() * metal_cons_pert;
			Log::Trace("", __FUNCTION__, "=PL0002= WEIGHT.[{0}] ==", tfbsm14["WEIGHT"].ToString());
			tfbsm14["MAT_CODE"] = "PL0002";
			tfbsm14["MAT_NAME"] = "出钢成分";
			tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14["ST_NO"] = st_no;
			tfbsm14["BACKLOG_EA"] = backlog_ea;
			tfbsm14["DATE_C"] = date;
			tfbsm14["REC_CREATOR"] = s.userid;
			tfbsm14["REC_CREATE_TIME"] = datetime;
			tfbsm14["STATION_ID"] = st_2;
			tfbsm14["MATERIAL_CODE"] = material_code;
			if (tfbsm14.QueryCount("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID") > 0){
				//已经存在了删除后新增
				tfbsm14.Delete("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID");
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
			}
			//tfbsm14.Print();
			tfbsm14.Insert();
		}

		// 标记是否生成了PL0003
		bool has_pl0003 = false;

		// 1. 查询Z/E站的PL0002数据并汇总生成PL0003（STATION_ID=A）
		CString sql_z_e_pl0002 = "SELECT "
			"COMPOSE_LIST_NO, "
			"Z_WEIGHT + E_WEIGHT  AS TOTAL_WEIGHT, "
			"round(((Z_WEIGHT * Z_C_VALUE) + (E_WEIGHT * E_C_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS C_VALUE, "
			"round(((Z_WEIGHT * Z_S_VALUE) + (E_WEIGHT * E_S_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS S_VALUE, "
			"round(((Z_WEIGHT * Z_SI_VALUE) + (E_WEIGHT * E_SI_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS SI_VALUE, "
			"round(((Z_WEIGHT * Z_MN_VALUE) + (E_WEIGHT * E_MN_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS MN_VALUE, "
			"round(((Z_WEIGHT * Z_P_VALUE) + (E_WEIGHT * E_P_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS P_VALUE, "
			"round(((Z_WEIGHT * Z_CR_VALUE) + (E_WEIGHT * E_CR_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS CR_VALUE, "
			"round(((Z_WEIGHT * Z_NI_VALUE) + (E_WEIGHT * E_NI_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS NI_VALUE, "
			"round(((Z_WEIGHT * Z_MO_VALUE) + (E_WEIGHT * E_MO_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS MO_VALUE, "
			"round(((Z_WEIGHT * Z_CU_VALUE) + (E_WEIGHT * E_CU_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS CU_VALUE, "
			"round(((Z_WEIGHT * Z_CO_VALUE) + (E_WEIGHT * E_CO_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS CO_VALUE "
			"FROM ( "
			"    SELECT "
			"        COMPOSE_LIST_NO, "
			"        MAT_CODE, "
			"        DATE_C, "
			"        ST_NO, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN WEIGHT END), 0)  AS Z_WEIGHT, " 
			"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN WEIGHT END), 0)  AS E_WEIGHT, " 
			"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN C_VALUE END), 0)   AS Z_C_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN S_VALUE END), 0)   AS Z_S_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN SI_VALUE END), 0)  AS Z_SI_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN MN_VALUE END), 0)  AS Z_MN_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN P_VALUE END), 0)   AS Z_P_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN CR_VALUE END), 0)  AS Z_CR_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN NI_VALUE END), 0)  AS Z_NI_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN MO_VALUE END), 0)  AS Z_MO_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN CU_VALUE END), 0)  AS Z_CU_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN CO_VALUE END), 0)  AS Z_CO_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN C_VALUE END), 0)   AS E_C_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN S_VALUE END), 0)   AS E_S_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN SI_VALUE END), 0)  AS E_SI_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN MN_VALUE END), 0)  AS E_MN_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN P_VALUE END), 0)   AS E_P_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN CR_VALUE END), 0)  AS E_CR_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN NI_VALUE END), 0)  AS E_NI_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN MO_VALUE END), 0)  AS E_MO_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN CU_VALUE END), 0)  AS E_CU_VALUE, "
			"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN CO_VALUE END), 0)  AS E_CO_VALUE "
			"    FROM TFBSM14 "
			"    WHERE COMPOSE_LIST_NO = '" + compose_list_no + "' "
			"      AND STATION_ID IN ('Z', 'E') "
			"      AND MAT_CODE = 'PL0002' "
			"      AND DATE_C = '" + date + "' "
			"      AND ST_NO = '" + st_no + "' "
			"    GROUP BY COMPOSE_LIST_NO, MAT_CODE, DATE_C, ST_NO "
			") TEMP_TABLE";
		cmd_inq.SetCommandText(sql_z_e_pl0002);
		Log::Trace("", __FUNCTION__, "== sql_z_e_pl0002.sqlstr[{0}] ==", sql_z_e_pl0002.TrimOrBlank());
		cmd_inq.ExecuteReader();
		
		// 临时数组存储读取的值（精简核心：仅存字符串，避免CDecimal转换）
		CString pl0003_vals[13] = { "" }; 
		if (cmd_inq.Read()) {
			pl0003_vals[1] = cmd_inq.GetString(1).TrimOrBlank();  // TOTAL_WEIGHT
			pl0003_vals[2] = cmd_inq.GetString(2).TrimOrBlank();  // C_VALUE
			pl0003_vals[3] = cmd_inq.GetString(3).TrimOrBlank();  // S_VALUE
			pl0003_vals[4] = cmd_inq.GetString(4).TrimOrBlank();  // SI_VALUE
			pl0003_vals[5] = cmd_inq.GetString(5).TrimOrBlank();  // MN_VALUE
			pl0003_vals[6] = cmd_inq.GetString(6).TrimOrBlank();  // P_VALUE
			pl0003_vals[7] = cmd_inq.GetString(7).TrimOrBlank();  // CR_VALUE
			pl0003_vals[8] = cmd_inq.GetString(8).TrimOrBlank();  // NI_VALUE
			pl0003_vals[9] = cmd_inq.GetString(9).TrimOrBlank();  // MO_VALUE
			pl0003_vals[10] = cmd_inq.GetString(10).TrimOrBlank(); // CU_VALUE
			pl0003_vals[11] = cmd_inq.GetString(11).TrimOrBlank(); // CO_VALUE
			pl0003_vals[12] = cmd_inq.GetString(12).TrimOrBlank(); // CO_VALUE
			has_pl0003 = true;
		}
		cmd_inq.Close();

		// 2. 生成PL0003记录（插入TFBSM14，STATION_ID=A）
		if (has_pl0003) {
			tfbsm14.Reset();

			// 第一步：先赋值删除所需的关键字段（用于定位要删除的旧数据）
			tfbsm14["MAT_CODE"] = "PL0003";
			tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14["ST_NO"] = st_no;
			tfbsm14["BACKLOG_EA"] = backlog_ea;
			tfbsm14["DATE_C"] = date;
			tfbsm14["STATION_ID"] = "A";

			// 第二步：先删后插（核心优化：删除逻辑提前）
			if (tfbsm14.QueryCount("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID") > 0) {
				tfbsm14.Delete("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID");
				Log::Trace("", __FUNCTION__, "== 删除旧的PL0003记录成功 ==");
			}

			tfbsm14["MAT_NAME"] = "预溶液成分";
			tfbsm14["REC_CREATOR"] = s.userid;
			tfbsm14["REC_CREATE_TIME"] = datetime;
			tfbsm14["MATERIAL_CODE"] = material_code;
			tfbsm14["WEIGHT"] = pl0003_vals[2];          // TOTAL_WEIGHT
			tfbsm14["C_VALUE"] = pl0003_vals[3];          // C_VALUE
			tfbsm14["S_VALUE"] = pl0003_vals[4];          // S_VALUE
			tfbsm14["SI_VALUE"] = pl0003_vals[5];         // SI_VALUE
			tfbsm14["MN_VALUE"] = pl0003_vals[6];         // MN_VALUE
			tfbsm14["P_VALUE"] = pl0003_vals[7];          // P_VALUE
			tfbsm14["CR_VALUE"] = pl0003_vals[8];         // CR_VALUE
			tfbsm14["NI_VALUE"] = pl0003_vals[9];         // NI_VALUE
			tfbsm14["MO_VALUE"] = pl0003_vals[10];         // MO_VALUE
			tfbsm14["CU_VALUE"] = pl0003_vals[11];        // CU_VALUE
			tfbsm14["CO_VALUE"] = pl0003_vals[12];        // CO_VALUE
			
			CString sql_insert_pl0003 = "INSERT INTO TFBSM14 ("
				"    REC_CREATOR,REC_CREATE_TIME,DATE_C,ST_NO,BACKLOG_EA,"
				"    MAT_CODE,MAT_NAME,COMPOSE_LIST_NO,WEIGHT,C_VALUE,"
				"    SI_VALUE,MN_VALUE,P_VALUE,S_VALUE,CR_VALUE,"
				"    NI_VALUE,MO_VALUE,CU_VALUE,CO_VALUE,STATION_ID,MATERIAL_CODE"
				") VALUES ("
				"    '" + tfbsm14["REC_CREATOR"].ToString() + "',"
				"    '" + datetime + "',"
				"    '" + tfbsm14["DATE_C"].ToString() + "',"
				"    '" + tfbsm14["ST_NO"].ToString() + "',"
				"    '" + tfbsm14["BACKLOG_EA"].ToString() + "',"
				"    '" + tfbsm14["MAT_CODE"].ToString() + "',"
				"    '" + tfbsm14["MAT_NAME"].ToString() + "',"
				"    '" + tfbsm14["COMPOSE_LIST_NO"].ToString() + "',"
				"    " + tfbsm14["WEIGHT"].ToString() + ","
				"    " + tfbsm14["C_VALUE"].ToString() + ","
				"    " + tfbsm14["SI_VALUE"].ToString() + ","
				"    " + tfbsm14["MN_VALUE"].ToString() + ","
				"    " + tfbsm14["P_VALUE"].ToString() + ","
				"    " + tfbsm14["S_VALUE"].ToString() + ","
				"    " + tfbsm14["CR_VALUE"].ToString() + ","
				"    " + tfbsm14["NI_VALUE"].ToString() + ","
				"    " + tfbsm14["MO_VALUE"].ToString() + ","
				"    " + tfbsm14["CU_VALUE"].ToString() + ","
				"    " + tfbsm14["CO_VALUE"].ToString() + ","
				"    '" + tfbsm14["STATION_ID"].ToString() + "',"
				"    '" + tfbsm14["MATERIAL_CODE"].ToString() + "' "
				" ) ";
			cmd_inq.SetCommandText(sql_insert_pl0003);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
			Log::Trace("", __FUNCTION__, "== insert PL0003.[{0}] ==", sql_insert_pl0003);

			// 标记是否生成了PL0003
			bool has_pl0003 = false;

			// 1. 查询Z/E站的PL0002数据并汇总生成PL0003（STATION_ID=A）
			CString sql_z_e_pl0002 = "SELECT "
				"COMPOSE_LIST_NO, "
				"Z_WEIGHT + E_WEIGHT  AS TOTAL_WEIGHT, "
				"round(((Z_WEIGHT * Z_C_VALUE) + (E_WEIGHT * E_C_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS C_VALUE, "
				"round(((Z_WEIGHT * Z_S_VALUE) + (E_WEIGHT * E_S_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS S_VALUE, "
				"round(((Z_WEIGHT * Z_SI_VALUE) + (E_WEIGHT * E_SI_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS SI_VALUE, "
				"round(((Z_WEIGHT * Z_MN_VALUE) + (E_WEIGHT * E_MN_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS MN_VALUE, "
				"round(((Z_WEIGHT * Z_P_VALUE) + (E_WEIGHT * E_P_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS P_VALUE, "
				"round(((Z_WEIGHT * Z_CR_VALUE) + (E_WEIGHT * E_CR_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS CR_VALUE, "
				"round(((Z_WEIGHT * Z_NI_VALUE) + (E_WEIGHT * E_NI_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS NI_VALUE, "
				"round(((Z_WEIGHT * Z_MO_VALUE) + (E_WEIGHT * E_MO_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS MO_VALUE, "
				"round(((Z_WEIGHT * Z_CU_VALUE) + (E_WEIGHT * E_CU_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS CU_VALUE, "
				"round(((Z_WEIGHT * Z_CO_VALUE) + (E_WEIGHT * E_CO_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS CO_VALUE "
				"FROM ( "
				"    SELECT "
				"        COMPOSE_LIST_NO, "
				"        MAT_CODE, "
				"        DATE_C, "
				"        ST_NO, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN WEIGHT END), 0)  AS Z_WEIGHT, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN WEIGHT END), 0)  AS E_WEIGHT, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN C_VALUE END), 0)   AS Z_C_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN S_VALUE END), 0)   AS Z_S_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN SI_VALUE END), 0)  AS Z_SI_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN MN_VALUE END), 0)  AS Z_MN_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN P_VALUE END), 0)   AS Z_P_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN CR_VALUE END), 0)  AS Z_CR_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN NI_VALUE END), 0)  AS Z_NI_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN MO_VALUE END), 0)  AS Z_MO_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN CU_VALUE END), 0)  AS Z_CU_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN CO_VALUE END), 0)  AS Z_CO_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN C_VALUE END), 0)   AS E_C_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN S_VALUE END), 0)   AS E_S_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN SI_VALUE END), 0)  AS E_SI_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN MN_VALUE END), 0)  AS E_MN_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN P_VALUE END), 0)   AS E_P_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN CR_VALUE END), 0)  AS E_CR_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN NI_VALUE END), 0)  AS E_NI_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN MO_VALUE END), 0)  AS E_MO_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN CU_VALUE END), 0)  AS E_CU_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN CO_VALUE END), 0)  AS E_CO_VALUE "
				"    FROM TFBSM14 "
				"    WHERE COMPOSE_LIST_NO = '" + compose_list_no + "' "
				"      AND STATION_ID IN ('Z', 'E') "
				"      AND MAT_CODE = 'PL0002' "
				"      AND DATE_C = '" + date + "' "
				"      AND ST_NO = '" + st_no + "' "
				"    GROUP BY COMPOSE_LIST_NO, MAT_CODE, DATE_C, ST_NO "
				") TEMP_TABLE";
			cmd_inq.SetCommandText(sql_z_e_pl0002);
			Log::Trace("", __FUNCTION__, "== sql_z_e_pl0002.sqlstr[{0}] ==", sql_z_e_pl0002.TrimOrBlank());
			cmd_inq.ExecuteReader();

			// 临时数组存储读取的值（精简核心：仅存字符串，避免CDecimal转换）
			CString pl0003_vals[13] = { "" };
			if (cmd_inq.Read()) {
				pl0003_vals[1] = cmd_inq.GetString(1).TrimOrBlank();  // TOTAL_WEIGHT
				pl0003_vals[2] = cmd_inq.GetString(2).TrimOrBlank();  // C_VALUE
				pl0003_vals[3] = cmd_inq.GetString(3).TrimOrBlank();  // S_VALUE
				pl0003_vals[4] = cmd_inq.GetString(4).TrimOrBlank();  // SI_VALUE
				pl0003_vals[5] = cmd_inq.GetString(5).TrimOrBlank();  // MN_VALUE
				pl0003_vals[6] = cmd_inq.GetString(6).TrimOrBlank();  // P_VALUE
				pl0003_vals[7] = cmd_inq.GetString(7).TrimOrBlank();  // CR_VALUE
				pl0003_vals[8] = cmd_inq.GetString(8).TrimOrBlank();  // NI_VALUE
				pl0003_vals[9] = cmd_inq.GetString(9).TrimOrBlank();  // MO_VALUE
				pl0003_vals[10] = cmd_inq.GetString(10).TrimOrBlank(); // CU_VALUE
				pl0003_vals[11] = cmd_inq.GetString(11).TrimOrBlank(); // CO_VALUE
				pl0003_vals[12] = cmd_inq.GetString(12).TrimOrBlank(); // CO_VALUE
				has_pl0003 = true;
			}
			cmd_inq.Close();

			// 2. 生成PL0003记录（插入TFBSM14，STATION_ID=A）
			if (has_pl0003) {
				tfbsm14.Reset();

				// 第一步：先赋值删除所需的关键字段（用于定位要删除的旧数据）
				tfbsm14["MAT_CODE"] = "PL0003";
				tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
				tfbsm14["ST_NO"] = st_no;
				tfbsm14["BACKLOG_EA"] = backlog_ea;
				tfbsm14["DATE_C"] = date;
				tfbsm14["STATION_ID"] = "A";

				// 第二步：先删后插（核心优化：删除逻辑提前）
				if (tfbsm14.QueryCount("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID") > 0) {
					tfbsm14.Delete("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID");
					Log::Trace("", __FUNCTION__, "== 删除旧的PL0003记录成功 ==");
				}

				tfbsm14["MAT_NAME"] = "预溶液成分";
				tfbsm14["REC_CREATOR"] = s.userid;
				tfbsm14["REC_CREATE_TIME"] = datetime;
				tfbsm14["MATERIAL_CODE"] = material_code;
				tfbsm14["WEIGHT"] = pl0003_vals[2];          // TOTAL_WEIGHT
				tfbsm14["C_VALUE"] = pl0003_vals[3];          // C_VALUE
				tfbsm14["S_VALUE"] = pl0003_vals[4];          // S_VALUE
				tfbsm14["SI_VALUE"] = pl0003_vals[5];         // SI_VALUE
				tfbsm14["MN_VALUE"] = pl0003_vals[6];         // MN_VALUE
				tfbsm14["P_VALUE"] = pl0003_vals[7];          // P_VALUE
				tfbsm14["CR_VALUE"] = pl0003_vals[8];         // CR_VALUE
				tfbsm14["NI_VALUE"] = pl0003_vals[9];         // NI_VALUE
				tfbsm14["MO_VALUE"] = pl0003_vals[10];         // MO_VALUE
				tfbsm14["CU_VALUE"] = pl0003_vals[11];        // CU_VALUE
				tfbsm14["CO_VALUE"] = pl0003_vals[12];        // CO_VALUE

				CString sql_insert_pl0003 = "INSERT INTO TFBSM14 ("
					"    REC_CREATOR,REC_CREATE_TIME,DATE_C,ST_NO,BACKLOG_EA,"
					"    MAT_CODE,MAT_NAME,COMPOSE_LIST_NO,WEIGHT,C_VALUE,"
					"    SI_VALUE,MN_VALUE,P_VALUE,S_VALUE,CR_VALUE,"
					"    NI_VALUE,MO_VALUE,CU_VALUE,CO_VALUE,STATION_ID,MATERIAL_CODE"
					") VALUES ("
					"    '" + tfbsm14["REC_CREATOR"].ToString() + "',"
					"    '" + datetime + "',"
					"    '" + tfbsm14["DATE_C"].ToString() + "',"
					"    '" + tfbsm14["ST_NO"].ToString() + "',"
					"    '" + tfbsm14["BACKLOG_EA"].ToString() + "',"
					"    '" + tfbsm14["MAT_CODE"].ToString() + "',"
					"    '" + tfbsm14["MAT_NAME"].ToString() + "',"
					"    '" + tfbsm14["COMPOSE_LIST_NO"].ToString() + "',"
					"    " + tfbsm14["WEIGHT"].ToString() + ","
					"    " + tfbsm14["C_VALUE"].ToString() + ","
					"    " + tfbsm14["SI_VALUE"].ToString() + ","
					"    " + tfbsm14["MN_VALUE"].ToString() + ","
					"    " + tfbsm14["P_VALUE"].ToString() + ","
					"    " + tfbsm14["S_VALUE"].ToString() + ","
					"    " + tfbsm14["CR_VALUE"].ToString() + ","
					"    " + tfbsm14["NI_VALUE"].ToString() + ","
					"    " + tfbsm14["MO_VALUE"].ToString() + ","
					"    " + tfbsm14["CU_VALUE"].ToString() + ","
					"    " + tfbsm14["CO_VALUE"].ToString() + ","
					"    '" + tfbsm14["STATION_ID"].ToString() + "',"
					"    '" + tfbsm14["MATERIAL_CODE"].ToString() + "' "
					" ) ";
				cmd_inq.SetCommandText(sql_insert_pl0003);
				cmd_inq.ExecuteNonQuery();
				cmd_inq.Close();
				Log::Trace("", __FUNCTION__, "== insert PL0003.[{0}] ==", sql_insert_pl0003);
			}
		}

		Log::Trace("", __FUNCTION__, "st_3 = [{0}]", st_3);
		if (st_3 != " " && st_3 != ""){
			sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + st_no + "' and ELM_CODE='001' and MATERIAL_CODE='" + material_code + "' and BACKLOG_EA='" + backlog_ea + "' ";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteReader();
			if (cmd.Read()){
				tfbsm14["C_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
				tfbsm14["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal() / 2;

			}
			cmd.Close();
			sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + st_no + "' and ELM_CODE='005' and MATERIAL_CODE='" + material_code + "' and BACKLOG_EA='" + backlog_ea + "' ";
			cmd.SetCommandText(sqlstr);
			cmd.ExecuteReader();
			if (cmd.Read()){
				tfbsm14["S_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
				tfbsm14["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal() / 2;

			}
			cmd.Close();
			//计算配料信息
			sqlstr = " select SUM(WEIGHT) WEIGHT, "
				" round(sum(SI_VALUE)/COUNT(*),2) SI_VALUE, "
				" round(sum(MN_VALUE)/COUNT(*),2) MN_VALUE, "
				" round(sum(P_VALUE)/COUNT(*),3) P_VALUE, "
				" round(sum(CR_VALUE)/COUNT(*),2) CR_VALUE, "
				" round(SUM(NI_VALUE)/COUNT(*),2) NI_VALUE, "
				" round(SUM(MO_VALUE)/COUNT(*),2) MO_VALUE, "
				" round(SUM(CU_VALUE)/COUNT(*),2) CU_VALUE, "
				" round(SUM(CO_VALUE)/COUNT(*),2) CO_VALUE  from tfbsm14 where COMPOSE_LIST_NO='" + compose_list_no + "' and STATION_ID='" + st_3 + "' and DATE_C='" + date + "' and ST_NO='" + st_no + "' and ( MAT_CODE not like 'PL00%' OR MAT_CODE = 'PL0003') ";
			Log::Trace("", __FUNCTION__, "== 14.sql[{0}] ==", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				//算出来PL0001的重量及成分
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
				//cmd_inq.Fetch(tfbsm14);
				tfbsm14["WEIGHT"] = cmd_inq.GetString(1);
				tfbsm14["SI_VALUE"] = cmd_inq.GetString(2);
				tfbsm14["MN_VALUE"] = cmd_inq.GetString(3);
				tfbsm14["P_VALUE"] = cmd_inq.GetString(4);
				tfbsm14["CR_VALUE"] = cmd_inq.GetString(5);
				tfbsm14["NI_VALUE"] = cmd_inq.GetString(6);
				tfbsm14["MO_VALUE"] = cmd_inq.GetString(7);
				tfbsm14["CU_VALUE"] = cmd_inq.GetString(8);
				tfbsm14["CO_VALUE"] = cmd_inq.GetString(9);
			}
			cmd_inq.Close();
			Log::Trace("", __FUNCTION__, "== WEIGHT.[{0}] ==", tfbsm14["WEIGHT"].ToString());
			tfbsm14["MAT_CODE"] = "PL0001";
			tfbsm14["MAT_NAME"] = "配料成分";
			tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14["ST_NO"] = st_no;
			tfbsm14["BACKLOG_EA"] = backlog_ea;
			tfbsm14["DATE_C"] = date;
			tfbsm14["REC_CREATOR"] = s.userid;
			tfbsm14["REC_CREATE_TIME"] = datetime;
			tfbsm14["STATION_ID"] = st_3;
			tfbsm14["MATERIAL_CODE"] = material_code;
			if (tfbsm14.QueryCount("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID") > 0){
				//已经存在了删除后新增
				tfbsm14.Delete("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID");
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
			}
			tfbsm14.Insert();
			//计算PL0002的重量及成分
			//去查询04表数据
			sqlstr = " select  round(1000/(METAL_CONS_PERT),5) AS METAL_CONS_PERT from ( "
				" SELECT A.*, "
				" ROW_NUMBER() OVER(PARTITION BY A.STATION_ID ORDER BY CASE "
				" WHEN A.ST_NO = '" + st_no + "' THEN 1 "
				" WHEN A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "' "
				" THEN 2 "
				" WHEN A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' "
				" THEN 3 "
				" WHEN A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "' "
				" THEN 4 "
				" WHEN A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' "
				" THEN 5 "
				" ELSE 99 END) as rn "
				" FROM TFBSM04 A "
				" WHERE(A.ST_NO = '" + st_no + "' OR(A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "') OR "
				" (A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "') OR "
				" (A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "') OR "
				" (A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "'))) WHERE rn = 1 and  STATION_ID = '" + st_3 + "'  ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				metal_cons_pert = cmd_inq.GetDecimal(1);
			}
			cmd_inq.Close();
			//查询原料
			sqlstr = " select CASE "
				" WHEN '" + backlog_ea + "' = '03' AND '" + st_3 + "' = 'E' AND "
				" (SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND '" + comm_fmly_code + "' != 'V' THEN 0.95 "
				" WHEN G.YEILD_MINUS = 0 THEN 1 "
				" ELSE G.YEILD_MINUS/100 END       YEILD_MINUS "
				" from(select * "
				" from tfbsm14 "
				" where COMPOSE_LIST_NO = '" + compose_list_no + "' "
				" and STATION_ID = '" + st_3 + "' "
				" and DATE_C = '" + date + "' "
				" and ST_NO = '" + st_no + "' "
				" and MAT_CODE not like 'PL00%' "
				" ) S LEFT join TMMSM50 G on s.MAT_CODE = g.MAT_CODE ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read()){
				if (yeild_minus == " "){
					yeild_minus = yeild_minus + cmd_inq.GetString(1);
				}
				else{
					yeild_minus = yeild_minus + " * " + cmd_inq.GetString(1);
				}
			}
			cmd_inq.Close();
			Log::Trace("", __FUNCTION__, "== yeild_minus.[{0}] ==", yeild_minus);
			//计算PL0002出钢成分
			//tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal() * metal_cons_pert * yeild_minus;
			sqlstr = " SELECT " + tfbsm14["SI_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as SI_VALUE, "
				" " + tfbsm14["MN_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as MN_VALUE, "
				" " + tfbsm14["P_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as P_VALUE, "
				" " + tfbsm14["CR_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as CR_VALUE, "
				" " + tfbsm14["NI_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as NI_VALUE, "
				" " + tfbsm14["MO_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as MO_VALUE, "
				" " + tfbsm14["CU_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as CU_VALUE, "
				" " + tfbsm14["CO_VALUE"].ToString() + " * " + metal_cons_pert.ToString() + "* " + yeild_minus + " as CO_VALUE  "
				" FROM DUAL ";
			cmd_inq.SetCommandText(sqlstr);
			Log::Trace("", __FUNCTION__, "== PL0002/sqlstr.[{0}] ==", sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				//cmd_inq.Fetch(tfbsm14);
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
				tfbsm14["SI_VALUE"] = cmd_inq.GetString(1);
				tfbsm14["MN_VALUE"] = cmd_inq.GetString(2);
				tfbsm14["P_VALUE"] = cmd_inq.GetString(3);
				tfbsm14["CR_VALUE"] = cmd_inq.GetString(4);
				tfbsm14["NI_VALUE"] = cmd_inq.GetString(5);
				tfbsm14["MO_VALUE"] = cmd_inq.GetString(6);
				tfbsm14["CU_VALUE"] = cmd_inq.GetString(7);
				tfbsm14["CO_VALUE"] = cmd_inq.GetString(8);
			}
			cmd_inq.Close();
			tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal() * metal_cons_pert;
			Log::Trace("", __FUNCTION__, "=PL0002= WEIGHT.[{0}] ==", tfbsm14["WEIGHT"].ToString());
			tfbsm14["MAT_CODE"] = "PL0002";
			tfbsm14["MAT_NAME"] = "出钢成分";
			tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
			tfbsm14["ST_NO"] = st_no;
			tfbsm14["BACKLOG_EA"] = backlog_ea;
			tfbsm14["DATE_C"] = date;
			tfbsm14["REC_CREATOR"] = s.userid;
			tfbsm14["REC_CREATE_TIME"] = datetime;
			tfbsm14["STATION_ID"] = st_3;
			tfbsm14["MATERIAL_CODE"] = material_code;
			if (tfbsm14.QueryCount("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID") > 0){
				//已经存在了删除后新增
				tfbsm14.Delete("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID");
				Log::Trace("", __FUNCTION__, "== .sqlstr[{0}] ==", __LINE__);
			}
			//tfbsm14.Print();
			tfbsm14.Insert();
			// 标记是否生成了PL0003
			bool has_pl0003 = false;

			// 1. 查询Z/E站的PL0002数据并汇总生成PL0003（STATION_ID=A）
			CString sql_z_e_pl0002 = "SELECT "
				"COMPOSE_LIST_NO, "
				"Z_WEIGHT + E_WEIGHT  AS TOTAL_WEIGHT, "
				"round(((Z_WEIGHT * Z_C_VALUE) + (E_WEIGHT * E_C_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS C_VALUE, "
				"round(((Z_WEIGHT * Z_S_VALUE) + (E_WEIGHT * E_S_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS S_VALUE, "
				"round(((Z_WEIGHT * Z_SI_VALUE) + (E_WEIGHT * E_SI_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS SI_VALUE, "
				"round(((Z_WEIGHT * Z_MN_VALUE) + (E_WEIGHT * E_MN_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS MN_VALUE, "
				"round(((Z_WEIGHT * Z_P_VALUE) + (E_WEIGHT * E_P_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS P_VALUE, "
				"round(((Z_WEIGHT * Z_CR_VALUE) + (E_WEIGHT * E_CR_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS CR_VALUE, "
				"round(((Z_WEIGHT * Z_NI_VALUE) + (E_WEIGHT * E_NI_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS NI_VALUE, "
				"round(((Z_WEIGHT * Z_MO_VALUE) + (E_WEIGHT * E_MO_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS MO_VALUE, "
				"round(((Z_WEIGHT * Z_CU_VALUE) + (E_WEIGHT * E_CU_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS CU_VALUE, "
				"round(((Z_WEIGHT * Z_CO_VALUE) + (E_WEIGHT * E_CO_VALUE)) / NVL(NULLIF(Z_WEIGHT + E_WEIGHT, 0), 1), 3) AS CO_VALUE "
				"FROM ( "
				"    SELECT "
				"        COMPOSE_LIST_NO, "
				"        MAT_CODE, "
				"        DATE_C, "
				"        ST_NO, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN WEIGHT END), 0)  AS Z_WEIGHT, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN WEIGHT END), 0)  AS E_WEIGHT, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN C_VALUE END), 0)   AS Z_C_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN S_VALUE END), 0)   AS Z_S_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN SI_VALUE END), 0)  AS Z_SI_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN MN_VALUE END), 0)  AS Z_MN_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN P_VALUE END), 0)   AS Z_P_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN CR_VALUE END), 0)  AS Z_CR_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN NI_VALUE END), 0)  AS Z_NI_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN MO_VALUE END), 0)  AS Z_MO_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN CU_VALUE END), 0)  AS Z_CU_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'Z' THEN CO_VALUE END), 0)  AS Z_CO_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN C_VALUE END), 0)   AS E_C_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN S_VALUE END), 0)   AS E_S_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN SI_VALUE END), 0)  AS E_SI_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN MN_VALUE END), 0)  AS E_MN_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN P_VALUE END), 0)   AS E_P_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN CR_VALUE END), 0)  AS E_CR_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN NI_VALUE END), 0)  AS E_NI_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN MO_VALUE END), 0)  AS E_MO_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN CU_VALUE END), 0)  AS E_CU_VALUE, "
				"        NVL(MAX(CASE WHEN STATION_ID = 'E' THEN CO_VALUE END), 0)  AS E_CO_VALUE "
				"    FROM TFBSM14 "
				"    WHERE COMPOSE_LIST_NO = '" + compose_list_no + "' "
				"      AND STATION_ID IN ('Z', 'E') "
				"      AND MAT_CODE = 'PL0002' "
				"      AND DATE_C = '" + date + "' "
				"      AND ST_NO = '" + st_no + "' "
				"    GROUP BY COMPOSE_LIST_NO, MAT_CODE, DATE_C, ST_NO "
				") TEMP_TABLE";
			cmd_inq.SetCommandText(sql_z_e_pl0002);
			Log::Trace("", __FUNCTION__, "== sql_z_e_pl0002.sqlstr[{0}] ==", sql_z_e_pl0002.TrimOrBlank());
			cmd_inq.ExecuteReader();

			// 临时数组存储读取的值（精简核心：仅存字符串，避免CDecimal转换）
			CString pl0003_vals[13] = { "" };
			if (cmd_inq.Read()) {
				pl0003_vals[1] = cmd_inq.GetString(1).TrimOrBlank();  // TOTAL_WEIGHT
				pl0003_vals[2] = cmd_inq.GetString(2).TrimOrBlank();  // C_VALUE
				pl0003_vals[3] = cmd_inq.GetString(3).TrimOrBlank();  // S_VALUE
				pl0003_vals[4] = cmd_inq.GetString(4).TrimOrBlank();  // SI_VALUE
				pl0003_vals[5] = cmd_inq.GetString(5).TrimOrBlank();  // MN_VALUE
				pl0003_vals[6] = cmd_inq.GetString(6).TrimOrBlank();  // P_VALUE
				pl0003_vals[7] = cmd_inq.GetString(7).TrimOrBlank();  // CR_VALUE
				pl0003_vals[8] = cmd_inq.GetString(8).TrimOrBlank();  // NI_VALUE
				pl0003_vals[9] = cmd_inq.GetString(9).TrimOrBlank();  // MO_VALUE
				pl0003_vals[10] = cmd_inq.GetString(10).TrimOrBlank(); // CU_VALUE
				pl0003_vals[11] = cmd_inq.GetString(11).TrimOrBlank(); // CO_VALUE
				pl0003_vals[12] = cmd_inq.GetString(12).TrimOrBlank(); // CO_VALUE
				has_pl0003 = true;
			}
			cmd_inq.Close();

			// 2. 生成PL0003记录（插入TFBSM14，STATION_ID=A）
			if (has_pl0003) {
				tfbsm14.Reset();

				// 第一步：先赋值删除所需的关键字段（用于定位要删除的旧数据）
				tfbsm14["MAT_CODE"] = "PL0003";
				tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
				tfbsm14["ST_NO"] = st_no;
				tfbsm14["BACKLOG_EA"] = backlog_ea;
				tfbsm14["DATE_C"] = date;
				tfbsm14["STATION_ID"] = "A";

				// 第二步：先删后插（核心优化：删除逻辑提前）
				if (tfbsm14.QueryCount("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID") > 0) {
					tfbsm14.Delete("MAT_CODE,COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C,STATION_ID");
					Log::Trace("", __FUNCTION__, "== 删除旧的PL0003记录成功 ==");
				}

				tfbsm14["MAT_NAME"] = "预溶液成分";
				tfbsm14["REC_CREATOR"] = s.userid;
				tfbsm14["REC_CREATE_TIME"] = datetime;
				tfbsm14["MATERIAL_CODE"] = material_code;
				tfbsm14["WEIGHT"] = pl0003_vals[2];          // TOTAL_WEIGHT
				tfbsm14["C_VALUE"] = pl0003_vals[3];          // C_VALUE
				tfbsm14["S_VALUE"] = pl0003_vals[4];          // S_VALUE
				tfbsm14["SI_VALUE"] = pl0003_vals[5];         // SI_VALUE
				tfbsm14["MN_VALUE"] = pl0003_vals[6];         // MN_VALUE
				tfbsm14["P_VALUE"] = pl0003_vals[7];          // P_VALUE
				tfbsm14["CR_VALUE"] = pl0003_vals[8];         // CR_VALUE
				tfbsm14["NI_VALUE"] = pl0003_vals[9];         // NI_VALUE
				tfbsm14["MO_VALUE"] = pl0003_vals[10];         // MO_VALUE
				tfbsm14["CU_VALUE"] = pl0003_vals[11];        // CU_VALUE
				tfbsm14["CO_VALUE"] = pl0003_vals[12];        // CO_VALUE

				CString sql_insert_pl0003 = "INSERT INTO TFBSM14 ("
					"    REC_CREATOR,REC_CREATE_TIME,DATE_C,ST_NO,BACKLOG_EA,"
					"    MAT_CODE,MAT_NAME,COMPOSE_LIST_NO,WEIGHT,C_VALUE,"
					"    SI_VALUE,MN_VALUE,P_VALUE,S_VALUE,CR_VALUE,"
					"    NI_VALUE,MO_VALUE,CU_VALUE,CO_VALUE,STATION_ID,MATERIAL_CODE"
					") VALUES ("
					"    '" + tfbsm14["REC_CREATOR"].ToString() + "',"
					"    '" + datetime + "',"
					"    '" + tfbsm14["DATE_C"].ToString() + "',"
					"    '" + tfbsm14["ST_NO"].ToString() + "',"
					"    '" + tfbsm14["BACKLOG_EA"].ToString() + "',"
					"    '" + tfbsm14["MAT_CODE"].ToString() + "',"
					"    '" + tfbsm14["MAT_NAME"].ToString() + "',"
					"    '" + tfbsm14["COMPOSE_LIST_NO"].ToString() + "',"
					"    " + tfbsm14["WEIGHT"].ToString() + ","
					"    " + tfbsm14["C_VALUE"].ToString() + ","
					"    " + tfbsm14["SI_VALUE"].ToString() + ","
					"    " + tfbsm14["MN_VALUE"].ToString() + ","
					"    " + tfbsm14["P_VALUE"].ToString() + ","
					"    " + tfbsm14["S_VALUE"].ToString() + ","
					"    " + tfbsm14["CR_VALUE"].ToString() + ","
					"    " + tfbsm14["NI_VALUE"].ToString() + ","
					"    " + tfbsm14["MO_VALUE"].ToString() + ","
					"    " + tfbsm14["CU_VALUE"].ToString() + ","
					"    " + tfbsm14["CO_VALUE"].ToString() + ","
					"    '" + tfbsm14["STATION_ID"].ToString() + "',"
					"    '" + tfbsm14["MATERIAL_CODE"].ToString() + "' "
					" ) ";
				cmd_inq.SetCommandText(sql_insert_pl0003);
				cmd_inq.ExecuteNonQuery();
				cmd_inq.Close();
				Log::Trace("", __FUNCTION__, "== insert PL0003.[{0}] ==", sql_insert_pl0003);
			}
		}
		////返回新的配料单
		Log::Trace("", __FUNCTION__, "linke = [{0}]", __LINE__);
		bcls_ret->Tables[0].Rows[0]["COMPOSE_LIST_NO"] = compose_list_no;
		Log::Trace("", __FUNCTION__, "bcls_ret.linke = [{0}]", compose_list_no);
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
