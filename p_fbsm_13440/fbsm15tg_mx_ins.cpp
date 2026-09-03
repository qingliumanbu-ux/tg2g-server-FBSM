/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2026
Author:      XXX
Version:     1.0
Date:        2026/04/02
Description: 碳钢模型调用程序（多钢种并行计算版）
**************************************************/
#include "stdafx.h"
#include <map>
#include <vector>
#include <set>
BM2F_ENTERACE(fbsm15tg_mx_ins)

/* ***** 外部函数声明 ***** */
// 模型计算
void f_epex_call_rest_svc(CDbConnection *conn, const CString &system_code, const CString &svc_name, EIClass *blks_in, EIClass *blks_out);
int f_mmsm_getprice(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn);

int f_fbsm15tg_mx_ins(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;

	// ==================== 1. 变量声明区 ====================
	EIClass inDMST02;  // 模型传入块
	EIClass outDMST02; // 模型传出块

	EIClass bcls_rec_rep; // 价格计算传入块
	EIClass bcls_ret_rep; // 价格计算传出块

	CString sqlstr = " ";			  // 储存访问数据库的SQL的字符串
	CString st_no = " ";			  // 出钢记号
	CString seq_no = " ";			  // 序号
	CString date = " ";				  // 日期
	CString tapping_wt_value = " ";	  // 配料模式TAPPING_WT
	CString require_tapping_wt = " "; // 指定TAPPING_WT

	CString yesterday_start = " "; // 前一天开始时间
	CString yesterday_end = " ";   // 前一天结束时间
	CString query_st_no = " ";	   // 实际查询用的ST_NO（品种钢=自身，非品种钢=ALL）

	CString mat_code = "";		   // 物料代码
	CString mat_name = "";		   // 物料名称
	CString lot_no = "";		   // 批次号
	CString stock_name = "";	   // 库区（原值，无转换）
	CString cost = "0";			   // 价格
	CString compose_list_no = " "; // 配料单号，在传出块处理中使用
	CString sql_ENVIRONMENT = " "; // 环境参数查询SQL，在DLLINFO构建中使用
	CString cool_coef = "";		   // 冷却系数

	CDecimal special_count = 0;

	CDecimal safety_stock = 100;	 // 【安全库存量】单位：吨，至少保留此数量不分配
	CDecimal raw_weight = 0;		 // 从数据库查出的原始重量（单位视库区而定：仓库5为吨，其他为公斤）
	CDecimal stock_wt_raw = 0;		 // 单位转换后的库存重量（吨，已扣除安全库存）
	CDecimal occupied_wt = 0;		 // 已占用库存重量（吨）
	CDecimal stock_wt_qc = 0;		 // 可用库存重量（扣减已占用后）
	CDecimal yield_minus = 0;		 // 收得率
	CDecimal total_weight = 0;		 // 配料重量
	CDecimal total_yield_weight = 0; // 出钢重量

	int price_ret = 0; // 价格计算函数返回值

	int is_special = 0; // 当前行是否为品种钢：0=否，1=是

	// MatTable加载标记（key: "ALL"或具体ST_NO，value: 1=已加载）
	// 用于避免重复加载同一钢种的物料配置
	map<CString, int> matLoadedMap;
	// PLAN_ID 到配料单号的映射表，确保相同 PLAN_ID 始终使用同一单号
	map<CString, CString> planIdToComposeMap;
	// 标记该 PLAN_ID 的 14 表是否已插入（避免重复插入明细）
	map<CString, int> planIdInsertedMap;

	// ===== 性能优化：预加载缓存 =====
	// 收得率全量缓存 (mat_code -> yield_minus)
	map<CString, CDecimal> yieldCacheMap;
	// 占用库存缓存 (date|mat_code -> occupied_wt)
	map<CString, CDecimal> occupiedCacheMap;
	// 已预加载的date集合
	set<CString> occupiedDateLoadedSet;
	// Main表索引 (st_no|seq_no -> furnace_count)
	map<CString, int> mainFurnaceMap;

	CDbCommand cmd_inq(conn);		// 环境参数查询
	CDbCommand cmd_check(conn);		// 品种钢判断
	CDbCommand cmd_mat(conn);		// MatTable构建
	CDbCommand cmd_range(conn);		// CHARGE_RANGE构建
	CDbCommand cmd_out(conn);		// 传出处理
	CDbCommand cmd_mat_list(conn);	// 查询TFBSM10物料清单
	CDbCommand cmd_inventory(conn); // 查询FBSM_13_AI库存
	CDbCommand cmd_occupied(conn);	// 查询已占用库存
	CDbCommand cmd_yield(conn);		// 查询收得率

	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");

	try
	{
		// ==================== 2. 初始化输入Block结构 ====================
		// 【Block 1: LG_PSSM】装入所有未审核的炼钢计划
		inDMST02.Tables.Add("LG_PSSM");
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "ST_NO");
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "SEQ_NO");
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "SUM_FURNACES");
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "DATE_C");
		inDMST02.Tables["LG_PSSM"].Columns.Add(DT_STRING, "TAPPING_WT");

		// 【Block 2: MatTable】累加所有钢种的物料（品种钢各自独立，非品种钢用ALL）
		inDMST02.Tables.Add("MatTable");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "ST_NO"); // 原始出钢记号
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MAT_CODE");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "MAT_NAME");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "STOCK_WT");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "STOCK_NAME");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "BACK_C2");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "COST");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "YEILD_MINUS");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "COOL_COEF");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "UPPER_LIMIT_VALUE");
		inDMST02.Tables["MatTable"].Columns.Add(DT_STRING, "LOWER_LIMIT_VALUE");

		// 【Block 3: CHARGE_RANGE】重量模式与投料范围限制
		inDMST02.Tables.Add("CHARGE_RANGE");
		inDMST02.Tables["CHARGE_RANGE"].Columns.Add(DT_STRING, "TAPPING_WT");
		inDMST02.Tables["CHARGE_RANGE"].Columns.Add(DT_STRING, "BACK_C2");
		inDMST02.Tables["CHARGE_RANGE"].Columns.Add(DT_STRING, "UPPER_LIMIT_VALUE");
		inDMST02.Tables["CHARGE_RANGE"].Columns.Add(DT_STRING, "LOWER_LIMIT_VALUE");
		inDMST02.Tables["CHARGE_RANGE"].Columns.Add(DT_STRING, "TYPE_QTY_LIMIT");
		inDMST02.Tables["CHARGE_RANGE"].Columns.Add(DT_STRING, "ST_NO"); // 【新增】钢种标识

		// 【Block 4: DLLINFO】模型运行所需必要参数
		inDMST02.Tables[0].set_TableName("DLLINFO");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "DLLName");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "ClassName");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "MethodName");
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "CHECK_FLAG"); // CHECK_FLAG
		inDMST02.Tables["DLLINFO"].Columns.Add(DT_STRING, "ENVIRONMENT");
		CDataRow &DLLINFOrow = inDMST02.Tables["DLLINFO"].Rows.Add();
		// 这三个值待定
		DLLINFOrow["DLLName"] = "TGPLModel.dll";
		DLLINFOrow["ClassName"] = "TGPLModel.TGPLModel";
		DLLINFOrow["MethodName"] = "CalForPLPlan_Carbon";
		DLLINFOrow["CHECK_FLAG"] = "1"; // 不再区分是否检查标记，都传1，代表需校验

		sql_ENVIRONMENT = " SELECT CODE FROM TEP0002 WHERE CODE_CLASS = 'FBSM23' ";

		cmd_inq.SetCommandText(sql_ENVIRONMENT);
		Log::Trace("", "", "sql_ENVIRONMENT = {0}", sql_ENVIRONMENT);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			DLLINFOrow["ENVIRONMENT"] = cmd_inq.GetString(1);
		}
		cmd_inq.Close();
		cmd_inq.Close();

		// ===== 性能优化：预加载收得率缓存 =====
		// 一次性加载TMMSM50所有收得率，避免内层循环重复查询
		sqlstr = "SELECT MAT_CODE, YEILD_MINUS FROM TMMSM50";
		cmd_yield.SetCommandText(sqlstr);
		cmd_yield.ExecuteReader();
		while (cmd_yield.Read())
		{
			CString y_mat_code = cmd_yield.GetString(1);
			CDecimal y_yield = cmd_yield.GetDecimal(2);
			yieldCacheMap[y_mat_code] = y_yield;
		}
		cmd_yield.Close();
		// Log::Trace("", __FUNCTION__, "收得率缓存加载完成，共[{0}]条", yieldCacheMap.size());

		// ==================== 3. 构建模型传入块CHARGE_RANGE ====================

		// 根据TMMSM21表前一天铁水温度和硅含量确定出钢重量模式
		// 计算前一天的起止时间
		yesterday_start = CDateTime::Now().AddDays(-1).ToString("yyyyMMdd") + "000000";
		yesterday_end = CDateTime::Now().ToString("yyyyMMdd") + "000000";

		sqlstr = " SELECT IRON_SI, IRON_TEMP FROM TMMSM21 "
			" WHERE START_TIME >= @START_TIME "
			" AND START_TIME < @END_TIME ";

		cmd_check.Parameters.Set("START_TIME", yesterday_start);
		cmd_check.Parameters.Set("END_TIME", yesterday_end);
		cmd_check.SetCommandText(sqlstr);
		cmd_check.ExecuteReader();

		map<CString, int> tappingWtCountMap;
		CString recordTappingWt = "25"; // 默认值

		while (cmd_check.Read())
		{
			CDecimal iron_si = cmd_check.GetDecimal(1);
			CDecimal iron_temp = cmd_check.GetDecimal(2);
			CString wt = "25";

			if (iron_temp < 1250)
			{
				if (iron_si < 0.3)
					wt = "20";
				else if (iron_si <= 0.7)
					wt = "23";
				else
					wt = "25";
			}
			else if (iron_temp <= 1350)
			{
				if (iron_si < 0.3)
					wt = "25";
				else if (iron_si <= 0.7)
					wt = "28";
				else
					wt = "30";
			}
			else
			{
				if (iron_si < 0.3)
					wt = "30";
				else if (iron_si <= 0.7)
					wt = "32";
				else
					wt = "35";
			}

			tappingWtCountMap[wt]++;
			Log::Trace("", __FUNCTION__, "TMMSM21记录：IRON_SI=[{0}]，IRON_TEMP=[{1}]，对应模式=[{2}]",
				iron_si.ToString(), iron_temp.ToString(), wt);
		}
		cmd_check.Close();

		// 找出出现次数最多的模式
		int maxCount = 0;
		for (auto it = tappingWtCountMap.begin(); it != tappingWtCountMap.end(); ++it)
		{
			if (it->second > maxCount)
			{
				maxCount = it->second;
				recordTappingWt = it->first;
			}
		}

		if (tappingWtCountMap.empty())
		{
			Log::Trace("", __FUNCTION__, "未查询到前一天TMMSM21数据，使用默认模式25t");
			tapping_wt_value = "25";
		}
		else
		{
			tapping_wt_value = recordTappingWt;
			Log::Trace("", __FUNCTION__, "前一天TMMSM21数据统计，最终TAPPING_WT=[{0}]，出现次数=[{1}]",
				tapping_wt_value, maxCount);
		}

		// 检测前台是否传入 TAPPING_WT 表，不存在则跳过用户指定模式处理
		if (bcls_rec->Tables.Contains("TAPPING_WT") && bcls_rec->Tables["TAPPING_WT"].Rows.get_Count() > 0)
		{
			require_tapping_wt = bcls_rec->Tables["TAPPING_WT"].Rows[0]["TAPPING_WT"].ToString().TrimOrBlank();

			if (require_tapping_wt.Trim() != "" && tapping_wt_value != require_tapping_wt)
			{
				Log::Trace("", __FUNCTION__, "计算出的模式[{0}]与用户指定模式[{1}]不符，采用用户指定模式",
					tapping_wt_value, require_tapping_wt);
				tapping_wt_value = require_tapping_wt; // 以用户指定为准
			}
		}

		Log::Trace("", __FUNCTION__, "最终TAPPING_WT=[{0}]",
			tapping_wt_value);

		// 因为需区分不同钢种的投料限制范围，CHARGE_RANGE的构建移至循环内部

		// 【归并】对 Main 表中 ST_NO + DATE_C 相同的未审核记录进行炉数归并
		struct MainGroup
		{
			CString st_no;
			CString date_c;
			int furnace_count;
		};
		map<CString, MainGroup> mainGroupMap; // key = st_no + "|" + date_c

		for (size_t j = 0; j < bcls_rec->Tables["Main"].Rows.get_Count(); j++)
		{
			CString m_check_flag = bcls_rec->Tables["Main"].Rows[j]["CHECK_FLAG"].ToString().TrimOrBlank();
			if (m_check_flag == "1")
				continue; // 跳过已审核

			CString m_st_no = bcls_rec->Tables["Main"].Rows[j]["ST_NO"].ToString().TrimOrBlank();
			CString m_date_c = bcls_rec->Tables["Main"].Rows[j]["DATE_C"].ToString().TrimOrBlank();
			int m_fc = bcls_rec->Tables["Main"].Rows[j]["FURNACE_COUNT"].ToDecimal().ToInt32();

			if (m_st_no == "" || m_date_c == "")
				continue;

			CString key = m_st_no + "|" + m_date_c;
			if (mainGroupMap.find(key) == mainGroupMap.end())
			{
				mainGroupMap[key].st_no = m_st_no;
				mainGroupMap[key].date_c = m_date_c;
				mainGroupMap[key].furnace_count = 0;
			}
			mainGroupMap[key].furnace_count += m_fc;
		}

		// Log::Trace("", __FUNCTION__, "Main表归并完成，共[{0}]组", mainGroupMap.size());

		// 【条件限制】先收集前台 Main 表中所有未审核的唯一 ST_NO，用于预加载 SQL 加 IN 条件
		set<CString> mainStNoSet;
		for (auto &it : mainGroupMap)
		{
			mainStNoSet.insert(it.second.st_no);
		}
		CString stNoInClause = "'ALL'";
		for (auto &st : mainStNoSet)
		{
			stNoInClause += ",'" + st + "'";
		}
		Log::Trace("", __FUNCTION__, "预加载SQL条件：ST_NO IN ({0})", stNoInClause);

		// ===== 性能优化：预加载数据表到内存，避免循环内反复查询 =====
		// 预加载 TFBSM10：用于品种钢判断 + 物料清单 + BACK_C2 查询
		CDataTable dtTFBSM10;
		sqlstr = "SELECT ST_NO, MAT_CODE, MAT_NAME, BACK_C2, DECODE(COOL_COEF,0,1,COOL_COEF) AS COOL_COEF, DECODE(NVL(UPPER_LIMIT_VALUE,0),0,999,UPPER_LIMIT_VALUE) AS UPPER_LIMIT_VALUE, NVL(LOWER_LIMIT_VALUE,0) AS LOWER_LIMIT_VALUE FROM TFBSM10 WHERE ST_NO IN (" + stNoInClause + ")";
		cmd_mat_list.SetCommandText(sqlstr);
		cmd_mat_list.ExecuteQuery(dtTFBSM10);
		cmd_mat_list.Close();

		map<CString, vector<int>> tfbsm10RowMap;  // ST_NO -> 行号列表
		map<CString, CString> tfbsm10BigClassMap; // ST_NO|MAT_CODE -> BACK_C2
		for (int i = 0; i < dtTFBSM10.Rows.get_Count(); i++)
		{
			CString s_st_no = dtTFBSM10.Rows[i]["ST_NO"].ToString().Trim();
			CString s_mat_code = dtTFBSM10.Rows[i]["MAT_CODE"].ToString().Trim();
			tfbsm10RowMap[s_st_no].push_back(i);
			tfbsm10BigClassMap[s_st_no + "|" + s_mat_code] = dtTFBSM10.Rows[i]["BACK_C2"].ToString().Trim();
		}

		// 预加载 FBSM_13_AI：用于库存查询（不JOIN TFBSM10，MAT_NAME/BACK_C2从TFBSM10索引取）
		CDataTable dtFBSM13AI;
		sqlstr = "SELECT LOT_NO, MAT_CODE, STOCK_NAME, WEIGHT, NVL(Cr, 0) AS CR_VALUE, NVL(Ni, 0) AS NI_VALUE, NVL(Mo, 0) AS MO_VALUE FROM FBSM_13_AI WHERE MAT_CODE IN (SELECT MAT_CODE FROM TFBSM10 WHERE ST_NO IN (" + stNoInClause + "))";
		cmd_inventory.SetCommandText(sqlstr);
		cmd_inventory.ExecuteQuery(dtFBSM13AI);
		cmd_inventory.Close();

		map<CString, vector<int>> fbsm13aiRowMap; // MAT_CODE -> 行号列表
		for (int i = 0; i < dtFBSM13AI.Rows.get_Count(); i++)
		{
			CString s_mat_code = dtFBSM13AI.Rows[i]["MAT_CODE"].ToString().Trim();
			fbsm13aiRowMap[s_mat_code].push_back(i);
		}

		// 预加载 TFBSM30：用于CHARGE_RANGE查询
		CDataTable dtTFBSM30;
		sqlstr = "SELECT TAPPING_WT, ST_NO, BACK_C2, UPPER_LIMIT_VALUE, LOWER_LIMIT_VALUE, DECODE(NVL(TYPE_QTY_LIMIT,0),0,1,TYPE_QTY_LIMIT) AS TYPE_QTY_LIMIT FROM TFBSM30 WHERE TAPPING_WT = '" + tapping_wt_value + "' AND ST_NO IN (" + stNoInClause + ")";
		cmd_range.SetCommandText(sqlstr);
		cmd_range.ExecuteQuery(dtTFBSM30);
		cmd_range.Close();

		map<CString, vector<int>> tfbsm30RowMap; // TAPPING_WT|ST_NO -> 行号列表
		for (int i = 0; i < dtTFBSM30.Rows.get_Count(); i++)
		{
			CString s_tapping_wt = dtTFBSM30.Rows[i]["TAPPING_WT"].ToString().Trim();
			CString s_st_no = dtTFBSM30.Rows[i]["ST_NO"].ToString().Trim();
			tfbsm30RowMap[s_tapping_wt + "|" + s_st_no].push_back(i);
		}

		// ==================== 3.5 根据 Main 表实际配料单构建 LG_PSSM ====================
		for (size_t j = 0; j < bcls_rec->Tables["Main"].Rows.get_Count(); j++)
		{
			CString m_check_flag = bcls_rec->Tables["Main"].Rows[j]["CHECK_FLAG"].ToString().TrimOrBlank();
			if (m_check_flag == "1")
				continue;

			CString m_st_no = bcls_rec->Tables["Main"].Rows[j]["ST_NO"].ToString().TrimOrBlank();
			CString m_date_c = bcls_rec->Tables["Main"].Rows[j]["DATE_C"].ToString().TrimOrBlank();
			CString m_compose_list_no = bcls_rec->Tables["Main"].Rows[j]["COMPOSE_LIST_NO"].ToString();

			if (m_st_no == "" || m_date_c == "")
				continue;
			if (m_compose_list_no == "")
				m_compose_list_no = " ";

			CString groupKey = m_st_no + "|" + m_date_c;
			auto groupIt = mainGroupMap.find(groupKey);
			if (groupIt == mainGroupMap.end())
				continue;
			int sum_furnaces = groupIt->second.furnace_count;

			sqlstr = " SELECT SEQ_NO FROM TFBSM31 "
				" WHERE ST_NO=@ST_NO AND DATE_C=@DATE_C AND COMPOSE_LIST_NO=@COMPOSE_LIST_NO "
				" ORDER BY SEQ_NO ";
			cmd_out.Parameters.Set("ST_NO", m_st_no);
			cmd_out.Parameters.Set("DATE_C", m_date_c);
			cmd_out.Parameters.Set("COMPOSE_LIST_NO", m_compose_list_no);
			Log::Trace("", __FUNCTION__, "LG_PSSM构建m_st_no[{0}]", m_st_no);
			Log::Trace("", __FUNCTION__, "LG_PSSM构建m_date_c[{0}]", m_date_c);
			Log::Trace("", __FUNCTION__, "LG_PSSM构建m_compose_list_no[{0}]", m_compose_list_no);
			cmd_out.SetCommandText(sqlstr);
			cmd_out.ExecuteReader();
			while (cmd_out.Read())
			{
				CString real_seq_no = cmd_out.GetString(1);
				CDataRow &row = inDMST02.Tables["LG_PSSM"].Rows.Add();
				row["ST_NO"] = m_st_no;
				row["SEQ_NO"] = real_seq_no;
				row["SUM_FURNACES"] = sum_furnaces;
				row["DATE_C"] = m_date_c;
				row["TAPPING_WT"] = tapping_wt_value;
			}
			cmd_out.Close();

			// 清空计划相关信息
			sqlstr = " update tfbsm31 set ORIGIN_CODE = ' ',COMPOSE_LIST_NO = ' '"
				",check_flag=' ',CHECK_MAKE=' ',CHECK_DATE=' '"
				",MB_CREATOR=' ',MB_CREATE_TIME=' ',MB_MARK=' '"
				",ERROR_REMARK=' '"
				",SINGLECOST=0,COST_DG=0"
				" WHERE ST_NO=@ST_NO AND DATE_C=@DATE_C AND COMPOSE_LIST_NO=@COMPOSE_LIST_NO "
				" and check_flag !='1'" // 审核过的不能启动模型
				;
			cmd_out.SetCommandText(sqlstr);
			cmd_out.Parameters.Set("ST_NO", m_st_no);
			cmd_out.Parameters.Set("DATE_C", m_date_c);
			cmd_out.Parameters.Set("COMPOSE_LIST_NO", m_compose_list_no);
			cmd_out.ExecuteNonQuery();
			cmd_out.Close();
		}

		Log::Trace("", __FUNCTION__, "LG_PSSM构建完成，共[{0}]行", inDMST02.Tables["LG_PSSM"].Rows.get_Count());

		// ==================== 4. 外层循环（仅用于 MatTable 加载） ====================
		for (auto &it : mainGroupMap)
		{
			st_no = it.second.st_no;
			date = it.second.date_c;

			// -------------------- 4.1 判断品种钢，确定查询键值 --------------------
			auto specialIt = tfbsm10RowMap.find(st_no);
			is_special = (specialIt != tfbsm10RowMap.end() && !specialIt->second.empty()) ? 1 : 0;
			query_st_no = (is_special == 1) ? st_no : "ALL"; // 品种钢用自身，非品种钢用ALL

			// -------------------- 4.2 MatTable块数据处理（去重加载） --------------------
			// 如果该query_st_no（钢种配置）尚未加载，则查询并加入MatTable
			if (matLoadedMap.find(query_st_no) == matLoadedMap.end())
			{
				Log::Trace("", __FUNCTION__, "加载MatTable配置：原始ST_NO=[{0}]，查询键=[{1}]", st_no, query_st_no);

				// 【新增】查询该钢种的CHARGE_RANGE（不同钢种投料范围不同）
				CString rangeKey = tapping_wt_value + "|" + query_st_no;
				auto rangeIt = tfbsm30RowMap.find(rangeKey);
				if (rangeIt != tfbsm30RowMap.end())
				{
					for (size_t ri = 0; ri < rangeIt->second.size(); ri++)
					{
						int rowIdx = rangeIt->second[ri];
						CDataRow &row = inDMST02.Tables["CHARGE_RANGE"].Rows.Add();
						row["TAPPING_WT"] = dtTFBSM30.Rows[rowIdx]["TAPPING_WT"];
						row["BACK_C2"] = dtTFBSM30.Rows[rowIdx]["BACK_C2"];
						row["UPPER_LIMIT_VALUE"] = dtTFBSM30.Rows[rowIdx]["UPPER_LIMIT_VALUE"];
						row["LOWER_LIMIT_VALUE"] = dtTFBSM30.Rows[rowIdx]["LOWER_LIMIT_VALUE"];
						row["TYPE_QTY_LIMIT"] = dtTFBSM30.Rows[rowIdx]["TYPE_QTY_LIMIT"];
						row["ST_NO"] = query_st_no; // 【关键】标识该限制属于哪个钢种
					}
				}

				Log::Trace("", __FUNCTION__, "CHARGE_RANGE加载完成：查询键=[{0}]，当前块行数=[{1}]",
					query_st_no, inDMST02.Tables["CHARGE_RANGE"].Rows.get_Count());

				if (inDMST02.Tables["CHARGE_RANGE"].Rows.get_Count() <= 0)
				{
					strcpy(s.msg, "出钢记号:" + query_st_no + "没有投料模式为" + tapping_wt_value + "的投料限制数据，请维护!!!");
					throw CApplicationException(-1, s.msg, log.Location);
				}

				// 3. 从预加载的TFBSM10获取该钢种（或ALL）的物料清单
				auto matIt = tfbsm10RowMap.find(query_st_no);
				if (matIt != tfbsm10RowMap.end())
				{
					for (size_t mi = 0; mi < matIt->second.size(); mi++)
					{
						int matRowIdx = matIt->second[mi];
						mat_code = dtTFBSM10.Rows[matRowIdx]["MAT_CODE"].ToString().Trim();
						mat_name = dtTFBSM10.Rows[matRowIdx]["MAT_NAME"].ToString().Trim();
						cool_coef = dtTFBSM10.Rows[matRowIdx]["COOL_COEF"].ToString().Trim();

						// 4. 从预加载的FBSM_13_AI获取该物料的库存、成分信息
						auto invIt = fbsm13aiRowMap.find(mat_code);
						if (invIt == fbsm13aiRowMap.end())
							continue;

						for (size_t ii = 0; ii < invIt->second.size(); ii++)
						{
							int invRowIdx = invIt->second[ii];

							// 提取原始数据
							lot_no = dtFBSM13AI.Rows[invRowIdx]["LOT_NO"].ToString().Trim();
							stock_name = dtFBSM13AI.Rows[invRowIdx]["STOCK_NAME"].ToString().Trim();
							raw_weight = dtFBSM13AI.Rows[invRowIdx]["WEIGHT"].ToDecimal();
							Log::Trace("", __FUNCTION__, "物料库存信息：mat_code=[{0}]，mat_name=[{1}]", mat_code, mat_name);
							Log::Trace("", __FUNCTION__, "物料库存信息：stock_name=[{0}]，raw_weight=[{1}]", stock_name, raw_weight);

							// 【单位转换逻辑（含安全库存）】
							// 仓库5：直接是吨，不减安全库存（或根据业务决定是否减）
							// 仓库1或VS开头：公斤转吨，再减安全库存量
							// 其他仓库：公斤转吨
							if (stock_name == "5")
							{
								stock_wt_raw = raw_weight; // 仓库5直接是吨
							}
							else if (stock_name == "1" || stock_name.SubstringNE(0, 2) == "VS")
							{
								// 公斤转吨，并扣除安全库存量（可配置变量safety_stock）
								stock_wt_raw = (raw_weight / 1000) - safety_stock;
							}
							else
							{
								stock_wt_raw = raw_weight / 1000; // 其他库区公斤转吨
							}

							// 如果转换后小于等于0，跳过此条（无可用库存）
							if (stock_wt_raw <= 0)
							{
								Log::Trace("", __FUNCTION__, "物料无可用库存：mat_code=[{0}]，mat_name=[{1}]", mat_code, mat_name);
								continue;
							}

							// 5. 从缓存获取占用库存（性能优化：按date预加载，避免逐条查询）
							// 检查是否需要预加载当前date的占用库存
							if (occupiedDateLoadedSet.find(date) == occupiedDateLoadedSet.end())
							{
								// 一次性加载该date下所有mat_code的占用量
								sqlstr = " SELECT B.MAT_CODE, NVL(SUM(B.WEIGHT), 0) FROM TFBSM31 A "
									" JOIN TFBSM32 B ON A.COMPOSE_LIST_NO = B.COMPOSE_LIST_NO "
									" WHERE A.CHECK_FLAG = '1' "
									" AND A.DATE_C = @DATE_C "
									" GROUP BY B.MAT_CODE ";
								cmd_occupied.Parameters.Set("DATE_C", date);
								cmd_occupied.SetCommandText(sqlstr);
								cmd_occupied.ExecuteReader();
								while (cmd_occupied.Read())
								{
									CString occ_mat_code = cmd_occupied.GetString(1);
									CDecimal occ_wt = cmd_occupied.GetDecimal(2);
									occupiedCacheMap[date + "|" + occ_mat_code] = occ_wt;
								}
								cmd_occupied.Close();
								occupiedDateLoadedSet.insert(date);
								Log::Trace("", __FUNCTION__, "占用库存预加载完成，date=[{0}]", date);
							}

							// 从缓存读取
							CString occKey = date + "|" + mat_code;
							auto occIt = occupiedCacheMap.find(occKey);
							if (occIt != occupiedCacheMap.end())
							{
								occupied_wt = occIt->second;
							}
							else
							{
								occupied_wt = 0;
							}

							// 计算实际可用库存（吨）
							stock_wt_qc = stock_wt_raw - occupied_wt;

							// 如果扣减后小于等于0，跳过此条
							if (stock_wt_qc <= 0)
							{
								Log::Trace("", __FUNCTION__, "物料可用库存已全部占用：mat_code=[{0}]，mat_name=[{1}]", mat_code, mat_name);
								continue;
							}

							// 6. 价格计算（调用f_mmsm_getprice）
							bcls_rec_rep.Tables.Clear(); // 清空传入块，防止上一次循环的数据残留
							bcls_ret_rep.Tables.Clear(); // 清空传出块，防止上一次循环的数据残留
							bcls_rec_rep.Tables.Add();
							bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_CODE");
							bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MAT_NAME");
							bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "CR_VALUE");
							bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "NI_VALUE");
							bcls_rec_rep.Tables[0].Columns.Add(DT_STRING, "MO_VALUE");
							bcls_rec_rep.Tables[0].Rows.Add();
							bcls_rec_rep.Tables[0].Rows[0]["MAT_CODE"] = mat_code;
							bcls_rec_rep.Tables[0].Rows[0]["MAT_NAME"] = mat_name;
							bcls_rec_rep.Tables[0].Rows[0]["CR_VALUE"] = dtFBSM13AI.Rows[invRowIdx]["CR_VALUE"];
							bcls_rec_rep.Tables[0].Rows[0]["NI_VALUE"] = dtFBSM13AI.Rows[invRowIdx]["NI_VALUE"];
							bcls_rec_rep.Tables[0].Rows[0]["MO_VALUE"] = dtFBSM13AI.Rows[invRowIdx]["MO_VALUE"];

							bcls_ret_rep.Tables.Add();

							price_ret = f_mmsm_getprice(&bcls_rec_rep, &bcls_ret_rep, conn);
							cost = "0";
							if (price_ret >= 0 && bcls_ret_rep.Tables[0].Rows.get_Count() > 0)
							{
								cost = bcls_ret_rep.Tables[0].Rows[0]["UNIT_PRICE"].ToString();
							}

							// 7. 从缓存获取收得率（性能优化：避免重复查询数据库）
							auto yieldIt = yieldCacheMap.find(mat_code);
							if (yieldIt != yieldCacheMap.end())
							{
								yield_minus = yieldIt->second;
							}
							else
							{
								yield_minus = 100; // 缓存未命中，使用默认值
							}

							// 获取BACK_C2（从预加载的TFBSM10索引）
							CString back_c2_key = query_st_no + "|" + mat_code;
							CString back_c2 = " ";
							auto bcIt = tfbsm10BigClassMap.find(back_c2_key);
							if (bcIt != tfbsm10BigClassMap.end())
							{
								back_c2 = bcIt->second;
							}

							// 8. 组装数据行（只加可用库存>=1的）
							if (stock_wt_qc >= 1)
							{
								CDataRow &row = inDMST02.Tables["MatTable"].Rows.Add();
								row["ST_NO"] = query_st_no;
								row["MAT_CODE"] = mat_code;
								row["MAT_NAME"] = mat_name;
								row["STOCK_WT"] = stock_wt_qc.ToString(); // 已是吨
								row["STOCK_NAME"] = stock_name;			  // 原值，无仓库转换
								row["BACK_C2"] = back_c2;
								row["COST"] = cost;
								row["COOL_COEF"] = cool_coef;
								row["YEILD_MINUS"] = (yield_minus > 0) ? yield_minus.ToString() : "100";
								row["UPPER_LIMIT_VALUE"] = dtTFBSM10.Rows[matRowIdx]["UPPER_LIMIT_VALUE"];
								row["LOWER_LIMIT_VALUE"] = dtTFBSM10.Rows[matRowIdx]["LOWER_LIMIT_VALUE"];
							}
						}
					}
				}

				// 标记该钢种配置已加载
				matLoadedMap[query_st_no] = 1;
				Log::Trace("", __FUNCTION__, "MatTable配置加载完成：查询键=[{0}]，累计MatTable行数=[{1}]",
					query_st_no, inDMST02.Tables["MatTable"].Rows.get_Count());
			}
			else
			{
				Log::Trace("", __FUNCTION__, "跳过重复MatTable：查询键=[{0}]已加载", query_st_no);
			}

		} // 外层循环结束

		// ==================== 5. 调用模型（只调用一次，处理所有钢种） ====================
		if (inDMST02.Tables["LG_PSSM"].Rows.get_Count() > 0)
		{
			int matLoadedMapsize = matLoadedMap.size();
			Log::Trace("", __FUNCTION__, "调用模型：LG_PSSM共[{0}]行，MatTable共[{1}]种配置",
				inDMST02.Tables["LG_PSSM"].Rows.get_Count(),
				matLoadedMapsize);

			// 准备启动模型
			Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc start ==");
			f_epex_call_rest_svc(conn, "FBSM_SSMODEL", "RUN_PLCARBON_MODEL", &inDMST02, &outDMST02);
			Log::Trace("", __FUNCTION__, "== f_epex_call_rest_svc end ==");

			// ===== 性能优化：预建Main表索引 =====
			// 避免在WORK_TABLE循环中线性扫描Main表
			for (auto &it : mainGroupMap)
			{
				CString m_st_no = it.second.st_no;
				int m_furnace = it.second.furnace_count;
				for (int f = 1; f <= m_furnace; f++)
				{
					CString m_seq_no = CConvert::ToString(f);
					mainFurnaceMap[m_st_no + "|" + m_seq_no] = m_furnace;
				}
			}
			// Log::Trace("", __FUNCTION__, "Main表索引建立完成，共[{0}]条", mainFurnaceMap.size());

			// ==================== 6. 传出块处理（模型传出块数据处理，按PLAN_ID关联WORK_TABLE和RESULT_TABLE） ====================

			for (size_t w = 0; w < outDMST02.Tables["WORK_TABLE"].Rows.get_Count(); w++)
			{
				// 提取 WORK_TABLE 数据
				CString w_plan_id = outDMST02.Tables["WORK_TABLE"].Rows[w]["PLAN_ID"].ToString().Trim();
				CString p_true = outDMST02.Tables["WORK_TABLE"].Rows[w]["P_TRUE"].ToString().Trim();
				CString w_date = outDMST02.Tables["WORK_TABLE"].Rows[w]["DATE_C"].ToString().Trim();
				CString w_st_no = outDMST02.Tables["WORK_TABLE"].Rows[w]["ST_NO"].ToString().Trim();
				CString w_seq_no = outDMST02.Tables["WORK_TABLE"].Rows[w]["SEQ_NO"].ToString().Trim();
				CString w_singlecost = outDMST02.Tables["WORK_TABLE"].Rows[w]["SINGLECOST"].ToString().Trim();
				CString w_cost_dg = outDMST02.Tables["WORK_TABLE"].Rows[w]["COST_DG"].ToString().Trim();

				// 【处理模型计算失败的情况】
				if (p_true == "0")
				{
					sqlstr = "UPDATE TFBSM31 SET COMPOSE_LIST_NO=' ', PLAN_ID=' ',SINGLECOST = 0 ,COST_DG = 0 "
						"WHERE DATE_C = @DATE_C AND ST_NO = @ST_NO AND SEQ_NO = @SEQ_NO";
					cmd_out.Parameters.Set("DATE_C", w_date);
					cmd_out.Parameters.Set("ST_NO", w_st_no);
					cmd_out.Parameters.Set("SEQ_NO", w_seq_no);
					cmd_out.SetCommandText(sqlstr);
					cmd_out.ExecuteNonQuery();
					cmd_out.Close();
					continue;
				}

				// 【获取或生成配料单号】
				if (planIdToComposeMap.find(w_plan_id) != planIdToComposeMap.end())
				{
					// 已存在该 PLAN_ID 的配料单号，直接复用
					compose_list_no = planIdToComposeMap[w_plan_id];
					Log::Trace("", __FUNCTION__, "复用配料单号：[{0}] 对应 PLAN_ID=[{1}]",
						compose_list_no, w_plan_id);
				}
				else
				{
					// 首次遇到该 PLAN_ID，生成新配料单号
					CString st_no_second = w_st_no.SubstringNE(1, 1);
					// DATE_C 是6位（如260403），直接使用，不截位
					CString date_6 = w_date;

					// 查询当天最大流水号
					sqlstr = " SELECT MAX(COMPOSE_LIST_NO) "
						" KEEP (DENSE_RANK LAST "
						" ORDER BY "
						" TO_NUMBER(SUBSTR(COMPOSE_LIST_NO, 2))) "
						" FROM TFBSM32 "
						" WHERE DATE_C = @DATE_C";
					cmd_out.Parameters.Set("DATE_C", w_date);
					cmd_out.SetCommandText(sqlstr);
					cmd_out.ExecuteReader();

					CString max_compose_no = " ";
					if (cmd_out.Read())
					{
						max_compose_no = cmd_out.GetString(1);
					}
					cmd_out.Close();

					// 解析流水号（第8-10位，即索引7开始的3位）
					CDecimal next_seq = 1;
					if (max_compose_no.Trim() != "" && max_compose_no.GetLength() >= 10)
					{
						CString seq_str = max_compose_no.SubstringNE(7, 3);
						next_seq = CDecimal::Parse(seq_str) + 1;
						if (next_seq > 999)
							next_seq = 1;
					}

					// 格式化为3位流水号（不足补零）
					CString seq_str;
					if (next_seq < 10)
						seq_str = "00" + next_seq.ToString(); // 001-009
					else if (next_seq < 100)
						seq_str = "0" + next_seq.ToString(); // 010-099
					else
						seq_str = next_seq.ToString(); // 100-999

					// 拼接10位配料单号：1位+6位日期+3位流水
					compose_list_no = st_no_second + date_6 + seq_str;

					// 记录到映射表
					planIdToComposeMap[w_plan_id] = compose_list_no;

					Log::Trace("", __FUNCTION__, "新PLAN_ID[{0}]生成配料单号[{1}]",
						w_plan_id, compose_list_no);
				}

				// 【插入TFBSM32】仅在该 PLAN_ID 首次出现时插入（且只插入一次）
				if (planIdInsertedMap.find(w_plan_id) == planIdInsertedMap.end())
				{
					// 初始化统计值
					total_weight = 0;
					total_yield_weight = 0;
					// 遍历 RESULT_TABLE，插入该 PLAN_ID 的所有明细
					for (size_t r = 0; r < outDMST02.Tables["RESULT_TABLE"].Rows.get_Count(); r++)
					{

						if (outDMST02.Tables["RESULT_TABLE"].Rows[r]["PLAN_ID"].ToString().Trim() == w_plan_id)
						{
							// 获取当前物料信息
							CString r_mat_code = outDMST02.Tables["RESULT_TABLE"].Rows[r]["MAT_CODE"].ToString();
							CString r_mat_name = outDMST02.Tables["RESULT_TABLE"].Rows[r]["MAT_NAME"].ToString();
							CString r_station_id = outDMST02.Tables["RESULT_TABLE"].Rows[r]["STATION_ID"].ToString();
							CString r_stock_name = outDMST02.Tables["RESULT_TABLE"].Rows[r]["STOCK_NAME"].ToString();
							CDecimal r_weight = outDMST02.Tables["RESULT_TABLE"].Rows[r]["WEIGHT"].ToDecimal();
							CDecimal r_cost = outDMST02.Tables["RESULT_TABLE"].Rows[r]["COST"].ToDecimal();
							CDecimal r_cool_coef = outDMST02.Tables["RESULT_TABLE"].Rows[r]["COOL_COEF"].ToDecimal();
							if (r_cool_coef == 0)
								r_cool_coef = 1;

							// 累加配料重量
							total_weight = total_weight + r_weight;

							// 从缓存获取收得率（性能优化：避免重复查询数据库）
							// 默认100%
							auto yieldIt2 = yieldCacheMap.find(r_mat_code);
							if (yieldIt2 != yieldCacheMap.end())
							{
								yield_minus = yieldIt2->second;
							}

							// 如果查到的收得率为零，说明数据有问题，置为默认值100
							if (yield_minus == 0)
							{
								yield_minus = 100;
							}

							// 累加出钢重量 = 重量 × 收得率 × 冷却系数 / 100
							total_yield_weight = total_yield_weight + (r_weight * yield_minus * r_cool_coef / 100);

							// 插入物料明细行
							sqlstr = "INSERT INTO TFBSM32 ("
								"COMPOSE_LIST_NO, MAT_NAME, MAT_CODE, STATION_ID, "
								"STOCK_NAME, WEIGHT, COST, REC_CREATOR, REC_CREATE_TIME, "
								"DATE_C, ST_NO"
								") VALUES ("
								"@COMPOSE_LIST_NO, @MAT_NAME, @MAT_CODE, @STATION_ID, "
								"@STOCK_NAME, @WEIGHT, @COST, @REC_CREATOR, @REC_CREATE_TIME,"
								"@DATE_C, @ST_NO)";

							cmd_out.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
							cmd_out.Parameters.Set("MAT_NAME", r_mat_name);
							cmd_out.Parameters.Set("MAT_CODE", r_mat_code);
							cmd_out.Parameters.Set("STATION_ID", r_station_id);
							cmd_out.Parameters.Set("STOCK_NAME", r_stock_name);
							cmd_out.Parameters.Set("WEIGHT", r_weight);
							cmd_out.Parameters.Set("COST", r_cost);
							cmd_out.Parameters.Set("REC_CREATOR", s.userid);
							cmd_out.Parameters.Set("REC_CREATE_TIME", datetime);
							cmd_out.Parameters.Set("DATE_C", w_date);
							cmd_out.Parameters.Set("ST_NO", w_st_no);

							cmd_out.SetCommandText(sqlstr);
							cmd_out.ExecuteNonQuery();
							cmd_out.Close();
						}
					}
					// 插入配料重量统计行 (MAT_CODE = PL0001)
					sqlstr = "INSERT INTO TFBSM32 ("
						"COMPOSE_LIST_NO, MAT_NAME, MAT_CODE, STATION_ID, "
						"STOCK_NAME, WEIGHT, COST, REC_CREATOR, REC_CREATE_TIME, "
						"DATE_C, ST_NO"
						") VALUES ("
						"@COMPOSE_LIST_NO, @MAT_NAME, @MAT_CODE, @STATION_ID, "
						"@STOCK_NAME, @WEIGHT, @COST, @REC_CREATOR, @REC_CREATE_TIME,"
						"@DATE_C, @ST_NO)";

					cmd_out.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
					cmd_out.Parameters.Set("MAT_NAME", "配料重量");
					cmd_out.Parameters.Set("MAT_CODE", "PL0001");
					cmd_out.Parameters.Set("STATION_ID", "B");
					cmd_out.Parameters.Set("STOCK_NAME", " ");
					cmd_out.Parameters.Set("WEIGHT", total_weight.Round(3));
					cmd_out.Parameters.Set("COST", 0);
					cmd_out.Parameters.Set("REC_CREATOR", s.userid);
					cmd_out.Parameters.Set("REC_CREATE_TIME", datetime);
					cmd_out.Parameters.Set("DATE_C", w_date);
					cmd_out.Parameters.Set("ST_NO", w_st_no);
					cmd_out.SetCommandText(sqlstr);
					cmd_out.ExecuteNonQuery();
					cmd_out.Close();

					// 2026.7.17 修改，王老要求不要出钢重量行了
					// 插入出钢重量统计行 (MAT_CODE = PL0002)
					// cmd_out.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
					// cmd_out.Parameters.Set("MAT_NAME", "出钢重量");
					// cmd_out.Parameters.Set("MAT_CODE", "PL0002");
					// cmd_out.Parameters.Set("STATION_ID", "B");
					// cmd_out.Parameters.Set("STOCK_NAME", " ");
					// cmd_out.Parameters.Set("WEIGHT", total_yield_weight.Round(3));
					// cmd_out.Parameters.Set("COST", 0);
					// cmd_out.Parameters.Set("REC_CREATOR", s.userid);
					// cmd_out.Parameters.Set("REC_CREATE_TIME", datetime);
					// cmd_out.Parameters.Set("DATE_C", w_date);
					// cmd_out.Parameters.Set("ST_NO", w_st_no);
					// cmd_out.SetCommandText(sqlstr);
					// cmd_out.ExecuteNonQuery();
					// cmd_out.Close();

					// 标记该 PLAN_ID 的 32 表已插入完成
					planIdInsertedMap[w_plan_id] = 1;
					Log::Trace("", __FUNCTION__, "PLAN_ID[{0}]的32表明细已插入", w_plan_id);
				}

				// 【更新TFBSM31】每行 WORK_TABLE 都更新（因为每行对应不同炉次）
				sqlstr = "UPDATE TFBSM31 SET "
					"PLAN_ID = @PLAN_ID, "
					"SINGLECOST = @SINGLECOST, "
					"COST_DG = @COST_DG, "
					"COMPOSE_LIST_NO = @COMPOSE_LIST_NO, "
					"ORIGIN_CODE = ' ' "
					"WHERE DATE_C = @DATE_C "
					"AND ST_NO = @ST_NO "
					"AND SEQ_NO = @SEQ_NO";

				cmd_out.Parameters.Set("PLAN_ID", w_plan_id);
				cmd_out.Parameters.Set("SINGLECOST", w_singlecost);
				cmd_out.Parameters.Set("COST_DG", w_cost_dg);
				cmd_out.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
				cmd_out.Parameters.Set("DATE_C", w_date);
				cmd_out.Parameters.Set("ST_NO", w_st_no);
				cmd_out.Parameters.Set("SEQ_NO", w_seq_no);

				cmd_out.SetCommandText(sqlstr);
				cmd_out.ExecuteNonQuery();
				cmd_out.Close();
				Log::Trace("", __FUNCTION__, "配料单号[{0}]，SEQ_NO[{1}]的31表已更新", compose_list_no, w_seq_no);
			}

			// 处理模型报错信息 ERROR_INFO
			// 当模型返回 ERROR_INFO 块时，将错误信息回写到 TFBSM31.ERROR_REMARK 字段
			if (outDMST02.Tables.Contains("ERROR_INFO"))
			{
				for (size_t e = 0; e < outDMST02.Tables["ERROR_INFO"].Rows.get_Count(); e++)
				{
					CString e_date = outDMST02.Tables["ERROR_INFO"].Rows[e]["DATE_C"].ToString();
					CString e_st_no = outDMST02.Tables["ERROR_INFO"].Rows[e]["ST_NO"].ToString();
					CString e_seq_no = outDMST02.Tables["ERROR_INFO"].Rows[e]["SEQ_NO"].ToString();
					CString e_msg = outDMST02.Tables["ERROR_INFO"].Rows[e]["ERROR_MSG"].ToString();

					sqlstr = " UPDATE TFBSM31 SET ERROR_REMARK = @MSG "
						" WHERE ST_NO = @ST_NO AND DATE_C = @DATE_C AND SEQ_NO = @SEQ_NO ";
					cmd_out.Parameters.Set("MSG", e_msg);
					cmd_out.Parameters.Set("ST_NO", e_st_no);
					cmd_out.Parameters.Set("DATE_C", e_date);
					cmd_out.Parameters.Set("SEQ_NO", e_seq_no);
					cmd_out.SetCommandText(sqlstr);
					cmd_out.ExecuteNonQuery();
					cmd_out.Close();
					Log::Trace("", __FUNCTION__, "钢种[{0}]，日期[{1}]，SEQ_NO[{2}]的错误备注已写入", e_st_no, e_date, e_seq_no);
				}
			}

			// ==================== 7. 存储过程数据与配料单归档 ====================

			// 7.0 建立映射关系和数据聚合
			map<CString, CString> composeToQueryStNoMap; // compose_list_no -> query_st_no
			struct PssmAggData
			{
				CString st_no;		// 品种钢存ST_NO，非品种钢存"ALL"
				int sum_furnaces;	// 累加炉数
				CString date_c;		// 日期
				CString tapping_wt; // 出钢量
				CString singlecost; // 单炉成本
				CString cost_dg;	// 吨钢成本
				int is_archived;	// 标记是否已归档（防止重复）
			};
			map<CString, PssmAggData> composeToPssmAggMap;
			// 失败记录集合
			struct FailedRecord
			{
				CString st_no;
				CString seq_no;
				CString date_c;
				CString query_st_no;
				int furnace_count;
				CString tapping_wt;
			};
			vector<FailedRecord> failedRecords;
			set<CString> failedQueryStNoSet;

			// 遍历WORK_TABLE收集每个配料单的信息
			for (size_t w = 0; w < outDMST02.Tables["WORK_TABLE"].Rows.get_Count(); w++)
			{
				CString w_st_no = outDMST02.Tables["WORK_TABLE"].Rows[w]["ST_NO"].ToString().Trim();
				CString w_seq_no = outDMST02.Tables["WORK_TABLE"].Rows[w]["SEQ_NO"].ToString().Trim();
				CString w_plan_id = outDMST02.Tables["WORK_TABLE"].Rows[w]["PLAN_ID"].ToString().Trim();
				CString w_date = outDMST02.Tables["WORK_TABLE"].Rows[w]["DATE_C"].ToString().Trim();
				CString w_singlecost = outDMST02.Tables["WORK_TABLE"].Rows[w]["SINGLECOST"].ToString().Trim();
				CString w_cost_dg = outDMST02.Tables["WORK_TABLE"].Rows[w]["COST_DG"].ToString().Trim();
				CString p_true = outDMST02.Tables["WORK_TABLE"].Rows[w]["P_TRUE"].ToString().Trim();

				// 从预建索引获取炉数（性能优化：O(1)替代O(n)）
				int furnace_count = 0;
				auto furnaceIt = mainFurnaceMap.find(w_st_no + "|" + w_seq_no);
				if (furnaceIt != mainFurnaceMap.end())
				{
					furnace_count = furnaceIt->second;
				}

				// 判断品种钢
				CString query_st_no = "ALL";
				if (matLoadedMap.find(w_st_no) != matLoadedMap.end())
				{
					query_st_no = w_st_no;
				}

				// ==================== 计算失败 ====================
				if (p_true == "0")
				{
					FailedRecord fr;
					fr.st_no = w_st_no;
					fr.seq_no = w_seq_no;
					fr.date_c = w_date;
					fr.query_st_no = query_st_no;
					fr.furnace_count = 1; // 失败记录每行代表一炉
					fr.tapping_wt = tapping_wt_value;
					failedRecords.push_back(fr);
					failedQueryStNoSet.insert(query_st_no);

					continue;
				}

				// ==================== 计算成功 ====================
				if (planIdToComposeMap.find(w_plan_id) == planIdToComposeMap.end())
					continue;

				compose_list_no = planIdToComposeMap[w_plan_id];
				composeToQueryStNoMap[compose_list_no] = query_st_no;

				if (composeToPssmAggMap.find(compose_list_no) == composeToPssmAggMap.end())
				{
					composeToPssmAggMap[compose_list_no].st_no = w_st_no;
					composeToPssmAggMap[compose_list_no].sum_furnaces = 0;
					composeToPssmAggMap[compose_list_no].date_c = w_date;
					composeToPssmAggMap[compose_list_no].tapping_wt = tapping_wt_value;
					composeToPssmAggMap[compose_list_no].singlecost = w_singlecost;
					composeToPssmAggMap[compose_list_no].cost_dg = w_cost_dg;
				}
				composeToPssmAggMap[compose_list_no].sum_furnaces += 1; // 每个WORK_TABLE成功行代表一炉
			}

			// -------------------- 7.1 存储LG_PSSM聚合数据到TFBSM33 --------------------
			// 成功的（有配料单号，聚合存储）
			for (auto it = composeToPssmAggMap.begin(); it != composeToPssmAggMap.end(); ++it)
			{
				compose_list_no = it->first;
				auto &data = it->second;

				sqlstr = "INSERT INTO TFBSM33 (COMPOSE_LIST_NO, ST_NO, FURNACE_COUNT, DATE_C, TAPPING_WT, REC_CREATOR, REC_CREATE_TIME) "
					"VALUES (@COMPOSE_LIST_NO, @ST_NO, @FURNACE_COUNT, @DATE_C, @TAPPING_WT, @REC_CREATOR, @REC_CREATE_TIME)";

				cmd_out.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
				cmd_out.Parameters.Set("ST_NO", data.st_no);
				cmd_out.Parameters.Set("FURNACE_COUNT", data.sum_furnaces);
				cmd_out.Parameters.Set("DATE_C", data.date_c);
				cmd_out.Parameters.Set("TAPPING_WT", data.tapping_wt);
				cmd_out.Parameters.Set("REC_CREATOR", s.userid);
				cmd_out.Parameters.Set("REC_CREATE_TIME", datetime);

				cmd_out.SetCommandText(sqlstr);
				cmd_out.ExecuteNonQuery();
				cmd_out.Close();

				Log::Trace("", __FUNCTION__, "存储LG_PSSM到TFBSM33: COMPOSE_LIST_NO=[{0}], ST_NO=[{1}], FURNACE_COUNT=[{2}]",
					compose_list_no, data.st_no, data.sum_furnaces);
			}

			// 失败的（配料单号为空格，逐条存储）
			for (size_t i = 0; i < failedRecords.size(); i++)
			{
				auto &fr = failedRecords[i];

				sqlstr = "INSERT INTO TFBSM33 (COMPOSE_LIST_NO, ST_NO, SEQ_NO, FURNACE_COUNT, DATE_C, TAPPING_WT, REC_CREATOR, REC_CREATE_TIME) "
					"VALUES (@COMPOSE_LIST_NO, @ST_NO, @SEQ_NO, @FURNACE_COUNT, @DATE_C, @TAPPING_WT, @REC_CREATOR, @REC_CREATE_TIME)";

				cmd_out.Parameters.Set("COMPOSE_LIST_NO", " "); // 失败：空格
				cmd_out.Parameters.Set("ST_NO", fr.st_no);
				cmd_out.Parameters.Set("SEQ_NO", fr.seq_no);			   // 【新增】存入序号
				cmd_out.Parameters.Set("FURNACE_COUNT", fr.furnace_count); // 单炉数（一般为1）
				cmd_out.Parameters.Set("DATE_C", fr.date_c);
				cmd_out.Parameters.Set("TAPPING_WT", fr.tapping_wt);
				cmd_out.Parameters.Set("REC_CREATOR", s.userid);
				cmd_out.Parameters.Set("REC_CREATE_TIME", datetime);

				cmd_out.SetCommandText(sqlstr);
				cmd_out.ExecuteNonQuery();
				cmd_out.Close();
			}

			// -------------------- 7.2 存储MatTable到TFBSM34（每个compose_list_no一份） --------------------
			// 成功的
			for (auto it = composeToQueryStNoMap.begin(); it != composeToQueryStNoMap.end(); ++it)
			{
				compose_list_no = it->first;
				CString query_st_no = it->second;

				for (size_t i = 0; i < inDMST02.Tables["MatTable"].Rows.get_Count(); i++)
				{
					CString mat_st_no = inDMST02.Tables["MatTable"].Rows[i]["ST_NO"].ToString().Trim();

					if (mat_st_no == query_st_no)
					{
						sqlstr = "INSERT INTO TFBSM34 (COMPOSE_LIST_NO, ST_NO, MAT_CODE, MAT_NAME, STOCK_WT, STOCK_NAME, BACK_C2, COST, YEILD_MINUS, REC_CREATOR, REC_CREATE_TIME,COOL_COEF,UPPER_LIMIT_VALUE,LOWER_LIMIT_VALUE) "
							"VALUES (@COMPOSE_LIST_NO, @ST_NO, @MAT_CODE, @MAT_NAME, @STOCK_WT, @STOCK_NAME, @BACK_C2, @COST, @YEILD_MINUS, @REC_CREATOR, @REC_CREATE_TIME,@COOL_COEF,@UPPER_LIMIT_VALUE,@LOWER_LIMIT_VALUE)";

						cmd_out.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
						cmd_out.Parameters.Set("ST_NO", query_st_no);
						cmd_out.Parameters.Set("MAT_CODE", inDMST02.Tables["MatTable"].Rows[i]["MAT_CODE"]);
						cmd_out.Parameters.Set("MAT_NAME", inDMST02.Tables["MatTable"].Rows[i]["MAT_NAME"]);
						cmd_out.Parameters.Set("STOCK_WT", inDMST02.Tables["MatTable"].Rows[i]["STOCK_WT"]);
						cmd_out.Parameters.Set("STOCK_NAME", inDMST02.Tables["MatTable"].Rows[i]["STOCK_NAME"]);
						cmd_out.Parameters.Set("BACK_C2", inDMST02.Tables["MatTable"].Rows[i]["BACK_C2"]);
						cmd_out.Parameters.Set("COOL_COEF", inDMST02.Tables["MatTable"].Rows[i]["COOL_COEF"]);
						cmd_out.Parameters.Set("COST", inDMST02.Tables["MatTable"].Rows[i]["COST"]);
						cmd_out.Parameters.Set("YEILD_MINUS", inDMST02.Tables["MatTable"].Rows[i]["YEILD_MINUS"]);
						cmd_out.Parameters.Set("UPPER_LIMIT_VALUE", inDMST02.Tables["MatTable"].Rows[i]["UPPER_LIMIT_VALUE"]);
						cmd_out.Parameters.Set("LOWER_LIMIT_VALUE", inDMST02.Tables["MatTable"].Rows[i]["LOWER_LIMIT_VALUE"]);
						cmd_out.Parameters.Set("REC_CREATOR", s.userid);
						cmd_out.Parameters.Set("REC_CREATE_TIME", datetime);

						cmd_out.SetCommandText(sqlstr);
						cmd_out.ExecuteNonQuery();
						cmd_out.Close();
					}
				}
			}

			// 失败的（配料单号为空格）
			for (auto &query_st_no : failedQueryStNoSet)
			{
				for (size_t i = 0; i < inDMST02.Tables["MatTable"].Rows.get_Count(); i++)
				{
					CString mat_st_no = inDMST02.Tables["MatTable"].Rows[i]["ST_NO"].ToString().Trim();
					if (mat_st_no == query_st_no)
					{
						sqlstr = "INSERT INTO TFBSM34 (COMPOSE_LIST_NO, ST_NO, MAT_CODE, MAT_NAME, STOCK_WT, STOCK_NAME, BACK_C2, COST, YEILD_MINUS, REC_CREATOR, REC_CREATE_TIME,COOL_COEF) "
							"VALUES (@COMPOSE_LIST_NO, @ST_NO, @MAT_CODE, @MAT_NAME, @STOCK_WT, @STOCK_NAME, @BACK_C2, @COST, @YEILD_MINUS, @REC_CREATOR, @REC_CREATE_TIME,@COOL_COEF)";

						cmd_out.Parameters.Set("COMPOSE_LIST_NO", " "); // 失败：空格
						cmd_out.Parameters.Set("ST_NO", query_st_no);
						cmd_out.Parameters.Set("MAT_CODE", inDMST02.Tables["MatTable"].Rows[i]["MAT_CODE"]);
						cmd_out.Parameters.Set("MAT_NAME", inDMST02.Tables["MatTable"].Rows[i]["MAT_NAME"]);
						cmd_out.Parameters.Set("STOCK_WT", inDMST02.Tables["MatTable"].Rows[i]["STOCK_WT"]);
						cmd_out.Parameters.Set("STOCK_NAME", inDMST02.Tables["MatTable"].Rows[i]["STOCK_NAME"]);
						cmd_out.Parameters.Set("BACK_C2", inDMST02.Tables["MatTable"].Rows[i]["BACK_C2"]);
						cmd_out.Parameters.Set("COOL_COEF", inDMST02.Tables["MatTable"].Rows[i]["COOL_COEF"]);
						cmd_out.Parameters.Set("COST", inDMST02.Tables["MatTable"].Rows[i]["COST"]);
						cmd_out.Parameters.Set("YEILD_MINUS", inDMST02.Tables["MatTable"].Rows[i]["YEILD_MINUS"]);
						cmd_out.Parameters.Set("REC_CREATOR", s.userid);
						cmd_out.Parameters.Set("REC_CREATE_TIME", datetime);

						cmd_out.SetCommandText(sqlstr);
						cmd_out.ExecuteNonQuery();
						cmd_out.Close();
					}
				}
			}

			// -------------------- 7.3 存储CHARGE_RANGE到TFBSM35（每个compose_list_no一份） --------------------
			// 成功的
			for (auto it = composeToQueryStNoMap.begin(); it != composeToQueryStNoMap.end(); ++it)
			{
				compose_list_no = it->first;
				CString query_st_no = it->second;

				for (size_t i = 0; i < inDMST02.Tables["CHARGE_RANGE"].Rows.get_Count(); i++)
				{
					CString range_st_no = inDMST02.Tables["CHARGE_RANGE"].Rows[i]["ST_NO"].ToString().Trim();

					if (range_st_no == query_st_no)
					{
						sqlstr = "INSERT INTO TFBSM35 (COMPOSE_LIST_NO, TAPPING_WT, BACK_C2, UPPER_LIMIT_VALUE, LOWER_LIMIT_VALUE, ST_NO, REC_CREATOR, REC_CREATE_TIME,TYPE_QTY_LIMIT) "
							"VALUES (@COMPOSE_LIST_NO, @TAPPING_WT, @BACK_C2, @UPPER_LIMIT_VALUE, @LOWER_LIMIT_VALUE, @ST_NO, @REC_CREATOR, @REC_CREATE_TIME,@TYPE_QTY_LIMIT)";

						cmd_out.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
						cmd_out.Parameters.Set("TAPPING_WT", inDMST02.Tables["CHARGE_RANGE"].Rows[i]["TAPPING_WT"]);
						cmd_out.Parameters.Set("BACK_C2", inDMST02.Tables["CHARGE_RANGE"].Rows[i]["BACK_C2"]);
						cmd_out.Parameters.Set("UPPER_LIMIT_VALUE", inDMST02.Tables["CHARGE_RANGE"].Rows[i]["UPPER_LIMIT_VALUE"]);
						cmd_out.Parameters.Set("LOWER_LIMIT_VALUE", inDMST02.Tables["CHARGE_RANGE"].Rows[i]["LOWER_LIMIT_VALUE"]);
						cmd_out.Parameters.Set("TYPE_QTY_LIMIT", inDMST02.Tables["CHARGE_RANGE"].Rows[i]["TYPE_QTY_LIMIT"]);
						cmd_out.Parameters.Set("ST_NO", query_st_no);
						cmd_out.Parameters.Set("REC_CREATOR", s.userid);
						cmd_out.Parameters.Set("REC_CREATE_TIME", datetime);

						cmd_out.SetCommandText(sqlstr);
						cmd_out.ExecuteNonQuery();
						cmd_out.Close();
					}
				}
			}

			// 失败的（配料单号为空格）
			for (auto &query_st_no : failedQueryStNoSet)
			{
				for (size_t i = 0; i < inDMST02.Tables["CHARGE_RANGE"].Rows.get_Count(); i++)
				{
					CString range_st_no = inDMST02.Tables["CHARGE_RANGE"].Rows[i]["ST_NO"].ToString().Trim();
					if (range_st_no == query_st_no)
					{
						sqlstr = "INSERT INTO TFBSM35 (COMPOSE_LIST_NO, TAPPING_WT, BACK_C2, UPPER_LIMIT_VALUE, LOWER_LIMIT_VALUE, ST_NO, REC_CREATOR, REC_CREATE_TIME) "
							"VALUES (@COMPOSE_LIST_NO, @TAPPING_WT, @BACK_C2, @UPPER_LIMIT_VALUE, @LOWER_LIMIT_VALUE, @ST_NO, @REC_CREATOR, @REC_CREATE_TIME)";

						cmd_out.Parameters.Set("COMPOSE_LIST_NO", " "); // 失败：空格
						cmd_out.Parameters.Set("TAPPING_WT", inDMST02.Tables["CHARGE_RANGE"].Rows[i]["TAPPING_WT"]);
						cmd_out.Parameters.Set("BACK_C2", inDMST02.Tables["CHARGE_RANGE"].Rows[i]["BACK_C2"]);
						cmd_out.Parameters.Set("UPPER_LIMIT_VALUE", inDMST02.Tables["CHARGE_RANGE"].Rows[i]["UPPER_LIMIT_VALUE"]);
						cmd_out.Parameters.Set("LOWER_LIMIT_VALUE", inDMST02.Tables["CHARGE_RANGE"].Rows[i]["LOWER_LIMIT_VALUE"]);
						cmd_out.Parameters.Set("ST_NO", query_st_no);
						cmd_out.Parameters.Set("REC_CREATOR", s.userid);
						cmd_out.Parameters.Set("REC_CREATE_TIME", datetime);

						cmd_out.SetCommandText(sqlstr);
						cmd_out.ExecuteNonQuery();
						cmd_out.Close();
					}
				}
			}

			// -------------------- 7.4 配料单归档到TFBSM14A --------------------
			for (auto it = composeToPssmAggMap.begin(); it != composeToPssmAggMap.end(); ++it)
			{
				compose_list_no = it->first;
				auto &data = it->second;

				// TFBSM14A: 配料单归档表
				sqlstr = "INSERT INTO TFBSM14A (COMPOSE_LIST_NO, ST_NO, SINGLECOST, COST_DG, DATE_C, BACKLOG_EA, REC_CREATOR, REC_CREATE_TIME) "
					"VALUES (@COMPOSE_LIST_NO, @ST_NO, @SINGLECOST, @COST_DG, @DATE_C, @BACKLOG_EA, @REC_CREATOR, @REC_CREATE_TIME)";

				cmd_out.Parameters.Set("COMPOSE_LIST_NO", compose_list_no);
				cmd_out.Parameters.Set("ST_NO", data.st_no);		   // 存原本的ST_NO
				cmd_out.Parameters.Set("SINGLECOST", data.singlecost); // 单炉成本
				cmd_out.Parameters.Set("COST_DG", data.cost_dg);	   // 吨钢成本
				cmd_out.Parameters.Set("DATE_C", data.date_c);		   // 日期
				cmd_out.Parameters.Set("BACKLOG_EA", "09");			   // 钢区工序途径，固定值09
				cmd_out.Parameters.Set("REC_CREATOR", s.userid);	   // 记录创建责任者
				cmd_out.Parameters.Set("REC_CREATE_TIME", datetime);   // 记录创建时刻

				cmd_out.SetCommandText(sqlstr);
				cmd_out.ExecuteNonQuery();
				cmd_out.Close();

				Log::Trace("", __FUNCTION__, "配料单归档到TFBSM14A: COMPOSE_LIST_NO=[{0}], ST_NO=[{1}], SINGLECOST=[{2}], COST_DG=[{3}]",
					compose_list_no, data.st_no, data.singlecost, data.cost_dg);
			}

			int count1 = 0;
			int count2 = 0;
			count1 = composeToPssmAggMap.size();
			count2 = failedRecords.size();
			Log::Trace("", __FUNCTION__, "过程数据归档完成：成功配料单[{0}]个, 失败炉次[{1}]个", count1, count2);
		}
		else
		{
			Log::Trace("", __FUNCTION__, "无未审核数据，跳过模型调用");
		}
	}
	catch (CDbException &ex)
	{
		CFormattable arguments[] = { ex.GetCode(), ex.GetMsg() };
		CMessageFormat::Format(s.msg, "Database Error,sqlcode=[{0}],sqlmsg=[{1}]", arguments, 2);
		s.flag = -1;
		doFlag = -1;
	}
	catch (CApplicationException &ex)
	{
		s.flag = ex.GetCode();
		strncpy(s.msg, ex.GetMsg(), sizeof(s.msg) - 1);
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