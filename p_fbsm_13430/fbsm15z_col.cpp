/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/11/24
Description: 模型不锈钢钢种配料
工序成分基准：如果是AOD则取内控成分TQMTS02，否则取维护的工序成分基准TFBSM04.
**************************************************/
#include "stdafx.h"
#include<map>
BM2F_ENTERACE(fbsm15z_col)
/* ***** 静态函数申明 ***** */
//模型计算
int f_epex_call_rest_svc(CDbConnection* conn, const CString& system_code, const CString& svc_name, EIClass* blks_in, EIClass * blks_out);
int f_mmsm_getprice(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//价格计算

int f_fbsm15z_col(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int ret = 0;
	CString sqlstr = " ";
	CString st_no = " ";//出钢记号
	CString st_no_1 = "";
	CString st_no_2 = "";
	CString backlog_ea = " ";//工艺路线
	CString mat_st_no = " ";//使用出钢记号
	CString mat_backlog_ea = " ";//使用工艺路线
	CString date_c = " ";//日期
	CString material_code = " ";//中类代码
	CString comm_fmly_code = " ";//大类代码 	
	CString seq_no = " ";//序号
	CString s_matear_code = " ";
	CString compose_list_no = " ";//配料单号
	CDecimal f_si_pdi = 0;
	CString plan_id = "";	
	CDecimal p_seq = 0;
	CString sql_k = "";
	CDecimal cg_if = 0;
	CDecimal if_wt_mat = 0;
	CString f_comm_fmly_code = " ";
	CString sql_ENVIRONMENT = " ";
	CString check_flag = "0";
	CString p_mode = "0";
	CDecimal spe_max = 0;
	CString flag_co = "0"; //低估
	CString flag_vod = "0"; //过vod
	CDecimal stock_wt_sy = 0; //已使用库存
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_01(conn);
	CDbCommand cmd(conn);
	CDbCommand cmd_k(conn);
	CDbCommand cmd_si_pdi(conn);
	CDecimal f_flag = 0;
	CModel tfbsm18("TFBSM18");
	CModel tfbsm14("TFBSM14");
	CModel tfbsm04("TFBSM04");
	CModel tfbsm09("TFBSM09");
	CModel tfbsm01("TFBSM01");
	CModel tfbsm13("TFBSM13");
	CModel tfbsm19("TFBSM19");
	CModel tfbsm26("TFBSM26");
	CModel tfbsm27("TFBSM27");
	CModel tfbsm28("TFBSM28");
	CModel tfbsm14A("TFBSM14A");
	CDbCommand cmd_plan(conn);
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CString model_id = CDateTime::Now().ToString("yyyyMMddHHmmssfff");
	CString mat_code_list = "";

	//模型定义
	EIClass inDMST02; //传入类
	EIClass outDMST02;	//返回类
	EIClass outDMST02_1;	//返回类
	EIClass outPLAN;	//计算返回值
	outPLAN.Tables[0].Columns.Add(DT_STRING, "PLAN_ID");
	outPLAN.Tables[0].Columns.Add(DT_STRING, "COMPOSE_LIST_NO");
	outPLAN.Tables[0].Columns.Add(DT_STRING, "BACKLOG_EA");
	outPLAN.Tables[0].Columns.Add(DT_STRING, "ST_NO");
	outPLAN.Tables[0].Columns.Add(DT_STRING, "DATE_C");
	outPLAN.Tables[0].Columns.Add(DT_STRING, "SUM_FURNACES");

	map<CString, CDecimal> modelResultMap;	//物料代码+批次号+库区，模型使用量
	map<CString, CDecimal> planFurnaceCountMap; //记录每个成功配料单(PLAN_ID)的炉数
	map<CString, CDecimal> initMap;		// 物料代码+批次号+库区，初始时的重量
	try
	{
		inDMST02.Tables[0].set_TableName("DLLINFO");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "DLLName");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "ClassName");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "MethodName");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "CHECK_FLAG");//CHECK_FLAG
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "ENVIRONMENT");
		inDMST02.Tables["DLLINFO"].Rows.Add();
		//这三个值待定
		inDMST02.Tables["DLLINFO"].Rows[0]["DLLName"] = "TGPLModel.dll";
		inDMST02.Tables["DLLINFO"].Rows[0]["ClassName"] = "TGPLModel.TGPLModel";
		inDMST02.Tables["DLLINFO"].Rows[0]["MethodName"] = "CalForPLPlan";
		inDMST02.Tables["DLLINFO"].Rows[0]["CHECK_FLAG"] = "1";//不再区分是否检查标记，都传1，代表需校验

		sqlstr = " SELECT CODE FROM TEP0002 "
			" WHERE CODE_CLASS = 'FBSM23' "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			inDMST02.Tables["DLLINFO"].Rows[0]["ENVIRONMENT"] = cmd_inq.GetString(1);
		}
		cmd_inq.Close();

		if (bcls_rec->Tables["query"].Columns.Contains("MAT_CODE_LIST"))
			mat_code_list = bcls_rec->Tables["query"].Rows[0]["MAT_CODE_LIST"].ToString();
		Log::Trace("", __FUNCTION__, "mat_code_list = [{0} ]", mat_code_list);

		//配料单信息
		inDMST02.Tables.Add("LG_PSSM");
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "BACKLOG_EA"); //工艺路径
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "ST_NO"); //出钢记号
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "MATERIAL_CODE"); //中类代码
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "DATE_C"); //日期
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "FURNACE_COUNT");//总炉数
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "SEQ_NO");//序号
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "SUM_FURNACES");//总炉数
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "P_MODE"); //Ni钢电炉涉及到工艺脱磷，设定P的值
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "SPE_MAX"); //出钢定值

		//MatTable(原料库存及成分)
		inDMST02.Tables.Add("MatTable");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "LOT_NO"); //批次号
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "ST_NO"); //出钢记号
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "STOCK_NAME"); //库区
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MAT_CODE"); //物料代码
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MAT_NAME"); //物料名称
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "BACK_C2"); //物料类型
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "STOCK_WT"); //重量
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "C_VALUE");//C%
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "SI_VALUE");//Si%
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MN_VALUE");//Mn
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "P_VALUE");//P
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "S_VALUE");//S
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "CR_VALUE");//Cr
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "NI_VALUE");//Ni
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MO_VALUE");//Mo
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "CU_VALUE");//Cu
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "CO_VALUE");//CO
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "TI_VALUE");//TI
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "NB_VALUE");//NB
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "AL_VALUE");//AL
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "COST");//价格
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "LESS_1");//是否可以小于1吨
		//后续蹭加成分
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "V_VALUE");//V
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "B_VALUE");//B
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "N_VALUE");//N
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "CA_VALUE");//CA
		//WORKTABLE（工序基准成分）
		inDMST02.Tables.Add("WORKTABLE");
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "COMM_FMLY_CODE"); //产品大类代码
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "MATERIAL_CODE"); //产品中类代码
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "BACKLOG_EA"); //工艺路径
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "ST_NO"); //出钢记号
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "STATION_ID"); //工序
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "ELM_NAME"); //元素
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "SPE_MIN"); //最小值
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "SPE_MAX"); //最大值
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "YIELD"); //收得率
		

		//GX_MAT（原料总表）
		inDMST02.Tables.Add("GX_MAT");
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "COMM_FMLY_CODE"); //产品大类代码
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "MATERIAL_CODE"); //产品中类代码
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "ST_NO"); //出钢记号
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "YEILD_MINUS"); //收得率
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "STATION_ID"); //工序
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "MAT_CODE"); //物料代码
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "MAT_NAME"); //物料名称
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "UPPER_LIMIT_VALUE"); //投料上限
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "LOWER_LIMIT_VALUE"); //投料下限
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "MUST_DO_FLAG");//MUST_DO_FLAG 必做标记

		//GX_WEIGHT(可投重量)
		inDMST02.Tables.Add("GX_WEIGHT");
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "ST_NO"); //出钢记号
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "STATION_ID"); //工序
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "BACKLOG_EA"); //工艺路线
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "METAL_CONS_PERT"); //金属料消耗
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "SCRAP_RATIO_MAX"); //废钢最大
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "SCRAP_RATIO_MIN"); //废钢最小
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "TAPPING_WT"); //重量
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "TAPPING_WT_MIN"); //最小重量
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "MAT_COUNT"); //最多可投物料数量
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "COMBY_GX"); //路线
		//GX_LC（原料料槽表）
		inDMST02.Tables.Add("GX_LC");
		inDMST02.Tables["GX_LC"].Columns.Add(DT_STRING, "BACKLOG_EA"); //工艺路径
		inDMST02.Tables["GX_LC"].Columns.Add(DT_STRING, "MAT_CODE"); //物料代码
		inDMST02.Tables["GX_LC"].Columns.Add(DT_STRING, "MAT_NAME"); //物料名称
		inDMST02.Tables["GX_LC"].Columns.Add(DT_STRING, "BIG_LOAD_WT_MAX"); //大料槽最大装入量
		inDMST02.Tables["GX_LC"].Columns.Add(DT_STRING, "BIG_LOAD_WT_MIN"); //大料槽最小装入量
		inDMST02.Tables["GX_LC"].Columns.Add(DT_STRING, "SMALL_LOAD_WT_MAX"); //小料槽最大装入量
		inDMST02.Tables["GX_LC"].Columns.Add(DT_STRING, "SMALL_LOAD_WT_MIN"); //小料槽最小装入量

		//LOCKED_TABLE(锁定物料表)
		inDMST02.Tables.Add("LOCKED_TABLE");
		inDMST02.Tables["LOCKED_TABLE"].Columns.Add(DT_STRING, "ST_NO"); 
		inDMST02.Tables["LOCKED_TABLE"].Columns.Add(DT_STRING, "STATION_ID");
		inDMST02.Tables["LOCKED_TABLE"].Columns.Add(DT_STRING, "MAT_CODE");
		inDMST02.Tables["LOCKED_TABLE"].Columns.Add(DT_STRING, "MAT_NAME");
		inDMST02.Tables["LOCKED_TABLE"].Columns.Add(DT_STRING, "LOT_NO");
		inDMST02.Tables["LOCKED_TABLE"].Columns.Add(DT_STRING, "STOCK_NAME"); 
		inDMST02.Tables["LOCKED_TABLE"].Columns.Add(DT_STRING, "LOCKED_WT");
		inDMST02.Tables["LOCKED_TABLE"].Columns.Add(DT_STRING, "LOCK_MODE");


		//价格计算
		EIClass bcls_rec_rep;
		EIClass bcls_ret_rep;
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_CODE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_NAME");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "CR_VALUE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "NI_VALUE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MO_VALUE");
		bcls_rec_rep.Tables[0].Rows.Add();
		//获取传入的信息
		compose_list_no = bcls_rec->Tables["query"].Rows[0]["COMPOSE_LIST_NO"].ToString().TrimOrBlank();
		st_no = bcls_rec->Tables["query"].Rows[0]["ST_NO"].ToString();
		backlog_ea = bcls_rec->Tables["query"].Rows[0]["BACKLOG_EA"].ToString();		
		date_c = bcls_rec->Tables["query"].Rows[0]["DATE_TIME"].ToString().SubstringNE(2,6);

		sqlstr = " select check_flag"
			" from tfbsm12"
			" where 1=1"
			" and compose_list_no = @compose_list_no"
			" and st_no = @st_no"
			" and backlog_ea = @backlog_ea"
			" and date_c = @date_c"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("st_no", st_no);
		cmd_inq.Parameters.Set("backlog_ea", backlog_ea);
		cmd_inq.Parameters.Set("date_c", date_c);
		cmd_inq.Parameters.Set("compose_list_no", compose_list_no);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			check_flag = cmd_inq.GetString(1);
		}
		cmd_inq.Close();
		if (check_flag == "1")
		{
			strcpy(s.msg, "配料单已审核不能锁定物料计算!");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		sqlstr = " update tfbsm12 set MODEL_ID = @model_id,ORIGIN_CODE = ' ',COMPOSE_LIST_NO = ' '"
			",check_flag=' ',CHECK_MAKE=' ',CHECK_DATE=' '"
			",MB_CREATOR=' ',MB_CREATE_TIME=' ',MB_MARK=' '"
			" where COMPOSE_LIST_NO =@compose_list_no"
			" and check_flag !='1'" //审核过的不能启动模型
			" and ST_NO = @st_no"
			" and BACKLOG_EA = @backlog_ea"
			" and date_c = @date_c"
			;
		cmd_plan.SetCommandText(sqlstr);
		cmd_plan.Parameters.Set("model_id", model_id);
		cmd_plan.Parameters.Set("compose_list_no", compose_list_no);
		cmd_plan.Parameters.Set("st_no", st_no);
		cmd_plan.Parameters.Set("backlog_ea", backlog_ea);
		cmd_plan.Parameters.Set("date_c", date_c);
		cmd_plan.ExecuteNonQuery();
		cmd_plan.Close();
		

		//进行重新核算
		sqlstr = " select st_no,backlog_ea,date_c"
			" from tfbsm12"
			" where MODEL_ID = @model_id"
			" group by  st_no,backlog_ea,date_c"
			;
		cmd_plan.SetCommandText(sqlstr);
		cmd_plan.Parameters.Set("model_id", model_id);
		cmd_plan.ExecuteReader();
		while (cmd_plan.Read())
		{
			st_no = cmd_plan.GetString(1);
			backlog_ea = cmd_plan.GetString(2);
			date_c = cmd_plan.GetString(3);
			//取大类和中类
		sqlstr = " select COMM_FMLY_CODE,MATERIAL_CODE,CO_ST_NO,VOD_USE_FLAG  "
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
		cmd_inq.Close();

		//如果是Ni钢
		if (st_no.SubstringNE(1, 1) == "A" || st_no.SubstringNE(1, 1) == "D" || ((st_no.SubstringNE(1, 1) == "M" || st_no.SubstringNE(1, 1) == "F") && comm_fmly_code == 'V'))
		{
			
			sqlstr = " select nvl(max(case when ELM_NAME = 'P' then SPE_MAX else 0 end),100)"
				" FROM TQMTS02"
				" WHERE 1=1"
				" and not exists (select 1 from tfbsm06 where  P_FLAG = '0' and backlog_ea = @backlog_ea and st_no = @st_no)"
				" and ELM_NAME = 'P' "
				" and IDX_NO = (SELECT ELM_STD_IDX_A FROM TQMTS0X WHERE ST_NO = @st_no)"
					;
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.Parameters.Set("st_no", st_no);
				cmd_inq.Parameters.Set("backlog_ea", backlog_ea);
				cmd_inq.ExecuteReader();
				spe_max = 0;
				p_mode = " ";
				while (cmd_inq.Read())
				{
					if (cmd_inq.GetDecimal(1) <= 0.03)
					{
						p_mode = "1";
						spe_max = 0.015;
					}
					if (cmd_inq.GetDecimal(1) <= 0.025)
					{
						p_mode = "1";
						spe_max = 0.01;
					}
					if (cmd_inq.GetDecimal(1) <= 0.02)
					{
						p_mode = "1";
						spe_max = 0.007;
					}
					if (cmd_inq.GetDecimal(1) <= 0.015)
					{
						p_mode = "1";
						spe_max = 0.005;
					}

				}

		}

		//插入模型计算的主档项
		sqlstr = " select seq_no,FURNACE_COUNT"
			" from tfbsm12"
			" where 1=1"
			" and model_id = @model_id"
			" and st_no = @st_no"
			" and backlog_ea = @backlog_ea"
			" and date_c = @date_c"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("st_no", st_no);
		cmd_inq.Parameters.Set("backlog_ea", backlog_ea);
		cmd_inq.Parameters.Set("date_c", date_c);
		cmd_inq.Parameters.Set("model_id", model_id);
		cmd_inq.ExecuteReader();
		int i = 0;
		while (cmd_inq.Read())
		{
			inDMST02.Tables["LG_PSSM"].Rows.Add();
			inDMST02.Tables["LG_PSSM"].Rows[i]["ST_NO"] = st_no;
			inDMST02.Tables["LG_PSSM"].Rows[i]["BACKLOG_EA"] = backlog_ea;
			inDMST02.Tables["LG_PSSM"].Rows[i]["DATE_C"] = date_c;
			inDMST02.Tables["LG_PSSM"].Rows[i]["SEQ_NO"] = cmd_inq.GetString(1);
			inDMST02.Tables["LG_PSSM"].Rows[i]["FURNACE_COUNT"] = cmd_inq.GetDecimal(2);
			inDMST02.Tables["LG_PSSM"].Rows[i]["SUM_FURNACES"] = cmd_inq.GetDecimal(2);
			inDMST02.Tables["LG_PSSM"].Rows[i]["MATERIAL_CODE"] = material_code;
			inDMST02.Tables["LG_PSSM"].Rows[i]["P_MODE"] = p_mode;
			inDMST02.Tables["LG_PSSM"].Rows[i]["SPE_MAX"] = spe_max.ToString();
			i++;
		}
		cmd_inq.Close();		


		//原料库存成分信息(取工序的消耗),获取需要对应那一套的消耗项
		sqlstr =
			" SELECT st_no,backlog_ea,rn"
			" from ("
			" SELECT  st_no,backlog_ea,1 rn from TFBSM01 where backlog_ea=@backlog_ea and st_no=@st_no"
			" union "
			" SELECT  st_no,backlog_ea,2 rn from TFBSM01 t where  st_no=@st_no  and backlog_ea = ' '"
			" union "
			" SELECT  st_no,backlog_ea,3 rn from TFBSM01 t where  backlog_ea=@backlog_ea and st_no=@st_no_1"
			" union "
			" SELECT  st_no,backlog_ea,4 rn from TFBSM01 t where  st_no=@st_no_1  and backlog_ea = ' '"
			" union "
			" SELECT  st_no,backlog_ea,5 rn from TFBSM01 t where  backlog_ea=@backlog_ea and st_no=@st_no_2"
			" union "
			" SELECT  st_no,backlog_ea,6 rn from TFBSM01 t where  st_no=@st_no_2  and backlog_ea = ' '"
			" )"
			" order by rn"
			;
		cmd_inq.SetCommandText(sqlstr);		
		cmd_inq.Parameters.Set("st_no", st_no);
		cmd_inq.Parameters.Set("st_no_1", st_no_1);
		cmd_inq.Parameters.Set("st_no_2", st_no_2);
		cmd_inq.Parameters.Set("backlog_ea", backlog_ea);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			mat_st_no = cmd_inq.GetString(1);
			mat_backlog_ea = cmd_inq.GetString(2);
		}
		cmd_inq.Close();

		//锁定物料
		int rowcount = 0;
		if (bcls_rec->Tables.Contains("IF"))
		{
			for (int i = 0; i < bcls_rec->Tables["IF"].Rows.get_Count(); i++)
			{
				tfbsm14.Reset();
				tfbsm14.MergeFrom(bcls_rec->Tables["IF"].Rows[i]);
				inDMST02.Tables["LOCKED_TABLE"].Rows.Add();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["ST_NO"] = st_no;
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["STATION_ID"] = "Z";
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["MAT_CODE"] = tfbsm14["MAT_CODE"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["MAT_NAME"] = tfbsm14["MAT_NAME"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOT_NO"] = tfbsm14["LOT_NO"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOCK_MODE"] = " ";
				
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

				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["STOCK_NAME"] = tfbsm14["STOCK_NAME"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOCKED_WT"] = tfbsm14["WEIGHT"].ToString();
				rowcount++;
			}
		}
		if (bcls_rec->Tables.Contains("BOF"))
		{
			for (int i = 0; i < bcls_rec->Tables["BOF"].Rows.get_Count(); i++)
			{
				tfbsm14.Reset();
				tfbsm14.MergeFrom(bcls_rec->Tables["BOF"].Rows[i]);
				inDMST02.Tables["LOCKED_TABLE"].Rows.Add();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["ST_NO"] = st_no;
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["STATION_ID"] = "B";
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["MAT_CODE"] = tfbsm14["MAT_CODE"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["MAT_NAME"] = tfbsm14["MAT_NAME"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOT_NO"] = tfbsm14["LOT_NO"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOCK_MODE"] = " ";
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

				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["STOCK_NAME"] = tfbsm14["STOCK_NAME"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOCKED_WT"] = tfbsm14["WEIGHT"].ToString();
				rowcount++;
			}
		}
		if (bcls_rec->Tables.Contains("EAF"))
		{
			for (int i = 0; i < bcls_rec->Tables["EAF"].Rows.get_Count(); i++)
			{
				tfbsm14.Reset();
				tfbsm14.MergeFrom(bcls_rec->Tables["EAF"].Rows[i]);
				inDMST02.Tables["LOCKED_TABLE"].Rows.Add();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["ST_NO"] = st_no;
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["STATION_ID"] = "E";
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["MAT_CODE"] = tfbsm14["MAT_CODE"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["MAT_NAME"] = tfbsm14["MAT_NAME"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOT_NO"] = tfbsm14["LOT_NO"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOCK_MODE"] = " ";
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

				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["STOCK_NAME"] = tfbsm14["STOCK_NAME"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOCKED_WT"] = tfbsm14["WEIGHT"].ToString();
				rowcount++;
			}
		}
		if (bcls_rec->Tables.Contains("AOD"))
		{
			for (int i = 0; i < bcls_rec->Tables["AOD"].Rows.get_Count(); i++)
			{
				tfbsm14.Reset();
				tfbsm14.MergeFrom(bcls_rec->Tables["AOD"].Rows[i]);
				inDMST02.Tables["LOCKED_TABLE"].Rows.Add();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["ST_NO"] = st_no;
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["STATION_ID"] = "A";
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["MAT_CODE"] = tfbsm14["MAT_CODE"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["MAT_NAME"] = tfbsm14["MAT_NAME"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOT_NO"] = tfbsm14["LOT_NO"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOCK_MODE"] = " ";
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

				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["STOCK_NAME"] = tfbsm14["STOCK_NAME"].ToString();
				inDMST02.Tables["LOCKED_TABLE"].Rows[rowcount]["LOCKED_WT"] = tfbsm14["WEIGHT"].ToString();
				rowcount++;
			}
		}

		sqlstr = " select t1.mat_code, nvl(t3.MAT_TYPE_PL1, ' ') as BACK_C2 ,t3.mat_name,t2.STOCK_NAME,t2.lot_no"
			",case when t2.STOCK_NAME = '5' then t2.WEIGHT when  t2.STOCK_NAME like 'VS%' and t2.STOCK_NAME not in ( select BUNKER_NO from tmmsm60 where CO_BUNKER='1') and t1.mat_code!='AB070297' then (t2.WEIGHT / 1000) - 100 else  t2.WEIGHT / 1000 END - nvl(t4.weight, 0) as STOCK_WT"
			" ,case when t2.STOCK_NAME = '5' then t2.WEIGHT when  t2.STOCK_NAME like 'VS%' and t2.STOCK_NAME not in ( select BUNKER_NO from tmmsm60 where CO_BUNKER='1') and t1.mat_code!='AB070297' then (t2.WEIGHT / 1000) - 100 else  t2.WEIGHT / 1000 END AS stock_wt_qc"
			" ,NVL(C ,0) C_VALUE,NVL(Si  ,0)  SI_VALUE,NVL(Mn,0)  MN_VALUE,NVL(P  ,0) P_VALUE,NVL(S ,0) S_VALUE,NVL(Cr ,0)  CR_VALUE,NVL(Ni,0)  NI_VALUE, "
			" NVL(Mo,0) MO_VALUE,NVL(Cu,0) Cu_VALUE,NVL(co,0) CO_VALUE,NVL(TI,0) TI_VALUE,NVL(nb,0) NB_VALUE,NVL(al,0) AL_VALUE "
			" ,NVL(V,0) V_VALUE ,NVL(B,0)  B_VALUE ,NVL(N,0)  N_VALUE ,NVL(CA,0) CA_VALUE  "
			", nvl(t3.LESS_1, ' ') as LESS_1"
			" from ("
			" select mat_code"
			" from tfbsm01  "
			" where 1=1"
			" and backlog_ea ='" + mat_backlog_ea + "'"
			" and st_no='" + mat_st_no + "'"
			" group by mat_code"
			" ) t1"
			" left join fbsm_13_ai t2 on t1.mat_code=t2.mat_code "
			" left join tmmsm50 t3 on t1.mat_code=t3.mat_code "
			//扣减审核配料单消耗掉的
			" left join ("
			"   select mat_code,lot_no,stock_name,sum(WEIGHT) WEIGHT"
			" from tfbsm12 t1"
			" left join tfbsm14 t2 on t1.compose_list_no = t2.compose_list_no"
			" where CHECK_FLAG = '1'"
			" AND t1.DATE_C =  '" + date_c + "'"
			" group by mat_code,lot_no,stock_name"
			" ) t4 on t2.mat_code = t4.mat_code and t2.lot_no = t4.lot_no and t2.stock_name=t4.stock_name"
			" where case when t2.STOCK_NAME = '5' then t2.WEIGHT when  t2.STOCK_NAME like 'VS%' and t2.STOCK_NAME not in ( select BUNKER_NO from tmmsm60 where CO_BUNKER='1') and t1.mat_code!='AB070297' then (t2.WEIGHT / 1000) - 100 else  t2.WEIGHT / 1000 END - nvl(t4.weight, 0) >0"
			;
		if (mat_code_list.Trim() != "") //一级库的物料编码可用
		{
			sqlstr = sqlstr + " and (t2.STOCK_NAME !='5' or (t2.STOCK_NAME ='5' and INSTR('" + mat_code_list + "', t2.mat_code) >0))";
		}
		else
		{
			sqlstr = sqlstr + " and t2.STOCK_NAME !='5'";
		}
		//扣除镍生铁配置不能用的		
		{
			sqlstr = sqlstr + "  and not exists ( select 1 from tfbsm24 t24 where NVL(t2.Ni, 0)<t24.UPPER_LIMIT_VALUE and NVL(t2.Ni, 0)>t24.LOWER_LIMIT_VALUE and t24.st_no=@st_no and t3.MAT_TYPE_PL1='6')";
			sqlstr = sqlstr + "  and not exists ( select 1 from tfbsm24 t24 where NVL(t2.Ni, 0)<t24.UPPER_LIMIT_VALUE and NVL(t2.Ni, 0)>t24.LOWER_LIMIT_VALUE and t24.material_code=@material_code and MARK_POS_CODE = '1' and t3.MAT_TYPE_PL1='6')";
			sqlstr = sqlstr + "  and not exists ( select 1 from tfbsm24 t24 where NVL(t2.Ni, 0)<t24.UPPER_LIMIT_VALUE and NVL(t2.Ni, 0)>t24.LOWER_LIMIT_VALUE and t24.comm_fmly_code=@comm_fmly_code and MARK_POS_CODE = '2' and t3.MAT_TYPE_PL1='6')";
		}
		if (flag_co != "1")
		{
			sqlstr = sqlstr + " and t2.STOCK_NAME not in (select BUNKER_NO from tmmsm60 where CO_BUNKER='1') ";
		}
		else
		{
			sqlstr = sqlstr + " and  ((t2.STOCK_NAME  like 'VS%' and t2.STOCK_NAME in (select BUNKER_NO from tmmsm60 where CO_BUNKER='1')) or t2.STOCK_NAME not like 'VS%') ";
		}

		Log::Trace("", __FUNCTION__, "sqlstr = [{0} ]", sqlstr);
		cmd.SetCommandText(sqlstr);
		cmd.Parameters.Set("st_no", st_no);
		cmd.Parameters.Set("material_code", material_code);
		cmd.Parameters.Set("comm_fmly_code", comm_fmly_code);
		cmd.SetCommandText(sqlstr);
		cmd.ExecuteReader();
		while (cmd.Read())
		{
			tfbsm18.Reset(); //清空成默认值
			cmd.Fetch(tfbsm18);
			tfbsm18["ST_NO"] = st_no;
			tfbsm18["BACKLOG_EA"] = backlog_ea;

			if (bcls_rec->Tables.Contains("IF"))
			{
				for (int i = 0; i < bcls_rec->Tables["IF"].Rows.get_Count(); i++)
				{
					tfbsm14.Reset();
					tfbsm14.MergeFrom(bcls_rec->Tables["IF"].Rows[i]);
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

					if (tfbsm18["MAT_CODE"].ToString() == tfbsm14["MAT_CODE"].ToString() && tfbsm18["STOCK_NAME"].ToString() == tfbsm14["STOCK_NAME"].ToString() && tfbsm18["STOCK_NAME"].ToString() == tfbsm14["STOCK_NAME"].ToString())
					{
						tfbsm18["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal();
						tfbsm18["SI_VALUE"] = tfbsm14["SI_VALUE"].ToDecimal();
						tfbsm18["MN_VALUE"] = tfbsm14["MN_VALUE"].ToDecimal();
						tfbsm18["P_VALUE"] = tfbsm14["P_VALUE"].ToDecimal();
						tfbsm18["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal();
						tfbsm18["CR_VALUE"] = tfbsm14["CR_VALUE"].ToDecimal();
						tfbsm18["NI_VALUE"] = tfbsm14["NI_VALUE"].ToDecimal();
						tfbsm18["MO_VALUE"] = tfbsm14["MO_VALUE"].ToDecimal();
						tfbsm18["CU_VALUE"] = tfbsm14["CU_VALUE"].ToDecimal();
						tfbsm18["CO_VALUE"] = tfbsm14["CO_VALUE"].ToDecimal();
						tfbsm18["TI_VALUE"] = tfbsm14["TI_VALUE"].ToDecimal();
						tfbsm18["NB_VALUE"] = tfbsm14["NB_VALUE"].ToDecimal();
						tfbsm18["AL_VALUE"] = tfbsm14["AL_VALUE"].ToDecimal();
						tfbsm18["V_VALUE"] = tfbsm14["V_VALUE"].ToDecimal();
						tfbsm18["B_VALUE"] = tfbsm14["B_VALUE"].ToDecimal();
						tfbsm18["N_VALUE"] = tfbsm14["N_VALUE"].ToDecimal();
						tfbsm18["CA_VALUE"] = tfbsm14["CA_VALUE"].ToDecimal();
						break;
					}
				}
			}

			if (bcls_rec->Tables.Contains("BOF"))
			{
				for (int i = 0; i < bcls_rec->Tables["BOF"].Rows.get_Count(); i++)
				{
					tfbsm14.Reset();
					tfbsm14.MergeFrom(bcls_rec->Tables["BOF"].Rows[i]);
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

					if (tfbsm18["MAT_CODE"].ToString() == tfbsm14["MAT_CODE"].ToString() && tfbsm18["STOCK_NAME"].ToString() == tfbsm14["STOCK_NAME"].ToString() && tfbsm18["STOCK_NAME"].ToString() == tfbsm14["STOCK_NAME"].ToString())
					{
						tfbsm18["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal();
						tfbsm18["SI_VALUE"] = tfbsm14["SI_VALUE"].ToDecimal();
						tfbsm18["MN_VALUE"] = tfbsm14["MN_VALUE"].ToDecimal();
						tfbsm18["P_VALUE"] = tfbsm14["P_VALUE"].ToDecimal();
						tfbsm18["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal();
						tfbsm18["CR_VALUE"] = tfbsm14["CR_VALUE"].ToDecimal();
						tfbsm18["NI_VALUE"] = tfbsm14["NI_VALUE"].ToDecimal();
						tfbsm18["MO_VALUE"] = tfbsm14["MO_VALUE"].ToDecimal();
						tfbsm18["CU_VALUE"] = tfbsm14["CU_VALUE"].ToDecimal();
						tfbsm18["CO_VALUE"] = tfbsm14["CO_VALUE"].ToDecimal();
						tfbsm18["TI_VALUE"] = tfbsm14["TI_VALUE"].ToDecimal();
						tfbsm18["NB_VALUE"] = tfbsm14["NB_VALUE"].ToDecimal();
						tfbsm18["AL_VALUE"] = tfbsm14["AL_VALUE"].ToDecimal();
						tfbsm18["V_VALUE"] = tfbsm14["V_VALUE"].ToDecimal();
						tfbsm18["B_VALUE"] = tfbsm14["B_VALUE"].ToDecimal();
						tfbsm18["N_VALUE"] = tfbsm14["N_VALUE"].ToDecimal();
						tfbsm18["CA_VALUE"] = tfbsm14["CA_VALUE"].ToDecimal();
						break;
					}
				}
			}

			if (bcls_rec->Tables.Contains("EAF"))
			{
				for (int i = 0; i < bcls_rec->Tables["EAF"].Rows.get_Count(); i++)
				{
					tfbsm14.Reset();
					tfbsm14.MergeFrom(bcls_rec->Tables["EAF"].Rows[i]);
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

					if (tfbsm18["MAT_CODE"].ToString() == tfbsm14["MAT_CODE"].ToString() && tfbsm18["STOCK_NAME"].ToString() == tfbsm14["STOCK_NAME"].ToString() && tfbsm18["STOCK_NAME"].ToString() == tfbsm14["STOCK_NAME"].ToString())
					{
						tfbsm18["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal();
						tfbsm18["SI_VALUE"] = tfbsm14["SI_VALUE"].ToDecimal();
						tfbsm18["MN_VALUE"] = tfbsm14["MN_VALUE"].ToDecimal();
						tfbsm18["P_VALUE"] = tfbsm14["P_VALUE"].ToDecimal();
						tfbsm18["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal();
						tfbsm18["CR_VALUE"] = tfbsm14["CR_VALUE"].ToDecimal();
						tfbsm18["NI_VALUE"] = tfbsm14["NI_VALUE"].ToDecimal();
						tfbsm18["MO_VALUE"] = tfbsm14["MO_VALUE"].ToDecimal();
						tfbsm18["CU_VALUE"] = tfbsm14["CU_VALUE"].ToDecimal();
						tfbsm18["CO_VALUE"] = tfbsm14["CO_VALUE"].ToDecimal();
						tfbsm18["TI_VALUE"] = tfbsm14["TI_VALUE"].ToDecimal();
						tfbsm18["NB_VALUE"] = tfbsm14["NB_VALUE"].ToDecimal();
						tfbsm18["AL_VALUE"] = tfbsm14["AL_VALUE"].ToDecimal();
						tfbsm18["V_VALUE"] = tfbsm14["V_VALUE"].ToDecimal();
						tfbsm18["B_VALUE"] = tfbsm14["B_VALUE"].ToDecimal();
						tfbsm18["N_VALUE"] = tfbsm14["N_VALUE"].ToDecimal();
						tfbsm18["CA_VALUE"] = tfbsm14["CA_VALUE"].ToDecimal();
						break;
					}
				}
			}


			//如果有锁定物料，则实用锁定物料配置的成分进行更新库存成分
			if (bcls_rec->Tables.Contains("AOD"))
			{
				for (int i = 0; i < bcls_rec->Tables["AOD"].Rows.get_Count(); i++)
				{
					tfbsm14.Reset();
					tfbsm14.MergeFrom(bcls_rec->Tables["AOD"].Rows[i]);
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

					if (tfbsm18["MAT_CODE"].ToString() == tfbsm14["MAT_CODE"].ToString() && tfbsm18["STOCK_NAME"].ToString() == tfbsm14["STOCK_NAME"].ToString() && tfbsm18["STOCK_NAME"].ToString() == tfbsm14["STOCK_NAME"].ToString())
					{
						tfbsm18["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal();
						tfbsm18["SI_VALUE"] = tfbsm14["SI_VALUE"].ToDecimal();
						tfbsm18["MN_VALUE"] = tfbsm14["MN_VALUE"].ToDecimal();
						tfbsm18["P_VALUE"] = tfbsm14["P_VALUE"].ToDecimal();
						tfbsm18["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal();
						tfbsm18["CR_VALUE"] = tfbsm14["CR_VALUE"].ToDecimal();
						tfbsm18["NI_VALUE"] = tfbsm14["NI_VALUE"].ToDecimal();
						tfbsm18["MO_VALUE"] = tfbsm14["MO_VALUE"].ToDecimal();
						tfbsm18["CU_VALUE"] = tfbsm14["CU_VALUE"].ToDecimal();
						tfbsm18["CO_VALUE"] = tfbsm14["CO_VALUE"].ToDecimal();
						tfbsm18["TI_VALUE"] = tfbsm14["TI_VALUE"].ToDecimal();
						tfbsm18["NB_VALUE"] = tfbsm14["NB_VALUE"].ToDecimal();
						tfbsm18["AL_VALUE"] = tfbsm14["AL_VALUE"].ToDecimal();
						tfbsm18["V_VALUE"] = tfbsm14["V_VALUE"].ToDecimal();
						tfbsm18["B_VALUE"] = tfbsm14["B_VALUE"].ToDecimal();
						tfbsm18["N_VALUE"] = tfbsm14["N_VALUE"].ToDecimal();
						tfbsm18["CA_VALUE"] = tfbsm14["CA_VALUE"].ToDecimal();
						break;
					}
				}
			}

			CString key = "";
			if (tfbsm18["LOT_NO"].ToString().Trim() == "")
			{
				key = tfbsm18["STOCK_NAME"].ToString().Trim() + "|" + tfbsm18["MAT_CODE"].ToString().Trim();
			}
			else
			{
				key = tfbsm18["STOCK_NAME"].ToString().Trim() + "|" + tfbsm18["MAT_CODE"].ToString().Trim() + "|" + tfbsm18["LOT_NO"].ToString().Trim();
			}
			CDecimal stock_value = tfbsm18["STOCK_WT"].ToDecimal();
			auto row_c = initMap.find(key);
			if (row_c == initMap.end())
			{//库存初始化
				initMap[key] = tfbsm18["STOCK_WT"].ToDecimal();
			}

			tfbsm18["STOCK_WT_QC"] = initMap[key];  //启动的起初
			tfbsm18["STOCK_WT"] = initMap[key] - modelResultMap[key];	//库存可用量	
			if (tfbsm18["STOCK_WT"].ToDecimal() > 0)
			{
				//价格计算
				bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = tfbsm18["MAT_CODE"];
				bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = tfbsm18["MAT_NAME"];
				bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = tfbsm18["CR_VALUE"];
				bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = tfbsm18["NI_VALUE"];
				bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = tfbsm18["MO_VALUE"];
				doFlag = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
				if (doFlag < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
				tfbsm18["PRICE"] = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"];

				CDataRow& row_MatTable = inDMST02.Tables["MatTable"].Rows.Add();
				row_MatTable.Merge(tfbsm18);
				row_MatTable["COST"] = tfbsm18["PRICE"].ToString().Trim();
			}
			
		}
		cmd.Close();

		Log::Trace("", __FUNCTION__, "库存量 = [{0} ]", inDMST02.Tables["MatTable"].Rows.get_Count());
		//全脱，三脱，脱磷预熔液的成分需要传进去
		if (backlog_ea == "06" || backlog_ea == "07" || backlog_ea == "08")
		{
			sqlstr = " SELECT * FROM TFBSM13 WHERE BACKLOG_EA='" + backlog_ea + "' ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				tfbsm13.Reset();
				cmd_inq.Fetch(tfbsm13);
				CDataRow& row_MatTable = inDMST02.Tables["MatTable"].Rows.Add();				
				row_MatTable["LOT_NO"] = " ";
				row_MatTable["BACK_C2"] = " ";
				row_MatTable["ST_NO"] = st_no;
				row_MatTable["STOCK_NAME"] = " ";
				row_MatTable["STOCK_WT"] = "9999";
				row_MatTable["MAT_CODE"] = "TS0000";
				row_MatTable["MAT_NAME"] = "铁水";
				row_MatTable["C_VALUE"] = tfbsm13["C_VALUE"].ToString().Trim();
				row_MatTable["SI_VALUE"] = tfbsm13["SI_VALUE"].ToString().Trim();
				row_MatTable["MN_VALUE"] = tfbsm13["MN_VALUE"].ToString().Trim();
				row_MatTable["P_VALUE"] = tfbsm13["P_VALUE"].ToString().Trim();
				row_MatTable["S_VALUE"] = tfbsm13["S_VALUE"].ToString().Trim();
				row_MatTable["CR_VALUE"] = tfbsm13["CR_VALUE"].ToString().Trim();
				row_MatTable["NI_VALUE"] = tfbsm13["NI_VALUE"].ToString().Trim();
				row_MatTable["MO_VALUE"] = tfbsm13["MO_VALUE"].ToString().Trim();
				row_MatTable["CU_VALUE"] = tfbsm13["CU_VALUE"].ToString().Trim();
				row_MatTable["CO_VALUE"] = "0";
				row_MatTable["TI_VALUE"] = "0";
				row_MatTable["NB_VALUE"] = "0";
				row_MatTable["AL_VALUE"] = "0";
				row_MatTable["V_VALUE"] = "0";
				row_MatTable["B_VALUE"] = "0";
				row_MatTable["N_VALUE"] = "0";
				row_MatTable["CA_VALUE"] = "0";
				row_MatTable["COST"] = "2294";
				row_MatTable["LESS_1"] = " ";
			}
			cmd_inq.Close();
		}
		//WORKTABLE（工序基准成分）
		sqlstr = "SELECT "
			"    '" + comm_fmly_code + "' COMM_FMLY_CODE, "
			"    '" + material_code + "'  MATERIAL_CODE, "
			"    '" + st_no + "'  ST_NO, "
			"    STATION_ID, "
			"    ELM_NAME, "
			"    max(SPE_MIN) SPE_MIN, "
			"    max(SPE_MAX) SPE_MAX, "
			"    '" + backlog_ea + "' BACKLOG_EA, "
			"    CASE WHEN MAX(YIELD)=0 THEN 100 ELSE MAX(YIELD) END AS YIELD"
			"  FROM ( "
			"    SELECT STATION_ID, ELM_NAME, SPE_MIN,  SPE_MAX,0 YIELD "
			"    FROM ( "
			"  SELECT  A.*,  ROW_NUMBER() OVER ( PARTITION BY A.ELM_NAME, A.STATION_ID ORDER BY  SEQ_RN ) as rn "
			"  FROM ("
			"	select STATION_ID,ELM_NAME,SPE_MAX,SPE_MIN,MAIN_AIM,1 as seq_rn from tfbsm05 where  BACKLOG_EA = '" + backlog_ea + "' and ST_NO = '" + st_no + "'"
			"	union all "
			"	select STATION_ID,ELM_NAME,SPE_MAX,SPE_MIN,MAIN_AIM,2 as seq_rn from tfbsm05 where  BACKLOG_EA = '" + backlog_ea + "' and ST_NO = '" + st_no_1 + "'"
			"	union all "
			"	select STATION_ID,ELM_NAME,SPE_MAX,SPE_MIN,MAIN_AIM,3 as seq_rn from tfbsm05 where  BACKLOG_EA = '" + backlog_ea + "' and ST_NO = '" + st_no_2 + "'"		
			"	union all "
			"	select 'A' as STATION_ID,ELM_NAME, SPE_MAX, SPE_MIN, MAIN_AIM,0 as seq_rn from TQMTS02 WHERE ELM_NAME in ('C','Si','Mn','P','S','Cr','Ni','Mo','Cu','Co') and IDX_NO = (SELECT ELM_STD_IDX_A FROM TQMTS0X WHERE ST_NO = '" + st_no + "')"
			"	) A"
			" )"
			"  WHERE rn = 1 "
			" union all  "
			//收得率信息
			" SELECT STATION_ID, ELM_NAME, 0 SPE_MIN,  0 SPE_MAX, YIELD from ( "
			"  SELECT  B.*,  ROW_NUMBER() OVER ( PARTITION BY B.ELM_NAME, B.STATION_ID ORDER BY  SEQ_RN ) as rn "
			"  from ("
			" select STATION_ID,'Cr' ELM_NAME, CASE WHEN YIELD_CR != 0 THEN YIELD_CR else 100  END YIELD, 1 seq_rn"
			" FROM tfbsm04  WHERE  BACKLOG_EA = '" + backlog_ea + "'   AND ST_NO = '" + st_no + "' "
			" union all"
			" select STATION_ID,'Ni' ELM_NAME, CASE WHEN YIELD_ni != 0 THEN YIELD_ni else 100  END YIELD, 1 seq_rn"
			" FROM tfbsm04  WHERE  BACKLOG_EA = '" + backlog_ea + "'   AND ST_NO = '" + st_no + "' "
			" union all"
			" select STATION_ID,'Mo' ELM_NAME, CASE WHEN YIELD_mo != 0 THEN YIELD_mo else 100  END YIELD, 1 seq_rn"
			" FROM tfbsm04  WHERE  BACKLOG_EA = '" + backlog_ea + "'   AND ST_NO = '" + st_no + "' "
			" union all"
			" select STATION_ID,'Cr' ELM_NAME, CASE WHEN YIELD_CR != 0 THEN YIELD_CR else 100  END YIELD, 2 seq_rn"
			" FROM tfbsm04  WHERE  BACKLOG_EA = '" + backlog_ea + "'   AND ST_NO = '" + st_no_1 + "' "
			" union all"
			" select STATION_ID,'Ni' ELM_NAME, CASE WHEN YIELD_ni != 0 THEN YIELD_ni else 100  END YIELD, 2 seq_rn"
			" FROM tfbsm04  WHERE  BACKLOG_EA = '" + backlog_ea + "'   AND ST_NO = '" + st_no_1 + "' "
			" union all"
			" select STATION_ID,'Mo' ELM_NAME, CASE WHEN YIELD_mo != 0 THEN YIELD_mo else 100  END YIELD, 2 seq_rn"
			" FROM tfbsm04  WHERE  BACKLOG_EA = '" + backlog_ea + "'   AND ST_NO = '" + st_no_1 + "' "
			" union all"
			" select STATION_ID,'Cr' ELM_NAME, CASE WHEN YIELD_CR != 0 THEN YIELD_CR else 100  END YIELD, 3 seq_rn"
			" FROM tfbsm04  WHERE  BACKLOG_EA = '" + backlog_ea + "'   AND ST_NO = '" + st_no_2 + "' "
			" union all"
			" select STATION_ID,'Ni' ELM_NAME, CASE WHEN YIELD_ni != 0 THEN YIELD_ni else 100  END YIELD, 3 seq_rn"
			" FROM tfbsm04  WHERE  BACKLOG_EA = '" + backlog_ea + "'   AND ST_NO = '" + st_no_2 + "' "
			" union all"
			" select STATION_ID,'Mo' ELM_NAME, CASE WHEN YIELD_mo != 0 THEN YIELD_mo else 100  END YIELD, 3 seq_rn"
			" FROM tfbsm04  WHERE  BACKLOG_EA = '" + backlog_ea + "'   AND ST_NO = '" + st_no_2 + "' "
			" ) B"
			" )"
			" where rn = 1"
			" )"
			" WHERE  ELM_NAME != 'C' and  ELM_NAME != 'S' "
			" group by STATION_ID,ELM_NAME"
			;
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "inDMST02.WORKTABLE.sqlstr = [{0}]", sqlstr);
		inDMST02.Tables["WORKTABLE"].Rows.Clear();
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tfbsm19.Reset(); //清空成默认值
			cmd_inq.Fetch(tfbsm19);
			tfbsm19["ST_NO"] = st_no;
			tfbsm19["BACKLOG_EA"] = backlog_ea;

			CDataRow& row = inDMST02.Tables["WORKTABLE"].Rows.Add();
			row["COMM_FMLY_CODE"] = cmd_inq.GetString(1).Trim();
			row["MATERIAL_CODE"] = cmd_inq.GetString(2).Trim();
			row["ST_NO"] = cmd_inq.GetString(3).Trim();
			row["STATION_ID"] = cmd_inq.GetString(4).Trim();
			row["ELM_NAME"] = cmd_inq.GetString(5).Trim();
			row["SPE_MIN"] = cmd_inq.GetDecimal(6);
			row["SPE_MAX"] = cmd_inq.GetDecimal(7);
			row["BACKLOG_EA"] = cmd_inq.GetString(8).Trim();
			row["YIELD"] = cmd_inq.GetString(9).Trim();
		}
		cmd_inq.Close();

		sqlstr = "SELECT "
			"    CASE "
			"        WHEN g.WEIGHT_MINUS <> 0 THEN g.WEIGHT_MINUS "
			"        WHEN g.EAF_MIN <> 0 AND F.STATION_ID = 'B' THEN g.EAF_MIN "
			"        WHEN g.IF_MIN <> 0 AND F.STATION_ID = 'Z' THEN g.IF_MIN "
			"        ELSE f.LOWER_LIMIT_VALUE "
			"    END WEIGHT_MINUS, "
			"    CASE "
			"        WHEN g.WEIGHT_POSITIVE <> 0 THEN g.WEIGHT_POSITIVE "
			"        WHEN g.EAF_MAX <> 0 AND F.STATION_ID = 'B' THEN g.EAF_MAX "
			"        WHEN g.IF_MAX <> 0 AND F.STATION_ID = 'Z' THEN g.IF_MAX "
			"        ELSE f.UPPER_LIMIT_VALUE "
			"    END WEIGHT_POSITIVE, "
			"    CASE  WHEN G.YEILD_MINUS = 0 THEN 100 ELSE G.YEILD_MINUS END YEILD_MINUS, "
			"    F.* "
			" FROM ( "
			"    SELECT "
			"        COMM_FMLY_CODE, "
			"        MATERIAL_CODE, "
			"        '" + st_no + "'  as ST_NO, "
			"        STATION_ID, "
			"        MAT_CODE, "
			"        MAT_NAME, "
			"        UPPER_LIMIT_VALUE, "
			"        LOWER_LIMIT_VALUE, "
			"        CASE WHEN STATION_ID = 'A' THEN MUST_DO_FLAG ELSE 0 END MUST_DO_FLAG "
			"    FROM "
			" TFBSM01 WHERE ST_NO = '" + mat_st_no + "' " + "AND BACKLOG_EA = '" + mat_backlog_ea + "'"
			") F "
			"LEFT JOIN TMMSM50 G "
			"    ON F.MAT_CODE = G.MAT_CODE "
			;
		Log::Trace("", __FUNCTION__, "inDMST02.GX_MAT.sqlstr = [{0}]", sqlstr);
		cmd_inq.SetCommandText(sqlstr);
		inDMST02.Tables["GX_MAT"].Rows.Clear();
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			tfbsm26.Reset();
			cmd_inq.Fetch(tfbsm26);
			tfbsm26.TrimOrBlank();
			CDataRow& row = inDMST02.Tables["GX_MAT"].Rows.Add();
			if (cmd_inq.GetString(1) != tfbsm26["LOWER_LIMIT_VALUE"].ToString() && backlog_ea == "01" && tfbsm26["STATION_ID"].ToString() == "Z"){
				//如果值是50表的并且工艺路线是多中频炉则需要单独处理
				tfbsm26["LOWER_LIMIT_VALUE"] = cmd_inq.GetDecimal(1) + cmd_inq.GetDecimal(1);
			}
			else if (cmd_inq.GetString(1) != tfbsm26["LOWER_LIMIT_VALUE"].ToString() && backlog_ea == "04" && tfbsm26["STATION_ID"].ToString() == "Z"){
				tfbsm26["LOWER_LIMIT_VALUE"] = cmd_inq.GetDecimal(1) + cmd_inq.GetDecimal(1) + cmd_inq.GetDecimal(1);
			}
			else{
				tfbsm26["LOWER_LIMIT_VALUE"] = cmd_inq.GetDecimal(1);
			}
			if (cmd_inq.GetString(2) != tfbsm26["UPPER_LIMIT_VALUE"].ToString() && backlog_ea == "01" && tfbsm26["STATION_ID"].ToString() == "Z"){
				//如果值是50表的并且工艺路线是多中频炉则需要单独处理
				tfbsm26["UPPER_LIMIT_VALUE"] = cmd_inq.GetDecimal(2) + cmd_inq.GetDecimal(2);
			}
			else if (cmd_inq.GetString(2) != tfbsm26["UPPER_LIMIT_VALUE"].ToString() && backlog_ea == "04" && tfbsm26["STATION_ID"].ToString() == "Z"){
				tfbsm26["UPPER_LIMIT_VALUE"] = cmd_inq.GetDecimal(2) + cmd_inq.GetDecimal(2) + cmd_inq.GetDecimal(2);
			}
			else{
				tfbsm26["UPPER_LIMIT_VALUE"] = cmd_inq.GetDecimal(2);
			}

			row.Merge(tfbsm26);
			

			
		}
		cmd_inq.Close();

		//铁水的上下线
		if (backlog_ea == "06" || backlog_ea == "07" || backlog_ea == "08")
		{
			CDataRow& row_mat = inDMST02.Tables["GX_MAT"].Rows.Add();
				row_mat["LOWER_LIMIT_VALUE"] = "0";
				row_mat["UPPER_LIMIT_VALUE"] = "99999";
				row_mat["YEILD_MINUS"] = "100";
				row_mat["COMM_FMLY_CODE"] = comm_fmly_code;
				row_mat["MATERIAL_CODE"] = material_code;
				row_mat["ST_NO"] = st_no;
				if (backlog_ea == "06") //转炉工序取13表成分，其他两道工序传的与镍钢一致（01画面配置的）
				{
					row_mat["STATION_ID"] = "D";
				}
				if (backlog_ea == "07") //转炉工序取13表成分，其他两道工序传的与镍钢一致（01画面配置的）
				{
					row_mat["STATION_ID"] = "B";
				}
				if (backlog_ea == "08") //转炉工序取13表成分，其他两道工序传的与镍钢一致（01画面配置的）
				{
					row_mat["STATION_ID"] = "B";
				}
				row_mat["MAT_CODE"] = "TS0000";
				row_mat["MAT_NAME"] = "铁水";
				row_mat["MUST_DO_FLAG"] = "0";
			
		}
		//工位，金属收得率，出钢量范围，废钢范围，路径说明
		sqlstr = "SELECT '" + st_no + "' ST_NO,"
			"       A.STATION_ID,"
			"       A.BACKLOG_EA,"
			"       NVL(C.METAL_CONS_PERT,1000) METAL_CONS_PERT,"
			"               CASE WHEN A.STATION_ID = 'E' THEN NVL(B.EAF_MIN,0) "
			"                    WHEN A.STATION_ID = 'Z' THEN NVL(B.IF_MIN,0)"
			"                   WHEN A.STATION_ID = 'A' THEN NVL(B.AOD_MIN,0) "
			"                  ELSE 0  END AS SCRAP_RATIO_MIN,"
			"               CASE WHEN  A.STATION_ID = 'E' THEN NVL(B.EAF_MAX,0)"
			"                   WHEN  A.STATION_ID = 'Z' THEN NVL(B.IF_MAX,0)"
			"                   WHEN  A.STATION_ID = 'A' THEN NVL(B.AOD_MAX,0)"
			"                   ELSE 0 END AS SCRAP_RATIO_MAX,"
			"               CASE WHEN  A.STATION_ID = 'B' THEN NVL(B.IRON_RATIO2_MAX,0)"
			"                   WHEN  A.STATION_ID = 'D' THEN NVL(B.IRON_RATIO1_MAX,0)"
			"                   WHEN  A.STATION_ID = 'E' THEN NVL(B.DEVO_RATIO_EAF,0)"
			"                   WHEN  A.STATION_ID = 'Z' THEN NVL(B.DEVO_RATIO_IF,0)"
			"                   WHEN  A.STATION_ID = 'A' THEN case when @flag_vod='1' then VOD_MAX else TAPPING_WT end "
			"               ELSE 0 END AS TAPPING_WT,"
			"               CASE WHEN  A.STATION_ID = 'B' THEN NVL(B.IRON_RATIO2,0)"
			"                   WHEN  A.STATION_ID = 'D' THEN NVL(B.IRON_RATIO1,0)"
			"                   WHEN  A.STATION_ID = 'E' THEN NVL(B.DEVO_RATIO_EAF_MIN,0)"
			"                   WHEN  A.STATION_ID = 'Z' THEN NVL(B.DEVO_RATIO_IF_MIN,0)"
			"                   WHEN  A.STATION_ID = 'A' THEN case when @flag_vod='1'  then VOD_MIN else TAPPING_WT_MIN end "
			"               ELSE 0 END AS TAPPING_WT_MIN,"
			"               CASE"
			"                   WHEN A.STATION_ID = 'E' THEN '4'"
			"                   WHEN A.STATION_ID = 'Z' THEN '4'"
			"                   WHEN A.STATION_ID = 'A' THEN '12'"
			"                   WHEN A.STATION_ID = 'B' THEN '10'"
			"               ELSE '0' END AS MAT_COUNT,"
			"       case when A.STATION_ID = 'A' THEN A.DESCRIPTION ELSE ' ' END COMBY_GX "
			"  FROM "
			" (  select CODE AS BACKLOG_EA,DESCRIPTION,'A'  AS STATION_ID   FROM TFBSM02  WHERE code = '" + backlog_ea + "' "
			" union "
			" select CODE AS BACKLOG_EA,DESCRIPTION,TRIM(REGEXP_SUBSTR(DESCRIPTION, '[^,]+', 1, LEVEL)) AS STATION_ID   FROM TFBSM02  WHERE   code = '" + backlog_ea + "' "
			" and DESCRIPTION IS NOT NULL CONNECT BY PRIOR BACKLOG_EA = BACKLOG_EA"
			" AND PRIOR SYS_GUID() IS NOT NULL	AND LEVEL <= REGEXP_COUNT(DESCRIPTION, ',') + 1 "
			" ) A"
			"  left join ("
			" select * from ("
			"    SELECT t.*,ROW_NUMBER() OVER ( PARTITION BY t.STATION_ID ORDER BY seq_rn ) as rn"
			"      from ("
			"         select t.STATION_ID ,METAL_CONS_PERT,MARK_POS_CODE,COMM_FMLY_CODE,BACKLOG_EA,1 seq_rn  from tfbsm04 t where BACKLOG_EA ='" + backlog_ea + "' and ST_NO = '" + st_no + "'"
			"		  union "
			"         select t.STATION_ID ,METAL_CONS_PERT,MARK_POS_CODE,COMM_FMLY_CODE,BACKLOG_EA,2 seq_rn  from tfbsm04 t where BACKLOG_EA ='" + backlog_ea + "' AND ST_NO = '" + st_no_1 + "' "
			"		  union "
			"         select t.STATION_ID ,METAL_CONS_PERT,MARK_POS_CODE,COMM_FMLY_CODE,BACKLOG_EA,3 seq_rn  from tfbsm04 t where BACKLOG_EA ='" + backlog_ea + "' AND ST_NO = '" + st_no_2 + "' "
			"      ) t"
			" ) "
			"  where rn = 1 "
			"  ) C on A.STATION_ID = C.STATION_ID"
			//工序投料比
			" LEFT JOIN ("
			" select * from ("
			"    SELECT t.*,ROW_NUMBER() OVER (ORDER BY seq_rn ) as rn"
			"      from ("
			"         select t.*,1 seq_rn  from tfbsm09 t where   BACKLOG_EA ='" + backlog_ea + "' and ST_NO = '" + st_no + "'"
			"		  union "
			"         select t.*,2 seq_rn  from tfbsm09 t where   BACKLOG_EA ='" + backlog_ea + "' AND ST_NO = '" + st_no_1 + "' "
			"		  union "
			"         select t.*,3 seq_rn  from tfbsm09 t where   BACKLOG_EA ='" + backlog_ea + "' AND ST_NO = '" + st_no_2 + "' "
			"      ) t"
			" ) "
			"  where rn = 1 "
			" ) B ON A.backlog_ea = B.backlog_ea"
			;
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("st_no", st_no);
		cmd_inq.Parameters.Set("flag_vod", flag_vod);
		Log::Trace("", __FUNCTION__, "inDMST02.GX_WEIGHT.sqlstr = [{0}]", sqlstr);
		inDMST02.Tables["GX_WEIGHT"].Rows.Clear();
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			CDataRow& row = inDMST02.Tables["GX_WEIGHT"].Rows.Add();
			row["ST_NO"] = cmd_inq.GetString(1);
			row["STATION_ID"] = cmd_inq.GetString(2);
			row["BACKLOG_EA"] = cmd_inq.GetString(3);
			row["METAL_CONS_PERT"] = cmd_inq.GetString(4);
			row["SCRAP_RATIO_MIN"] = cmd_inq.GetString(5);
			row["SCRAP_RATIO_MAX"] = cmd_inq.GetString(6);
			row["TAPPING_WT"] = cmd_inq.GetString(7);
			if (cmd_inq.GetDecimal(8) != 0){
				row["TAPPING_WT_MIN"] = cmd_inq.GetString(8);
			}
			else{
				row["TAPPING_WT_MIN"] = cmd_inq.GetString(7);
			}
			row["MAT_COUNT"] = cmd_inq.GetString(9);
			row["COMBY_GX"] = cmd_inq.GetString(10);

			if (row["STATION_ID"].ToString() == "Z"){
				cg_if = row["TAPPING_WT"].ToDecimal();
			}			
		}
		cmd_inq.Close();

		//GX_LC（料槽管理） 1大于，0小于
		sqlstr = " select distinct IF_WT_MAX from tfbsm25 where BACKLOG_EA='" + backlog_ea + "'  ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			//判断如果取到的值比现在中频炉的值大则取1的数据
			if_wt_mat = cmd_inq.GetDecimal(1);
		}
		cmd_inq.Close();
		//判断数据
		if (cg_if >= if_wt_mat){
			//140>=135取1的数据
			sqlstr = " select  BACKLOG_EA,MAT_CODE,MAT_NAME,BIG_LOAD_WT_MAX,BIG_LOAD_WT_Min,SMALL_LOAD_WT_MAX,SMALL_LOAD_WT_MIN from tfbsm25 where BACKLOG_EA='" + backlog_ea + "' and IF_WT_CODE='1'  ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				CDataRow& row = inDMST02.Tables["GX_LC"].Rows.Add();
				row["BACKLOG_EA"] = cmd_inq.GetString(1);
				row["MAT_CODE"] = cmd_inq.GetString(2);
				row["MAT_NAME"] = cmd_inq.GetString(3);
				row["BIG_LOAD_WT_MAX"] = cmd_inq.GetString(4);
				row["BIG_LOAD_WT_MIN"] = cmd_inq.GetString(5);
				row["SMALL_LOAD_WT_MAX"] = cmd_inq.GetString(6);
				row["SMALL_LOAD_WT_MIN"] = cmd_inq.GetString(7);
			}
			cmd_inq.Close();

		}
		else{
			//120<135取0的数据
			sqlstr = " select  BACKLOG_EA,MAT_CODE,MAT_NAME,BIG_LOAD_WT_MAX,BIG_LOAD_WT_Min,SMALL_LOAD_WT_MAX,SMALL_LOAD_WT_MIN from tfbsm25 where BACKLOG_EA='" + backlog_ea + "' and IF_WT_CODE='0'  ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				CDataRow& row = inDMST02.Tables["GX_LC"].Rows.Add();
				row["BACKLOG_EA"] = cmd_inq.GetString(1);
				row["MAT_CODE"] = cmd_inq.GetString(2);
				row["MAT_NAME"] = cmd_inq.GetString(3);
				row["BIG_LOAD_WT_MAX"] = cmd_inq.GetString(4);
				row["BIG_LOAD_WT_MIN"] = cmd_inq.GetString(5);
				row["SMALL_LOAD_WT_MAX"] = cmd_inq.GetString(6);
				row["SMALL_LOAD_WT_MIN"] = cmd_inq.GetString(7);
			}
			cmd_inq.Close();
		}
		//小料槽的数据
		sqlstr = " select  BACKLOG_EA,MAT_CODE,MAT_NAME,BIG_LOAD_WT_MAX,BIG_LOAD_WT_Min,SMALL_LOAD_WT_MAX,SMALL_LOAD_WT_MIN from tfbsm25 where BACKLOG_EA=' '   ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			CDataRow& row = inDMST02.Tables["GX_LC"].Rows.Add();
			row["BACKLOG_EA"] = cmd_inq.GetString(1);
			row["MAT_CODE"] = cmd_inq.GetString(2);
			row["MAT_NAME"] = cmd_inq.GetString(3);
			row["BIG_LOAD_WT_MAX"] = cmd_inq.GetString(4);
			row["BIG_LOAD_WT_MIN"] = cmd_inq.GetString(5);
			row["SMALL_LOAD_WT_MAX"] = cmd_inq.GetString(6);
			row["SMALL_LOAD_WT_MIN"] = cmd_inq.GetString(7);
		}
		cmd_inq.Close();

		//Log::Trace("", __FUNCTION__, "11111111111111111 = [ ]");

		planFurnaceCountMap.clear();
		doFlag = f_epex_call_rest_svc(conn, "FBSM_SSMODEL", "RUN_PLMODEL", &inDMST02, &outDMST02);
		if (doFlag < 0)
		{
			throw CApplicationException(-1, s.msg, log.Location);
		}

		Log::Trace("", __FUNCTION__, "执行结束");
		if (outDMST02.Tables.Contains("WORK_TABLE"))
		{
			// 1.遍历WORK_TABLE，统计每个成功配料单的炉数
			int j = 0;
			for (int i = 0; i < outDMST02.Tables["WORK_TABLE"].Rows.get_Count(); i++)
			{
				CDataRow& workRow = outDMST02.Tables["WORK_TABLE"].Rows[i];
				CString planId = workRow["PLAN_ID"].ToString();
				CString pTrue = workRow["P_TRUE"].ToString();

				Log::Trace("", __FUNCTION__, "planId=[{0}],pTrue=[{1}]", planId, pTrue);

				// 只处理成功的配料单（P_TRUE不为0）
				if (pTrue != "0")
				{
					// 统计每个配料单的炉数（相同PLAN_ID在WORK_TABLE中出现的次数）
					if (planFurnaceCountMap.find(planId) != planFurnaceCountMap.end())
					{
						planFurnaceCountMap[planId] = planFurnaceCountMap[planId] + 1;
					}
					else
					{
						planFurnaceCountMap[planId] = 1;
						outPLAN.Tables[0].Rows.Add();
						outPLAN.Tables[0].Rows[j]["PLAN_ID"] = planId;
						outPLAN.Tables[0].Rows[j]["ST_NO"] = workRow["ST_NO"].ToString();
						outPLAN.Tables[0].Rows[j]["DATE_C"] = workRow["DATE_C"].ToString();
						outPLAN.Tables[0].Rows[j]["BACKLOG_EA"] = workRow["BACKLOG_EA"].ToString();
						//生成配料单信息
						sqlstr = " select  max(substr(COMPOSE_LIST_NO,12,14)),max(substr(COMPOSE_LIST_NO,14,1)),max(substr(COMPOSE_LIST_NO,13, 1)),max(substr(COMPOSE_LIST_NO,13, 2))"
							" from tfbsm14a"
							" where 1=1  "
							" and compose_list_no like @compose_list_no||'%' "
							;
						Log::Trace("", __FUNCTION__, "sqlstr = [{0} ]", sqlstr);
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("compose_list_no", workRow["ST_NO"].ToString().SubstringNE(1, 1) + workRow["MATERIAL_CODE"].ToString() + workRow["BACKLOG_EA"].ToString() + workRow["DATE_C"].ToString());
						cmd_inq.ExecuteReader();
						if (cmd_inq.Read())
						{
							if (cmd_inq.GetDecimal(3) == 0 && cmd_inq.GetDecimal(2) != 9){
								p_seq = cmd_inq.GetDecimal(2);
								p_seq = p_seq + 1;
								compose_list_no = workRow["ST_NO"].ToString().SubstringNE(1, 1) + workRow["MATERIAL_CODE"].ToString() + workRow["BACKLOG_EA"].ToString() + workRow["DATE_C"].ToString() + "0" + p_seq.ToString();
							}
							else
							{
								p_seq = cmd_inq.GetDecimal(4);
								p_seq = p_seq + 1;
								compose_list_no = workRow["ST_NO"].ToString().SubstringNE(1, 1) + workRow["MATERIAL_CODE"].ToString() + workRow["BACKLOG_EA"].ToString() + workRow["DATE_C"].ToString() + p_seq.ToString();
							}
						}
						else
						{
							compose_list_no = workRow["ST_NO"].ToString().SubstringNE(1, 1) + workRow["MATERIAL_CODE"].ToString() + workRow["BACKLOG_EA"].ToString() + workRow["DATE_C"].ToString() + "01";
						}
						cmd_inq.Close();

						outPLAN.Tables[0].Rows[j]["COMPOSE_LIST_NO"] = compose_list_no;
						j++;

						//插入配料单主档表
						sqlstr = " insert into tfbsm14A(COMPOSE_LIST_NO, DATE_C, ST_NO, BACKLOG_EA, SINGLECOST, COST_DG, REC_CREATOR, REC_CREATE_TIME,MODEL_ID) "
							" values (@compose_list_no, @date_c, @st_no, @backlog_ea, @singlecost, @cost_dg, @rec_creator, @rec_create_time,@model_id)"
							;
						Log::Trace("", __FUNCTION__, "sqlstr = [{0} ]", sqlstr);
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.Parameters.Set("compose_list_no", compose_list_no);
						cmd_inq.Parameters.Set("st_no", workRow["ST_NO"].ToString());
						cmd_inq.Parameters.Set("backlog_ea", workRow["BACKLOG_EA"].ToString());
						cmd_inq.Parameters.Set("date_c", workRow["DATE_C"].ToString());
						cmd_inq.Parameters.Set("singlecost", workRow["SINGLECOST"].ToDecimal());
						cmd_inq.Parameters.Set("cost_dg", workRow["COST_DG"].ToDecimal());
						cmd_inq.Parameters.Set("rec_creator", s.userid);
						cmd_inq.Parameters.Set("rec_create_time", datetime);
						cmd_inq.Parameters.Set("model_id", model_id);						
						cmd_inq.ExecuteNonQuery();
						cmd_inq.Close();
					}

					//更新配料单的
					for (int m = 0; m < outPLAN.Tables[0].Rows.get_Count(); m++)
					{
						if (workRow["PLAN_ID"].ToString() == outPLAN.Tables[0].Rows[m]["PLAN_ID"].ToString())
						{
							compose_list_no = outPLAN.Tables[0].Rows[m]["COMPOSE_LIST_NO"].ToString();
							break;
						}
					}
					sqlstr = " update tfbsm12 set compose_list_no = @compose_list_no "
						",PLAN_ID = @plan_id"
						",singlecost = @singlecost"
						",cost_dg = @cost_dg"
						",ERROR_REMARK = ' '"
						" where 1=1"
						" and st_no = @st_no"
						" and backlog_ea = @backlog_ea"
						" and date_c = @date_c"
						" and seq_no = @seq_no"
						;
					Log::Trace("", __FUNCTION__, "sqlstr = [{0} ]", sqlstr);
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("compose_list_no", compose_list_no);
					cmd_inq.Parameters.Set("seq_no", workRow["SEQ_NO"].ToString());
					cmd_inq.Parameters.Set("st_no", workRow["ST_NO"].ToString());
					cmd_inq.Parameters.Set("backlog_ea", workRow["BACKLOG_EA"].ToString());
					cmd_inq.Parameters.Set("date_c", workRow["DATE_C"].ToString());
					cmd_inq.Parameters.Set("plan_id", workRow["PLAN_ID"].ToString());
					cmd_inq.Parameters.Set("singlecost", workRow["SINGLECOST"].ToDecimal());
					cmd_inq.Parameters.Set("cost_dg", workRow["COST_DG"].ToDecimal());
					cmd_inq.ExecuteNonQuery();
					cmd_inq.Close();
				}
				else
				{
					sqlstr = " update tfbsm12 set COMPOSE_LIST_NO=' ',PLAN_ID=' ',singlecost = 0,cost_dg = 0"
						" where 1=1"
						" and st_no = @st_no"
						" and backlog_ea = @backlog_ea"
						" and date_c = @date_c"
						" and seq_no = @seq_no"
						;
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("seq_no", workRow["SEQ_NO"].ToString());
					cmd_inq.Parameters.Set("st_no", workRow["ST_NO"].ToString());
					cmd_inq.Parameters.Set("backlog_ea", workRow["BACKLOG_EA"].ToString());
					cmd_inq.Parameters.Set("date_c", workRow["DATE_C"].ToString());
					cmd_inq.ExecuteNonQuery();
					cmd_inq.Close();
				}
			}			

				Log::Trace("", __FUNCTION__, "插入还原硅信息开始");

				//插入还原硅信息
				for (int n = 0; n < outPLAN.Tables[0].Rows.get_Count(); n++)
				{
					tfbsm14.Reset();
					tfbsm14.MergeFrom(outPLAN.Tables[0].Rows[n]);
					compose_list_no = tfbsm14["COMPOSE_LIST_NO"].ToString();
					tfbsm14["REC_CREATOR"] = s.userid;
					tfbsm14["REC_CREATE_TIME"] = datetime;
					Log::Trace("", __FUNCTION__, "插入还原硅配料单号[{0}]", outPLAN.Tables[0].Rows[n]["COMPOSE_LIST_NO"].ToString());

					sqlstr = " select SI_PDI from tfbsm20 "
						" where COMM_FMLY_CODE='" + comm_fmly_code + "' and BACKLOG_EA='" + tfbsm14["BACKLOG_EA"].ToString() + "'  "
						;
					cmd_si_pdi.SetCommandText(sqlstr);
					cmd_si_pdi.ExecuteReader();
					f_si_pdi = 0;
					if (cmd_si_pdi.Read())
					{
						f_si_pdi = cmd_si_pdi.GetDecimal(1);
					}
					cmd_si_pdi.Close();
					if (f_si_pdi > 0)
					{
						sqlstr = "   SELECT C_VALUE, SI_VALUE, MN_VALUE, CR_VALUE, "
							"   STOCK_NAME, TI_VALUE, AL_VALUE, CA_VALUE "
							"   FROM TFBSM14 "
							"   WHERE MAT_CODE = 'AT000258' AND STATION_ID = 'A' "
							"   ORDER BY REC_CREATE_TIME DESC, DATE_C DESC "
							;
						Log::Trace("", __FUNCTION__, "sqlstr=[{0}]", sqlstr);
						cmd.SetCommandText(sqlstr);
						cmd.ExecuteReader();
						if (cmd.Read())
						{
							tfbsm14["C_VALUE"] = cmd.GetDecimal(1);
							tfbsm14["SI_VALUE"] = cmd.GetDecimal(2);
							tfbsm14["MN_VALUE"] = cmd.GetDecimal(3);
							tfbsm14["CR_VALUE"] = cmd.GetDecimal(4);
							tfbsm14["STOCK_NAME"] = cmd.GetString(5);
							tfbsm14["TI_VALUE"] = cmd.GetDecimal(6);
							tfbsm14["AL_VALUE"] = cmd.GetDecimal(7);
							tfbsm14["CA_VALUE"] = cmd.GetDecimal(8);
						}
						cmd.Close();

						tfbsm14["MATERIAL_CODE"] = material_code;
						tfbsm14["BACK_C2"] = "29";		  // 物料类型代码：还原硅铁
						tfbsm14["MAT_CODE"] = "AT000258"; // 硅铁
						tfbsm14["MAT_NAME"] = "硅铁 FeSi72Al2.0"; // 硅铁
						tfbsm14["STATION_ID"] = "A";
						tfbsm14["WEIGHT"] = f_si_pdi;
						tfbsm14.TrimOrBlank();
						tfbsm14.Insert();

						Log::Trace("", __FUNCTION__, "还原硅=[{0}]", tfbsm14["MAT_CODE"].ToString());
					}
					compose_list_no = tfbsm14["COMPOSE_LIST_NO"].ToString();

					//插入过程数据
					for (int i = 0; i < inDMST02.Tables["MatTable"].Rows.get_Count(); i++)
					{
						tfbsm18.Reset();
						tfbsm18.MergeFrom(inDMST02.Tables["MatTable"].Rows[i]);
						tfbsm18["PRICE"] = inDMST02.Tables["MatTable"].Rows[i]["COST"].ToString();
						CString key = "";
						if (tfbsm18["LOT_NO"].ToString().Trim() == "")
						{
							key = tfbsm18["STOCK_NAME"].ToString().Trim() + "|" + tfbsm18["MAT_CODE"].ToString().Trim();
						}
						else
						{
							key = tfbsm18["STOCK_NAME"].ToString().Trim() + "|" + tfbsm18["MAT_CODE"].ToString().Trim() + "|" +
								tfbsm18["LOT_NO"].ToString().Trim();
						}

						tfbsm18["STOCK_WT_QC"] = initMap[key];
						tfbsm18["STOCK_WT"] = initMap[key] - modelResultMap[key];

						tfbsm18["DATE_C"] = date_c;
						tfbsm18["BACKLOG_EA"] = backlog_ea;
						tfbsm18["REC_CREATOR"] = s.userid;
						tfbsm18["REC_CREATE_TIME"] = datetime;
						tfbsm18["COMPOSE_LIST_NO"] = compose_list_no;
						tfbsm18.Insert();
					}


					// 2. 收集模型计算结果
					CString planId = tfbsm14["PLAN_ID"].ToString();

					// 检查这个配料单是否成功且有炉数信息
					auto planIt = planFurnaceCountMap.find(planId);
					if (planIt == planFurnaceCountMap.end())
					{
						continue;
					}
					// 获取这个配料单对应的炉数
					CDecimal furnaceCount = planIt->second;
					// RESULT_TABLE中的WEIGHT是单炉重量，需要乘以炉数
					CDecimal weightPerFurnace = 0;
					CDecimal totalWeight = 0;

					for (int f = 0; f < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); f++)
					{						
						if (outDMST02.Tables["RESULT_TABLE"].Rows[f]["PLAN_ID"].ToString() == outPLAN.Tables[0].Rows[n]["PLAN_ID"].ToString())
						{
							//插入配料单表
							tfbsm14.Reset();
							tfbsm14.MergeFrom(outDMST02.Tables["RESULT_TABLE"].Rows[f]);
							tfbsm14["REC_CREATOR"] = s.userid;
							tfbsm14["REC_CREATE_TIME"] = datetime;

							tfbsm14["ST_NO"] = outPLAN.Tables[0].Rows[n]["ST_NO"].ToString();
							tfbsm14["BACKLOG_EA"] = outPLAN.Tables[0].Rows[n]["BACKLOG_EA"].ToString();
							tfbsm14["DATE_C"] = outPLAN.Tables[0].Rows[n]["DATE_C"].ToString();
							tfbsm14["COMPOSE_LIST_NO"] = outPLAN.Tables[0].Rows[n]["COMPOSE_LIST_NO"].ToString();
							compose_list_no = tfbsm14["COMPOSE_LIST_NO"].ToString();

							CString key = "";
							if (tfbsm14["LOT_NO"].ToString().Trim() == "")
							{
								key = tfbsm14["STOCK_NAME"].ToString().Trim() + "|" + tfbsm14["MAT_CODE"].ToString().Trim();
							}
							else
							{
								key = tfbsm14["STOCK_NAME"].ToString().Trim() + "|" + tfbsm14["MAT_CODE"].ToString().Trim() + "|" +
									tfbsm14["LOT_NO"].ToString().Trim();
							}
							// 累加到模型结果映射中
							weightPerFurnace = tfbsm14["WEIGHT"].ToDecimal();
							totalWeight = weightPerFurnace * furnaceCount;
							if (modelResultMap.find(key) != modelResultMap.end())
							{
								modelResultMap[key] = modelResultMap[key] + totalWeight;  // 累加相同键的总重量								
							}
							else
							{
								modelResultMap[key] = totalWeight;
							}


							if (tfbsm14["MAT_CODE"].ToString() == "PL0002"  && tfbsm14["MAT_CODE"].ToString() == "A")
							{
								//取内控的值
								sqlstr = " select MAX(CASE WHEN ELM_NAME = 'C' THEN MAIN_AIM ELSE 0 END) C_AIM "
									" ,MAX(CASE WHEN ELM_NAME = 'S' THEN MAIN_AIM ELSE 0 END) S_AIM"
									"  FROM TQMTS02 "
									" WHERE IDX_NO = (SELECT ELM_STD_IDX_A FROM TQMTS0X WHERE ST_NO = @st_no)"
									;
								cmd.SetCommandText(sqlstr);
								cmd.Parameters.Set("st_no", tfbsm14["ST_NO"].ToString());
								cmd.ExecuteReader();
								if (cmd.Read())
								{
									tfbsm14["C_VALUE"] = cmd.GetDecimal(1);
									tfbsm14["S_VALUE"] = cmd.GetDecimal(2);
								}
								cmd.Close();
							}
							tfbsm14.TrimOrBlank();
							tfbsm14.Insert();
						}
					}

					//Log::Trace("", __FUNCTION__, "tfbsm19开始");
					for (size_t i = 0; i < inDMST02.Tables["WORKTABLE"].Rows.get_Count(); i++){
						tfbsm19.Reset();
						tfbsm19.MergeFrom(inDMST02.Tables["WORKTABLE"].Rows[i]);
						tfbsm19["REC_CREATOR"] = s.userid;
						tfbsm19["REC_CREATE_TIME"] = datetime;
						tfbsm19["DATE_C"] = date_c;
						tfbsm19["COMPOSE_LIST_NO"] = compose_list_no;
						tfbsm19.Insert();
					}
					//Log::Trace("", __FUNCTION__, "tfbsm26开始");
					for (size_t i = 0; i < inDMST02.Tables["GX_MAT"].Rows.get_Count(); i++){
						tfbsm26.Reset();
						tfbsm26.MergeFrom(inDMST02.Tables["GX_MAT"].Rows[i]);
						tfbsm26["REC_CREATOR"] = s.userid;
						tfbsm26["REC_CREATE_TIME"] = datetime;
						tfbsm26["DATE_C"] = date_c;
						tfbsm26["COMPOSE_LIST_NO"] = compose_list_no;
						tfbsm26.Insert();
					}
					//Log::Trace("", __FUNCTION__, "tfbsm27开始");
					for (size_t i = 0; i < inDMST02.Tables["GX_WEIGHT"].Rows.get_Count(); i++){
						tfbsm27.Reset();
						tfbsm27.MergeFrom(inDMST02.Tables["GX_WEIGHT"].Rows[i]);
						tfbsm27["REC_CREATOR"] = s.userid;
						tfbsm27["REC_CREATE_TIME"] = datetime;
						tfbsm27["DATE_C"] = date_c;
						tfbsm27["COMPOSE_LIST_NO"] = compose_list_no;
						tfbsm27.Insert();
					}
					//Log::Trace("", __FUNCTION__, "tfbsm28开始");
					for (size_t i = 0; i < inDMST02.Tables["GX_LC"].Rows.get_Count(); i++){
						tfbsm28.Reset();
						tfbsm28.MergeFrom(inDMST02.Tables["GX_LC"].Rows[i]);
						tfbsm28["REC_CREATOR"] = s.userid;
						tfbsm28["REC_CREATE_TIME"] = datetime;
						tfbsm28["DATE_C"] = date_c;
						tfbsm28["COMPOSE_LIST_NO"] = compose_list_no;
						tfbsm28["BACKLOG_EA"] = backlog_ea;
						tfbsm28.Insert();
					}
				}
			}

			Log::Trace("", __FUNCTION__, "更新错误信息开始");
			//更新错误信息
			if (outDMST02.Tables.Contains("ERROR_INFO"))
			{
				for (int z = 0; z < outDMST02.Tables["ERROR_INFO"].Rows.get_Count(); z++)
				{
					sqlstr = " update tfbsm12 set ERROR_REMARK=@col_remark"
						" where 1=1"
						" and seq_no = @seq_no"
						" and st_no = @st_no"
						" and backlog_ea = @backlog_ea"
						" and date_c = @date_c"
						;
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.Parameters.Set("st_no", outDMST02.Tables["ERROR_INFO"].Rows[z]["ST_NO"].ToString());
					cmd_inq.Parameters.Set("backlog_ea", outDMST02.Tables["ERROR_INFO"].Rows[z]["BACKLOG_EA"].ToString());
					cmd_inq.Parameters.Set("date_c", outDMST02.Tables["ERROR_INFO"].Rows[z]["DATE_C"].ToString());
					cmd_inq.Parameters.Set("seq_no", outDMST02.Tables["ERROR_INFO"].Rows[z]["SEQ_NO"].ToString());
					cmd_inq.Parameters.Set("col_remark", outDMST02.Tables["ERROR_INFO"].Rows[z]["ERROR_MSG"].ToString());
					cmd_inq.ExecuteNonQuery();
					cmd_inq.Close();
				}

			}
			}
			cmd_plan.Close();

			//更新改核算的配料单的信息
			sqlstr = "update tfbsm14a t1 set  ERROR_REMARK = (select ERROR_REMARK from tfbsm12 t2 where t1.COMPOSE_LIST_NO = t2.COMPOSE_LIST_NO and t2.model_id=@model_id and rownum=1)"
				" where 1=1"
				" and exists (select 1 from tfbsm12 t2 where t1.COMPOSE_LIST_NO = t2.COMPOSE_LIST_NO and t2.model_id=@model_id)"
				" and model_id = @model_id"
				;
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("model_id", model_id);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();


		bcls_ret->Tables[0].Columns.Add(DT_STRING, "COMPOSE_LIST_NO");
		bcls_ret->Tables[0].Rows.Add();
		bcls_ret->Tables[0].Rows[0]["COMPOSE_LIST_NO"] = compose_list_no;

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
