/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/11/24
Description: 模型不锈钢钢种配料
**************************************************/
#include "stdafx.h"
#include <map>
BM2F_ENTERACE(fbsm15_mx_ins)
/* ***** 静态函数申明 ***** */
// 模型计算
void f_epex_call_rest_svc(CDbConnection *conn, const CString &system_code, const CString &svc_name, EIClass *blks_in, EIClass *blks_out);
int f_mmsm_getprice(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn); // 价格计算

int f_fbsm15_mx_ins(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	int ret = 0;
	int row_l = 0;
	CString sqlstr = " ";
	CString st_no = " ";		   // 出钢记号
	CString backlog_ea = " ";	   // 工艺路线
	CString date = " ";			   // 日期
	CString material_code = " ";   // 中类代码
	CString comm_fmly_code = " ";  // 大类代码
	CString f_material_code = " "; // 模型返回的中类代码
	CString f_st_no = " ";		   // 模型返回的中类代码
	CString f_date = " ";		   // 返回日期
	CString seq_no = " ";		   // 序号
	CDecimal f_count = 0;		   // 总炉数
	CDecimal fu_count = 0;
	CString s_matear_code = " ";
	CString compose_list_no = " "; // 配料单号
	CDecimal f_si_pdi = 0;
	CString plan_id = "";
	CString plan_id_1 = "";
	CString f_backlog_ea = " "; // 返回工艺路线
	CString f_back_c2 = " ";
	CDecimal p_seq = 0;
	CString sql_k = "";
	CString f_comm_fmly_code = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd(conn);
	CDbCommand cmd_k(conn);
	CDbCommand cmd_si_pdi(conn);
	CModel tfbsm18("TFBSM18");
	CModel tfbsm14("TFBSM14");
	CModel tfbsm04("TFBSM04");
	CModel tfbsm09("TFBSM09");
	CModel tfbsm01("TFBSM01");
	CModel tfbsm13("TFBSM13");
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	EIClass Data_KC; // 库存信息
	EIClass out_KC;	 // 使用信息
	// 模型定义
	EIClass inDMST02;	 // 传入类
	EIClass outDMST02;	 // 返回类
	EIClass outDMST02_1; // 返回类

	map<CString, CDecimal> modelResultMap;
	map<CString, CDecimal> inventoryIndexMap; // 物料代码+批次号+库区，实时库存-模型使用量
	try
	{
		// 库存信息
		Data_KC.Tables.Add("Data_WT");
		Data_KC.Tables["Data_WT"].Columns.Add(DT_STRING, "LOT_NO");		 // 批次号
		Data_KC.Tables["Data_WT"].Columns.Add(DT_STRING, "STOCK_NAME");	 // 库区
		Data_KC.Tables["Data_WT"].Columns.Add(DT_STRING, "MAT_CODE");	 // 物料代码
		Data_KC.Tables["Data_WT"].Columns.Add(DT_STRING, "STOCK_WT_QC"); // 重量

		inDMST02.Tables[0].set_TableName("DLLINFO");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "DLLName");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "ClassName");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "MethodName");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "CHECK_FLAG"); // CHECK_FLAG

		CDataRow &row = inDMST02.Tables["DLLINFO"].Rows.Add();

		// 这三个值待定
		row["DLLName"] = "TGPLModel.dll";
		row["ClassName"] = "TGPLModel.TGPLModel";
		row["MethodName"] = "CalForPLPlan";

		if (bcls_rec->Tables["Main"].Rows.get_Count() >= 20)
		{
			row["CHECK_FLAG"] = "0";
		}
		else
		{
			row["CHECK_FLAG"] = "1";
		}
		// 配料单信息
		inDMST02.Tables.Add("LG_PSSM");
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "BACKLOG_EA");	// 工艺路径
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "ST_NO");			// 出钢记号
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "MATERIAL_CODE"); // 中类代码
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "DATE_C");		// 日期
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "FURNACE_COUNT"); // 总炉数
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "SEQ_NO");		// 序号
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "SUM_FURNACES");	// 总炉数
		/*inDMST02.Tables["LG_PSSM"].Rows.Add();*/
		// MatTable(原料库存及成分)
		inDMST02.Tables.Add("MatTable");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "LOT_NO");	  // 批次号
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "ST_NO");	  // 出钢记号
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "STOCK_NAME"); // 库区
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MAT_CODE");	  // 物料代码
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MAT_NAME");	  // 物料名称
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "BACK_C2");	  // 物料类型
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "STOCK_WT");	  // 重量
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "C_VALUE");	  // C%
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "SI_VALUE");	  // Si%
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MN_VALUE");	  // Mn
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "P_VALUE");	  // P
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "S_VALUE");	  // S
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "CR_VALUE");	  // Cr
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "NI_VALUE");	  // Ni
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MO_VALUE");	  // Mo
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "CU_VALUE");	  // Cu
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "CO_VALUE");	  // CO
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "TI_VALUE");	  // TI
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "NB_VALUE");	  // NB
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "AL_VALUE");	  // AL
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "COST");		  // 价格
		// WORKTABLE（工序基准成分）
		inDMST02.Tables.Add("WORKTABLE");
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "COMM_FMLY_CODE"); // 产品大类代码
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "MATERIAL_CODE");  // 产品中类代码
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "BACKLOG_EA");	   // 工艺路径
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "ST_NO");		   // 出钢记号
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "STATION_ID");	   // 工序
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "ELM_NAME");	   // 元素
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "SPE_MIN");		   // 最小值
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "SPE_MAX");		   // 最大值
		inDMST02.Tables["WORKTABLE"].Columns.Add(DT_STRING, "YIELD");		   // 收得率
		////GX_RATE(收得率)
		// inDMST02.Tables.Add("GX_RATE");
		// inDMST02.Tables["GX_RATE"].Columns.Add(DT_STRING, "COMM_FMLY_CODE"); //产品大类代码
		// inDMST02.Tables["GX_RATE"].Columns.Add(DT_STRING, "MATERIAL_CODE");//产品中类
		// inDMST02.Tables["GX_RATE"].Columns.Add(DT_STRING, "ST_NO"); //出钢记号
		// inDMST02.Tables["GX_RATE"].Columns.Add(DT_STRING, "STATION_ID"); //工序
		// inDMST02.Tables["GX_RATE"].Columns.Add(DT_STRING, "BACKLOG_EA"); //工艺路径
		// inDMST02.Tables["GX_RATE"].Columns.Add(DT_STRING, "YIELD_CR"); //CR收得率
		// inDMST02.Tables["GX_RATE"].Columns.Add(DT_STRING, "YIELD_MO"); //MO收得率
		// inDMST02.Tables["GX_RATE"].Columns.Add(DT_STRING, "YIELD_NI"); //NI收得率
		// inDMST02.Tables["GX_RATE"].Columns.Add(DT_STRING, "METAL_CONS_PERT"); //金属料消耗
		////GX_PARA（工序投料比）
		// inDMST02.Tables.Add("GX_PARA");
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "COMM_FMLY_CODE"); //产品大类代码
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "MATERIAL_CODE");//产品中类
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "ST_NO"); //出钢记号
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "BACKLOG_EA"); //工艺路径
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "IRON_RATIO1"); //三脱站铁水(%)
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "IRON_RATIO2"); //脱磷铁水(%)
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "DEVO_RATIO_IF"); //IF投料(%)
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "DEVO_RATIO_EAF"); //EAF投料(%)
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "DEVO_RATIO_AOD"); //AOD投料(%)
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "SCRAP_RATIO_IF"); //IF废钢比(%)
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "SCRAP_RATIO_EAF"); //EAF废钢比(%)
		// inDMST02.Tables["GX_PARA"].Columns.Add(DT_STRING, "SCRAP_RATIO_AOD"); //AOD废钢比(%)
		// GX_MAT（原料总表）
		inDMST02.Tables.Add("GX_MAT");
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "COMM_FMLY_CODE");	   // 产品大类代码
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "MATERIAL_CODE");	   // 产品中类代码
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "ST_NO");			   // 出钢记号
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "YEILD_MINUS");	   // 收得率
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "STATION_ID");		   // 工序
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "MAT_CODE");		   // 物料代码
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "MAT_NAME");		   // 物料名称
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "UPPER_LIMIT_VALUE"); // 投料上限
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "LOWER_LIMIT_VALUE"); // 投料下限
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "MUST_DO_FLAG");	   // MUST_DO_FLAG 必做标记
		// ROUTE（工艺路线）
		// inDMST02.Tables.Add("ROUTE");
		// inDMST02.Tables["ROUTE"].Columns.Add(DT_STRING, "CODE"); //代码
		// inDMST02.Tables["ROUTE"].Columns.Add(DT_STRING, "BACKLOG_EA"); //工艺路线
		// inDMST02.Tables["ROUTE"].Columns.Add(DT_STRING, "DESCRIP"); //工艺路线描述
		// GX_WEIGHT(可投重量)
		inDMST02.Tables.Add("GX_WEIGHT");
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "ST_NO");			// 出钢记号
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "STATION_ID");		// 工序
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "BACKLOG_EA");		// 工艺路线
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "METAL_CONS_PERT"); // 金属料消耗
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "SCRAP_RATIO_MAX"); // 废钢最大
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "SCRAP_RATIO_MIN"); // 废钢最小
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "TAPPING_WT");		// 重量
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "TAPPING_WT_MIN");	// 最小重量
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "MAT_COUNT");		// 最多可投物料数量
		inDMST02.Tables["GX_WEIGHT"].Columns.Add(DT_STRING, "COMBY_GX");		// 路线
		// 价格计算
		EIClass bcls_rec_rep;
		EIClass bcls_ret_rep;
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_CODE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_NAME");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "CR_VALUE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "NI_VALUE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MO_VALUE");
		bcls_rec_rep.Tables[0].Rows.Add();
		// 获取需要计算的模型
		for (size_t j = 0; j < bcls_rec->Tables["Main"].Rows.get_Count(); j++)
		{
			seq_no = bcls_rec->Tables["Main"].Rows[j]["SEQ_NO"].ToString().TrimOrBlank();
			Log::Trace("", __FUNCTION__, "seq_no = [{0}]", seq_no);
			Log::Trace("", __FUNCTION__, "count = [{0}]", bcls_rec->Tables["Main"].Rows.get_Count());
			st_no = bcls_rec->Tables["Main"].Rows[j]["ST_NO"].ToString().TrimOrBlank();
			backlog_ea = bcls_rec->Tables["Main"].Rows[j]["BACKLOG_EA"].ToString().TrimOrBlank();
			date = bcls_rec->Tables["Main"].Rows[j]["DATE_C"].ToString().TrimOrBlank();
			Log::Trace("", __FUNCTION__, "backlog_ea = [{0}]", backlog_ea);
			Log::Trace("", __FUNCTION__, "j = [{0}]");
			f_count = bcls_rec->Tables["Main"].Rows[j]["FURNACE_COUNT"].ToDecimal();
			// MATERIAL_CODE
			s_matear_code = bcls_rec->Tables["Main"].Rows[j]["MATERIAL_CODE"].ToString();
			// 配料单信息 LG_PSSM
			// CDataRow& row = inDMST02.Tables["LG_PSSM"].Rows.Add();
			CDataRow &row_lp = inDMST02.Tables["LG_PSSM"].Rows.Add();
			row_lp["ST_NO"] = st_no;
			row_lp["BACKLOG_EA"] = backlog_ea;
			row_lp["DATE_C"] = date;
			row_lp["SEQ_NO"] = seq_no;
			Log::Trace("", __FUNCTION__, "row_lp.SEQ_NO = [{0}]", row_lp["SEQ_NO"].ToString());
			if (j != 0)
			{
				if (st_no == bcls_rec->Tables["Main"].Rows[j - 1]["ST_NO"].ToString().TrimOrBlank())
				{
					fu_count = fu_count + 1;
					Log::Trace("", __FUNCTION__, "fu_count = [{0}]", fu_count.ToString());
				}
				else
				{
					fu_count = 1;
					Log::Trace("", __FUNCTION__, "fu_count = [{0}]", fu_count.ToString());
				}
			}
			else
			{
				fu_count = 1;
				Log::Trace("", __FUNCTION__, "fu_count = [{0}]", fu_count.ToString());
			}
			row_lp["FURNACE_COUNT"] = fu_count;
			Log::Trace("", __FUNCTION__, "ST_NO = [{0}]", row_lp["ST_NO"].ToString());
			Log::Trace("", __FUNCTION__, "SEQ_NO = [{0}]", row_lp["SEQ_NO"].ToString());
			Log::Trace("", __FUNCTION__, "FURNACE_COUNT = [{0}]", inDMST02.Tables["LG_PSSM"].Rows[0]["FURNACE_COUNT"].ToString());
			row_l = inDMST02.Tables["LG_PSSM"].Rows.get_Count();
			Log::Trace("", __FUNCTION__, "row_lp.COUNT = [{0}]", row_l);
			sqlstr = " select COMM_FMLY_CODE,MATERIAL_CODE from TFBSM11 where ST_NO='" + st_no + "' ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				comm_fmly_code = cmd_inq.GetString(1);
				material_code = cmd_inq.GetString(2);
			}
			cmd_inq.Close();
			row_lp["MATERIAL_CODE"] = material_code;
			row_lp["SUM_FURNACES"] = bcls_rec->Tables["Main"].Rows[j]["FURNACE_COUNT"].ToDecimal();
			/*row_lp["FURNACE_COUNT"] = fu_count;*/
			if (seq_no == "1" || j == 0)
			{
				// LG_PSSM
				// 代表是一个新的工艺路线和出钢记号
				// 根据出钢记号去查产品对应关系
				// 04表校验
				sqlstr = " SELECT COUNT(1)  "
						 " FROM (select * from tfbsm04 where  BACKLOG_EA='" +
						 backlog_ea + "') a "
									  " WHERE(A.ST_NO = '" +
						 st_no + "' OR(A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "') OR "
																										" (A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "') OR "
										  " (A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" +
						 material_code + "') OR "
										 " (A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "')) ";
				cmd_inq.SetCommandText(sqlstr);
				Log::Trace("", __FUNCTION__, "TFBSM04.sqlstr = [{0}]", sqlstr);
				CDecimal fbsm04_count = cmd_inq.ExecuteScalar();
				Log::Trace("", __FUNCTION__, "fbsm04_count.Rows = [{0}]", fbsm04_count.ToString());
				if (fbsm04_count == 0)
				{
					strcpy(s.msg, "出钢记号:" + st_no + ",工艺路径:" + backlog_ea + "收得率没有数据!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				cmd_inq.Close();
				// 09表校验
				sqlstr = " SELECT COUNT(1)  "
						 " FROM TFBSM09 a "
						 " WHERE(A.ST_NO = '" +
						 st_no + "' OR(A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "') OR "
																										" (A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "') OR "
										  " (A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" +
						 material_code + "') OR "
										 " (A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "')) "
										  " and  A.BACKLOG_EA = '" +
						 backlog_ea + "'  ";
				cmd_inq.SetCommandText(sqlstr);
				CDecimal fbsm09_count = cmd_inq.ExecuteScalar();
				Log::Trace("", __FUNCTION__, "fbsm04_count.Rows = [{0}]", fbsm09_count.ToString());
				if (fbsm09_count == 0 && st_no.SubstringNE(1, 1) != "F" && st_no.SubstringNE(1, 1) != "M")
				{
					strcpy(s.msg, "出钢记号:" + st_no + ",工艺路径:" + backlog_ea + "工序投料比没有维护!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				cmd_inq.Close();
				Log::Trace("", __FUNCTION__, "TFBSM09.sqlstr = [{0}]", sqlstr);
				// MatTable (原料库存及成分)
				sqlstr = " SELECT "
						 " LOT_NO,'" +
						 st_no + "' as st_no,MAT_CODE,MAT_NAME,STOCK_NAME,case when STOCK_NAME!='5' then WEIGHT/1000 else WEIGHT END AS STOCK_WT_QC,BACK_C2, "
								 " NVL(C ,0)           C_VALUE,NVL(Si  ,0)         SI_VALUE,NVL(Mn,0)           MN_VALUE,NVL(P  ,0)          P_VALUE,NVL(S ,0)           S_VALUE,NVL(Cr ,0)          CR_VALUE,NVL(Ni,0)           NI_VALUE, "
								 " NVL(Mo,0)           MO_VALUE,NVL(Cu,0)           Cu_VALUE,NVL(co,0)           CO_VALUE,NVL(TI,0)           TI_VALUE,NVL(nb,0)           NB_VALUE,NVL(al,0)           AL_VALUE "
								 " FROM ( "
								 " SELECT A.*,b.BACK_C2,b.MAT_NAME "
								 " FROM FBSM_13 A "
								 " LEFT JOIN ( "
								 " SELECT t.MAT_CODE,t.MAT_NAME,t.MATERIAL_CODE,t.ST_NO,t.BACK_C2 "
								 " FROM ( "
								 " SELECT "
								 " t.*, "
								 " ROW_NUMBER() OVER ( "
								 " PARTITION BY t.MAT_CODE "
								 " ORDER BY CASE "
								 " WHEN t.ST_NO = '" +
						 st_no + "' THEN 1 WHEN t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "' THEN 2 "
																												 " WHEN t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "' THEN 3 WHEN t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "' THEN 4 "
																														  " WHEN t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "' THEN 5 ELSE 99   END "
										  " ) as rn "
										  " FROM TFBSM01 t "
										  " WHERE "
										  " t.ST_NO = '" +
						 st_no + "' OR (t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "') "
																										 " OR (t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "') OR (t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "') "
																												   " OR (t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "') "
										  " ) t "
										  " WHERE rn = 1 "
										  " GROUP BY t.MAT_CODE, t.MAT_NAME, t.MATERIAL_CODE, t.ST_NO, t.BACK_C2   "
										  " ) B ON A.MAT_CODE = B.MAT_CODE "
										  " ) WHERE MAT_NAME!=' '  AND BACK_C2!='2' ";
				// sqlstr = " SELECT  '" + st_no + "' as st_no,A.LOT_NO,A.MAT_CODE,A.STOCK_NAME,A.MAT_NAME,A.BACK_C2,A.C_VALUE,A.SI_VALUE,A.MN_VALUE,A.P_VALUE,A.S_VALUE,A.CR_VALUE,A.NI_VALUE,A.MO_VALUE,STOCK_WT_QC FROM TFBSMCS a WHERE a.MAT_NAME!=' '  ";
				Log::Trace("", __FUNCTION__, "inDMST02.MatTable.sqlstr = [{0}]", sqlstr);
				cmd_inq.SetCommandText(sqlstr);
				// Data_KC
				cmd_inq.ExecuteReader();

				// 1. 创建库存数据的查找表（用于快速查找）
				/*for (size_t p = 0; p < Data_KC.Tables["Data_WT"].Rows.get_Count(); p++)
				{
				CString key = Data_KC.Tables["Data_WT"].Rows[p]["LOT_NO"].ToString().Trim() + "|" +
				Data_KC.Tables["Data_WT"].Rows[p]["MAT_CODE"].ToString() + "|" +
				Data_KC.Tables["Data_WT"].Rows[p]["STOCK_NAME"].ToString();
				inventoryIndexMap[key] = p;
				}*/
				inDMST02.Tables["MatTable"].Rows.Clear();
				while (cmd_inq.Read())
				{
					tfbsm18.Reset(); // 清空成默认值
					cmd_inq.Fetch(tfbsm18);

					// if (j == 0){
					//	//第一次循环不用管，直接存数据
					//	CDataRow& row_wt = Data_KC.Tables["Data_WT"].Rows.Add();
					//	row_wt["LOT_NO"] = tfbsm18["LOT_NO"].ToString().Trim();
					//	row_wt["MAT_CODE"] = tfbsm18["MAT_CODE"].ToString().Trim();
					//	row_wt["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
					// }
					// else
					//{
					//	Log::Trace("", __FUNCTION__, "Data_WT/linke = [{0}]", Data_KC.Tables["Data_WT"].Rows.get_Count());
					//	for (size_t p = 0; p < Data_KC.Tables["Data_WT"].Rows.get_Count(); p++)
					//	{//遍历库存表去修改重量
					//		if (tfbsm18["LOT_NO"].ToString().Trim() == Data_KC.Tables["Data_WT"].Rows[p]["LOT_NO"].ToString().Trim() && tfbsm18["MAT_CODE"].ToString() == Data_KC.Tables["Data_WT"].Rows[p]["MAT_CODE"].ToString() && tfbsm18["STOCK_NAME"].ToString() == Data_KC.Tables["Data_WT"].Rows[p]["STOCK_NAME"].ToString()){
					//			//代表是一个物料代码和批次号以及库区
					//			//遍历上次模型返回的数据
					//			Log::Trace("", __FUNCTION__, "RESULT_TABLE/linke = [{0}]", outDMST02.Tables["RESULT_TABLE"].Rows.get_Count());
					//			for (size_t f = 0; f < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); f++){
					//				//outDMST02.Tables["RESULT_TABLE"].Rows[i]
					//				Log::Trace("", __FUNCTION__, "linke = [{0}]", __LINE__);
					//				if (Data_KC.Tables["Data_WT"].Rows[p]["LOT_NO"].ToString().Trim() == outDMST02.Tables["RESULT_TABLE"].Rows[f]["LOT_NO"].ToString().Trim() && Data_KC.Tables["Data_WT"].Rows[p]["MAT_CODE"].ToString() == outDMST02.Tables["RESULT_TABLE"].Rows[f]["MAT_CODE"].ToString() && Data_KC.Tables["Data_WT"].Rows[p]["STOCK_NAME"].ToString() == outDMST02.Tables["RESULT_TABLE"].Rows[f]["STOCK_NAME"].ToString()){
					//					//去判断是否在上一炉使用过
					//					//统计上一炉＋之前使用的情况
					//					Log::Trace("", __FUNCTION__, "linke = [{0}]", __LINE__);
					//					Data_KC.Tables["Data_WT"].Rows[p]["STOCK_WT_QC"] = Data_KC.Tables["Data_WT"].Rows[p]["STOCK_WT_QC"].ToDecimal() + outDMST02.Tables["RESULT_TABLE"].Rows[f]["WEIGHT"].ToDecimal();
					//					//总重量=总重量-之前使用重量
					//					tfbsm18["STOCK_WT_QC"] = tfbsm18["STOCK_WT_QC"].ToDecimal() - Data_KC.Tables["Data_WT"].Rows[p]["STOCK_WT_QC"].ToDecimal();
					//				}
					//			}
					//		}
					//		else
					//		{
					//			Log::Trace("", __FUNCTION__, "linke = [{0}]", __LINE__);
					//			//是一个新的库存信息，需要存储下来
					//			CDataRow& row_wt = Data_KC.Tables["Data_WT"].Rows.Add();
					//			row_wt["LOT_NO"] = tfbsm18["LOT_NO"].ToString().Trim();
					//			row_wt["MAT_CODE"] = tfbsm18["MAT_CODE"].ToString().Trim();
					//			row_wt["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
					//			//row_wt["STOCK_WT_QC"] = tfbsm18["STOCK_WT_QC"].ToString().Trim();
					//		}
					//	}
					// }

					if (j == 0)
					{
						CString key = "";
						if (tfbsm18["LOT_NO"].ToString().Trim() == "")
						{
							key = tfbsm18["STOCK_NAME"].ToString().Trim() + "|" + tfbsm18["MAT_CODE"].ToString().Trim();
						}
						else
						{
							key = tfbsm18["STOCK_NAME"].ToString().Trim() + "|" + tfbsm18["MAT_CODE"].ToString().Trim() + "|" + tfbsm18["LOT_NO"].ToString().Trim();
						}
						// CString key = tfbsm18["STOCK_NAME"].ToString().Trim() + "|" + tfbsm18["MAT_CODE"].ToString().Trim() + "|" + tfbsm18["LOT_NO"].ToString().Trim();
						CDecimal stock_value = tfbsm18["STOCK_WT_QC"].ToDecimal();
						auto row_c = inventoryIndexMap.find(key);
						if (row_c == inventoryIndexMap.end())
						{
							inventoryIndexMap[key] = stock_value;
						}
						// else
						//{
						//	//inventoryIndexMap[key] = inventoryIndexMap[key] + stock_value;
						//	inventoryIndexMap[key] = stock_value;
						// }
					}
					else
					{
						CString key = "";
						if (tfbsm18["LOT_NO"].ToString().Trim() == "")
						{
							key = tfbsm18["STOCK_NAME"].ToString().Trim() + "|" + tfbsm18["MAT_CODE"].ToString().Trim();
						}
						else
						{
							key = tfbsm18["STOCK_NAME"].ToString().Trim() + "|" + tfbsm18["MAT_CODE"].ToString().Trim() + "|" + tfbsm18["LOT_NO"].ToString().Trim();
						}

						CDecimal stock_value = tfbsm18["STOCK_WT_QC"].ToDecimal();
						auto row_c = inventoryIndexMap.find(key);
						if (row_c == inventoryIndexMap.end())
						{
							inventoryIndexMap[key] = stock_value;
						}
						// else
						//{
						//	//inventoryIndexMap[key] = inventoryIndexMap[key] + stock_value;
						//	inventoryIndexMap[key] = stock_value;
						// }

						// 1. 创建模型结果的查找表
						for (int f = 0; f < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); f++)
						{
							CString key = " ";
							CDataRow &row_t = outDMST02.Tables["RESULT_TABLE"].Rows[f];

							// if (row_t["STOCK_NAME"].ToString().Trim() != "1" || row_t["STOCK_NAME"].ToString().Trim() != "2" || row_t["STOCK_NAME"].ToString().Trim() != "3" || row_t["STOCK_NAME"].ToString().Trim() != "4" || row_t["STOCK_NAME"].ToString().Trim() != "5"){
							//	//获取模型返回得镍生铁得库
							//	if (row_t["STOCK_NAME"].ToString().Trim() == "5"){
							//		row_t["STOCK_NAME"] = "5";
							//	}
							//	else{
							//		row_t["STOCK_NAME"] = "1";
							//	}
							// }

							if (row_t["LOT_NO"].ToString().Trim() == "")
							{
								key = row_t["STOCK_NAME"].ToString().Trim() + "|" + row_t["MAT_CODE"].ToString().Trim();
							}
							else
							{
								key = row_t["STOCK_NAME"].ToString().Trim() + "|" + row_t["MAT_CODE"].ToString().Trim() + "|" +
									  row_t["LOT_NO"].ToString().Trim();
							}
							CDecimal weight = row_t["WEIGHT"].ToDecimal();
							if (modelResultMap.find(key) != modelResultMap.end())
							{
								modelResultMap[key] = modelResultMap[key] + weight; // 累加相同键的重量
							}
							else
							{
								modelResultMap[key] = weight;
							}
							Log::Trace("", __FUNCTION__, "for使用量 = [{0}]，key=[{1}]", modelResultMap[key], key);
						}
						// 2. 处理当前数据
						CString currentKey = "";
						if (tfbsm18["LOT_NO"].ToString().Trim() == "")
						{
							currentKey = tfbsm18["STOCK_NAME"].ToString().Trim() + "|" + tfbsm18["MAT_CODE"].ToString().Trim();
						}
						else
						{
							currentKey = tfbsm18["STOCK_NAME"].ToString().Trim() + "|" + tfbsm18["MAT_CODE"].ToString().Trim() + "|" +
										 tfbsm18["LOT_NO"].ToString().Trim();
						}

						// 查找库存中是否存在
						auto invIt = inventoryIndexMap.find(currentKey);
						if (invIt != inventoryIndexMap.end())
						{
							auto p = invIt->second;

							// 从模型结果中查找对应的重量
							auto modelIt = modelResultMap.find(currentKey);
							if (modelIt != modelResultMap.end())
							{
								CDecimal usedWeight = modelIt->second;
								Log::Trace("", __FUNCTION__, "模型使用量 = [{0}]，currentKey=[{1}]", usedWeight, currentKey);
								// 更新库存的已使用重量
								CDecimal currentUsed = inventoryIndexMap[currentKey];
								Log::Trace("", __FUNCTION__, "当前实时库存量 = [{0}]，currentKey=[{1}]", currentUsed, currentKey);

								// 更新总重量
								CDecimal totalWeight = currentUsed; // tfbsm18["STOCK_WT_QC"].ToDecimal();
								tfbsm18["STOCK_WT_QC"] = totalWeight - usedWeight;
								Log::Trace("", __FUNCTION__, "STOCK_WT_QCline = [{0}]", tfbsm18["STOCK_WT_QC"].ToString());
								inventoryIndexMap[currentKey] = totalWeight - usedWeight;
								if (totalWeight - usedWeight < 0)
								{ // 如果库存计算出小于等于0，跳过
									continue;
								}
								Log::Trace("", __FUNCTION__, "Updated existing inventory, line = [{0}]", __LINE__);
							}
						}
					}
					// tfbsm18.Print();
					// 价格计算
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
					CDataRow &row_MatTable = inDMST02.Tables["MatTable"].Rows.Add();
					row_MatTable["LOT_NO"] = tfbsm18["LOT_NO"].ToString().Trim();
					Log::Trace("", __FUNCTION__, "MatTable.LOT_NO.sqlstr = [{0}]", row_MatTable["LOT_NO"].ToString());
					row_MatTable["ST_NO"] = tfbsm18["ST_NO"].ToString().Trim();
					row_MatTable["MAT_CODE"] = tfbsm18["MAT_CODE"].ToString().Trim();
					row_MatTable["MAT_NAME"] = tfbsm18["MAT_NAME"].ToString().Trim();
					// 去查找库
					if (tfbsm18["LOT_NO"].ToString().Trim() != "" && tfbsm18["STOCK_NAME"].ToString() != "5")
					{
						sql_k = " SELECT MAT_NAME FROM TMMSM60 WHERE BUNKER_NO in (SELECT BUNKER_NO   FROM TMMSM85 WHERE  MAT_CODE='" + tfbsm18["MAT_CODE"].ToString().Trim() + "' AND LOT_NO='" + tfbsm18["LOT_NO"].ToString().Trim() + "' and mat_name like '%镍生铁%' ) ";
						cmd_k.SetCommandText(sql_k);
						cmd_k.ExecuteReader();
						if (cmd_k.Read())
						{
							if (cmd_k.GetString(1) != " " && cmd_k.GetString(1) != "")
							{
								tfbsm18["STOCK_NAME"] = cmd_k.GetString(1);
								row_MatTable["STOCK_NAME"] = cmd_k.GetString(1);
							}
							else
							{
								row_MatTable["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
							}
						}
						cmd_k.Close();
					}
					else
					{
						row_MatTable["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
					}
					if (row_MatTable["STOCK_NAME"].ToString().Trim() == "")
					{
						row_MatTable["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
					}
					row_MatTable["STOCK_WT"] = tfbsm18["STOCK_WT_QC"].ToString().Trim();
					Log::Trace("", __FUNCTION__, "MatTable.STOCK_WT.sqlstr = [{0}]", row_MatTable["STOCK_WT"].ToString());
					row_MatTable["BACK_C2"] = tfbsm18["BACK_C2"].ToString().Trim();
					row_MatTable["C_VALUE"] = tfbsm18["C_VALUE"].ToString().Trim();
					row_MatTable["SI_VALUE"] = tfbsm18["SI_VALUE"].ToString().Trim();
					row_MatTable["MN_VALUE"] = tfbsm18["MN_VALUE"].ToString().Trim();
					row_MatTable["P_VALUE"] = tfbsm18["P_VALUE"].ToString().Trim();
					row_MatTable["S_VALUE"] = tfbsm18["S_VALUE"].ToString().Trim();
					row_MatTable["CR_VALUE"] = tfbsm18["CR_VALUE"].ToString().Trim();
					row_MatTable["NI_VALUE"] = tfbsm18["NI_VALUE"].ToString().Trim();
					row_MatTable["MO_VALUE"] = tfbsm18["MO_VALUE"].ToString().Trim();
					row_MatTable["CU_VALUE"] = tfbsm18["CU_VALUE"].ToString().Trim();
					row_MatTable["CO_VALUE"] = tfbsm18["CO_VALUE"].ToString().Trim();
					row_MatTable["TI_VALUE"] = tfbsm18["TI_VALUE"].ToString().Trim();
					row_MatTable["NB_VALUE"] = tfbsm18["NB_VALUE"].ToString().Trim();
					row_MatTable["AL_VALUE"] = tfbsm18["AL_VALUE"].ToString().Trim();
					row_MatTable["COST"] = tfbsm18["PRICE"].ToString().Trim();
					/*tfbsm18["DATE_C"] = date;
					tfbsm18["SEQ_NO"] = seq_no;
					tfbsm18["BACKLOG_EA"] = backlog_ea;
					tfbsm18["REC_CREATOR"] = s.userid;
					tfbsm18["REC_CREATE_TIME"] = datetime;
					tfbsm18.Insert();*/
				}
				cmd_inq.Close();
				ret = inDMST02.Tables["MatTable"].Rows.get_Count();
				Log::Trace("", __FUNCTION__, "inDMST02.MatTable.Rows = [{0}]", ret);
				if (ret == 0)
				{
					strcpy(s.msg, "出钢记号:" + st_no + ",工艺路径:" + backlog_ea + "库存成分表没有数据!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				else
				{
					// 判断是否是脱磷或者三脱
					if (st_no.SubstringNE(1, 1) == "F" || st_no.SubstringNE(1, 1) == "M")
					{
						if (comm_fmly_code != "V")
						{
							sqlstr = " SELECT * FROM TFBSM13 WHERE BACKLOG_EA='" + backlog_ea + "' ";
							cmd_inq.SetCommandText(sqlstr);
							cmd_inq.ExecuteReader();
							if (cmd_inq.Read())
							{
								cmd_inq.Fetch(tfbsm13);
								inDMST02.Tables["MatTable"].Rows.Add();
								inDMST02.Tables["MatTable"].Rows[ret]["LOT_NO"] = " ";
								inDMST02.Tables["MatTable"].Rows[ret]["BACK_C2"] = " ";
								inDMST02.Tables["MatTable"].Rows[ret]["ST_NO"] = st_no;
								inDMST02.Tables["MatTable"].Rows[ret]["STOCK_NAME"] = " ";
								inDMST02.Tables["MatTable"].Rows[ret]["STOCK_WT"] = "9999";
								inDMST02.Tables["MatTable"].Rows[ret]["MAT_CODE"] = "PL0003";
								inDMST02.Tables["MatTable"].Rows[ret]["MAT_NAME"] = "预溶液成分";
								inDMST02.Tables["MatTable"].Rows[ret]["C_VALUE"] = tfbsm13["C_VALUE"].ToString().Trim();
								inDMST02.Tables["MatTable"].Rows[ret]["SI_VALUE"] = tfbsm13["SI_VALUE"].ToString().Trim();
								inDMST02.Tables["MatTable"].Rows[ret]["MN_VALUE"] = tfbsm13["MN_VALUE"].ToString().Trim();
								inDMST02.Tables["MatTable"].Rows[ret]["P_VALUE"] = tfbsm13["P_VALUE"].ToString().Trim();
								inDMST02.Tables["MatTable"].Rows[ret]["S_VALUE"] = tfbsm13["S_VALUE"].ToString().Trim();
								inDMST02.Tables["MatTable"].Rows[ret]["CR_VALUE"] = tfbsm13["CR_VALUE"].ToString().Trim();
								inDMST02.Tables["MatTable"].Rows[ret]["NI_VALUE"] = tfbsm13["NI_VALUE"].ToString().Trim();
								inDMST02.Tables["MatTable"].Rows[ret]["MO_VALUE"] = tfbsm13["MO_VALUE"].ToString().Trim();
								inDMST02.Tables["MatTable"].Rows[ret]["CU_VALUE"] = tfbsm13["CU_VALUE"].ToString().Trim();
								inDMST02.Tables["MatTable"].Rows[ret]["CO_VALUE"] = "0";
								inDMST02.Tables["MatTable"].Rows[ret]["TI_VALUE"] = "0";
								inDMST02.Tables["MatTable"].Rows[ret]["NB_VALUE"] = "0";
								inDMST02.Tables["MatTable"].Rows[ret]["AL_VALUE"] = "0";
								inDMST02.Tables["MatTable"].Rows[ret]["COST"] = "2294";
							}
							cmd_inq.Close();
							ret = inDMST02.Tables["MatTable"].Rows.get_Count();
							Log::Trace("", __FUNCTION__, "inDMST021.MatTable.Rows = [{0}]", ret);
						}
					}
				}
				// WORKTABLE（工序基准成分）
				sqlstr = "SELECT "
						 "    t1.COMM_FMLY_CODE, "
						 "    t1.MATERIAL_CODE, "
						 "    t1.ST_NO, "
						 "    t1.STATION_ID, "
						 "    t1.ELM_NAME, "
						 "    t1.SPE_MIN, "
						 "    t1.SPE_MAX, "
						 "    t1.BACKLOG_EA, "
						 "    CASE "
						 "        WHEN t1.STATION_ID != 'Y' AND (t1.ELM_NAME = 'CR' or t1.ELM_NAME = 'Cr') THEN t2.CR "
						 "        WHEN t1.STATION_ID != 'Y' AND (t1.ELM_NAME = 'Mo' or t1.ELM_NAME = 'MO' ) THEN t2.MO "
						 "        WHEN t1.STATION_ID != 'Y' AND (t1.ELM_NAME = 'NI' OR t1.ELM_NAME = 'Ni') THEN t2.NI "
						 "        ELSE 100 "
						 "    END AS YIELD "
						 "FROM ( "
						 "    SELECT "
						 "        COMM_FMLY_CODE, "
						 "        '" +
						 material_code + "'    MATERIAL_CODE, "
										 "        '" +
						 st_no + "' ST_NO, "
								 "        BACKLOG_EA, "
								 "        STATION_ID, "
								 "        ELM_NAME, "
								 "        SPE_MIN, "
								 "        SPE_MAX, "
								 "        MARK_POS_CODE "
								 "    FROM ( "
								 "        SELECT "
								 "            A.*, "
								 "            ROW_NUMBER() OVER ( "
								 "                PARTITION BY A.ELM_NAME, A.STATION_ID "
								 "                ORDER BY CASE "
								 "                    WHEN A.ST_NO = '" +
						 st_no + "' THEN 1 "
								 "                    WHEN A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" +
						 material_code + "' THEN 2 "
										 "                    WHEN A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "' THEN 3 "
										  "                    WHEN A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" +
						 material_code + "' THEN 4 "
										 "                    WHEN A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "' THEN 5 "
										  "                    ELSE 99 "
										  "                END "
										  "            ) as rn "
										  "        FROM (select * from tfbsm05 where BACKLOG_EA = '" +
						 backlog_ea + "') A "
									  "        WHERE (A.ST_NO = '" +
						 st_no + "' OR (A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "') OR "
																										 "               (A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "') OR "
										  "               (A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" +
						 material_code + "') OR "
										 "               (A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "')) "
										  "        "
										  "    ) "
										  "    WHERE rn = 1 "
										  ") t1 "
										  "LEFT JOIN( "
										  "    SELECT "
										  "        '" +
						 st_no + "' ST_NO, "
								 "        COMM_FMLY_CODE, "
								 "        STATION_ID, "
								 "        BACKLOG_EA, "
								 "        '" +
						 material_code + "' MATERIAL_CODE, "
										 "        MARK_POS_CODE,  "
										 "        MAX(CASE WHEN YIELD_CR != 0 THEN YIELD_CR END) AS CR, "
										 "        MAX(CASE WHEN YIELD_MO != 0 THEN YIELD_MO END) AS MO, "
										 "        MAX(CASE WHEN YIELD_NI != 0 THEN YIELD_NI END) AS NI "
										 "    FROM tfbsm04 "
										 "    WHERE "
										 "        BACKLOG_EA = '" +
						 backlog_ea + "' "
									  "        AND (ST_NO = '" +
						 st_no + "' "
								 "             OR (MARK_POS_CODE = '1' AND MATERIAL_CODE = '" +
						 material_code + "') "
										 "             OR (MARK_POS_CODE = '2' AND COMM_FMLY_CODE = '" +
						 comm_fmly_code + "') "
										  "             OR (MARK_POS_CODE = '3' AND MATERIAL_CODE = '" +
						 material_code + "') "
										 "             OR (MARK_POS_CODE = '3' AND COMM_FMLY_CODE = '" +
						 comm_fmly_code + "')) "
										  "    GROUP BY ST_NO, COMM_FMLY_CODE, STATION_ID, BACKLOG_EA, MATERIAL_CODE, MARK_POS_CODE "
										  ") t2 "
										  "ON t1.ST_NO = t2.ST_NO "
										  "   AND t1.COMM_FMLY_CODE = t2.COMM_FMLY_CODE "
										  "   AND t1.STATION_ID = t2.STATION_ID "
										  "   AND t1.MATERIAL_CODE = t2.MATERIAL_CODE "
										  "   AND t1.BACKLOG_EA = t2.BACKLOG_EA "
										  "WHERE (t1.ELM_NAME != 'C' and t1.ELM_NAME != 'S') "
										  "  AND t1.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "' ";
				cmd_inq.SetCommandText(sqlstr);
				Log::Trace("", __FUNCTION__, "inDMST02.WORKTABLE.sqlstr = [{0}]", sqlstr);
				inDMST02.Tables["WORKTABLE"].Rows.Clear();
				cmd_inq.ExecuteReader();
				while (cmd_inq.Read())
				{
					CDataRow &row = inDMST02.Tables["WORKTABLE"].Rows.Add();
					row["COMM_FMLY_CODE"] = cmd_inq.GetString(1).Trim();
					row["MATERIAL_CODE"] = cmd_inq.GetString(2).Trim();
					row["ST_NO"] = cmd_inq.GetString(3).Trim();
					row["STATION_ID"] = cmd_inq.GetString(4).Trim();
					row["ELM_NAME"] = cmd_inq.GetString(5).Trim();
					row["SPE_MIN"] = cmd_inq.GetDecimal(6);
					// Log::Trace("", __FUNCTION__, "inDMST02.SPE_MIN.sqlstr = [{0}]", row["SPE_MIN"].ToString());
					row["SPE_MAX"] = cmd_inq.GetDecimal(7);
					// Log::Trace("", __FUNCTION__, "inDMST02.SPE_MAX.sqlstr = [{0}]", row["SPE_MAX"].ToString());
					row["BACKLOG_EA"] = cmd_inq.GetString(8).Trim();
					row["YIELD"] = cmd_inq.GetString(9).Trim();
				}
				cmd_inq.Close();
				ret = inDMST02.Tables["WORKTABLE"].Rows.get_Count();
				Log::Trace("", __FUNCTION__, "inDMST02.WORKTABLE.Rows = [{0}]", ret);
				if (ret == 0)
				{
					strcpy(s.msg, "出钢记号:" + st_no + ",工艺路径:" + backlog_ea + "工序基准成分没有数据!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				// GX_RATE(收得率)
				/*sqlstr = " SELECT * "
				" FROM( "
				" SELECT "
				" A.*, "
				" ROW_NUMBER() OVER(PARTITION BY A.STATION_ID ORDER BY "
				" CASE "
				" WHEN A.ST_NO = '" + st_no + "' THEN 1 "
				" WHEN A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "' THEN 2 "
				" WHEN A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 3 "
				" WHEN A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "' THEN 4 "
				" WHEN A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 5 "
				" ELSE 99 "
				" END) as rn "
				" FROM tfbsm04 A "
				" WHERE "
				" A.ST_NO = '" + st_no + "' "
				" OR(A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "') "
				" OR(A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "') "
				" OR(A.MARK_POS_CODE = '3' AND(A.MATERIAL_CODE = '" + material_code + "' OR A.COMM_FMLY_CODE = '" + comm_fmly_code + "')) "
				" ) "
				" WHERE rn = 1 ";
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.ExecuteReader();
				while (cmd_inq.Read())
				{
				tfbsm04.Reset();
				cmd_inq.Fetch(tfbsm04);
				tfbsm04.TrimOrBlank();
				CDataRow& row = inDMST02.Tables["GX_RATE"].Rows.Add();
				row["COMM_FMLY_CODE"] = tfbsm04["COMM_FMLY_CODE"].ToString();
				row["MATERIAL_CODE"] = tfbsm04["MATERIAL_CODE"].ToString();
				row["ST_NO"] = tfbsm04["ST_NO"].ToString();
				row["STATION_ID"] = tfbsm04["STATION_ID"].ToString();
				row["BACKLOG_EA"] = tfbsm04["BACKLOG_EA"].ToString();
				row["YIELD_CR"] = tfbsm04["YIELD_CR"].ToString();
				row["YIELD_MO"] = tfbsm04["YIELD_MO"].ToString();
				row["YIELD_NI"] = tfbsm04["YIELD_NI"].ToString();
				row["METAL_CONS_PERT"] = tfbsm04["METAL_CONS_PERT"].ToString();
				}
				cmd_inq.Close();
				ret = inDMST02.Tables["GX_RATE"].Rows.get_Count();
				Log::Trace("", __FUNCTION__, "inDMST02.GX_RATE.Rows = [{0}]", ret);*/
				// GX_PARA（工序投料比）
				/*sqlstr = " SELECT * "
				" FROM( "
				" SELECT "
				" A.*, "
				" ROW_NUMBER() OVER(PARTITION BY A.BACKLOG_EA ORDER BY "
				" CASE "
				" WHEN A.ST_NO = '" + st_no + "' THEN 1 "
				" WHEN A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "' THEN 2 "
				" WHEN A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 3 "
				" WHEN A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "' THEN 4 "
				" WHEN A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 5 "
				" ELSE 99 "
				" END) as rn "
				" FROM tfbsm09 A "
				" WHERE "
				" A.ST_NO = '" + st_no + "' "
				" OR(A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "') "
				" OR(A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "') "
				" OR(A.MARK_POS_CODE = '3' AND(A.MATERIAL_CODE = '" + material_code + "' OR A.COMM_FMLY_CODE = '" + comm_fmly_code + "')) "
				" ) "
				" WHERE rn = 1 ";
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.ExecuteReader();
				while (cmd_inq.Read())
				{
				tfbsm09.Reset();
				cmd_inq.Fetch(tfbsm09);
				tfbsm09.TrimOrBlank();
				CDataRow& row = inDMST02.Tables["GX_PARA"].Rows.Add();
				row["COMM_FMLY_CODE"] = tfbsm09["COMM_FMLY_CODE"].ToString();
				row["MATERIAL_CODE"] = tfbsm09["MATERIAL_CODE"].ToString();
				row["ST_NO"] = tfbsm09["ST_NO"].ToString();
				row["BACKLOG_EA"] = tfbsm09["BACKLOG_EA"].ToString();
				row["IRON_RATIO1"] = tfbsm09["IRON_RATIO1"].ToString();
				row["IRON_RATIO2"] = tfbsm09["IRON_RATIO2"].ToString();
				row["DEVO_RATIO_IF"] = tfbsm09["DEVO_RATIO_IF"].ToString();
				row["DEVO_RATIO_EAF"] = tfbsm09["DEVO_RATIO_EAF"].ToString();
				row["DEVO_RATIO_AOD"] = tfbsm09["DEVO_RATIO_AOD"].ToString();
				row["SCRAP_RATIO_IF"] = tfbsm09["SCRAP_RATIO_IF"].ToString();
				row["SCRAP_RATIO_EAF"] = tfbsm09["SCRAP_RATIO_EAF"].ToString();
				row["SCRAP_RATIO_AOD"] = tfbsm09["SCRAP_RATIO_AOD"].ToString();
				}
				cmd_inq.Close();
				ret = inDMST02.Tables["GX_PARA"].Rows.get_Count();
				Log::Trace("", __FUNCTION__, "inDMST02.GX_PARA.Rows = [{0}]", ret);*/
				// GX_MAT（原料总表） WEIGHT_MINUS WEIGHT_POSITIVE
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
						 "    CASE "
						 "        WHEN '" +
						 backlog_ea + "' = '03' AND F.STATION_ID = 'E' AND "
									  "             (SUBSTR('" +
						 st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND f.COMM_FMLY_CODE != 'V' THEN 95 "
																		"        WHEN G.YEILD_MINUS = 0 THEN 100 "
																		"        ELSE G.YEILD_MINUS "
																		"    END YEILD_MINUS, "
																		"    F.* "
																		"FROM ( "
																		"    SELECT "
																		"        COMM_FMLY_CODE, "
																		"        MATERIAL_CODE, "
																		"        ST_NO, "
																		"        STATION_ID, "
																		"        MAT_CODE, "
																		"        MAT_NAME, "
																		"        UPPER_LIMIT_VALUE, "
																		"        LOWER_LIMIT_VALUE, "
																		"        CASE WHEN STATION_ID = 'A' THEN MUST_DO_FLAG ELSE 0 END MUST_DO_FLAG "
																		"    FROM ( "
																		"        SELECT "
																		"            A.*, "
																		"            ROW_NUMBER() OVER ( "
																		"                PARTITION BY A.STATION_ID, A.MAT_CODE "
																		"                ORDER BY CASE "
																		"                            WHEN A.ST_NO = '" +
						 st_no + "' THEN 1 "
								 "                            WHEN A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" +
						 material_code + "' THEN 2 "
										 "                            WHEN A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "' THEN 3 "
										  "                            WHEN A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" +
						 material_code + "' THEN 4 "
										 "                            WHEN A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "' THEN 5 "
										  "                            ELSE 99 "
										  "                        END "
										  "            ) as rn "
										  "        FROM TFBSM01 A "
										  "        WHERE ( "
										  "            A.ST_NO = '" +
						 st_no + "' "
								 "            OR (A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" +
						 material_code + "') "
										 "            OR (A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "') "
										  "            OR (A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" +
						 material_code + "') "
										 "            OR (A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "') "
										  "        ) "
										  "    ) "
										  "    WHERE rn = 1 "
										  ") F "
										  "LEFT JOIN TMMSM50 G "
										  "    ON F.MAT_CODE = G.MAT_CODE ";
				Log::Trace("", __FUNCTION__, "inDMST02.GX_MAT.sqlstr = [{0}]", sqlstr);
				cmd_inq.SetCommandText(sqlstr);
				inDMST02.Tables["GX_MAT"].Rows.Clear();
				cmd_inq.ExecuteReader();
				while (cmd_inq.Read())
				{
					tfbsm01.Reset();
					cmd_inq.Fetch(tfbsm01);
					tfbsm01.TrimOrBlank();
					CDataRow &row = inDMST02.Tables["GX_MAT"].Rows.Add();
					if (cmd_inq.GetString(1) != tfbsm01["LOWER_LIMIT_VALUE"].ToString() && backlog_ea == "01" && tfbsm01["STATION_ID"].ToString() == "Z")
					{
						// 如果值是50表的并且工艺路线是多中频炉则需要单独处理
						row["LOWER_LIMIT_VALUE"] = cmd_inq.GetDecimal(1) + cmd_inq.GetDecimal(1);
					}
					else if (cmd_inq.GetString(1) != tfbsm01["LOWER_LIMIT_VALUE"].ToString() && backlog_ea == "04" && tfbsm01["STATION_ID"].ToString() == "Z")
					{
						row["LOWER_LIMIT_VALUE"] = cmd_inq.GetDecimal(1) + cmd_inq.GetDecimal(1) + cmd_inq.GetDecimal(1);
					}
					else
					{
						row["LOWER_LIMIT_VALUE"] = cmd_inq.GetDecimal(1);
					}
					if (cmd_inq.GetString(2) != tfbsm01["UPPER_LIMIT_VALUE"].ToString() && backlog_ea == "01" && tfbsm01["STATION_ID"].ToString() == "Z")
					{
						// 如果值是50表的并且工艺路线是多中频炉则需要单独处理
						row["UPPER_LIMIT_VALUE"] = cmd_inq.GetDecimal(2) + cmd_inq.GetDecimal(2);
					}
					else if (cmd_inq.GetString(2) != tfbsm01["UPPER_LIMIT_VALUE"].ToString() && backlog_ea == "04" && tfbsm01["STATION_ID"].ToString() == "Z")
					{
						row["UPPER_LIMIT_VALUE"] = cmd_inq.GetDecimal(2) + cmd_inq.GetDecimal(2) + cmd_inq.GetDecimal(2);
					}
					else
					{
						row["UPPER_LIMIT_VALUE"] = cmd_inq.GetDecimal(2);
					}
					Log::Trace("", __FUNCTION__, ".UPPER_LIMIT_VALUE = [{0}]", row["UPPER_LIMIT_VALUE"].ToString());
					// row["UPPER_LIMIT_VALUE"] = cmd_inq.GetDecimal(2);
					row["YEILD_MINUS"] = cmd_inq.GetString(3).Trim();
					row["COMM_FMLY_CODE"] = tfbsm01["COMM_FMLY_CODE"].ToString().Trim();
					row["MATERIAL_CODE"] = tfbsm01["MATERIAL_CODE"].ToString().Trim();
					row["ST_NO"] = tfbsm01["ST_NO"].ToString().Trim();
					row["STATION_ID"] = tfbsm01["STATION_ID"].ToString().Trim();
					row["MAT_CODE"] = tfbsm01["MAT_CODE"].ToString().Trim();
					row["MAT_NAME"] = tfbsm01["MAT_NAME"].ToString().Trim();
					row["MUST_DO_FLAG"] = tfbsm01["MUST_DO_FLAG"].ToString().Trim();
				}
				cmd_inq.Close();
				ret = inDMST02.Tables["GX_MAT"].Rows.get_Count();
				Log::Trace("", __FUNCTION__, "inDMST02.GX_MAT.Rows = [{0}]", ret);
				if (ret == 0)
				{
					strcpy(s.msg, "出钢记号:" + st_no + ",工艺路径:" + backlog_ea + "工序投料品名没有数据!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}
				else
				{
					if (st_no.SubstringNE(1, 1) == "F" || st_no.SubstringNE(1, 1) == "M")
					{
						if (comm_fmly_code != "V")
						{
							inDMST02.Tables["GX_MAT"].Rows.Add();
							inDMST02.Tables["GX_MAT"].Rows[ret]["LOWER_LIMIT_VALUE"] = "0";
							inDMST02.Tables["GX_MAT"].Rows[ret]["UPPER_LIMIT_VALUE"] = "99999";
							inDMST02.Tables["GX_MAT"].Rows[ret]["YEILD_MINUS"] = "100";
							inDMST02.Tables["GX_MAT"].Rows[ret]["COMM_FMLY_CODE"] = comm_fmly_code;
							inDMST02.Tables["GX_MAT"].Rows[ret]["MATERIAL_CODE"] = material_code;
							inDMST02.Tables["GX_MAT"].Rows[ret]["ST_NO"] = st_no;
							inDMST02.Tables["GX_MAT"].Rows[ret]["STATION_ID"] = "A";
							inDMST02.Tables["GX_MAT"].Rows[ret]["MAT_CODE"] = "PL0003";
							inDMST02.Tables["GX_MAT"].Rows[ret]["MAT_NAME"] = "预溶液成分";
							inDMST02.Tables["GX_MAT"].Rows[ret]["MUST_DO_FLAG"] = "0";
						}
					}
				}
				// ROUTE（工艺路线）
				/*sqlstr = " select CODE,BACKLOG_EA,DESCRIP from tfbsm02 ";
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.ExecuteReader();
				while (cmd_inq.Read())
				{
				CDataRow& row = inDMST02.Tables["ROUTE"].Rows.Add();
				row["CODE"] = cmd_inq.GetString(1);
				row["BACKLOG_EA"] = cmd_inq.GetString(2);
				row["DESCRIP"] = cmd_inq.GetString(3);
				}
				cmd_inq.Close();
				ret = inDMST02.Tables["ROUTE"].Rows.get_Count();
				Log::Trace("", __FUNCTION__, "inDMST02.ROUTE.Rows = [{0}]", ret);*/
				// GX_WEIGHT(可投重量)
				sqlstr = "SELECT '" + st_no + "'                                                     ST_NO,"
											  "       m.STATION_ID,"
											  "       m.BACKLOG_EA,"
											  "       m.METAL_CONS_PERT,"
											  "       nvl(m.SCRAP_RATIO_MIN, 0)                                    SCRAP_RATIO_MIN,"
											  "       nvl(m.SCRAP_RATIO_MAX, 0)                                    SCRAP_RATIO_MAX,"
											  "       CASE"
											  "           WHEN m.BACKLOG_EA = '03' AND m.STATION_ID = 'E' AND"
											  "                (SUBSTR('" +
						 st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' THEN 175"
																		"           WHEN m.BACKLOG_EA = '02' AND m.STATION_ID = 'E' AND"
																		"                (SUBSTR('" +
						 st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' THEN 165"
																		"           WHEN m.BACKLOG_EA = '02' AND m.STATION_ID = 'Z' AND"
																		"                (SUBSTR('" +
						 st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V'  THEN 30"
																		"           WHEN m.BACKLOG_EA = '07' AND m.STATION_ID = 'B' AND"
																		"                (SUBSTR('" +
						 st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' THEN 140"
																		"           WHEN m.BACKLOG_EA = '07' AND m.STATION_ID = 'Z' AND"
																		"                (SUBSTR('" +
						 st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' THEN 50 "
																		"           WHEN m.BACKLOG_EA = '07' AND m.STATION_ID = 'A' AND"
																		"                (SUBSTR('" +
						 st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' THEN 220 "
																		"			WHEN (m.STATION_ID='D' or m.STATION_ID='B') and (m.COMM_FMLY_CODE='L' or m.COMM_FMLY_CODE='M' OR m.COMM_FMLY_CODE='O') and (m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 190 "
																		"			WHEN (m.STATION_ID='D' or m.STATION_ID='B') and  m.COMM_FMLY_CODE='P' and (m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 180 "
																		"			WHEN (m.STATION_ID='D' or m.STATION_ID='B') and  m.COMM_FMLY_CODE='Q' and (m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 140 "
																		"			WHEN m.STATION_ID='E'  and m.BACKLOG_EA = '03' AND  (SUBSTR('" +
						 st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' then 175"
																		"           ELSE nvl(m.TAPPING_WT, 0) END                            TAPPING_WT,"
																		"       m.MAT_COUNT,"
																		"       case when M.STATION_ID = 'A' THEN L.DESCRIPTION ELSE ' ' END COMBY_GX, "
																		"		CASE "
																		"		WHEN(m.STATION_ID = 'D' or m.STATION_ID = 'B') and(m.COMM_FMLY_CODE = 'L' or m.COMM_FMLY_CODE = 'M' OR m.COMM_FMLY_CODE = 'O') and(m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 130 "
																		"		WHEN(m.STATION_ID = 'D' or m.STATION_ID = 'B') and  m.COMM_FMLY_CODE = 'P' and(m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 150 "
																		"		WHEN(m.STATION_ID = 'D' or m.STATION_ID = 'B') and  m.COMM_FMLY_CODE = 'Q' and(m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 120 "
																		"		WHEN m.STATION_ID = 'E'  and m.BACKLOG_EA = '03' AND(SUBSTR('" +
						 st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE != 'V' then 170 "
																		"		ELSE 0 END                            TAPPING_WT_MIN "
																		"    FROM ("
																		"    SELECT j.*,"
																		"           ROW_NUMBER() OVER ("
																		"               PARTITION BY J.STATION_ID"
																		"               ORDER BY CASE"
																		"                           WHEN j.ST_NO = '" +
						 st_no + "' THEN 1"
								 "                           WHEN j.MARK_POS_CODE = '1' AND j.MATERIAL_CODE = '" +
						 material_code + "' THEN 2"
										 "                           WHEN j.MARK_POS_CODE = '2' AND j.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "' THEN 3"
										  "                           WHEN j.MARK_POS_CODE = '3' AND j.MATERIAL_CODE = '" +
						 material_code + "' THEN 4"
										 "                           WHEN j.MARK_POS_CODE = '3' AND j.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "' THEN 5"
										  "                           ELSE 99"
										  "                       END"
										  "           ) as rn"
										  "    FROM ("
										  "        SELECT A.*,"
										  "               CASE"
										  "                   WHEN (B.EAF_MIN IS NOT NULL OR B.EAF_MIN != ' ') AND A.STATION_ID = 'E' THEN B.EAF_MIN"
										  "                   WHEN B.IF_MIN IS NOT NULL AND A.STATION_ID = 'Z' THEN B.IF_MIN"
										  "                   WHEN B.AOD_MIN IS NOT NULL AND A.STATION_ID = 'A' THEN B.AOD_MIN"
										  "               END AS SCRAP_RATIO_MIN,"
										  "               CASE"
										  "                   WHEN (B.EAF_MAX IS NOT NULL OR B.EAF_MIN != ' ') AND A.STATION_ID = 'E' THEN B.EAF_MAX"
										  "                   WHEN B.IF_MAX IS NOT NULL AND A.STATION_ID = 'Z' THEN B.IF_MAX"
										  "                   WHEN B.AOD_MAX IS NOT NULL AND A.STATION_ID = 'A' THEN B.AOD_MAX"
										  "               END AS SCRAP_RATIO_MAX,"
										  "               CASE"
										  "                   WHEN B.D_B IS NOT NULL AND A.STATION_ID = 'B' THEN B.D_B"
										  "                   WHEN B.D_D IS NOT NULL AND A.STATION_ID = 'D' THEN D_D"
										  "                   WHEN B.D_E IS NOT NULL AND A.STATION_ID = 'E' THEN D_E"
										  "                   WHEN B.D_Z IS NOT NULL AND A.STATION_ID = 'Z' THEN D_Z"
										  "                   WHEN B.TAPPING_WT IS NOT NULL AND A.STATION_ID = 'A' THEN B.TAPPING_WT"
										  "               END AS TAPPING_WT,"
										  "               CASE"
										  "                   WHEN A.STATION_ID = 'E' THEN '4'"
										  "                   WHEN A.STATION_ID = 'Z' THEN '4'"
										  "                   WHEN A.STATION_ID = 'A' THEN '12'"
										  "                   WHEN A.STATION_ID = 'B' THEN '10'"
										  "               END AS MAT_COUNT"
										  "        FROM ("
										  "            SELECT"
										  "                t.ST_NO,"
										  "                t.STATION_ID,"
										  "                t.BACKLOG_EA,"
										  "                t.METAL_CONS_PERT,"
										  "                t.COMM_FMLY_CODE,"
										  "                t.MATERIAL_CODE,"
										  "                t.MARK_POS_CODE"
										  "            FROM tfbsm04 t"
										  "            WHERE t.BACKLOG_EA = '" +
						 backlog_ea + "'"
									  "              AND ("
									  "                  t.ST_NO = '" +
						 st_no + "'"
								 "                  OR (t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" +
						 material_code + "')"
										 "                  OR (t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "')"
										  "                  OR (t.MARK_POS_CODE = '3' AND (t.MATERIAL_CODE = '" +
						 material_code + "' OR t.COMM_FMLY_CODE = '" + comm_fmly_code + "'))"
																						"              )"
																						"        ) A"
																						"        LEFT JOIN ("
																						"            SELECT *"
																						"            FROM ("
																						"                SELECT"
																						"                    t.ST_NO,"
																						"                    t.COMM_FMLY_CODE,"
																						"                    t.BACKLOG_EA,"
																						"                    t.MATERIAL_CODE,"
																						"                    t.TAPPING_WT,"
																						"                    MAX(CASE WHEN t.DEVO_RATIO_EAF != 0 THEN t.DEVO_RATIO_EAF END) AS D_E,"
																						"                    MAX(CASE WHEN t.DEVO_RATIO_IF != 0 THEN t.DEVO_RATIO_IF END) AS D_Z,"
																						"                    MAX(CASE WHEN t.IRON_RATIO2 != 0 THEN t.IRON_RATIO2 END) AS D_B,"
																						"                    MAX(CASE WHEN t.IRON_RATIO1 != 0 THEN t.IRON_RATIO1 END) AS D_D,"
																						"                    MAX(CASE WHEN t.IF_MAX != 0 THEN t.IF_MAX END) AS IF_MAX,"
																						"                    MAX(CASE WHEN t.IF_MIN != 0 THEN t.IF_MIN END) AS IF_MIN,"
																						"                    MAX(CASE WHEN t.AOD_MAX != 0 THEN t.AOD_MAX END) AS AOD_MAX,"
																						"                    MAX(CASE WHEN t.AOD_MIN != 0 THEN t.AOD_MIN END) AS AOD_MIN,"
																						"                    MAX(CASE WHEN t.EAF_MAX != 0 THEN t.EAF_MAX END) AS EAF_MAX,"
																						"                    MAX(CASE WHEN t.EAF_MIN != 0 THEN t.EAF_MIN END) AS EAF_MIN"
																						"                FROM TFBSM09 t"
																						"                WHERE t.BACKLOG_EA = '" +
						 backlog_ea + "'"
									  "                  AND ("
									  "                      t.ST_NO = '" +
						 st_no + "'"
								 "                      OR (t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" +
						 material_code + "')"
										 "                      OR (t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "')"
										  "                      OR (t.MARK_POS_CODE = '3' AND (t.MATERIAL_CODE = '" +
						 material_code + "' OR t.COMM_FMLY_CODE = '" + comm_fmly_code + "'))"
																						"                  )"
																						"                GROUP BY t.ST_NO, t.COMM_FMLY_CODE, t.BACKLOG_EA, t.MATERIAL_CODE, t.TAPPING_WT"
																						"            )"
																						"        ) B ON A.BACKLOG_EA = B.BACKLOG_EA"
																						"           AND A.COMM_FMLY_CODE = B.COMM_FMLY_CODE"
																						"           AND A.MATERIAL_CODE = B.MATERIAL_CODE"
																						"           AND (A.ST_NO = B.ST_NO OR B.ST_NO IS NULL)"
																						"    ) J"
																						"    WHERE ("
																						"        J.ST_NO = '" +
						 st_no + "'"
								 "        OR (J.MARK_POS_CODE = '1' AND J.MATERIAL_CODE = '" +
						 material_code + "')"
										 "        OR (J.MARK_POS_CODE = '2' AND J.COMM_FMLY_CODE = '" +
						 comm_fmly_code + "')"
										  "        OR (J.MARK_POS_CODE = '3' AND (J.MATERIAL_CODE = '" +
						 material_code + "' OR J.COMM_FMLY_CODE = '" + comm_fmly_code + "'))"
																						"    )"
																						" ) M"
																						" LEFT JOIN TFBSM02 L ON M.BACKLOG_EA = L.CODE"
																						" WHERE M.RN = 1";
				cmd_inq.SetCommandText(sqlstr);
				Log::Trace("", __FUNCTION__, "inDMST02.GX_WEIGHT.sqlstr = [{0}]", sqlstr);
				inDMST02.Tables["GX_WEIGHT"].Rows.Clear();
				cmd_inq.ExecuteReader();
				while (cmd_inq.Read())
				{
					CDataRow &row = inDMST02.Tables["GX_WEIGHT"].Rows.Add();
					row["ST_NO"] = cmd_inq.GetString(1).Trim();
					row["STATION_ID"] = cmd_inq.GetString(2).Trim();
					row["BACKLOG_EA"] = cmd_inq.GetString(3).Trim();
					row["METAL_CONS_PERT"] = cmd_inq.GetString(4).Trim();
					row["SCRAP_RATIO_MIN"] = cmd_inq.GetString(5).Trim();
					row["SCRAP_RATIO_MAX"] = cmd_inq.GetString(6).Trim();
					if (st_no.SubstringNE(1, 1) == "F" || st_no.SubstringNE(1, 1) == "M")
					{
						if (comm_fmly_code != "V")
						{
							if (row["STATION_ID"].ToString() == "A")
							{
								row["TAPPING_WT"] = "220";
							}
							else
							{
								row["TAPPING_WT"] = "0";
							}
						}
						else
						{
							row["TAPPING_WT"] = cmd_inq.GetString(7).Trim();
						}
					}
					else
					{
						row["TAPPING_WT"] = cmd_inq.GetString(7).Trim();
					}
					if (row["TAPPING_WT"].ToString().Trim() == "" || row["TAPPING_WT"].ToString().Trim() == "0")
					{
						row["TAPPING_WT"] = cmd_inq.GetString(7).Trim();
					}
					// row["TAPPING_WT"] = cmd_inq.GetString(7).Trim();
					Log::Trace("", __FUNCTION__, "TAPPING_WT = [{0}]", row["TAPPING_WT"].ToString());
					row["MAT_COUNT"] = cmd_inq.GetString(8).Trim();
					row["COMBY_GX"] = cmd_inq.GetString(9).Trim();
					if (cmd_inq.GetDecimal(10) != 0)
					{
						row["TAPPING_WT_MIN"] = cmd_inq.GetString(10).Trim();
					}
					else
					{
						row["TAPPING_WT_MIN"] = cmd_inq.GetString(7).Trim();
					}
				}
				cmd_inq.Close();
				ret = inDMST02.Tables["GX_WEIGHT"].Rows.get_Count();
				Log::Trace("", __FUNCTION__, "inDMST02.GX_WEIGHT.Rows = [{0}]", ret);
				if (ret == 0 && st_no.SubstringNE(1, 1) != "F" && st_no.SubstringNE(1, 1) != "M")
				{
					strcpy(s.msg, "出钢记号:" + st_no + ",工艺路径:" + backlog_ea + "收得率没有数据!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			if (bcls_rec->Tables["Main"].Rows.get_Count() != j + 1)
			{
				Log::Trace("", __FUNCTION__, "link = [{0}]", __LINE__);
				// inDMST02.Tables["LG_PSSM"].Rows[0]["FURNACE_COUNT"] = fu_count;
				Log::Trace("", __FUNCTION__, "FURNACE_COUNT = [{0}]", fu_count);
				if (bcls_rec->Tables["Main"].Rows[j + 1]["SEQ_NO"].ToString().TrimOrBlank() == "1")
				{
					Log::Trace("", __FUNCTION__, "link = [{0}]", __LINE__);
					// 准备启动模型
					Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc start ==");
					// CHECK_INPUT
					// f_epex_call_rest_svc(conn, "FBSM_SSMODEL", "CHECK_INPUT", &inDMST02, &outDMST02_1);
					// ei_sys the_s;
					// outDMST02_1.GetSYS(&the_s);
					// Log::Trace("", __FUNCTION__, "doFlag = [{0}]", doFlag);
					// if (the_s.flag < 0)
					//{
					//	Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc err[0] ==", the_s.msg);
					//	strcpy(the_s.msg, the_s.msg);
					//	//失败不报错
					//	throw CApplicationException(-1, s.msg, log.Location);

					//}
					// Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc MX ==");
					f_epex_call_rest_svc(conn, "FBSM_SSMODEL", "RUN_PLMODEL", &inDMST02, &outDMST02);
					Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc end ==");
					// 清空数据
					inDMST02.Tables["LG_PSSM"].Rows.Clear();
					Log::Trace("", __FUNCTION__, "WORK_TABLE = [{0}]", outDMST02.Tables["WORK_TABLE"].Rows.get_Count());
					// 循环存储报错信息 ERROR_INFO"
					Log::Trace("", __FUNCTION__, "ERROR_INFO = [{0}]", outDMST02.Tables["ERROR_INFO"].Rows.get_Count());
					for (size_t z = 0; z < outDMST02.Tables["ERROR_INFO"].Rows.get_Count(); z++)
					{
						sqlstr = " update tfbsm12 set REMARK_PS='" + outDMST02.Tables["ERROR_INFO"].Rows[z]["ERROR_MSG"].ToString() + "' where ST_NO='" + outDMST02.Tables["ERROR_INFO"].Rows[z]["ST_NO"].ToString() + "' AND BACKLOG_EA='" + outDMST02.Tables["ERROR_INFO"].Rows[z]["BACKLOG_EA"].ToString() + "' and FURNACE_COUNT='" + outDMST02.Tables["ERROR_INFO"].Rows[z]["SUM_FURNACES"].ToString() + "' and SEQ_NO='" + outDMST02.Tables["ERROR_INFO"].Rows[z]["SEQ_NO"].ToString() + "' ";
						Log::Trace("", __FUNCTION__, "ERROR_INFO== sqlstr[{0}] ==", sqlstr);
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.ExecuteNonQuery();
						cmd_inq.Close();
					}
					// MAT_CODE=PL0001   MAT_NAME=配料成分   MAT_CODE=PL0002 MAT_NAME=出钢成分 RESULT_TABLE
					for (size_t i = 0; i < outDMST02.Tables["WORK_TABLE"].Rows.get_Count(); i++)
					{
						// 钢种大类(A/M/P/D/F)+钢种小类编号+工艺路线编号+日期+2位流水
						CDecimal ts = 0; // 为了区分不同配料单的情况
						if (j == 0)
						{
							ts = bcls_rec->Tables["Main"].Rows[j]["SEQ_NO"].ToDecimal();
						}
						else
						{
							ts = i + 1;
						}
						if (outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() == "0")
						{
							// 模型没有计算出来
							sqlstr = " update tfbsm12 set COMPOSE_LIST_NO=' ',PLAN_ID=' ' where DATE_C='" + date + "' AND ST_NO='" + st_no + "' and BACKLOG_EA='" + backlog_ea + "' and SEQ_NO='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["SEQ_NO"].ToString() + "' ";
							Log::Trace("", __FUNCTION__, "WORK_TABLE== sqlstr[{0}] ==", sqlstr);
							cmd_inq.SetCommandText(sqlstr);
							cmd_inq.ExecuteNonQuery();
							cmd_inq.Close();
							continue;
						}
						plan_id = outDMST02.Tables["WORK_TABLE"].Rows[i]["PLAN_ID"].ToString();
						Log::Trace("", __FUNCTION__, "== date err[{0}] ==", date.SubstringNE(2, 6));
						Log::Trace("", __FUNCTION__, "== plan_id[{0}] ==", plan_id);
						f_material_code = outDMST02.Tables["WORK_TABLE"].Rows[i]["MATERIAL_CODE"].ToString();
						Log::Trace("", __FUNCTION__, "== f_material_code[{0}] ==", f_material_code);
						f_date = outDMST02.Tables["WORK_TABLE"].Rows[i]["DATE_C"].ToString();
						f_st_no = outDMST02.Tables["WORK_TABLE"].Rows[i]["ST_NO"].ToString();
						f_comm_fmly_code = outDMST02.Tables["WORK_TABLE"].Rows[i]["COMM_FMLY_CODE"].ToString();
						// BACKLOG_EA
						f_backlog_ea = outDMST02.Tables["WORK_TABLE"].Rows[i]["BACKLOG_EA"].ToString();
						// 获取出钢记号
						// 获取中类
						// AND MATERIAL_CODE=''
						if (i == 0 && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0")
						{
							// 默认是第一个配料单
							sqlstr = " select  max(substr(COMPOSE_LIST_NO,12,14)),max(substr(COMPOSE_LIST_NO,14,1)),max(substr(COMPOSE_LIST_NO,13, 1)),max(substr(COMPOSE_LIST_NO,13, 2)) from tfbsm14 where DATE_C='" + f_date + "'  and COMPOSE_LIST_NO!=' ' and MATERIAL_CODE='" + f_material_code + "'  and BACKLOG_EA='" + f_backlog_ea + "'   ";
							Log::Trace("", __FUNCTION__, "== sqlstr[{0}] ==", sqlstr);
							cmd_inq.SetCommandText(sqlstr);
							cmd_inq.ExecuteReader();
							if (cmd_inq.Read())
							{
								if (cmd_inq.GetDecimal(3) == 0 && cmd_inq.GetDecimal(2) != 9)
								{
									p_seq = cmd_inq.GetDecimal(2);
									Log::Trace("", __FUNCTION__, "== p_seq1[{0}] ==", p_seq.ToString());
									p_seq = p_seq + 1;
									Log::Trace("", __FUNCTION__, "== p_seq1[{0}] ==", p_seq.ToString());
									compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "0" + p_seq.ToString();
								}
								else
								{
									p_seq = cmd_inq.GetDecimal(4);
									Log::Trace("", __FUNCTION__, "== p_seq2[{0}] ==", p_seq.ToString());
									p_seq = p_seq + 1;
									Log::Trace("", __FUNCTION__, "== p_seq2[{0}] ==", p_seq.ToString());
									compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + p_seq.ToString();
								}
							}
							else
							{
								Log::Trace("", __FUNCTION__, "== linke[{0}] ==", __LINE__);
								compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "01";
							}
							cmd_inq.Close();
						}
						if (i != 0 && outDMST02.Tables["WORK_TABLE"].Rows[i]["PLAN_ID"].ToString() != outDMST02.Tables["WORK_TABLE"].Rows[i - 1]["PLAN_ID"].ToString() && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0")
						{
							sqlstr = " select  max(substr(COMPOSE_LIST_NO,12,14)),max(substr(COMPOSE_LIST_NO,14,1)),max(substr(COMPOSE_LIST_NO,13, 1)),max(substr(COMPOSE_LIST_NO,13, 2)) from tfbsm14 where DATE_C='" + f_date + "'  and COMPOSE_LIST_NO!=' ' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "'   ";
							Log::Trace("", __FUNCTION__, "== sqlstr!![{0}] ==", sqlstr);
							Log::Trace("", __FUNCTION__, "== 1Q1[{0}] ==", __LINE__);
							cmd_inq.SetCommandText(sqlstr);
							cmd_inq.ExecuteReader();
							if (cmd_inq.Read())
							{
								if (cmd_inq.GetDecimal(3) == 0 && cmd_inq.GetDecimal(2) != 9)
								{
									p_seq = cmd_inq.GetDecimal(2);
									p_seq = p_seq + 1;
									compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "0" + p_seq.ToString();
									Log::Trace("", __FUNCTION__, "==009compose_list_no[{0}] ==", compose_list_no);
								}
								else
								{
									p_seq = cmd_inq.GetDecimal(4);
									Log::Trace("", __FUNCTION__, "==seq[{0}] ==", p_seq.ToString());
									p_seq = p_seq + 1;
									compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + p_seq.ToString();
									Log::Trace("", __FUNCTION__, "==010compose_list_no[{0}] ==", compose_list_no);
									Log::Trace("", __FUNCTION__, "==seq[{0}] ==", p_seq.ToString());
								}
							}
							else
							{
								compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "01";
							}
							cmd_inq.Close();
						}
						Log::Trace("", __FUNCTION__, "== 1Q1compose_list_no[{0}] ==", compose_list_no);
						if (compose_list_no.Trim() == " " || compose_list_no.Trim() == "")
						{
							compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "01";
						}
						Log::Trace("", __FUNCTION__, "== 1111compose_list_no[{0}] ==", compose_list_no);
						sqlstr = " update tfbsm12 set COMPOSE_LIST_NO='" + compose_list_no + "',PLAN_ID='" + plan_id + "',SINGLECOST='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["SINGLECOST"].ToString() + "',COST_DG='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["COST_DG"].ToString() + "' where DATE_C='" + f_date + "' AND ST_NO='" + f_st_no + "' and BACKLOG_EA='" + f_backlog_ea + "' and SEQ_NO='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["SEQ_NO"].ToString() + "' ";
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.ExecuteNonQuery();
						cmd_inq.Close();
						Log::Trace("", __FUNCTION__, "== 12mmm.sqlstr[{0}] ==", sqlstr);
						// 还原SI的值
						sqlstr = " select SI_PDI from tfbsm20 where COMM_FMLY_CODE='" + f_comm_fmly_code + "' and BACKLOG_EA='" + f_backlog_ea + "'  ";
						cmd_si_pdi.SetCommandText(sqlstr);
						cmd_si_pdi.ExecuteReader();
						Log::Trace("", __FUNCTION__, "== SI.sqlstr[{0}] ==", sqlstr);
						if (cmd_si_pdi.Read())
						{
							f_si_pdi = cmd_si_pdi.GetDecimal(1);
						}
						cmd_si_pdi.Close();
						Log::Trace("", __FUNCTION__, "== SI.f_si_pdi[{0}] ==", f_si_pdi.ToString());
						if (i == 0 && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0")
						{
							for (size_t j = 0; j < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); j++)
							{
								tfbsm14.Reset();
								tfbsm14.MergeFrom(outDMST02.Tables["RESULT_TABLE"].Rows[j]);
								plan_id_1 = outDMST02.Tables["RESULT_TABLE"].Rows[j]["PLAN_ID"].ToString();
								f_back_c2 = outDMST02.Tables["RESULT_TABLE"].Rows[j]["BACK_C2"].ToString();
								Log::Trace("", __FUNCTION__, "== plan_id_1[{0}] ==", plan_id_1);
								if (tfbsm14["MAT_CODE"].ToString() == "PL0002" || tfbsm14["MAT_CODE"].ToString() == "PL0001" || tfbsm14["MAT_CODE"].ToString() == "PL0003")
								{
									// 除去这三个都取固定值
									sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='001' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
									cmd.SetCommandText(sqlstr);
									cmd.ExecuteReader();
									if (cmd.Read())
									{
										tfbsm14["C_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
										tfbsm14["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal() / 2;
									}
									cmd.Close();
									sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='005' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
									cmd.SetCommandText(sqlstr);
									cmd.ExecuteReader();
									if (cmd.Read())
									{
										tfbsm14["S_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
										tfbsm14["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal() / 2;
									}
									cmd.Close();
								}
								// C/S=(上限＋下限)/2
								if (plan_id_1 == plan_id)
								{
									// 代表他们两个是同一个配料单
									/*sqlstr = " select distinct  COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C from tfbsm12 where DATE_C='" + f_date + "' AND ST_NO='" + f_st_no + "' and BACKLOG_EA='" + f_backlog_ea + "' and PLAN_ID='" + plan_id + "'  ";
									cmd_inq.SetCommandText(sqlstr);
									cmd_inq.ExecuteReader();
									if (cmd_inq.Read()){
									tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
									tfbsm14["ST_NO"] = cmd_inq.GetString(2);
									tfbsm14["BACKLOG_EA"] = cmd_inq.GetString(3);
									tfbsm14["DATE_C"] = cmd_inq.GetString(4);
									}
									cmd_inq.Close();*/
									tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
									tfbsm14["ST_NO"] = f_st_no;
									tfbsm14["BACKLOG_EA"] = f_backlog_ea;
									tfbsm14["DATE_C"] = f_date;
									// tfbsm14.Print();
									tfbsm14["REC_CREATOR"] = s.userid;
									tfbsm14["REC_CREATE_TIME"] = datetime;
									tfbsm14["SEQ_NO"] = i + 1;
									tfbsm14["MATERIAL_CODE"] = f_material_code;
									tfbsm14.Print();
									tfbsm14.Insert();
								}
							}
						}
						if (i != 0 && outDMST02.Tables["WORK_TABLE"].Rows[i]["PLAN_ID"].ToString() != outDMST02.Tables["WORK_TABLE"].Rows[i - 1]["PLAN_ID"].ToString() && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0")
						{
							for (size_t x = 0; x < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); x++)
							{
								tfbsm14.Reset();
								tfbsm14.MergeFrom(outDMST02.Tables["RESULT_TABLE"].Rows[x]);
								plan_id_1 = outDMST02.Tables["RESULT_TABLE"].Rows[x]["PLAN_ID"].ToString();
								f_back_c2 = outDMST02.Tables["RESULT_TABLE"].Rows[x]["BACK_C2"].ToString();
								if (tfbsm14["MAT_CODE"].ToString() != "PL0002" && tfbsm14["MAT_CODE"].ToString() != "PL0001" && tfbsm14["MAT_CODE"].ToString() != "PL0003")
								{
									// 除去这三个都取固定值
									sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='001' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
									cmd.SetCommandText(sqlstr);
									cmd.ExecuteReader();
									if (cmd.Read())
									{
										tfbsm14["C_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
										tfbsm14["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal() / 2;
									}
									cmd.Close();
									sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='005' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
									cmd.SetCommandText(sqlstr);
									cmd.ExecuteReader();
									if (cmd.Read())
									{
										tfbsm14["S_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
										tfbsm14["S_VALUE"] = tfbsm14["C_VALUE"].ToDecimal() / 2;
									}
									cmd.Close();
								}
								// C/S=(上限＋下限)/2
								if (plan_id_1 == plan_id)
								{
									// 代表他们两个是同一个配料单
									/*sqlstr = " select distinct  COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C from tfbsm12 where DATE_C='" + f_date + "' AND ST_NO='" + f_st_no + "' and BACKLOG_EA='" + f_backlog_ea + "' and PLAN_ID='" + plan_id + "'  ";
									cmd_inq.SetCommandText(sqlstr);
									cmd_inq.ExecuteReader();
									if (cmd_inq.Read()){
									tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
									tfbsm14["ST_NO"] = cmd_inq.GetString(2);
									tfbsm14["BACKLOG_EA"] = cmd_inq.GetString(3);
									tfbsm14["DATE_C"] = cmd_inq.GetString(4);
									}
									cmd_inq.Close();*/
									tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
									tfbsm14["ST_NO"] = f_st_no;
									tfbsm14["BACKLOG_EA"] = f_backlog_ea;
									tfbsm14["DATE_C"] = f_date;
									if (tfbsm14["COMPOSE_LIST_NO"].ToString().Trim() == "")
									{
										tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
									}
									tfbsm14.Print();
									tfbsm14["SEQ_NO"] = i + 1;
									tfbsm14["MATERIAL_CODE"] = f_material_code;
									tfbsm14.Insert();
								}
							}
						}
					}
					// 针对18表的数据做存储操作
					for (size_t i = 0; i < inDMST02.Tables["MatTable"].Rows.get_Count(); i++)
					{
						tfbsm18.Reset();
						tfbsm18.MergeFrom(inDMST02.Tables["MatTable"].Rows[i]);
						tfbsm18["PRICE"] = inDMST02.Tables["MatTable"].Rows[i]["COST"].ToString();
						tfbsm18["STOCK_WT_QC"] = inDMST02.Tables["MatTable"].Rows[i]["STOCK_WT"].ToString();
						tfbsm18["DATE_C"] = date;
						tfbsm18["SEQ_NO"] = seq_no;
						tfbsm18["BACKLOG_EA"] = backlog_ea;
						tfbsm18["REC_CREATOR"] = s.userid;
						tfbsm18["REC_CREATE_TIME"] = datetime;
						tfbsm18["COMPOSE_LIST_NO"] = compose_list_no;
						tfbsm18.Insert();
					}
				}
			}
			else
			{
				Log::Trace("", __FUNCTION__, "row_lp.RET = [{0}]", ret);
				// 准备启动模型
				Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc start ==");
				// RUN_PLMODEL
				/*f_epex_call_rest_svc(conn, "FBSM_SSMODEL", "CHECK_INPUT", &inDMST02, &outDMST02_1);
				ei_sys the_s;
				outDMST02_1.GetSYS(&the_s);
				Log::Trace("", __FUNCTION__, "doFlag = [{0}]", doFlag);
				if (the_s.flag < 0)
				{
					Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc err[0] ==", the_s.msg);
					strcpy(s.msg, the_s.msg);
					throw CApplicationException(-1, s.msg, log.Location);
				}
				Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc MX ==");*/
				f_epex_call_rest_svc(conn, "FBSM_SSMODEL", "RUN_PLMODEL", &inDMST02, &outDMST02);
				Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc end ==");
				/*if (outDMST02.Tables["WORK_TABLE"].Rows.get_Count() == 0){
					strcpy(s.msg, "出钢记号:" + st_no + ",工艺路径:" + backlog_ea + "模型计算无解!!!");
					throw CApplicationException(-1, s.msg, log.Location);
					}*/
				// 循环存储报错信息 ERROR_INFO"
				Log::Trace("", __FUNCTION__, "ERROR_INFO = [{0}]", outDMST02.Tables["ERROR_INFO"].Rows.get_Count());
				for (size_t z = 0; z < outDMST02.Tables["ERROR_INFO"].Rows.get_Count(); z++)
				{
					Log::Trace("", __FUNCTION__, "ERROR_INFO== sqlstr[{0}] ==", __LINE__);
					sqlstr = " update tfbsm12 set REMARK_PS='" + outDMST02.Tables["ERROR_INFO"].Rows[z]["ERROR_MSG"].ToString() + "' where ST_NO='" + outDMST02.Tables["ERROR_INFO"].Rows[z]["ST_NO"].ToString() + "' AND BACKLOG_EA='" + outDMST02.Tables["ERROR_INFO"].Rows[z]["BACKLOG_EA"].ToString() + "' and FURNACE_COUNT='" + outDMST02.Tables["ERROR_INFO"].Rows[z]["SUM_FURNACES"].ToString() + "' and SEQ_NO='" + outDMST02.Tables["ERROR_INFO"].Rows[z]["SEQ_NO"].ToString() + "' ";
					Log::Trace("", __FUNCTION__, "ERROR_INFO== sqlstr[{0}] ==", sqlstr);
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.ExecuteNonQuery();
					cmd_inq.Close();
				}
				// 清空数据
				inDMST02.Tables["LG_PSSM"].Rows.Clear();
				Log::Trace("", __FUNCTION__, "== WORK_TABLE.row[{0}] ==", outDMST02.Tables["WORK_TABLE"].Rows.get_Count());
				for (size_t i = 0; i < outDMST02.Tables["WORK_TABLE"].Rows.get_Count(); i++)
				{
					CDecimal ts = 0; // 为了区分不同配料单的情况
					if (j == 0)
					{
						ts = bcls_rec->Tables["Main"].Rows[j]["SEQ_NO"].ToDecimal();
					}
					else
					{
						ts = i + 1;
					}
					Log::Trace("", __FUNCTION__, "== ts[{0}] ==", ts.ToString());
					if (outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() == "0")
					{
						sqlstr = " update tfbsm12 set COMPOSE_LIST_NO=' ',PLAN_ID=' '  where DATE_C='" + date + "' AND ST_NO='" + st_no + "' and BACKLOG_EA='" + backlog_ea + "' and SEQ_NO='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["SEQ_NO"].ToString() + "' ";
						Log::Trace("", __FUNCTION__, "WORK_TABLE== sqlstr[{0}] ==", sqlstr);
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.ExecuteNonQuery();
						cmd_inq.Close();
						continue;
					}
					// 钢种大类(A/M/P/D/F)+钢种小类编号+工艺路线编号+日期+2位流水
					plan_id = outDMST02.Tables["WORK_TABLE"].Rows[i]["PLAN_ID"].ToString();
					Log::Trace("", __FUNCTION__, "== date err[{0}] ==", date.SubstringNE(2, 6));
					Log::Trace("", __FUNCTION__, "== plan_id[{0}] ==", plan_id);
					f_material_code = outDMST02.Tables["WORK_TABLE"].Rows[i]["MATERIAL_CODE"].ToString();
					f_date = outDMST02.Tables["WORK_TABLE"].Rows[i]["DATE_C"].ToString();
					f_st_no = outDMST02.Tables["WORK_TABLE"].Rows[i]["ST_NO"].ToString();
					// BACKLOG_EA
					f_backlog_ea = outDMST02.Tables["WORK_TABLE"].Rows[i]["BACKLOG_EA"].ToString();
					// COMM_FMLY_CODE 大类代码
					f_comm_fmly_code = outDMST02.Tables["WORK_TABLE"].Rows[i]["COMM_FMLY_CODE"].ToString();
					Log::Trace("", __FUNCTION__, "== __LINE__[{0}] ==", __LINE__);
					// 获取出钢记号
					// 获取中类
					// AND MATERIAL_CODE=''
					if (i == 0 && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0")
					{
						// 默认是第一个配料单
						sqlstr = " select  max(substr(COMPOSE_LIST_NO,12,14)),max(substr(COMPOSE_LIST_NO,14,1)),max(substr(COMPOSE_LIST_NO,13, 1)),max(substr(COMPOSE_LIST_NO,13, 2)) from tfbsm14 where DATE_C='" + f_date + "'  and COMPOSE_LIST_NO!=' ' and MATERIAL_CODE='" + f_material_code + "'  and BACKLOG_EA='" + f_backlog_ea + "'   ";
						Log::Trace("", __FUNCTION__, "== sqlstr1[{0}] ==", sqlstr);
						cmd_inq.SetCommandText(sqlstr);
						cmd_inq.ExecuteReader();
						if (cmd_inq.Read())
						{
							if (cmd_inq.GetDecimal(3) == 0 && cmd_inq.GetDecimal(2) != 9)
							{
								p_seq = cmd_inq.GetDecimal(2);
								Log::Trace("", __FUNCTION__, "== p_seq1[{0}] ==", p_seq.ToString());
								p_seq = p_seq + 1;
								Log::Trace("", __FUNCTION__, "== p_seq1[{0}] ==", p_seq.ToString());
								compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "0" + p_seq.ToString();
							}
							else
							{
								p_seq = cmd_inq.GetDecimal(4);
								Log::Trace("", __FUNCTION__, "== p_seq2[{0}] ==", p_seq.ToString());
								p_seq = p_seq + 1;
								Log::Trace("", __FUNCTION__, "== p_seq2[{0}] ==", p_seq.ToString());
								compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + p_seq.ToString();
							}
						}
						else
						{
							Log::Trace("", __FUNCTION__, "== linke[{0}] ==", __LINE__);
							compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "01";
						}
						cmd_inq.Close();
					}
					else
					{
						if (outDMST02.Tables["WORK_TABLE"].Rows[i]["PLAN_ID"].ToString() != outDMST02.Tables["WORK_TABLE"].Rows[i - 1]["PLAN_ID"].ToString() && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0")
						{
							sqlstr = " select  max(substr(COMPOSE_LIST_NO,12,14)),max(substr(COMPOSE_LIST_NO,14,1)),max(substr(COMPOSE_LIST_NO,13, 1)),max(substr(COMPOSE_LIST_NO,13, 2)) from tfbsm14 where DATE_C='" + f_date + "'  and COMPOSE_LIST_NO!=' ' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "'   ";
							Log::Trace("", __FUNCTION__, "== sqlstr!![{0}] ==", sqlstr);
							cmd_inq.SetCommandText(sqlstr);
							cmd_inq.ExecuteReader();
							if (cmd_inq.Read())
							{
								if (cmd_inq.GetDecimal(3) == 0 && cmd_inq.GetDecimal(2) != 9)
								{
									p_seq = cmd_inq.GetDecimal(2);
									Log::Trace("", __FUNCTION__, "== p_seq2[{0}] ==", p_seq.ToString());
									p_seq = p_seq + 1;
									Log::Trace("", __FUNCTION__, "== p_seq3[{0}] ==", p_seq.ToString());
									compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "0" + p_seq.ToString();
								}
								else
								{
									p_seq = cmd_inq.GetDecimal(4);
									Log::Trace("", __FUNCTION__, "== p_seq2[{0}] ==", p_seq.ToString());
									p_seq = p_seq + 1;
									Log::Trace("", __FUNCTION__, "== p_seq4[{0}] ==", p_seq.ToString());
									compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + p_seq.ToString();
								}
							}
							else
							{
								Log::Trace("", __FUNCTION__, "== linke[{0}] ==", __LINE__);
								compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "01";
							}
							cmd_inq.Close();
						}
					}
					if (compose_list_no == " " || compose_list_no == "")
					{
						compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "01";
					}
					Log::Trace("", __FUNCTION__, "== compose_list_no[{0}] ==", compose_list_no);
					sqlstr = " update tfbsm12 set COMPOSE_LIST_NO='" + compose_list_no + "',PLAN_ID='" + plan_id + "',SINGLECOST='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["SINGLECOST"].ToString() + "',COST_DG='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["COST_DG"].ToString() + "' where DATE_C='" + f_date + "' AND ST_NO='" + f_st_no + "' and BACKLOG_EA='" + f_backlog_ea + "' and SEQ_NO='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["SEQ_NO"].ToString() + "' ";
					Log::Trace("", __FUNCTION__, "== sqlstr[{0}] ==", sqlstr);
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.ExecuteNonQuery();
					cmd_inq.Close();
					// 还原SI的值
					sqlstr = " select SI_PDI from tfbsm20 where COMM_FMLY_CODE='" + f_comm_fmly_code + "' and BACKLOG_EA='" + f_backlog_ea + "'  ";
					cmd_si_pdi.SetCommandText(sqlstr);
					cmd_si_pdi.ExecuteReader();
					Log::Trace("", __FUNCTION__, "== SI.sqlstr[{0}] ==", sqlstr);
					if (cmd_si_pdi.Read())
					{
						f_si_pdi = cmd_si_pdi.GetDecimal(1);
					}
					cmd_si_pdi.Close();
					Log::Trace("", __FUNCTION__, "== SI.f_si_pdi[{0}] ==", f_si_pdi.ToString());
					if (i == 0 && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0")
					{
						for (size_t x = 0; x < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); x++)
						{
							Log::Trace("", __FUNCTION__, "== MAT_CODE[{0}] ==", outDMST02.Tables["RESULT_TABLE"].Rows[x]["MAT_CODE"].ToString());
							Log::Trace("", __FUNCTION__, "== P_VALUE_1[{0}] ==", outDMST02.Tables["RESULT_TABLE"].Rows[x]["P_VALUE"].ToString());
							tfbsm14.Reset();
							tfbsm14.MergeFrom(outDMST02.Tables["RESULT_TABLE"].Rows[x]);
							plan_id_1 = outDMST02.Tables["RESULT_TABLE"].Rows[x]["PLAN_ID"].ToString();
							f_back_c2 = outDMST02.Tables["RESULT_TABLE"].Rows[x]["BACK_C2"].ToString();
							Log::Trace("", __FUNCTION__, "== plan_id_1[{0}] ==", plan_id_1);
							if (tfbsm14["MAT_CODE"].ToString() == "PL0002" || tfbsm14["MAT_CODE"].ToString() == "PL0001" || tfbsm14["MAT_CODE"].ToString() == "PL0003")
							{
								// 除去这三个都取固定值
								sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='001' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
								cmd.SetCommandText(sqlstr);
								cmd.ExecuteReader();
								if (cmd.Read())
								{
									tfbsm14["C_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
									tfbsm14["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal() / 2;
								}
								cmd.Close();
								sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='005' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
								cmd.SetCommandText(sqlstr);
								cmd.ExecuteReader();
								if (cmd.Read())
								{
									tfbsm14["S_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
									tfbsm14["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal() / 2;
								}
								cmd.Close();
							}
							// C/S=(上限＋下限)/2
							if (plan_id_1 == plan_id)
							{
								// 代表他们两个是同一个配料单
								/*sqlstr = " select distinct  COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C from tfbsm12 where DATE_C='" + f_date + "' AND ST_NO='" + f_st_no + "' and BACKLOG_EA='" + f_backlog_ea + "' and PLAN_ID='" + plan_id + "' and SEQ_NO='" + ts.ToString() + "'  ";
								cmd_inq.SetCommandText(sqlstr);
								cmd_inq.ExecuteReader();
								if (cmd_inq.Read()){
								tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
								tfbsm14["ST_NO"] = cmd_inq.GetString(2);
								tfbsm14["BACKLOG_EA"] = cmd_inq.GetString(3);
								tfbsm14["DATE_C"] = cmd_inq.GetString(4);
								}
								cmd_inq.Close();*/
								tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
								tfbsm14["ST_NO"] = f_st_no;
								tfbsm14["BACKLOG_EA"] = f_backlog_ea;
								tfbsm14["DATE_C"] = f_date;
								// tfbsm14.Print();
								tfbsm14["REC_CREATOR"] = s.userid;
								tfbsm14["REC_CREATE_TIME"] = datetime;
								tfbsm14["SEQ_NO"] = i + 1;
								tfbsm14["MATERIAL_CODE"] = f_material_code;
								tfbsm14.Print();
								tfbsm14.Insert();
							}
						}
						// ====== 插入还原硅铁（BACK_C2='9'）======
						Log::Trace("", __FUNCTION__, "A位置插入还原硅铁: WEIGHT=[{0}]", f_si_pdi.ToString());
						if (f_si_pdi > 0)
						{
							tfbsm14.Reset();
							tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
							tfbsm14["ST_NO"] = f_st_no;
							tfbsm14["BACKLOG_EA"] = f_backlog_ea;
							tfbsm14["DATE_C"] = f_date;
							tfbsm14["MATERIAL_CODE"] = f_material_code;
							tfbsm14["BACK_C2"] = "9";		  // 物料类型代码：还原硅铁
							tfbsm14["MAT_CODE"] = "AT000258"; // 硅铁
							tfbsm14["MAT_NAME"] = "硅铁 FeSi72Al2.0"; // 硅铁
							tfbsm14["STATION_ID"] = "A";
							tfbsm14["WEIGHT"] = f_si_pdi;

							sqlstr = " SELECT * FROM ( "
									 "   SELECT C_VALUE, SI_VALUE, MN_VALUE, CR_VALUE "
									 "   FROM TFBSM14 "
									 "   WHERE MAT_CODE = 'AT000258' AND STATION_ID = 'A' "
									 "   ORDER BY REC_CREATE_TIME DESC, DATE_C DESC "
									 " ) WHERE ROWNUM = 1 ";
							cmd.SetCommandText(sqlstr);
							cmd.ExecuteReader();
							if (cmd.Read())
							{
								tfbsm14["C_VALUE"] = cmd.GetDecimal(1);
								tfbsm14["SI_VALUE"] = cmd.GetDecimal(2);
								tfbsm14["MN_VALUE"] = cmd.GetDecimal(3);
								tfbsm14["CR_VALUE"] = cmd.GetDecimal(4);
							}
							cmd.Close();
							tfbsm14["REC_CREATOR"] = s.userid;
							tfbsm14["REC_CREATE_TIME"] = datetime;
							tfbsm14["SEQ_NO"] = i + 1;
							tfbsm14.Insert();
							Log::Trace("", __FUNCTION__, "插入还原硅铁: BACK_C2=9, WEIGHT=[{0}]", f_si_pdi.ToString());
						}
						// ====== 结束 ======
					}

					if (i != 0 && outDMST02.Tables["WORK_TABLE"].Rows[i]["PLAN_ID"].ToString() != outDMST02.Tables["WORK_TABLE"].Rows[i - 1]["PLAN_ID"].ToString() && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0")
					{
						for (size_t x = 0; x < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); x++)
						{
							Log::Trace("", __FUNCTION__, "== MAT_CODE[{0}] ==", outDMST02.Tables["RESULT_TABLE"].Rows[x]["MAT_CODE"].ToString());
							Log::Trace("", __FUNCTION__, "== P_VALUE_1[{0}] ==", outDMST02.Tables["RESULT_TABLE"].Rows[x]["P_VALUE"].ToString());
							tfbsm14.Reset();
							tfbsm14.MergeFrom(outDMST02.Tables["RESULT_TABLE"].Rows[x]);
							plan_id_1 = outDMST02.Tables["RESULT_TABLE"].Rows[x]["PLAN_ID"].ToString();
							f_back_c2 = outDMST02.Tables["RESULT_TABLE"].Rows[x]["BACK_C2"].ToString();
							Log::Trace("", __FUNCTION__, "== plan_id_1[{0}] ==", plan_id_1);
							if (tfbsm14["MAT_CODE"].ToString() == "PL0002" || tfbsm14["MAT_CODE"].ToString() == "PL0001" || tfbsm14["MAT_CODE"].ToString() == "PL0003")
							{
								// 除去这三个都取固定值
								sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='001' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
								cmd.SetCommandText(sqlstr);
								cmd.ExecuteReader();
								if (cmd.Read())
								{
									tfbsm14["C_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
									tfbsm14["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal() / 2;
								}
								cmd.Close();
								sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='005' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
								cmd.SetCommandText(sqlstr);
								cmd.ExecuteReader();
								if (cmd.Read())
								{
									tfbsm14["S_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
									tfbsm14["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal() / 2;
								}
								cmd.Close();
							}
							// C/S=(上限＋下限)/2
							if (plan_id_1 == plan_id)
							{
								// 代表他们两个是同一个配料单
								/*sqlstr = " select distinct  COMPOSE_LIST_NO,ST_NO,BACKLOG_EA,DATE_C from tfbsm12 where DATE_C='" + f_date + "' AND ST_NO='" + f_st_no + "' and BACKLOG_EA='" + f_backlog_ea + "' and PLAN_ID='" + plan_id + "' and SEQ_NO='" + ts.ToString() + "'  ";
								cmd_inq.SetCommandText(sqlstr);
								cmd_inq.ExecuteReader();
								if (cmd_inq.Read()){
								tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
								tfbsm14["ST_NO"] = f_st_no;
								tfbsm14["BACKLOG_EA"] = f_backlog_ea;
								tfbsm14["DATE_C"] = f_date;
								}
								cmd_inq.Close();*/
								tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
								tfbsm14["ST_NO"] = f_st_no;
								tfbsm14["BACKLOG_EA"] = f_backlog_ea;
								tfbsm14["DATE_C"] = f_date;
								// tfbsm14.Print();
								tfbsm14["REC_CREATOR"] = s.userid;
								tfbsm14["REC_CREATE_TIME"] = datetime;
								tfbsm14["SEQ_NO"] = i + 1;
								tfbsm14["MATERIAL_CODE"] = f_material_code;
								tfbsm14.Print();
								tfbsm14.Insert();
							}
						}
						// ====== 插入还原硅铁（BACK_C2='9'）======
						Log::Trace("", __FUNCTION__, "B位置插入还原硅铁: WEIGHT=[{0}]", f_si_pdi.ToString());
						if (f_si_pdi > 0)
						{
							tfbsm14.Reset();
							tfbsm14["COMPOSE_LIST_NO"] = compose_list_no;
							tfbsm14["ST_NO"] = f_st_no;
							tfbsm14["BACKLOG_EA"] = f_backlog_ea;
							tfbsm14["DATE_C"] = f_date;
							tfbsm14["MATERIAL_CODE"] = f_material_code;
							tfbsm14["BACK_C2"] = "9";		  // 物料类型代码：还原硅铁
							tfbsm14["MAT_CODE"] = "AT000258"; // 硅铁
							tfbsm14["STATION_ID"] = "A";
							tfbsm14["WEIGHT"] = f_si_pdi;

							sqlstr = " SELECT * FROM ( "
									 "   SELECT C_VALUE, SI_VALUE, MN_VALUE, CR_VALUE "
									 "   FROM TFBSM14 "
									 "   WHERE MAT_CODE = 'AT000258' AND STATION_ID = 'A' "
									 "   ORDER BY REC_CREATE_TIME DESC, DATE_C DESC "
									 " ) WHERE ROWNUM = 1 ";
							cmd.SetCommandText(sqlstr);
							cmd.ExecuteReader();
							if (cmd.Read())
							{
								tfbsm14["C_VALUE"] = cmd.GetDecimal(1);
								tfbsm14["SI_VALUE"] = cmd.GetDecimal(2);
								tfbsm14["MN_VALUE"] = cmd.GetDecimal(3);
								tfbsm14["CR_VALUE"] = cmd.GetDecimal(4);
							}
							cmd.Close();
							tfbsm14["REC_CREATOR"] = s.userid;
							tfbsm14["REC_CREATE_TIME"] = datetime;
							tfbsm14["SEQ_NO"] = i + 1;
							tfbsm14.Insert();
							Log::Trace("", __FUNCTION__, "插入还原硅铁: BACK_C2=9, WEIGHT=[{0}]", f_si_pdi.ToString());
						}
						// ====== 结束 ======
					}
				}
				// 针对18表的数据做存储操作
				Log::Trace("", __FUNCTION__, "link = [{0}]", __LINE__);
				for (size_t i = 0; i < inDMST02.Tables["MatTable"].Rows.get_Count(); i++)
				{
					tfbsm18.Reset();
					tfbsm18.MergeFrom(inDMST02.Tables["MatTable"].Rows[i]);
					tfbsm18["PRICE"] = inDMST02.Tables["MatTable"].Rows[i]["COST"].ToString();
					tfbsm18["STOCK_WT_QC"] = inDMST02.Tables["MatTable"].Rows[i]["STOCK_WT"].ToString();
					tfbsm18["DATE_C"] = date;
					tfbsm18["SEQ_NO"] = seq_no;
					tfbsm18["BACKLOG_EA"] = backlog_ea;
					tfbsm18["REC_CREATOR"] = s.userid;
					tfbsm18["REC_CREATE_TIME"] = datetime;
					tfbsm18["COMPOSE_LIST_NO"] = compose_list_no;
					/*tfbsm18.Print();*/
					tfbsm18.Insert();
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
