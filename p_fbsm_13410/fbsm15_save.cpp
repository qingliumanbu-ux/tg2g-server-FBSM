/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/10/16
Description: 不锈钢钢种配料维护
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15_save)
//模型计算
void f_epex_call_rest_svc(CDbConnection* conn, const CString& system_code, const CString& svc_name, EIClass* blks_in, EIClass * blks_out);
int f_mmsm_getprice(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn);//价格计算

int f_fbsm15_save(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString compose_list_no_l = " ";
	int ret = 0;
	CString st_no = " ";//出钢记号
	CString backlog_ea = " ";//工艺路线
	CString date = " ";//日期
	CString material_code = " ";//中类代码
	CString comm_fmly_code = " ";//大类代码 
	CString f_material_code = " ";//模型返回的中类代码
	CString f_st_no = " ";//模型返回的中类代码
	CString f_date = " ";//返回日期
	CString seq_no = " ";//序号
	CDecimal f_count = 0;//总炉数
	CDecimal fu_count = 0;
	CString s_matear_code = " ";
	CString compose_list_no = " ";//配料单号
	CDecimal f_si_pdi = 0;
	CString plan_id = "";
	CString plan_id_1 = "";
	CString f_backlog_ea = " ";//返回工艺路线
	CString f_back_c2 = " ";
	CDecimal p_seq = 0;
	CString sql_k = "";
	CString f_comm_fmly_code = " ";
	CDbCommand cmd(conn);
	CModel tfbsm14("TFBSM14");
	CModel tfbsm18("TFBSM18");
	CModel tfbsm14_copy("TFBSM14");
	CModel tfbsm01("TFBSM01");
	CModel tfbsm13("TFBSM13");
	CDbCommand cmd_si_pdi(conn);
	CDbCommand cmd_k(conn);
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	//模型定义
	EIClass inDMST02; //传入类
	EIClass outDMST02;	//返回类
	try
	{
		inDMST02.Tables[0].set_TableName("DLLINFO");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "DLLName");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "ClassName");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "MethodName");
		CDataRow& row = inDMST02.Tables["DLLINFO"].Rows.Add();
		//这三个值待定
		row["DLLName"] = "TGPLModel.dll";
		row["ClassName"] = "TGPLModel.TGPLModel";
		row["MethodName"] = "CalForPLPlan";

		//配料单信息
		inDMST02.Tables.Add("LG_PSSM");
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "BACKLOG_EA"); //工艺路径
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "ST_NO"); //出钢记号
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "MATERIAL_CODE"); //中类代码
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "DATE_C"); //日期
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "FURNACE_COUNT");//总炉数
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "SEQ_NO");//序号
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
		inDMST02.Tables["GX_MAT"].Columns.Add(DT_STRING, "YEILD_MINUS"); //金属收得率
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
		//价格计算
		EIClass bcls_rec_rep;
		EIClass bcls_ret_rep;
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_CODE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_NAME");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "CR_VALUE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "NI_VALUE");
		bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MO_VALUE");
		bcls_rec_rep.Tables[0].Rows.Add();
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
		//配料单信息 LG_PSSM
		sqlstr = " select SEQ_NO from tfbsm12 where COMPOSE_LIST_NO='" + compose_list_no_l + "' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		while  (cmd_inq.Read())
		{
			CDataRow& row1 = inDMST02.Tables["LG_PSSM"].Rows.Add();
			row1["ST_NO"] = st_no;
			row1["BACKLOG_EA"] = backlog_ea;
			//不需要了
			row1["FURNACE_COUNT"] = "1";
			row1["DATE_C"] = date;
			row1["MATERIAL_CODE"] = material_code;
			row1["SEQ_NO"] = cmd_inq.GetString(1);
		}
		cmd_inq.Close();
		//根据出钢记号去查产品对应关系
		sqlstr = " select COMM_FMLY_CODE,MATERIAL_CODE from TFBSM11 where ST_NO='" + st_no + "' ";
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			comm_fmly_code = cmd_inq.GetString(1);
		}
		cmd_inq.Close();
		//MatTable(原料库存及成分)
		for (size_t j = 0; j < bcls_rec->Tables["X1"].Rows.get_Count(); j++)
		{
			if (bcls_rec->Tables["X1"].Rows[j]["LOT_NO"].ToString() != " " && bcls_rec->Tables["X1"].Rows[j]["LOT_NO"].ToString() != ""){
				//有试批号的按照试批号查询
				sqlstr = " SELECT "
					" LOT_NO,'" + st_no + "' as st_no,MAT_CODE,MAT_NAME,STOCK_NAME,WEIGHT/1000   as STOCK_WT_QC,BACK_C2, "
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
					" WHEN t.ST_NO = '" + st_no + "' THEN 1 WHEN t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "' THEN 2 "
					" WHEN t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 3 WHEN t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "' THEN 4 "
					" WHEN t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 5 ELSE 99   END "
					" ) as rn "
					" FROM TFBSM01 t "
					" WHERE "
					" t.ST_NO = '" + st_no + "' OR (t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') OR (t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') "
					" ) t "
					" WHERE rn = 1 "
					" GROUP BY t.MAT_CODE, t.MAT_NAME, t.MATERIAL_CODE, t.ST_NO, t.BACK_C2   "
					" ) B ON A.MAT_CODE = B.MAT_CODE "
					" ) WHERE MAT_NAME!=' '  AND BACK_C2!='2' and  LOT_NO='" + bcls_rec->Tables["X1"].Rows[j]["LOT_NO"].ToString() + "' ";
			}
			else{
				//按照物料代码查询
				sqlstr = " SELECT "
					" LOT_NO,'" + st_no + "' as st_no,MAT_CODE,MAT_NAME,STOCK_NAME,WEIGHT/1000   as STOCK_WT_QC,BACK_C2, "
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
					" WHEN t.ST_NO = '" + st_no + "' THEN 1 WHEN t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "' THEN 2 "
					" WHEN t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 3 WHEN t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "' THEN 4 "
					" WHEN t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 5 ELSE 99   END "
					" ) as rn "
					" FROM TFBSM01 t "
					" WHERE "
					" t.ST_NO = '" + st_no + "' OR (t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') OR (t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') "
					" ) t "
					" WHERE rn = 1 "
					" GROUP BY t.MAT_CODE, t.MAT_NAME, t.MATERIAL_CODE, t.ST_NO, t.BACK_C2   "
					" ) B ON A.MAT_CODE = B.MAT_CODE "
					" ) WHERE MAT_NAME!=' '  AND BACK_C2!='2' and  MAT_CODE='" + bcls_rec->Tables["X1"].Rows[j]["MAT_CODE"].ToString() + "' ";
			}
			Log::Trace("", __FUNCTION__, "sqlstr.sqlstr = [{0}]", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				tfbsm18.Reset();
				cmd_inq.Fetch(tfbsm18);
				tfbsm18.TrimOrBlank();
				tfbsm18["ST_NO"] = st_no;
				CDataRow& row = inDMST02.Tables["MatTable"].Rows.Add();
				Log::Trace("", __FUNCTION__, "linke = [{0}]", __LINE__);
				row["LOT_NO"] = tfbsm18["LOT_NO"].ToString().Trim();
				//Log::Trace("", __FUNCTION__, "MatTable.LOT_NO.sqlstr = [{0}]", row["LOT_NO"].ToString());
				row["ST_NO"] = tfbsm18["ST_NO"].ToString().Trim();
				row["MAT_CODE"] = tfbsm18["MAT_CODE"].ToString().Trim();
				row["MAT_NAME"] = tfbsm18["MAT_NAME"].ToString().Trim();
				//row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
				row["STOCK_WT"] = tfbsm18["STOCK_WT_QC"].ToString().Trim();
				row["BACK_C2"] = tfbsm18["BACK_C2"].ToString().Trim();
				row["C_VALUE"] = tfbsm18["C_VALUE"].ToString().Trim();
				row["SI_VALUE"] = tfbsm18["SI_VALUE"].ToString().Trim();
				row["MN_VALUE"] = tfbsm18["MN_VALUE"].ToString().Trim();
				row["P_VALUE"] = tfbsm18["P_VALUE"].ToString().Trim();
				row["S_VALUE"] = tfbsm18["S_VALUE"].ToString().Trim();
				row["CR_VALUE"] = tfbsm18["CR_VALUE"].ToString().Trim();
				row["NI_VALUE"] = tfbsm18["NI_VALUE"].ToString().Trim();
				row["MO_VALUE"] = tfbsm18["MO_VALUE"].ToString().Trim();
				row["CU_VALUE"] = tfbsm18["CU_VALUE"].ToString().Trim();
				row["CO_VALUE"] = tfbsm18["CO_VALUE"].ToString().Trim();
				row["TI_VALUE"] = tfbsm18["TI_VALUE"].ToString().Trim();
				row["NB_VALUE"] = tfbsm18["NB_VALUE"].ToString().Trim();
				row["AL_VALUE"] = tfbsm18["AL_VALUE"].ToString().Trim();
				//STOCK_WT_QC
				Log::Trace("", __FUNCTION__, "STOCK_WT_QC = [{0}]", bcls_rec->Tables["X1"].Rows[j]["STOCK_WT_QC"].ToString());
				Log::Trace("", __FUNCTION__, "STOCK_WT = [{0}]", row["STOCK_WT"].ToString());
				//去查找库
				if (tfbsm18["LOT_NO"].ToString().Trim() != ""){
					sql_k = " SELECT MAT_NAME FROM TMMSM60 WHERE BUNKER_NO in (SELECT BUNKER_NO   FROM TMMSM85 WHERE  MAT_CODE='" + tfbsm18["MAT_CODE"].ToString().Trim() + "' AND LOT_NO='" + tfbsm18["LOT_NO"].ToString().Trim() + "' and mat_name like '%镍生铁%' ) ";
					cmd_k.SetCommandText(sql_k);
					cmd_k.ExecuteReader();
					if (cmd_k.Read()){
						if (cmd_k.GetString(1) != " " && cmd_k.GetString(1) != ""){
							tfbsm18["STOCK_NAME"] = cmd_k.GetString(1);
							row["STOCK_NAME"] = cmd_k.GetString(1);
						}
						else{
							row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
						}
					}
					cmd_k.Close();
				}
				else{
					row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
				}
				if (row["STOCK_NAME"].ToString().Trim() == ""){
					row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
				}
				//价格计算
				if (tfbsm14["MAT_CODE"].ToString() != "PL0002" && tfbsm14["MAT_CODE"].ToString() != "PL0001" && tfbsm14["MAT_CODE"].ToString() != "PL0003"){
					bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = tfbsm18["MAT_CODE"];
					bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = tfbsm18["MAT_NAME"];
					bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = tfbsm18["CR_VALUE"];
					bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = tfbsm18["NI_VALUE"];
					bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = tfbsm18["MO_VALUE"];
					//Log::Trace("", __FUNCTION__, "MatTable.MatTable.CR_VALUE = [{0}]", bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"].ToString());
					doFlag = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
					if (doFlag < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					Log::Trace("", __FUNCTION__, "linke = [{0}]", __LINE__);
					tfbsm18["PRICE"] = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"];
					Log::Trace("", __FUNCTION__, "linke = [{0}]", __LINE__);
					row["COST"] = tfbsm18["PRICE"].ToString().Trim();
					tfbsm18["DATE_C"] = date;
					Log::Trace("", __FUNCTION__, "linke = [{0}]", __LINE__);
					tfbsm18["SEQ_NO"] = seq_no;
					tfbsm18["BACKLOG_EA"] = backlog_ea;
					tfbsm18["REC_CREATOR"] = s.userid;
					tfbsm18["REC_CREATE_TIME"] = datetime;
					Log::Trace("", __FUNCTION__, "linke = [{0}]", __LINE__);
				}
			}
			cmd_inq.Close();
		}
		for (size_t j = 0; j < bcls_rec->Tables["X2"].Rows.get_Count(); j++)
		{
			if (bcls_rec->Tables["X2"].Rows[j]["LOT_NO"].ToString() != " " && bcls_rec->Tables["X2"].Rows[j]["LOT_NO"].ToString() != ""){
				//有试批号的按照试批号查询
				sqlstr = " SELECT "
					" LOT_NO,'" + st_no + "' as st_no,MAT_CODE,MAT_NAME,STOCK_NAME,WEIGHT/1000   as STOCK_WT_QC,BACK_C2, "
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
					" WHEN t.ST_NO = '" + st_no + "' THEN 1 WHEN t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "' THEN 2 "
					" WHEN t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 3 WHEN t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "' THEN 4 "
					" WHEN t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 5 ELSE 99   END "
					" ) as rn "
					" FROM TFBSM01 t "
					" WHERE "
					" t.ST_NO = '" + st_no + "' OR (t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') OR (t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') "
					" ) t "
					" WHERE rn = 1 "
					" GROUP BY t.MAT_CODE, t.MAT_NAME, t.MATERIAL_CODE, t.ST_NO, t.BACK_C2   "
					" ) B ON A.MAT_CODE = B.MAT_CODE "
					" ) WHERE MAT_NAME!=' '  AND BACK_C2!='2' and  LOT_NO='" + bcls_rec->Tables["X2"].Rows[j]["LOT_NO"].ToString() + "' ";
			}
			else{
				//按照物料代码查询
				sqlstr =" SELECT "
					" LOT_NO,'" + st_no + "' as st_no,MAT_CODE,MAT_NAME,STOCK_NAME,WEIGHT/1000   as STOCK_WT_QC,BACK_C2, "
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
					" WHEN t.ST_NO = '" + st_no + "' THEN 1 WHEN t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "' THEN 2 "
					" WHEN t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 3 WHEN t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "' THEN 4 "
					" WHEN t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 5 ELSE 99   END "
					" ) as rn "
					" FROM TFBSM01 t "
					" WHERE "
					" t.ST_NO = '" + st_no + "' OR (t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') OR (t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') "
					" ) t "
					" WHERE rn = 1 "
					" GROUP BY t.MAT_CODE, t.MAT_NAME, t.MATERIAL_CODE, t.ST_NO, t.BACK_C2   "
					" ) B ON A.MAT_CODE = B.MAT_CODE "
					" ) WHERE MAT_NAME!=' '  AND BACK_C2!='2' and  MAT_CODE='" + bcls_rec->Tables["X2"].Rows[j]["MAT_CODE"].ToString() + "' ";
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				tfbsm18.Reset();
				cmd_inq.Fetch(tfbsm18);
				tfbsm18.TrimOrBlank();
				tfbsm18["ST_NO"] = st_no;
				CDataRow& row = inDMST02.Tables["MatTable"].Rows.Add();
				row["LOT_NO"] = tfbsm18["LOT_NO"].ToString().Trim();
				//Log::Trace("", __FUNCTION__, "MatTable.LOT_NO.sqlstr = [{0}]", row["LOT_NO"].ToString());
				row["ST_NO"] = tfbsm18["ST_NO"].ToString().Trim();
				row["MAT_CODE"] = tfbsm18["MAT_CODE"].ToString().Trim();
				row["MAT_NAME"] = tfbsm18["MAT_NAME"].ToString().Trim();
				//row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
				//去查找库
				if (tfbsm18["LOT_NO"].ToString().Trim() != ""){
					sql_k = " SELECT MAT_NAME FROM TMMSM60 WHERE BUNKER_NO in (SELECT BUNKER_NO   FROM TMMSM85 WHERE  MAT_CODE='" + tfbsm18["MAT_CODE"].ToString().Trim() + "' AND LOT_NO='" + tfbsm18["LOT_NO"].ToString().Trim() + "' and mat_name like '%镍生铁%' ) ";
					cmd_k.SetCommandText(sql_k);
					cmd_k.ExecuteReader();
					if (cmd_k.Read()){
						if (cmd_k.GetString(1) != " " && cmd_k.GetString(1) != ""){
							tfbsm18["STOCK_NAME"] = cmd_k.GetString(1);
							row["STOCK_NAME"] = cmd_k.GetString(1);
						}
						else{
							row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
						}
					}
					cmd_k.Close();
				}
				else{
					row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
				}
				if (row["STOCK_NAME"].ToString().Trim() == ""){
					row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
				}
				row["STOCK_WT"] = tfbsm18["STOCK_WT_QC"].ToString().Trim();
				row["BACK_C2"] = tfbsm18["BACK_C2"].ToString().Trim();
				row["C_VALUE"] = tfbsm18["C_VALUE"].ToString().Trim();
				row["SI_VALUE"] = tfbsm18["SI_VALUE"].ToString().Trim();
				row["MN_VALUE"] = tfbsm18["MN_VALUE"].ToString().Trim();
				row["P_VALUE"] = tfbsm18["P_VALUE"].ToString().Trim();
				row["S_VALUE"] = tfbsm18["S_VALUE"].ToString().Trim();
				row["CR_VALUE"] = tfbsm18["CR_VALUE"].ToString().Trim();
				row["NI_VALUE"] = tfbsm18["NI_VALUE"].ToString().Trim();
				row["MO_VALUE"] = tfbsm18["MO_VALUE"].ToString().Trim();
				row["CU_VALUE"] = tfbsm18["CU_VALUE"].ToString().Trim();
				row["CO_VALUE"] = tfbsm18["CO_VALUE"].ToString().Trim();
				row["TI_VALUE"] = tfbsm18["TI_VALUE"].ToString().Trim();
				row["NB_VALUE"] = tfbsm18["NB_VALUE"].ToString().Trim();
				row["AL_VALUE"] = tfbsm18["AL_VALUE"].ToString().Trim();
				//STOCK_WT_QC
				Log::Trace("", __FUNCTION__, "STOCK_WT_QC = [{0}]", bcls_rec->Tables["X2"].Rows[j]["STOCK_WT_QC"].ToString());
				Log::Trace("", __FUNCTION__, "STOCK_WT = [{0}]", row["STOCK_WT"].ToString());
				//价格计算
				if (tfbsm14["MAT_CODE"].ToString() != "PL0002" && tfbsm14["MAT_CODE"].ToString() != "PL0001" && tfbsm14["MAT_CODE"].ToString() != "PL0003"){
					bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = tfbsm18["MAT_CODE"];
					bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = tfbsm18["MAT_NAME"];
					bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = tfbsm18["CR_VALUE"];
					bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = tfbsm18["NI_VALUE"];
					bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = tfbsm18["MO_VALUE"];
					//Log::Trace("", __FUNCTION__, "MatTable.MatTable.CR_VALUE = [{0}]", bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"].ToString());
					doFlag = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
					if (doFlag < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					tfbsm18["PRICE"] = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"];
					row["COST"] = tfbsm18["PRICE"].ToString().Trim();
					tfbsm18["DATE_C"] = date;
					tfbsm18["SEQ_NO"] = seq_no;
					tfbsm18["BACKLOG_EA"] = backlog_ea;
					tfbsm18["REC_CREATOR"] = s.userid;
					tfbsm18["REC_CREATE_TIME"] = datetime;
				}
			}
			cmd_inq.Close();
		}
		for (size_t j = 0; j < bcls_rec->Tables["X3"].Rows.get_Count(); j++)
		{
			if (bcls_rec->Tables["X3"].Rows[j]["LOT_NO"].ToString() != " " && bcls_rec->Tables["X3"].Rows[j]["LOT_NO"].ToString() != ""){
				//有试批号的按照试批号查询
				sqlstr = " SELECT "
					" LOT_NO,'" + st_no + "' as st_no,MAT_CODE,MAT_NAME,STOCK_NAME,WEIGHT/1000   as STOCK_WT_QC,BACK_C2, "
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
					" WHEN t.ST_NO = '" + st_no + "' THEN 1 WHEN t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "' THEN 2 "
					" WHEN t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 3 WHEN t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "' THEN 4 "
					" WHEN t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 5 ELSE 99   END "
					" ) as rn "
					" FROM TFBSM01 t "
					" WHERE "
					" t.ST_NO = '" + st_no + "' OR (t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') OR (t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') "
					" ) t "
					" WHERE rn = 1 "
					" GROUP BY t.MAT_CODE, t.MAT_NAME, t.MATERIAL_CODE, t.ST_NO, t.BACK_C2   "
					" ) B ON A.MAT_CODE = B.MAT_CODE "
					" ) WHERE MAT_NAME!=' '  AND BACK_C2!='2' and  LOT_NO='" + bcls_rec->Tables["X3"].Rows[j]["LOT_NO"].ToString() + "' ";
			}
			else{
				//按照物料代码查询
				sqlstr =" SELECT "
					" LOT_NO,'" + st_no + "' as st_no,MAT_CODE,MAT_NAME,STOCK_NAME,WEIGHT/1000   as STOCK_WT_QC,BACK_C2, "
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
					" WHEN t.ST_NO = '" + st_no + "' THEN 1 WHEN t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "' THEN 2 "
					" WHEN t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 3 WHEN t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "' THEN 4 "
					" WHEN t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 5 ELSE 99   END "
					" ) as rn "
					" FROM TFBSM01 t "
					" WHERE "
					" t.ST_NO = '" + st_no + "' OR (t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') OR (t.MARK_POS_CODE = '3' AND t.MATERIAL_CODE = '" + material_code + "') "
					" OR (t.MARK_POS_CODE = '3' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "') "
					" ) t "
					" WHERE rn = 1 "
					" GROUP BY t.MAT_CODE, t.MAT_NAME, t.MATERIAL_CODE, t.ST_NO, t.BACK_C2   "
					" ) B ON A.MAT_CODE = B.MAT_CODE "
					" ) WHERE MAT_NAME!=' '  AND BACK_C2!='2' and MAT_CODE='" + bcls_rec->Tables["X3"].Rows[j]["MAT_CODE"].ToString() + "' ";
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				tfbsm18.Reset();
				cmd_inq.Fetch(tfbsm18);
				tfbsm18.TrimOrBlank();
				tfbsm18["ST_NO"] = st_no;
				CDataRow& row = inDMST02.Tables["MatTable"].Rows.Add();
				row["LOT_NO"] = tfbsm18["LOT_NO"].ToString().Trim();
				//Log::Trace("", __FUNCTION__, "MatTable.LOT_NO.sqlstr = [{0}]", row["LOT_NO"].ToString());
				row["ST_NO"] = tfbsm18["ST_NO"].ToString().Trim();
				row["MAT_CODE"] = tfbsm18["MAT_CODE"].ToString().Trim();
				row["MAT_NAME"] = tfbsm18["MAT_NAME"].ToString().Trim();
				//row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
				//去查找库
				if (tfbsm18["LOT_NO"].ToString().Trim() != ""){
					sql_k = " SELECT MAT_NAME FROM TMMSM60 WHERE BUNKER_NO in (SELECT BUNKER_NO   FROM TMMSM85 WHERE  MAT_CODE='" + tfbsm18["MAT_CODE"].ToString().Trim() + "' AND LOT_NO='" + tfbsm18["LOT_NO"].ToString().Trim() + "' and mat_name like '%镍生铁%' ) ";
					cmd_k.SetCommandText(sql_k);
					cmd_k.ExecuteReader();
					if (cmd_k.Read()){
						if (cmd_k.GetString(1) != " " && cmd_k.GetString(1) != ""){
							tfbsm18["STOCK_NAME"] = cmd_k.GetString(1);
							row["STOCK_NAME"] = cmd_k.GetString(1);
						}
						else{
							row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
						}
					}
					cmd_k.Close();
				}
				else{
					row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
				}
				if (row["STOCK_NAME"].ToString().Trim() == ""){
					row["STOCK_NAME"] = tfbsm18["STOCK_NAME"].ToString().Trim();
				}
				row["STOCK_WT"] = tfbsm18["STOCK_WT_QC"].ToString().Trim();
				row["BACK_C2"] = tfbsm18["BACK_C2"].ToString().Trim();
				row["C_VALUE"] = tfbsm18["C_VALUE"].ToString().Trim();
				row["SI_VALUE"] = tfbsm18["SI_VALUE"].ToString().Trim();
				row["MN_VALUE"] = tfbsm18["MN_VALUE"].ToString().Trim();
				row["P_VALUE"] = tfbsm18["P_VALUE"].ToString().Trim();
				row["S_VALUE"] = tfbsm18["S_VALUE"].ToString().Trim();
				row["CR_VALUE"] = tfbsm18["CR_VALUE"].ToString().Trim();
				row["NI_VALUE"] = tfbsm18["NI_VALUE"].ToString().Trim();
				row["MO_VALUE"] = tfbsm18["MO_VALUE"].ToString().Trim();
				row["CU_VALUE"] = tfbsm18["CU_VALUE"].ToString().Trim();
				row["CO_VALUE"] = tfbsm18["CO_VALUE"].ToString().Trim();
				row["TI_VALUE"] = tfbsm18["TI_VALUE"].ToString().Trim();
				row["NB_VALUE"] = tfbsm18["NB_VALUE"].ToString().Trim();
				row["AL_VALUE"] = tfbsm18["AL_VALUE"].ToString().Trim();
				//STOCK_WT_QC
				Log::Trace("", __FUNCTION__, "STOCK_WT_QC = [{0}]", bcls_rec->Tables["X3"].Rows[j]["STOCK_WT_QC"].ToString());
				Log::Trace("", __FUNCTION__, "STOCK_WT = [{0}]", row["STOCK_WT"].ToString());
				//价格计算
				//价格计算
				if (tfbsm14["MAT_CODE"].ToString() != "PL0002" && tfbsm14["MAT_CODE"].ToString() != "PL0001" && tfbsm14["MAT_CODE"].ToString() != "PL0003"){
					bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = tfbsm18["MAT_CODE"];
					bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = tfbsm18["MAT_NAME"];
					bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = tfbsm18["CR_VALUE"];
					bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = tfbsm18["NI_VALUE"];
					bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = tfbsm18["MO_VALUE"];
					//Log::Trace("", __FUNCTION__, "MatTable.MatTable.CR_VALUE = [{0}]", bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"].ToString());
					doFlag = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
					if (doFlag < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					tfbsm18["PRICE"] = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"];
					row["COST"] = tfbsm18["PRICE"].ToString().Trim();
					tfbsm18["DATE_C"] = date;
					tfbsm18["SEQ_NO"] = seq_no;
					tfbsm18["BACKLOG_EA"] = backlog_ea;
					tfbsm18["REC_CREATOR"] = s.userid;
					tfbsm18["REC_CREATE_TIME"] = datetime;
				}
			}
			cmd_inq.Close();
		}
		ret = inDMST02.Tables["MatTable"].Rows.get_Count();
		Log::Trace("", __FUNCTION__, "inDMST02.MatTable.sqlstr = [{0}]", ret);
		// 判断是否是脱磷或者三脱
		if (st_no.SubstringNE(1, 1) == "F" || st_no.SubstringNE(1, 1) == "M"){
			sqlstr = " SELECT * FROM TFBSM13 WHERE BACKLOG_EA='" + backlog_ea + "' ";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read()){
				cmd_inq.Fetch(tfbsm13);
				inDMST02.Tables["MatTable"].Rows.Add();
				inDMST02.Tables["MatTable"].Rows[ret]["LOT_NO"] = " ";
				inDMST02.Tables["MatTable"].Rows[ret]["BACK_C2"] = " ";
				inDMST02.Tables["MatTable"].Rows[ret]["ST_NO"] = st_no;
				inDMST02.Tables["MatTable"].Rows[ret]["STOCK_NAME"] = " ";
				inDMST02.Tables["MatTable"].Rows[ret]["STOCK_WT"] = "9999";
				inDMST02.Tables["MatTable"].Rows[ret]["MAT_CODE"] = "TS0000";
				inDMST02.Tables["MatTable"].Rows[ret]["MAT_NAME"] = "普通铁水";
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
		//WORKTABLE（工序基准成分）
		sqlstr = " SELECT "
			" t1.COMM_FMLY_CODE, "
			" t1.MATERIAL_CODE, "
			" t1.ST_NO, "
			" t1.STATION_ID, "
			" t1.ELM_NAME, "
			" t1.SPE_MIN, "
			" t1.SPE_MAX, "
			" t1.BACKLOG_EA, "
			" CASE "
			" WHEN t1.STATION_ID != 'Y' AND t1.ELM_NAME = 'CR' THEN t2.CR "
			" WHEN t1.STATION_ID != 'Y' AND t1.ELM_NAME = 'Mo' THEN t2.MO "
			" WHEN t1.STATION_ID != 'Y' AND t1.ELM_NAME = 'NI' THEN t2.NI ELSE 100 "
			" END AS YIELD "
			" FROM( "
			" SELECT "
			" COMM_FMLY_CODE, "
			" '" + material_code + "' MATERIAL_CODE, "
			" '" + st_no + "' ST_NO, "
			" BACKLOG_EA, "
			" STATION_ID, "
			" ELM_NAME, "
			" SPE_MIN, "
			" SPE_MAX "
			" FROM( "
			" SELECT "
			" A.*, "
			" ROW_NUMBER() OVER( "
			" PARTITION BY A.ELM_NAME, A.STATION_ID "
			" ORDER BY CASE "
			" WHEN A.ST_NO = '" + st_no + "' THEN 1 "
			" WHEN A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "' THEN 2 "
			" WHEN A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 3 "
			" WHEN A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "' THEN 4 "
			" WHEN A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 5 "
			" ELSE 99 "
			" END "
			" ) as rn "
			" FROM(select * from tfbsm05 where BACKLOG_EA = '" + backlog_ea + "') A "
			" WHERE(A.ST_NO = '" + st_no + "' "
			" OR(A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "') "
			" OR(A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "') "
			" OR(A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "') "
			" OR(A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "')) "
			" OR A.STATION_ID = 'Y' "
			" ) "
			" WHERE rn = 1 "
			" ) t1 "
			" LEFT JOIN( "
			" SELECT "
			" ST_NO, "
			" COMM_FMLY_CODE, "
			" STATION_ID, "
			" BACKLOG_EA, "
			" MATERIAL_CODE, "
			" MAX(CASE WHEN YIELD_CR != 0 THEN YIELD_CR END) AS CR, "
			" MAX(CASE WHEN YIELD_MO != 0 THEN YIELD_MO END) AS MO, "
			" MAX(CASE WHEN YIELD_NI != 0 THEN YIELD_NI END) AS NI "
			" FROM tfbsm04 "
			" WHERE BACKLOG_EA = '" + backlog_ea + "' "
			" GROUP BY ST_NO, COMM_FMLY_CODE, STATION_ID, BACKLOG_EA, MATERIAL_CODE "
			" ) t2 "
			" ON  "
			"  t1.ST_NO = t2.ST_NO "
			" AND t1.COMM_FMLY_CODE = t2.COMM_FMLY_CODE "
			" AND t1.STATION_ID = t2.STATION_ID "
			" AND t1.MATERIAL_CODE = t2.MATERIAL_CODE "
			" AND t1.BACKLOG_EA = t2.BACKLOG_EA where (ELM_NAME!='C' and ELM_NAME!='S') and t1.STATION_ID != 'Y' ";
		cmd_inq.SetCommandText(sqlstr);
		Log::Trace("", __FUNCTION__, "inDMST02.WORKTABLE.sqlstr = [{0}]", sqlstr);
		inDMST02.Tables["WORKTABLE"].Rows.Clear();
		cmd_inq.ExecuteReader();
		while (cmd_inq.Read())
		{
			CDataRow& row = inDMST02.Tables["WORKTABLE"].Rows.Add();
			row["COMM_FMLY_CODE"] = cmd_inq.GetString(1).Trim();
			row["MATERIAL_CODE"] = cmd_inq.GetString(2).Trim();
			row["ST_NO"] = cmd_inq.GetString(3).Trim();
			row["STATION_ID"] = cmd_inq.GetString(4).Trim();
			row["ELM_NAME"] = cmd_inq.GetString(5).Trim();
			row["SPE_MIN"] = cmd_inq.GetDecimal(6);
			//Log::Trace("", __FUNCTION__, "inDMST02.SPE_MIN.sqlstr = [{0}]", row["SPE_MIN"].ToString());
			row["SPE_MAX"] = cmd_inq.GetDecimal(7);
			//Log::Trace("", __FUNCTION__, "inDMST02.SPE_MAX.sqlstr = [{0}]", row["SPE_MAX"].ToString());
			row["BACKLOG_EA"] = cmd_inq.GetString(8).Trim();
			row["YIELD"] = cmd_inq.GetString(9).Trim();
		}
		cmd_inq.Close();
		ret = inDMST02.Tables["WORKTABLE"].Rows.get_Count();
		Log::Trace("", __FUNCTION__, "inDMST02.WORKTABLE.Rows = [{0}]", ret);
		if (ret == 0){
			strcpy(s.msg, "出钢记号:" + st_no + ",工艺路径:" + backlog_ea + "工序基准成分没有数据!!!");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//GX_RATE(收得率)
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
		//GX_PARA（工序投料比）
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
		//GX_MAT（原料总表） WEIGHT_MINUS WEIGHT_POSITIVE
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
			"        WHEN '" + backlog_ea + "' = '03' AND F.STATION_ID = 'E' AND "
			"             (SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND f.COMM_FMLY_CODE != 'V' THEN 95 "
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
			"                            WHEN A.ST_NO = '" + st_no + "' THEN 1 "
			"                            WHEN A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "' THEN 2 "
			"                            WHEN A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 3 "
			"                            WHEN A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "' THEN 4 "
			"                            WHEN A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 5 "
			"                            ELSE 99 "
			"                        END "
			"            ) as rn "
			"        FROM TFBSM01 A "
			"        WHERE ( "
			"            A.ST_NO = '" + st_no + "' "
			"            OR (A.MARK_POS_CODE = '1' AND A.MATERIAL_CODE = '" + material_code + "') "
			"            OR (A.MARK_POS_CODE = '2' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "') "
			"            OR (A.MARK_POS_CODE = '3' AND A.MATERIAL_CODE = '" + material_code + "') "
			"            OR (A.MARK_POS_CODE = '3' AND A.COMM_FMLY_CODE = '" + comm_fmly_code + "') "
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
			CDataRow& row = inDMST02.Tables["GX_MAT"].Rows.Add();
			if (cmd_inq.GetString(1) != tfbsm01["LOWER_LIMIT_VALUE"].ToString() && backlog_ea == "01" && tfbsm01["STATION_ID"].ToString() == "Z"){
				//如果值是50表的并且工艺路线是多中频炉则需要单独处理
				row["LOWER_LIMIT_VALUE"] = cmd_inq.GetDecimal(1) + cmd_inq.GetDecimal(1);
			}
			else if (cmd_inq.GetString(1) != tfbsm01["LOWER_LIMIT_VALUE"].ToString() && backlog_ea == "04" && tfbsm01["STATION_ID"].ToString() == "Z"){
				row["LOWER_LIMIT_VALUE"] = cmd_inq.GetDecimal(1) + cmd_inq.GetDecimal(1) + cmd_inq.GetDecimal(1);
			}
			else{
				row["LOWER_LIMIT_VALUE"] = cmd_inq.GetDecimal(1);
			}
			if (cmd_inq.GetString(2) != tfbsm01["UPPER_LIMIT_VALUE"].ToString() && backlog_ea == "01" && tfbsm01["STATION_ID"].ToString() == "Z"){
				//如果值是50表的并且工艺路线是多中频炉则需要单独处理
				row["UPPER_LIMIT_VALUE"] = cmd_inq.GetDecimal(2) + cmd_inq.GetDecimal(2);
			}
			else if (cmd_inq.GetString(2) != tfbsm01["UPPER_LIMIT_VALUE"].ToString() && backlog_ea == "04" && tfbsm01["STATION_ID"].ToString() == "Z"){
				row["UPPER_LIMIT_VALUE"] = cmd_inq.GetDecimal(2) + cmd_inq.GetDecimal(2) + cmd_inq.GetDecimal(2);
			}
			else{
				row["UPPER_LIMIT_VALUE"] = cmd_inq.GetDecimal(2);
			}
			Log::Trace("", __FUNCTION__, ".UPPER_LIMIT_VALUE = [{0}]", row["UPPER_LIMIT_VALUE"].ToString());
			//row["UPPER_LIMIT_VALUE"] = cmd_inq.GetDecimal(2);
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
		if (ret == 0){
			strcpy(s.msg, "出钢记号:" + st_no + ",工艺路径:" + backlog_ea + "工序投料品名没有数据!!!");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		else{
			if (st_no.SubstringNE(1, 1) == "F" || st_no.SubstringNE(1, 1) == "M"){
				if (comm_fmly_code != "V"){
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
		//ROUTE（工艺路线）
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
		//GX_WEIGHT(可投重量)
		sqlstr = "SELECT '" + st_no + "'                                                     ST_NO,"
			"       m.STATION_ID,"
			"       m.BACKLOG_EA,"
			"       m.METAL_CONS_PERT,"
			"       nvl(m.SCRAP_RATIO_MIN, 0)                                    SCRAP_RATIO_MIN,"
			"       nvl(m.SCRAP_RATIO_MAX, 0)                                    SCRAP_RATIO_MAX,"
			"       CASE"
			"           WHEN m.BACKLOG_EA = '03' AND m.STATION_ID = 'E' AND"
			"                (SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' THEN 175"
			"           WHEN m.BACKLOG_EA = '02' AND m.STATION_ID = 'E' AND"
			"                (SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' THEN 165"
			"           WHEN m.BACKLOG_EA = '02' AND m.STATION_ID = 'Z' AND"
			"                (SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V'  THEN 30"
			"           WHEN m.BACKLOG_EA = '07' AND m.STATION_ID = 'B' AND"
			"                (SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' THEN 140"
			"           WHEN m.BACKLOG_EA = '07' AND m.STATION_ID = 'Z' AND"
			"                (SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' THEN 50 "
			"           WHEN m.BACKLOG_EA = '07' AND m.STATION_ID = 'A' AND"
			"                (SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' THEN 220 "
			"			WHEN (m.STATION_ID='D' or m.STATION_ID='B') and (m.COMM_FMLY_CODE='L' or m.COMM_FMLY_CODE='M' OR m.COMM_FMLY_CODE='O') and (m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 190 "
			"			WHEN (m.STATION_ID='D' or m.STATION_ID='B') and  m.COMM_FMLY_CODE='P' and (m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 180 "
			"			WHEN (m.STATION_ID='D' or m.STATION_ID='B') and  m.COMM_FMLY_CODE='Q' and (m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 140 "
			"			WHEN m.STATION_ID='E'  and m.BACKLOG_EA = '03' AND  (SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE!='V' then 175"
			"           ELSE nvl(m.TAPPING_WT, 0) END                            TAPPING_WT,"
			"       m.MAT_COUNT,"
			"       case when M.STATION_ID = 'A' THEN L.DESCRIPTION ELSE ' ' END COMBY_GX, "
			"		CASE "
			"		WHEN(m.STATION_ID = 'D' or m.STATION_ID = 'B') and(m.COMM_FMLY_CODE = 'L' or m.COMM_FMLY_CODE = 'M' OR m.COMM_FMLY_CODE = 'O') and(m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 130 "
			"		WHEN(m.STATION_ID = 'D' or m.STATION_ID = 'B') and  m.COMM_FMLY_CODE = 'P' and(m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 150 "
			"		WHEN(m.STATION_ID = 'D' or m.STATION_ID = 'B') and  m.COMM_FMLY_CODE = 'Q' and(m.BACKLOG_EA = '06' or m.BACKLOG_EA = '08') then 120 "
			"		WHEN m.STATION_ID = 'E'  and m.BACKLOG_EA = '03' AND(SUBSTR('" + st_no + "', 2, 1) = 'F' OR SUBSTR('" + st_no + "', 2, 1) = 'M') AND m.COMM_FMLY_CODE != 'V' then 170 "
			"		ELSE 0 END                            TAPPING_WT_MIN "
			"    FROM ("
			"    SELECT j.*,"
			"           ROW_NUMBER() OVER ("
			"               PARTITION BY J.STATION_ID"
			"               ORDER BY CASE"
			"                           WHEN j.ST_NO = '" + st_no + "' THEN 1"
			"                           WHEN j.MARK_POS_CODE = '1' AND j.MATERIAL_CODE = '" + material_code + "' THEN 2"
			"                           WHEN j.MARK_POS_CODE = '2' AND j.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 3"
			"                           WHEN j.MARK_POS_CODE = '3' AND j.MATERIAL_CODE = '" + material_code + "' THEN 4"
			"                           WHEN j.MARK_POS_CODE = '3' AND j.COMM_FMLY_CODE = '" + comm_fmly_code + "' THEN 5"
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
			"            WHERE t.BACKLOG_EA = '" + backlog_ea + "'"
			"              AND ("
			"                  t.ST_NO = '" + st_no + "'"
			"                  OR (t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "')"
			"                  OR (t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "')"
			"                  OR (t.MARK_POS_CODE = '3' AND (t.MATERIAL_CODE = '" + material_code + "' OR t.COMM_FMLY_CODE = '" + comm_fmly_code + "'))"
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
			"                WHERE t.BACKLOG_EA = '" + backlog_ea + "'"
			"                  AND ("
			"                      t.ST_NO = '" + st_no + "'"
			"                      OR (t.MARK_POS_CODE = '1' AND t.MATERIAL_CODE = '" + material_code + "')"
			"                      OR (t.MARK_POS_CODE = '2' AND t.COMM_FMLY_CODE = '" + comm_fmly_code + "')"
			"                      OR (t.MARK_POS_CODE = '3' AND (t.MATERIAL_CODE = '" + material_code + "' OR t.COMM_FMLY_CODE = '" + comm_fmly_code + "'))"
			"                  )"
			"                GROUP BY t.ST_NO, t.COMM_FMLY_CODE, t.BACKLOG_EA, t.MATERIAL_CODE, t.TAPPING_WT"
			"            )"
			"        ) B ON A.BACKLOG_EA = B.BACKLOG_EA"
			"           AND A.COMM_FMLY_CODE = B.COMM_FMLY_CODE"
			"           AND A.MATERIAL_CODE = B.MATERIAL_CODE"
			"           AND (A.ST_NO = B.ST_NO OR B.ST_NO IS NULL)"
			"    ) J"
			"    WHERE ("
			"        J.ST_NO = '" + st_no + "'"
			"        OR (J.MARK_POS_CODE = '1' AND J.MATERIAL_CODE = '" + material_code + "')"
			"        OR (J.MARK_POS_CODE = '2' AND J.COMM_FMLY_CODE = '" + comm_fmly_code + "')"
			"        OR (J.MARK_POS_CODE = '3' AND (J.MATERIAL_CODE = '" + material_code + "' OR J.COMM_FMLY_CODE = '" + comm_fmly_code + "'))"
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
			CDataRow& row = inDMST02.Tables["GX_WEIGHT"].Rows.Add();
			row["ST_NO"] = cmd_inq.GetString(1).Trim();
			row["STATION_ID"] = cmd_inq.GetString(2).Trim();
			row["BACKLOG_EA"] = cmd_inq.GetString(3).Trim();
			row["METAL_CONS_PERT"] = cmd_inq.GetString(4).Trim();
			row["SCRAP_RATIO_MIN"] = cmd_inq.GetString(5).Trim();
			row["SCRAP_RATIO_MAX"] = cmd_inq.GetString(6).Trim();
			if (st_no.SubstringNE(1, 1) == "F" || st_no.SubstringNE(1, 1) == "M"){
				if (comm_fmly_code != "V"){
					if (row["STATION_ID"].ToString() == "A"){
						row["TAPPING_WT"] = "220";
					}
					else{
						row["TAPPING_WT"] = "0";
					}
				}
				else{
					row["TAPPING_WT"] = cmd_inq.GetString(7).Trim();
				}
			}
			else{
				row["TAPPING_WT"] = cmd_inq.GetString(7).Trim();
			}
			if (row["TAPPING_WT"].ToString().Trim() == "" || row["TAPPING_WT"].ToString().Trim() == "0"){
				row["TAPPING_WT"] = cmd_inq.GetString(7).Trim();
			}
			//row["TAPPING_WT"] = cmd_inq.GetString(7).Trim();
			Log::Trace("", __FUNCTION__, "TAPPING_WT = [{0}]", row["TAPPING_WT"].ToString());
			row["MAT_COUNT"] = cmd_inq.GetString(8).Trim();
			row["COMBY_GX"] = cmd_inq.GetString(9).Trim();
			if (cmd_inq.GetDecimal(10) != 0){
				row["TAPPING_WT_MIN"] = cmd_inq.GetString(10).Trim();
			}
			else{
				row["TAPPING_WT_MIN"] = cmd_inq.GetString(7).Trim();
			}
		}
		cmd_inq.Close();
		ret = inDMST02.Tables["GX_WEIGHT"].Rows.get_Count();
		Log::Trace("", __FUNCTION__, "inDMST02.GX_WEIGHT.Rows = [{0}]", ret);
		if (ret == 0 && st_no.SubstringNE(1, 1) != "F" && st_no.SubstringNE(1, 1) != "M"){
			strcpy(s.msg, "出钢记号:" + st_no + ",工艺路径:" + backlog_ea + "收得率没有数据!!!");
			throw CApplicationException(-1, s.msg, log.Location);
		}
		//准备启动模型
		Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc start ==");
		//RUN_PLMODEL
		f_epex_call_rest_svc(conn, "FBSM_SSMODEL", "RUN_PLMODEL", &inDMST02, &outDMST02);
		Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc end ==");
		ei_sys the_s;
		outDMST02.GetSYS(&the_s);
		Log::Trace("", __FUNCTION__, "doFlag = [{0}]", doFlag);
		if (the_s.flag < 0)
		{
			Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc err[0] ==", the_s.msg);
			//失败不报错
			throw CApplicationException(the_s.msg);

		}
		// MAT_CODE=PL0001   MAT_NAME=配料成分   MAT_CODE=PL0002 MAT_NAME=出钢成分 RESULT_TABLE
		Log::Trace("", __FUNCTION__, "linke = [{0}]", __LINE__);
		//清空数据
		inDMST02.Tables["LG_PSSM"].Rows.Clear();
		Log::Trace("", __FUNCTION__, "== WORK_TABLE.row[{0}] ==", outDMST02.Tables["WORK_TABLE"].Rows.get_Count());
		for (size_t i = 0; i < outDMST02.Tables["WORK_TABLE"].Rows.get_Count(); i++){
			if (outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() == "0"){
				sqlstr = " update tfbsm12 set COMPOSE_LIST_NO=' ',PLAN_ID=' '  where DATE_C='" + date + "' AND ST_NO='" + st_no + "' and BACKLOG_EA='" + backlog_ea + "' and SEQ_NO='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["SEQ_NO"].ToString() + "' ";
				Log::Trace("", __FUNCTION__, "WORK_TABLE== sqlstr[{0}] ==", sqlstr);
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.ExecuteNonQuery();
				cmd_inq.Close();
				continue;
			}
			//钢种大类(A/M/P/D/F)+钢种小类编号+工艺路线编号+日期+2位流水
			plan_id = outDMST02.Tables["WORK_TABLE"].Rows[i]["PLAN_ID"].ToString();
			Log::Trace("", __FUNCTION__, "== date err[{0}] ==", date.SubstringNE(2, 6));
			Log::Trace("", __FUNCTION__, "== plan_id[{0}] ==", plan_id);
			f_material_code = outDMST02.Tables["WORK_TABLE"].Rows[i]["MATERIAL_CODE"].ToString();
			f_date = outDMST02.Tables["WORK_TABLE"].Rows[i]["DATE_C"].ToString();
			f_st_no = outDMST02.Tables["WORK_TABLE"].Rows[i]["ST_NO"].ToString();
			//BACKLOG_EA
			f_backlog_ea = outDMST02.Tables["WORK_TABLE"].Rows[i]["BACKLOG_EA"].ToString();
			//COMM_FMLY_CODE 大类代码
			f_comm_fmly_code = outDMST02.Tables["WORK_TABLE"].Rows[i]["COMM_FMLY_CODE"].ToString();
			Log::Trace("", __FUNCTION__, "== __LINE__[{0}] ==", __LINE__);
			//获取出钢记号
			//获取中类
			//AND MATERIAL_CODE=''
			if (i == 0 && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0"){
				//默认是第一个配料单
				sqlstr = " select  max(substr(COMPOSE_LIST_NO,12,14)),max(substr(COMPOSE_LIST_NO,14,1)),max(substr(COMPOSE_LIST_NO,13, 1)),max(substr(COMPOSE_LIST_NO,13, 2)) from tfbsm14 where DATE_C='" + f_date + "'  and COMPOSE_LIST_NO!=' ' and MATERIAL_CODE='" + f_material_code + "'  and BACKLOG_EA='" + f_backlog_ea + "'   ";
				Log::Trace("", __FUNCTION__, "== sqlstr1[{0}] ==", sqlstr);
				cmd_inq.SetCommandText(sqlstr);
				cmd_inq.ExecuteReader();
				if (cmd_inq.Read()){
					if (cmd_inq.GetDecimal(3) == 0 && cmd_inq.GetDecimal(2) != 9){
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
				else{
					Log::Trace("", __FUNCTION__, "== linke[{0}] ==", __LINE__);
					compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "01";
				}
				cmd_inq.Close();
			}
			else{
				if (outDMST02.Tables["WORK_TABLE"].Rows[i]["PLAN_ID"].ToString() != outDMST02.Tables["WORK_TABLE"].Rows[i - 1]["PLAN_ID"].ToString() && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0"){
					sqlstr = " select  max(substr(COMPOSE_LIST_NO,12,14)),max(substr(COMPOSE_LIST_NO,14,1)),max(substr(COMPOSE_LIST_NO,13, 1)),max(substr(COMPOSE_LIST_NO,13, 2)) from tfbsm14 where DATE_C='" + f_date + "'  and COMPOSE_LIST_NO!=' ' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "'   ";
					Log::Trace("", __FUNCTION__, "== sqlstr!![{0}] ==", sqlstr);
					cmd_inq.SetCommandText(sqlstr);
					cmd_inq.ExecuteReader();
					if (cmd_inq.Read()){
						if (cmd_inq.GetDecimal(3) == 0 && cmd_inq.GetDecimal(2) != 9){
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
					else{
						Log::Trace("", __FUNCTION__, "== linke[{0}] ==", __LINE__);
						compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "01";
					}
					cmd_inq.Close();
				}
			}
			if (compose_list_no == " " || compose_list_no == ""){
				compose_list_no = f_st_no.SubstringNE(1, 1) + f_material_code + f_backlog_ea + f_date + "01";
			}
			Log::Trace("", __FUNCTION__, "== compose_list_no[{0}] ==", compose_list_no);
			sqlstr = " update tfbsm12 set COMPOSE_LIST_NO='" + compose_list_no + "',PLAN_ID='" + plan_id + "',SINGLECOST='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["SINGLECOST"].ToString() + "',COST_DG='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["COST_DG"].ToString() + "' where DATE_C='" + f_date + "' AND ST_NO='" + f_st_no + "' and BACKLOG_EA='" + f_backlog_ea + "' and SEQ_NO='" + outDMST02.Tables["WORK_TABLE"].Rows[i]["SEQ_NO"].ToString() + "' ";
			Log::Trace("", __FUNCTION__, "== sqlstr[{0}] ==", sqlstr);
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.ExecuteNonQuery();
			cmd_inq.Close();
			//还原SI的值
			sqlstr = " select SI_PDI from tfbsm20 where COMM_FMLY_CODE='" + f_comm_fmly_code + "' and BACKLOG_EA='" + f_backlog_ea + "'  ";
			cmd_si_pdi.SetCommandText(sqlstr);
			cmd_si_pdi.ExecuteReader();
			Log::Trace("", __FUNCTION__, "== SI.sqlstr[{0}] ==", sqlstr);
			if (cmd_si_pdi.Read()){
				f_si_pdi = cmd_si_pdi.GetDecimal(1);
			}
			cmd_si_pdi.Close();
			Log::Trace("", __FUNCTION__, "== SI.f_si_pdi[{0}] ==", f_si_pdi.ToString());
			if (i == 0 && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0"){
				for (size_t x = 0; x < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); x++){
					Log::Trace("", __FUNCTION__, "== MAT_CODE[{0}] ==", outDMST02.Tables["RESULT_TABLE"].Rows[x]["MAT_CODE"].ToString());
					Log::Trace("", __FUNCTION__, "== P_VALUE_1[{0}] ==", outDMST02.Tables["RESULT_TABLE"].Rows[x]["P_VALUE"].ToString());
					tfbsm14.Reset();
					tfbsm14.MergeFrom(outDMST02.Tables["RESULT_TABLE"].Rows[x]);
					plan_id_1 = outDMST02.Tables["RESULT_TABLE"].Rows[x]["PLAN_ID"].ToString();
					f_back_c2 = outDMST02.Tables["RESULT_TABLE"].Rows[x]["BACK_C2"].ToString();
					Log::Trace("", __FUNCTION__, "== plan_id_1[{0}] ==", plan_id_1);
					if (tfbsm14["MAT_CODE"].ToString() == "PL0002" || tfbsm14["MAT_CODE"].ToString() == "PL0001" || tfbsm14["MAT_CODE"].ToString() == "PL0003"){
						//除去这三个都取固定值
						sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='001' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
						cmd.SetCommandText(sqlstr);
						cmd.ExecuteReader();
						if (cmd.Read()){
							tfbsm14["C_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
							tfbsm14["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal() / 2;

						}
						cmd.Close();
						sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='005' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
						cmd.SetCommandText(sqlstr);
						cmd.ExecuteReader();
						if (cmd.Read()){
							tfbsm14["S_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
							tfbsm14["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal() / 2;

						}
						cmd.Close();
					}
					//C/S=(上限＋下限)/2
					if (plan_id_1 == plan_id){
						//代表他们两个是同一个配料单
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
						//tfbsm14.Print();
						tfbsm14["REC_CREATOR"] = s.userid;
						tfbsm14["REC_CREATE_TIME"] = datetime;
						tfbsm14["SEQ_NO"] = i + 1;
						if ((f_back_c2 == "7" || tfbsm14["MAT_CODE"].ToString() == "PL0002" || tfbsm14["MAT_CODE"].ToString() == "PL0001") && tfbsm14["STATION_ID"].ToString() == "A"){
							tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal() + f_si_pdi;
						}
						else{
							tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal();
						}
						tfbsm14["MATERIAL_CODE"] = f_material_code;
						tfbsm14.Print();
						tfbsm14.Insert();
					}
				}

			}

			if (i != 0 && outDMST02.Tables["WORK_TABLE"].Rows[i]["PLAN_ID"].ToString() != outDMST02.Tables["WORK_TABLE"].Rows[i - 1]["PLAN_ID"].ToString() && outDMST02.Tables["WORK_TABLE"].Rows[i]["P_TRUE"].ToString() != "0"){
				for (size_t x = 0; x < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); x++){
					Log::Trace("", __FUNCTION__, "== MAT_CODE[{0}] ==", outDMST02.Tables["RESULT_TABLE"].Rows[x]["MAT_CODE"].ToString());
					Log::Trace("", __FUNCTION__, "== P_VALUE_1[{0}] ==", outDMST02.Tables["RESULT_TABLE"].Rows[x]["P_VALUE"].ToString());
					tfbsm14.Reset();
					tfbsm14.MergeFrom(outDMST02.Tables["RESULT_TABLE"].Rows[x]);
					plan_id_1 = outDMST02.Tables["RESULT_TABLE"].Rows[x]["PLAN_ID"].ToString();
					f_back_c2 = outDMST02.Tables["RESULT_TABLE"].Rows[x]["BACK_C2"].ToString();
					Log::Trace("", __FUNCTION__, "== plan_id_1[{0}] ==", plan_id_1);
					if (tfbsm14["MAT_CODE"].ToString() == "PL0002" || tfbsm14["MAT_CODE"].ToString() == "PL0001" || tfbsm14["MAT_CODE"].ToString() == "PL0003"){
						//除去这三个都取固定值
						sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='001' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
						cmd.SetCommandText(sqlstr);
						cmd.ExecuteReader();
						if (cmd.Read()){
							tfbsm14["C_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
							tfbsm14["C_VALUE"] = tfbsm14["C_VALUE"].ToDecimal() / 2;

						}
						cmd.Close();
						sqlstr = " select SPE_MAX,SPE_MIN from tfbsm05 where ST_NO='" + f_st_no + "' and ELM_CODE='005' and MATERIAL_CODE='" + f_material_code + "' and BACKLOG_EA='" + f_backlog_ea + "' ";
						cmd.SetCommandText(sqlstr);
						cmd.ExecuteReader();
						if (cmd.Read()){
							tfbsm14["S_VALUE"] = cmd.GetDecimal(1) + cmd.GetDecimal(2);
							tfbsm14["S_VALUE"] = tfbsm14["S_VALUE"].ToDecimal() / 2;

						}
						cmd.Close();
					}
					//C/S=(上限＋下限)/2
					if (plan_id_1 == plan_id){
						//代表他们两个是同一个配料单
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
						//tfbsm14.Print();
						tfbsm14["REC_CREATOR"] = s.userid;
						tfbsm14["REC_CREATE_TIME"] = datetime;
						tfbsm14["SEQ_NO"] = i + 1;
						if ((f_back_c2 == "7" || tfbsm14["MAT_CODE"].ToString() == "PL0002" || tfbsm14["MAT_CODE"].ToString() == "PL0001") && tfbsm14["STATION_ID"].ToString() == "A"){
							tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal() + f_si_pdi;
						}
						else{
							tfbsm14["WEIGHT"] = tfbsm14["WEIGHT"].ToDecimal();
						}
						tfbsm14["MATERIAL_CODE"] = f_material_code;
						tfbsm14.Print();
						tfbsm14.Insert();
					}
				}

			}
		}
		//针对18表的数据做存储操作
		Log::Trace("", __FUNCTION__, "link = [{0}]", __LINE__);
		for (size_t i = 0; i < inDMST02.Tables["MatTable"].Rows.get_Count(); i++){
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
