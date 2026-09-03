/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2026/3/5
Description: 配料单的信息新增
配料单生成规则：//钢种大类(A/M/P/D/F)+钢种小类编号+工艺路线编号+日期（6）+2位流水
**************************************************/
#include "stdafx.h"

// Service 入口
BM2F_ENTERACE(fbsm15z_save)
int f_mmsm_getprice(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//价格计算
int f_fbsm15z_save(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int doFlag = 0;
	int affectRows = 0;
	CString sqlstr = " ";
	CString dateNow = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CDecimal unit_price = 0;
	CDecimal unit_cost = 0;
	CDecimal aod_wt = 0;
	CModel tfbsm14("TFBSM14");
	CModel tfbsm12("TFBSM12");
	CModel tfbsm14a("TFBSM14A");

	CString back_if = " ";
	CString back_eaf = " ";
	CString back_bof = " ";
	CString back_aod = " ";
	CString back_ds = " ";	

	CDecimal p_seq = 0;
	CString compose_list_no = " ";//配料单号

	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);
	CDbCommand cmd_inq_2(conn);
	try
	{
		EIClass bcls_rec_rep;
		EIClass bcls_ret_rep;
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_CODE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_NAME");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "CR_VALUE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "NI_VALUE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MO_VALUE");
		bcls_rec_rep.Tables[0].Rows.Add();

		

		//先删除，再插入
		

		if (bcls_rec->Tables.Contains("Main_MODIFY"))
		{
			for (int i = 0; i < bcls_rec->Tables["Main_MODIFY"].Rows.get_Count(); i++)
			{
				tfbsm12.Reset();
				tfbsm12.MergeFrom(bcls_rec->Tables["Main_MODIFY"].Rows[i]);
				tfbsm12.TrimOrBlank();
				tfbsm12["REC_REVISOR"] = s.userid;
				tfbsm12["REC_REVISE_TIME"] = dateNow;
				//修改备注信息
				tfbsm12.Update("REC_REVISOR,REC_REVISE_TIME,REMARK","ST_NO,DATE_TIME,BACKLOG_EA,COMPOSE_LIST_NO");
			}
		}
		if (bcls_rec->Tables.Contains("query"))
		{
			tfbsm12.MergeFrom(bcls_rec->Tables["query"].Rows[0]);
			compose_list_no = tfbsm12["COMPOSE_LIST_NO"].ToString();
			//	Log::Trace("", __FUNCTION__, "1111compose_list_no = [{0}],st_no = [{1}],date_time = [{2}]", compose_list_no, tfbsm12["ST_NO"].ToString(), tfbsm12["DATE_TIME"].ToString());

			//判断配料单是否已经审核
			sqlstr = " select CHECK_FLAG,BACKLOG_EA,DATE_TIME,MATERIAL_CODE,DATE_C,ORIGIN_CODE,ST_NO"
				" from tfbsm12"
				" where 1=1"
				" and st_no=@st_no"
				" and COMPOSE_LIST_NO=@compose_list_no"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("st_no", tfbsm12["ST_NO"].ToString());
			cmd_inq.Parameters.Set("compose_list_no", tfbsm12["COMPOSE_LIST_NO"].ToString());
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				if (cmd_inq.GetString(1) == "1")
				{
					sprintf(s.msg, "该配料单号已经审核,不能修改保存。");
					//strcpy(s.sysmsg,s.msg);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tfbsm12["BACKLOG_EA"] = cmd_inq.GetString(2);
				tfbsm12["DATE_TIME"] = cmd_inq.GetString(3);
				tfbsm12["MATERIAL_CODE"] = cmd_inq.GetString(4);
				tfbsm12["DATE_C"] = cmd_inq.GetString(5);
				tfbsm12["ORIGIN_CODE"] = cmd_inq.GetString(6);
				tfbsm12["ST_NO"] = cmd_inq.GetString(7);

				if (cmd_inq.GetString(6) != "1")
				{
					sqlstr = " select  max(substr(COMPOSE_LIST_NO,12,14)),max(substr(COMPOSE_LIST_NO,14,1)),max(substr(COMPOSE_LIST_NO,13, 1)),max(substr(COMPOSE_LIST_NO,13, 2))"
						" from tfbsm14a"
						" where 1=1  "
						" and compose_list_no like @compose_list_no||'%' "
						;

					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("compose_list_no", tfbsm12["COMPOSE_LIST_NO"].ToString().SubstringNE(0, 6) + tfbsm12["DATE_TIME"].ToString().SubstringNE(2, 6));
					cmd_inq_1.ExecuteReader();
					if (cmd_inq_1.Read())
					{
						if (cmd_inq_1.GetDecimal(3) == 0 && cmd_inq_1.GetDecimal(2) != 9){
							p_seq = cmd_inq_1.GetDecimal(2);
							p_seq = p_seq + 1;
							compose_list_no = tfbsm12["COMPOSE_LIST_NO"].ToString().SubstringNE(0, 6) + tfbsm12["DATE_TIME"].ToString().SubstringNE(2, 6) + "0" + p_seq.ToString();
						}
						else
						{
							p_seq = cmd_inq_1.GetDecimal(4);
							p_seq = p_seq + 1;
							compose_list_no = tfbsm12["COMPOSE_LIST_NO"].ToString().SubstringNE(0, 6) + tfbsm12["DATE_TIME"].ToString().SubstringNE(2, 6) + p_seq.ToString();
						}
					}
					else
					{
						compose_list_no = tfbsm12["COMPOSE_LIST_NO"].ToString().SubstringNE(0, 6) + tfbsm12["DATE_TIME"].ToString().SubstringNE(2, 6) + "01";
					}
					cmd_inq_1.Close();

					//插入配料单信息
					tfbsm14a["ORIGIN_CODE"] = "1";
					tfbsm14a["COMPOSE_LIST_NO"] = compose_list_no;
					tfbsm14a["DATE_C"] = tfbsm12["DATE_C"].ToString();
					tfbsm14a["BACKLOG_EA"] = tfbsm12["BACKLOG_EA"].ToString();
					tfbsm14a["ST_NO"] = tfbsm12["ST_NO"].ToString();
					tfbsm14a["REC_CREATOR"] = s.userid;
					tfbsm14a["REC_CREATE_TIME"] = dateNow;
					tfbsm14a.TrimOrBlank();
					tfbsm14a.Insert();

					//更新信息为最新的配料单号
					sqlstr = " update tfbsm12"
						" set compose_list_no = @compose_list_no_new"
						",ORIGIN_CODE = '1'"
						" where 1=1"
						" and DATE_TIME = @date_time"
						" and ST_NO = @st_no"
						" and BACKLOG_EA =@backlog_ea"
						" and COMPOSE_LIST_NO = @compose_list_no"
						;
					cmd_inq_1.SetCommandText(sqlstr);
					cmd_inq_1.Parameters.Set("compose_list_no_new", compose_list_no);
					cmd_inq_1.Parameters.Set("compose_list_no", tfbsm12["COMPOSE_LIST_NO"].ToString());
					cmd_inq_1.Parameters.Set("date_time", tfbsm12["DATE_TIME"].ToString());
					cmd_inq_1.Parameters.Set("backlog_ea", tfbsm12["BACKLOG_EA"].ToString());
					cmd_inq_1.Parameters.Set("st_no", tfbsm12["ST_NO"].ToString());
					cmd_inq_1.ExecuteNonQuery();
					cmd_inq_1.Close();
				}
			}
			cmd_inq.Close();



			tfbsm14["ORIGIN_CODE"] = "1";
			tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
			//Log::Trace("", __FUNCTION__, "tfbsm14[COMPOSE_LIST_NO] = [{0}],ORIGIN_CODE=[{1}]", tfbsm14["COMPOSE_LIST_NO"], tfbsm14["ORIGIN_CODE"]);

			tfbsm14.Delete("COMPOSE_LIST_NO");

			if (bcls_rec->Tables.Contains("IF"))   //消耗维护
			{
				Log::Trace("", __FUNCTION__, "IF行数 = [{0}]", bcls_rec->Tables["IF"].Rows.get_Count());

				for (int i = 0; i < bcls_rec->Tables["IF"].Rows.get_Count(); i++)
				{
					tfbsm14.Reset();
					tfbsm14.MergeFrom(bcls_rec->Tables["IF"].Rows[i]);
					tfbsm14["ORIGIN_CODE"] = "1";
					tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
					tfbsm14["ST_NO"] = tfbsm12["ST_NO"].ToString();
					tfbsm14["STATION_ID"] = "Z";
					tfbsm14["DATE_C"] = tfbsm12["DATE_C"].ToString();
					tfbsm14["BACKLOG_EA"] = tfbsm12["BACKLOG_EA"].ToString();

					tfbsm14["REC_CREATOR"] = s.userid;
					tfbsm14["REC_CREATE_TIME"] = dateNow;
					tfbsm14.TrimOrBlank();
					//Log::Trace("", __FUNCTION__, "mat_code = [{0}],STOCK_NAME = [{1}],MAT_NAME = [{2}],成分C = [{3}]", tfbsm14["MAT_CODE"], tfbsm14["STOCK_NAME"], tfbsm14["MAT_NAME"], bcls_rec->Tables["IF"].Rows[i]["C_VALUE"].ToString());

					//获取库区代码
					if (tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标"
						&&tfbsm14["MAT_NAME"].ToString() != "配料成分"&&tfbsm14["MAT_NAME"].ToString() != "出钢成分"&&tfbsm14["MAT_NAME"].ToString() != "预溶液成分"&&tfbsm14["MAT_NAME"].ToString() != "备注")
					{
						sqlstr = "select CODE"
							" from tep0002 "
							" where 1=1"
							" and CODE_DESC_1_CONTENT = @stock_name"
							" and code_class = 'FBSM13'"
							" union "
							" select BUNKER_NO "
							" from tmmsm60 "
							" where  1=1"
							" and MAT_NAME = @stock_name"
							" and BUNKER_NO like 'VS%'"
							;
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("stock_name", tfbsm14["STOCK_NAME"].ToString());
						cmd_inq.ExecuteReader();
						if (cmd_inq.Read())
						{
							tfbsm14["STOCK_NAME"] = cmd_inq.GetString(1);
						}
						cmd_inq.Close();
					}

					if (tfbsm14["MAT_NAME"].ToString() == "配料成分")
					{
						tfbsm14["MAT_CODE"] = "PL0001";
					}
					if (tfbsm14["MAT_NAME"].ToString() == "出钢成分")
					{
						tfbsm14["MAT_CODE"] = "PL0002";
					}
					if (tfbsm14["MAT_NAME"].ToString() == "预溶液成分")
					{
						tfbsm14["MAT_CODE"] = "PL0003";
					}

					if (tfbsm14["MAT_NAME"].ToString() != "配料成分" && tfbsm14["MAT_NAME"].ToString() != "出钢成分" && tfbsm14["MAT_NAME"].ToString() != "预溶液成分" && tfbsm14["MAT_NAME"].ToString() != "备注" && tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标")
					{

						unit_price = 0;
						if (tfbsm14["MAT_CODE"].ToString() == "TS0000")
						{
							unit_price = 2294;
						}
						else
						{

							bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = tfbsm14["MAT_CODE"];
							bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = tfbsm14["MAT_NAME"];
							bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = tfbsm14["CR_VALUE"];
							bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = tfbsm14["NI_VALUE"];
							bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = tfbsm14["MO_VALUE"];
							doFlag = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
							if (doFlag < 0)
							{
								throw CApplicationException(-1, s.msg, log.Location);
							}
							unit_price = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"];
						}
						unit_cost = unit_cost + unit_price * tfbsm14["WEIGHT"].ToDecimal();

						tfbsm14["COST"] = unit_price;
					}

					if (tfbsm14["MAT_NAME"].ToString() == "备注")
					{
						back_if = tfbsm14["STOCK_NAME"].ToString();
					}
					else
					{
						if (tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标")
						{
							if (tfbsm14.QueryCount("COMPOSE_LIST_NO,STATION_ID,MAT_CODE,STOCK_NAME,LOT_NO,BACK_C2") > 0)
							{ 
								strcpy(s.msg, "IF配料单的物料:" + tfbsm14["MAT_NAME"].ToString() + ",库区:" + bcls_rec->Tables["IF"].Rows[i]["STOCK_NAME"].ToString() + ",批次:" + tfbsm14["LOT_NO"].ToString() + "数据重复,请调整!");
								throw CApplicationException(-1, s.msg, log.Location);
							}

							tfbsm14.Insert();
						}

					}
					
				}
			}
			if (bcls_rec->Tables.Contains("DS"))   //消耗维护
			{
				Log::Trace("", __FUNCTION__, "DS行数 = [{0}]", bcls_rec->Tables["DS"].Rows.get_Count());

				for (int i = 0; i < bcls_rec->Tables["DS"].Rows.get_Count(); i++)
				{
					tfbsm14.Reset();
					tfbsm14.MergeFrom(bcls_rec->Tables["DS"].Rows[i]);
					tfbsm14["ORIGIN_CODE"] = "1";
					tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
					tfbsm14["ST_NO"] = tfbsm12["ST_NO"].ToString();
					tfbsm14["STATION_ID"] = "D";
					tfbsm14["DATE_C"] = tfbsm12["DATE_C"].ToString();
					tfbsm14["BACKLOG_EA"] = tfbsm12["BACKLOG_EA"].ToString();

					tfbsm14["REC_CREATOR"] = s.userid;
					tfbsm14["REC_CREATE_TIME"] = dateNow;
					tfbsm14.TrimOrBlank();
					//Log::Trace("", __FUNCTION__, "mat_code = [{0}],STOCK_NAME = [{1}],MAT_NAME = [{2}],成分C = [{3}]", tfbsm14["MAT_CODE"], tfbsm14["STOCK_NAME"], tfbsm14["MAT_NAME"], bcls_rec->Tables["IF"].Rows[i]["C_VALUE"].ToString());

					//获取库区代码
					if (tfbsm14["MAT_NAME"].ToString() != "铁水"&&tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标"
						&&tfbsm14["MAT_NAME"].ToString() != "配料成分"&&tfbsm14["MAT_NAME"].ToString() != "出钢成分"&&tfbsm14["MAT_NAME"].ToString() != "预溶液成分"&&tfbsm14["MAT_NAME"].ToString() != "备注")
					{
						sqlstr = "select CODE"
							" from tep0002 "
							" where 1=1"
							" and CODE_DESC_1_CONTENT = @stock_name"
							" and code_class = 'FBSM13'"
							" union "
							" select BUNKER_NO "
							" from tmmsm60 "
							" where  1=1"
							" and MAT_NAME = @stock_name"
							" and BUNKER_NO like 'VS%'"
							;
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("stock_name", tfbsm14["STOCK_NAME"].ToString());
						cmd_inq.ExecuteReader();
						if (cmd_inq.Read())
						{
							tfbsm14["STOCK_NAME"] = cmd_inq.GetString(1);
						}
						cmd_inq.Close();
					}

					if (tfbsm14["MAT_NAME"].ToString() == "配料成分")
					{
						tfbsm14["MAT_CODE"] = "PL0001";
					}
					if (tfbsm14["MAT_NAME"].ToString() == "出钢成分")
					{
						tfbsm14["MAT_CODE"] = "PL0002";
					}
					if (tfbsm14["MAT_NAME"].ToString() == "预溶液成分")
					{
						tfbsm14["MAT_CODE"] = "PL0003";
					}

					if (tfbsm14["MAT_NAME"].ToString() != "配料成分" && tfbsm14["MAT_NAME"].ToString() != "出钢成分" && tfbsm14["MAT_NAME"].ToString() != "预溶液成分" && tfbsm14["MAT_NAME"].ToString() != "备注" && tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标")
					{

						unit_price = 0;
						if (tfbsm14["MAT_CODE"].ToString() == "TS0000")
						{
							unit_price = 2294;
						}
						else
						{

							bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = tfbsm14["MAT_CODE"];
							bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = tfbsm14["MAT_NAME"];
							bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = tfbsm14["CR_VALUE"];
							bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = tfbsm14["NI_VALUE"];
							bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = tfbsm14["MO_VALUE"];
							doFlag = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
							if (doFlag < 0)
							{
								throw CApplicationException(-1, s.msg, log.Location);
							}
							unit_price = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"];
						}
						unit_cost = unit_cost + unit_price * tfbsm14["WEIGHT"].ToDecimal();

						tfbsm14["COST"] = unit_price;
					}

					if (tfbsm14["MAT_NAME"].ToString() == "备注")
					{
						back_ds = tfbsm14["STOCK_NAME"].ToString();
					}
					else
					{
						if (tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标")
						{
							if (tfbsm14.QueryCount("COMPOSE_LIST_NO,STATION_ID,MAT_CODE,STOCK_NAME,LOT_NO,BACK_C2") > 0)
							{
								strcpy(s.msg, "脱磷配料单的物料:" + tfbsm14["MAT_NAME"].ToString() + ",库区:" + bcls_rec->Tables["DS"].Rows[i]["STOCK_NAME"].ToString() + ",批次:" + tfbsm14["LOT_NO"].ToString() + "数据重复,请调整!");
								throw CApplicationException(-1, s.msg, log.Location);
							}

							tfbsm14.Insert();
						}

					}

				}
			}

			if (bcls_rec->Tables.Contains("EAF"))   //消耗维护
			{
				Log::Trace("", __FUNCTION__, "EAF行数 = [{0}]", bcls_rec->Tables["EAF"].Rows.get_Count());
				for (int i = 0; i < bcls_rec->Tables["EAF"].Rows.get_Count(); i++)
				{
					tfbsm14.Reset();
					tfbsm14.MergeFrom(bcls_rec->Tables["EAF"].Rows[i]);
					tfbsm14["ORIGIN_CODE"] = "1";
					tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
					tfbsm14["ST_NO"] = tfbsm12["ST_NO"].ToString();
					tfbsm14["STATION_ID"] = "E";
					tfbsm14["DATE_C"] = tfbsm12["DATE_C"].ToString();
					tfbsm14["BACKLOG_EA"] = tfbsm12["BACKLOG_EA"].ToString();

					tfbsm14["REC_CREATOR"] = s.userid;
					tfbsm14["REC_CREATE_TIME"] = dateNow;
					tfbsm14.TrimOrBlank();
					if (tfbsm14["MAT_NAME"].ToString() == "配料成分")
					{
						tfbsm14["MAT_CODE"] = "PL0001";
					}
					if (tfbsm14["MAT_NAME"].ToString() == "出钢成分")
					{
						tfbsm14["MAT_CODE"] = "PL0002";
					}
					if (tfbsm14["MAT_NAME"].ToString() == "预溶液成分")
					{
						tfbsm14["MAT_CODE"] = "PL0003";
					}
					if (tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标"
						&&tfbsm14["MAT_NAME"].ToString() != "配料成分"&&tfbsm14["MAT_NAME"].ToString() != "出钢成分"&&tfbsm14["MAT_NAME"].ToString() != "预溶液成分"&&tfbsm14["MAT_NAME"].ToString() != "备注")
					{
						//获取库区代码
						sqlstr = "select CODE"
							" from tep0002 "
							" where 1=1"
							" and CODE_DESC_1_CONTENT = @stock_name"
							" and code_class = 'FBSM13'"
							" union "
							" select BUNKER_NO "
							" from tmmsm60 "
							" where  1=1"
							" and MAT_NAME = @stock_name"
							" and BUNKER_NO like 'VS%'"
							;
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("stock_name", tfbsm14["STOCK_NAME"].ToString());
						cmd_inq.ExecuteReader();
						if (cmd_inq.Read())
						{
							tfbsm14["STOCK_NAME"] = cmd_inq.GetString(1);
						}
						cmd_inq.Close();
					}

					if (tfbsm14["MAT_NAME"].ToString() != "配料成分" && tfbsm14["MAT_NAME"].ToString() != "出钢成分" && tfbsm14["MAT_NAME"].ToString() != "预溶液成分" && tfbsm14["MAT_NAME"].ToString() != "备注" && tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标")
					{

						unit_price = 0;
						if (tfbsm14["MAT_CODE"].ToString() == "TS0000")
						{
							unit_price = 2294;
						}
						else
						{

							bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = tfbsm14["MAT_CODE"];
							bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = tfbsm14["MAT_NAME"];
							bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = tfbsm14["CR_VALUE"];
							bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = tfbsm14["NI_VALUE"];
							bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = tfbsm14["MO_VALUE"];
							doFlag = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
							if (doFlag < 0)
							{
								throw CApplicationException(-1, s.msg, log.Location);
							}
							unit_price = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"];
						}
						unit_cost = unit_cost + unit_price * tfbsm14["WEIGHT"].ToDecimal();
						tfbsm14["COST"] = unit_price;
					}

					if (tfbsm14["MAT_NAME"].ToString() == "备注")
					{
						back_eaf = tfbsm14["STOCK_NAME"].ToString();
					}
					else
					{
						if (tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标")
						{
							if (tfbsm14.QueryCount("COMPOSE_LIST_NO,STATION_ID,MAT_CODE,STOCK_NAME,LOT_NO,BACK_C2") > 0)
							{
								strcpy(s.msg, "EAF配料单的物料:" + tfbsm14["MAT_NAME"].ToString() + ",库区:" + bcls_rec->Tables["EAF"].Rows[i]["STOCK_NAME"].ToString() + ",批次:" + tfbsm14["LOT_NO"].ToString() + "数据重复,请调整!");
								throw CApplicationException(-1, s.msg, log.Location);
							}
							tfbsm14.Insert();
						}
					}

					
				}
			}

			if (bcls_rec->Tables.Contains("AOD"))   //消耗维护
			{
				Log::Trace("", __FUNCTION__, "AOD行数 = [{0}]", bcls_rec->Tables["AOD"].Rows.get_Count());
				for (int i = 0; i < bcls_rec->Tables["AOD"].Rows.get_Count(); i++)
				{
					tfbsm14.Reset();
					tfbsm14.MergeFrom(bcls_rec->Tables["AOD"].Rows[i]);
					tfbsm14["ORIGIN_CODE"] = "1";
					tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
					tfbsm14["ST_NO"] = bcls_rec->Tables["query"].Rows[0]["ST_NO"].ToString();
					tfbsm14["STATION_ID"] = "A";
					tfbsm14["DATE_C"] = tfbsm12["DATE_C"].ToString();
					tfbsm14["BACKLOG_EA"] = tfbsm12["BACKLOG_EA"].ToString();

					tfbsm14["REC_CREATOR"] = s.userid;
					tfbsm14["REC_CREATE_TIME"] = dateNow;
					tfbsm14.TrimOrBlank();
					if (tfbsm14["MAT_NAME"].ToString() == "配料成分")
					{
						tfbsm14["MAT_CODE"] = "PL0001";
					}
					if (tfbsm14["MAT_NAME"].ToString() == "出钢成分")
					{
						tfbsm14["MAT_CODE"] = "PL0002";
						aod_wt = tfbsm14["WEIGHT"].ToDecimal();
					}
					if (tfbsm14["MAT_NAME"].ToString() == "预溶液成分")
					{
						tfbsm14["MAT_CODE"] = "PL0003";
					}
					if (tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标"
						&&tfbsm14["MAT_NAME"].ToString() != "配料成分"&&tfbsm14["MAT_NAME"].ToString() != "出钢成分"&&tfbsm14["MAT_NAME"].ToString() != "预溶液成分"&&tfbsm14["MAT_NAME"].ToString() != "备注")
					{
						//获取库区代码
						sqlstr = "select CODE"
							" from tep0002 "
							" where 1=1"
							" and CODE_DESC_1_CONTENT = @stock_name"
							" and code_class = 'FBSM13'"
							" union "
							" select BUNKER_NO "
							" from tmmsm60 "
							" where  1=1"
							" and MAT_NAME = @stock_name"
							" and BUNKER_NO like 'VS%'"
							;
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("stock_name", tfbsm14["STOCK_NAME"].ToString());
						cmd_inq.ExecuteReader();
						if (cmd_inq.Read())
						{
							tfbsm14["STOCK_NAME"] = cmd_inq.GetString(1);
						}
						cmd_inq.Close();
					}

					if (tfbsm14["MAT_NAME"].ToString() != "配料成分" && tfbsm14["MAT_NAME"].ToString() != "出钢成分" && tfbsm14["MAT_NAME"].ToString() != "预溶液成分" && tfbsm14["MAT_NAME"].ToString() != "备注" && tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标")
					{
						if (tfbsm14["BACK_C2"].ToString() != "9") //还原硅不算价格
						{


							unit_price = 0;
							if (tfbsm14["MAT_CODE"].ToString() == "TS0000")
							{
								unit_price = 2294;
							}
							else
							{

								bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = tfbsm14["MAT_CODE"];
								bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = tfbsm14["MAT_NAME"];
								bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = tfbsm14["CR_VALUE"];
								bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = tfbsm14["NI_VALUE"];
								bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = tfbsm14["MO_VALUE"];
								doFlag = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
								if (doFlag < 0)
								{
									throw CApplicationException(-1, s.msg, log.Location);
								}
								unit_price = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"];
							}
							unit_cost = unit_cost + unit_price * tfbsm14["WEIGHT"].ToDecimal();
							tfbsm14["COST"] = unit_price;
						}
					}
					if (tfbsm14["MAT_NAME"].ToString() == "备注")
					{
						back_aod = bcls_rec->Tables["AOD"].Rows[i]["WEIGHT"].ToString();
					}
					else
					{
						if (tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标")
						{
							if (tfbsm14.QueryCount("COMPOSE_LIST_NO,STATION_ID,MAT_CODE,STOCK_NAME,LOT_NO,BACK_C2") > 0)
							{
								strcpy(s.msg, "AOD配料单的物料:" + tfbsm14["MAT_NAME"].ToString() + ",库区:" + bcls_rec->Tables["AOD"].Rows[i]["STOCK_NAME"].ToString() + ",批次:" + tfbsm14["LOT_NO"].ToString() + "数据重复,请调整!");
								throw CApplicationException(-1, s.msg, log.Location);
							}
							tfbsm14.Insert();
						}
					}					
				}
			}

			if (bcls_rec->Tables.Contains("BOF"))   //消耗维护
			{
				for (int i = 0; i < bcls_rec->Tables["BOF"].Rows.get_Count(); i++)
				{
					Log::Trace("", __FUNCTION__, "BOF行数 = [{0}]", bcls_rec->Tables["BOF"].Rows.get_Count());

					tfbsm14.Reset();
					tfbsm14.MergeFrom(bcls_rec->Tables["BOF"].Rows[i]);
					tfbsm14["ORIGIN_CODE"] = "1";
					tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
					tfbsm14["ST_NO"] = bcls_rec->Tables["query"].Rows[0]["ST_NO"].ToString();
					tfbsm14["STATION_ID"] = "B";
					tfbsm14["DATE_C"] = tfbsm12["DATE_C"].ToString();
					tfbsm14["BACKLOG_EA"] = tfbsm12["BACKLOG_EA"].ToString();

					tfbsm14["REC_CREATOR"] = s.userid;
					tfbsm14["REC_CREATE_TIME"] = dateNow;
					tfbsm14.TrimOrBlank();
					if (tfbsm14["MAT_NAME"].ToString() == "配料成分")
					{
						tfbsm14["MAT_CODE"] = "PL0001";
					}
					if (tfbsm14["MAT_NAME"].ToString() == "出钢成分")
					{
						tfbsm14["MAT_CODE"] = "PL0002";
					}
					if (tfbsm14["MAT_NAME"].ToString() == "预溶液成分")
					{
						tfbsm14["MAT_CODE"] = "PL0003";
					}
					if (tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标"
						&&tfbsm14["MAT_NAME"].ToString() != "配料成分"&&tfbsm14["MAT_NAME"].ToString() != "出钢成分"&&tfbsm14["MAT_NAME"].ToString() != "预溶液成分"&&tfbsm14["MAT_NAME"].ToString() != "备注")
					{
						//获取库区代码
						sqlstr = "select CODE"
							" from tep0002 "
							" where 1=1"
							" and CODE_DESC_1_CONTENT = @stock_name"
							" and code_class = 'FBSM13'"
							" union "
							" select BUNKER_NO "
							" from tmmsm60 "
							" where  1=1"
							" and MAT_NAME = @stock_name"
							" and BUNKER_NO like 'VS%'"
							;
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("stock_name", tfbsm14["STOCK_NAME"].ToString());
						cmd_inq.ExecuteReader();
						if (cmd_inq.Read())
						{
							tfbsm14["STOCK_NAME"] = cmd_inq.GetString(1);
						}
						cmd_inq.Close();
					}
					if (tfbsm14["MAT_NAME"].ToString() != "配料成分" && tfbsm14["MAT_NAME"].ToString() != "出钢成分" && tfbsm14["MAT_NAME"].ToString() != "预溶液成分" && tfbsm14["MAT_NAME"].ToString() != "备注" && tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标")
					{

						unit_price = 0;
						if (tfbsm14["MAT_CODE"].ToString() == "TS0000")
						{
							unit_price = 2294;
						}
						else
						{

							bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = tfbsm14["MAT_CODE"];
							bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = tfbsm14["MAT_NAME"];
							bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = tfbsm14["CR_VALUE"];
							bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = tfbsm14["NI_VALUE"];
							bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = tfbsm14["MO_VALUE"];
							doFlag = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
							if (doFlag < 0)
							{
								throw CApplicationException(-1, s.msg, log.Location);
							}
							unit_price = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"];
						}
						unit_cost = unit_cost + unit_price * tfbsm14["WEIGHT"].ToDecimal();
						tfbsm14["COST"] = unit_price;
					}
					if (tfbsm14["MAT_NAME"].ToString() == "备注")
					{
						back_bof = tfbsm14["STOCK_NAME"].ToString();
					}
					else
					{
						if (tfbsm14["MAT_NAME"].ToString() != "内控上限"&&tfbsm14["MAT_NAME"].ToString() != "内控下限"&&tfbsm14["MAT_NAME"].ToString() != "内控目标")
						{
							if (tfbsm14.QueryCount("COMPOSE_LIST_NO,STATION_ID,MAT_CODE,STOCK_NAME,LOT_NO,BACK_C2") > 0)
							{
								strcpy(s.msg, "BOF配料单的物料:" + tfbsm14["MAT_NAME"].ToString() + ",库区:" + bcls_rec->Tables["BOF"].Rows[i]["STOCK_NAME"].ToString() + ",批次:" + tfbsm14["LOT_NO"].ToString() + "数据重复,请调整!");
								throw CApplicationException(-1, s.msg, log.Location);
							}
							tfbsm14.Insert();
						}
					}
				
				}
			}

			Log::Trace("", __FUNCTION__, "compose_list_no = [{0}],unit_cost = [{1}],aod_wt = [{2}]，back_if=【{3}】", compose_list_no, unit_cost.Round(2), aod_wt, back_if);


			sqlstr = " update tfbsm12"
				" set SINGLECOST = @unit_cost"
				",COST_DG = @cost_dg"
				",back_ds = @back_ds"
				",back_if = @back_if"
				",back_aod = @back_aod"
				",back_bof = @back_bof"
				",back_eaf = @back_eaf"
				" where 1=1"
				" and COMPOSE_LIST_NO = @compose_list_no_new"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("back_ds", back_ds);
			cmd_inq_1.Parameters.Set("back_if", back_if);
			cmd_inq_1.Parameters.Set("back_aod", back_aod);
			cmd_inq_1.Parameters.Set("back_bof", back_bof);
			cmd_inq_1.Parameters.Set("back_eaf", back_eaf);
			cmd_inq_1.Parameters.Set("compose_list_no_new", compose_list_no);
			cmd_inq_1.Parameters.Set("date_c", tfbsm12["DATE_C"].ToString());
			cmd_inq_1.Parameters.Set("backlog_ea", tfbsm12["BACKLOG_EA"].ToString());
			cmd_inq_1.Parameters.Set("st_no", tfbsm12["ST_NO"].ToString());
			cmd_inq_1.Parameters.Set("unit_cost", unit_cost.Round(2));
			if (aod_wt == 0)
			{
				cmd_inq_1.Parameters.Set("cost_dg", 0);
			}
			else
			{
				cmd_inq_1.Parameters.Set("cost_dg", (unit_cost / aod_wt).Round(2));
			}
			cmd_inq_1.ExecuteNonQuery();
			cmd_inq_1.Close();

			sqlstr = " update tfbsm14a"
				" set SINGLECOST = @unit_cost"
				",COST_DG = @cost_dg"
				",back_ds = @back_ds"
				",back_if = @back_if"
				",back_aod = @back_aod"
				",back_bof = @back_bof"
				",back_eaf = @back_eaf"
				" where 1=1"
				" and COMPOSE_LIST_NO = @compose_list_no_new"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("back_ds", back_ds);
			cmd_inq_1.Parameters.Set("back_if", back_if);
			cmd_inq_1.Parameters.Set("back_aod", back_aod);
			cmd_inq_1.Parameters.Set("back_bof", back_bof);
			cmd_inq_1.Parameters.Set("back_eaf", back_eaf);
			cmd_inq_1.Parameters.Set("compose_list_no_new", compose_list_no);
			cmd_inq_1.Parameters.Set("date_c", tfbsm12["DATE_C"].ToString());
			cmd_inq_1.Parameters.Set("backlog_ea", tfbsm12["BACKLOG_EA"].ToString());
			cmd_inq_1.Parameters.Set("st_no", tfbsm12["ST_NO"].ToString());
			cmd_inq_1.Parameters.Set("unit_cost", unit_cost.Round(2));
			if (aod_wt == 0)
			{
				cmd_inq_1.Parameters.Set("cost_dg", 0);
			}
			else
			{
				cmd_inq_1.Parameters.Set("cost_dg", (unit_cost / aod_wt).Round(2));
			}
			cmd_inq_1.ExecuteNonQuery();
			cmd_inq_1.Close();

			//返回配料单
			bcls_ret->Tables[0].Columns.Add(DT_STRING, "COMPOSE_LIST_NO");
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[0]["COMPOSE_LIST_NO"] = compose_list_no;
		}
		if (bcls_rec->Tables.Contains("query_remark"))
		{
				

			compose_list_no = bcls_rec->Tables["query_remark"].Rows[0]["COMPOSE_LIST_NO"].ToString();
			Log::Trace("", __FUNCTION__, "1111compose_list_no = [{0}]", compose_list_no);
			if (bcls_rec->Tables.Contains("DS_REMARK"))   //消耗维护
			{
				for (int i = 0; i < bcls_rec->Tables["DS_REMARK"].Rows.get_Count(); i++)
				{
					if (bcls_rec->Tables["DS_REMARK"].Rows[i]["MAT_NAME"].ToString() == "备注")
					{
						back_ds = bcls_rec->Tables["DS_REMARK"].Rows[i]["STOCK_NAME"].ToString();
					}
				}
			}
			if (bcls_rec->Tables.Contains("IF_REMARK"))   //消耗维护
			{
				for (int i = 0; i < bcls_rec->Tables["IF_REMARK"].Rows.get_Count(); i++)
				{
					if (bcls_rec->Tables["IF_REMARK"].Rows[i]["MAT_NAME"].ToString() == "备注")
					{
						back_if = bcls_rec->Tables["IF_REMARK"].Rows[i]["STOCK_NAME"].ToString();
					}
				}
			}
			if (bcls_rec->Tables.Contains("EAF_REMARK"))   //消耗维护
			{
				for (int i = 0; i < bcls_rec->Tables["EAF_REMARK"].Rows.get_Count(); i++)
				{
					if (bcls_rec->Tables["EAF_REMARK"].Rows[i]["MAT_NAME"].ToString() == "备注")
					{
						back_eaf = bcls_rec->Tables["EAF_REMARK"].Rows[i]["STOCK_NAME"].ToString();
					}
				}
			}
			if (bcls_rec->Tables.Contains("BOF_REMARK"))   //消耗维护
			{
				for (int i = 0; i < bcls_rec->Tables["BOF_REMARK"].Rows.get_Count(); i++)
				{
					if (bcls_rec->Tables["BOF_REMARK"].Rows[i]["MAT_NAME"].ToString() == "备注")
					{
						back_bof = bcls_rec->Tables["BOF_REMARK"].Rows[i]["STOCK_NAME"].ToString();
					}
				}
			}
			if (bcls_rec->Tables.Contains("AOD_REMARK"))   //消耗维护
			{
				for (int i = 0; i < bcls_rec->Tables["AOD_REMARK"].Rows.get_Count(); i++)
				{
					if (bcls_rec->Tables["AOD_REMARK"].Rows[i]["MAT_NAME"].ToString() == "备注")
					{
						back_aod = bcls_rec->Tables["AOD_REMARK"].Rows[i]["WEIGHT"].ToString();
					}
				}
			}
			sqlstr = " update tfbsm12"
				" set back_ds = @back_ds"
				" ,back_if = @back_if"
				",back_aod = @back_aod"
				",back_bof = @back_bof"
				",back_eaf = @back_eaf"
				" where 1=1"
				" and COMPOSE_LIST_NO = @compose_list_no"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("back_ds", back_ds);
			cmd_inq_1.Parameters.Set("back_if", back_if);
			cmd_inq_1.Parameters.Set("back_aod", back_aod);
			cmd_inq_1.Parameters.Set("back_bof", back_bof);
			cmd_inq_1.Parameters.Set("back_eaf", back_eaf);
			cmd_inq_1.Parameters.Set("compose_list_no", compose_list_no);			
			cmd_inq_1.ExecuteNonQuery();
			cmd_inq_1.Close();

			sqlstr = " update tfbsm14a"
				" set back_ds = @back_ds"
				",back_if = @back_if"
				",back_aod = @back_aod"
				",back_bof = @back_bof"
				",back_eaf = @back_eaf"
				" where 1=1"
				" and COMPOSE_LIST_NO = @compose_list_no"
				;
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("back_ds", back_ds);
			cmd_inq_1.Parameters.Set("back_if", back_if);
			cmd_inq_1.Parameters.Set("back_aod", back_aod);
			cmd_inq_1.Parameters.Set("back_bof", back_bof);
			cmd_inq_1.Parameters.Set("back_eaf", back_eaf);
			cmd_inq_1.Parameters.Set("compose_list_no", compose_list_no);
			cmd_inq_1.ExecuteNonQuery();
			cmd_inq_1.Close();

			//返回配料单
			bcls_ret->Tables[0].Columns.Add(DT_STRING, "COMPOSE_LIST_NO");
			bcls_ret->Tables[0].Rows.Add();
			bcls_ret->Tables[0].Rows[0]["COMPOSE_LIST_NO"] = compose_list_no;
		}

			
		

	}
	catch (CDbException& ex)  //捕获数据库操作异常 
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, "数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。", arguments, 1);
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.sysmsg, (const char*)str, sizeof(s.sysmsg) - 1);
		s.flag = -1;
		doFlag = -1;      //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.sysmsg, (const char*)ex.GetMsg(), sizeof(s.sysmsg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	return doFlag;
}


