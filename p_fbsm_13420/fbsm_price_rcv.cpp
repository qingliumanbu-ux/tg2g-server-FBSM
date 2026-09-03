/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2025/11/26
Description: 周利润原料价格
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm_price_rcv)

int f_fbsm_price_rcv(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	/* 数据库操作类定义：统一放在Service或函数前段 */
	CDbCommand cmd_inq(conn);
	CString datetime = CDateTime::Now().ToString("yyyyMMddHHmmss");
	CModel tfbsm03("TFBSM03");
	CModel tfbsm03_copy("TFBSM03");
	CModel tqmtscb00_dr("TQMTSCB00_DR");
	CModel tqmtscb00_dr_copy("TQMTSCB00_DR");
	CString compose_list_no = "";
	CDecimal unit_cr = 0;   // 铬单价
	CDecimal unit_ni = 0;   // 镍单价
	CDecimal unit_mo = 0;   // 钼单价
	CDecimal unit_fe = 0;   // 铁水单价
	CString date_c_value = " ";
	try
	{
		Log::Trace("", "", "linke = {0}", __LINE__);
		Log::Trace("", "", "rows = {0}", bcls_rec->Tables[0].Rows.get_Count());
		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			//把字段相同的值塞到表里面
			tfbsm03.Reset();
			tfbsm03.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			Log::Trace("", "", "UNIT_PRICE = {0}", tfbsm03["UNIT_PRICE"].ToString());
			if (i == 0) { // 仅取第一行的日期（和原逻辑一致）
				date_c_value = tfbsm03["VERSION_D"].ToString();
			}
			if (tfbsm03.QueryCount("VERSION_D,MAT_CODE")>0){
				//查询是否存在相同的数据
				//存在先删后加
				tfbsm03_copy.CopyFrom(tfbsm03);
				tfbsm03_copy["UNIT_PRICE"] = tfbsm03["UNIT_PRICE"].ToString();
				Log::Trace("", "", "tfbsm03_copy.UNIT_PRICE = {0}", tfbsm03_copy["UNIT_PRICE"].ToString());
				tfbsm03.Delete("VERSION_D,MAT_CODE");
				tfbsm03_copy["REC_REVISE_TIME"] = datetime;
				tfbsm03_copy["REC_REVISOR"] = s.userid;
				tfbsm03_copy.Insert();
			}
			else{
				//没有相同的数据直接添加
				tfbsm03["REC_CREATE_TIME"] = datetime;
				tfbsm03["REC_CREATOR"] = s.userid;
				tfbsm03_copy["UNIT_PRICE"] = tfbsm03["UNIT_PRICE"].ToString();
				tfbsm03.Insert();
			}

			// ===================== 新增：TQMTSCB00_DR表写入逻辑 =====================
			tqmtscb00_dr.Reset();  // 重置模型，清空上一次数据
			// 1. 字段映射赋值（严格按要求对应）
			tqmtscb00_dr["MAT_CODE"] = tfbsm03["MAT_CODE"];          // MAT_CODE → MAT_CODE
			tqmtscb00_dr["MAT_CODE_NAME"] = tfbsm03["MAT_NAME"];     // MAT_NAME → MAT_CODE_NAME
			tqmtscb00_dr["USE_CR"] = tfbsm03["CR_IN"];               // CR_IN → USE_CR
			tqmtscb00_dr["USE_NI"] = tfbsm03["NI_IN"];               // NI_IN → USE_NI
			tqmtscb00_dr["USE_MO"] = tfbsm03["MO_IN"];               // MO_IN → USE_MO
			tqmtscb00_dr["UNIT_PRICE"] = tfbsm03["UNIT_PRICE"];      // UNIT_PRICE → UNIT_PRICE
			tqmtscb00_dr["TYPE_CONVERT"] = tfbsm03["TYPE_CONVERT"];  // TYPE_CONVERT → TYPE_CONVERT
			tqmtscb00_dr["DATE_C"] = tfbsm03["VERSION_D"];           // VERSION_D → DATE_C
			tqmtscb00_dr["XY_FLAG"] = "是";

			// 2. 按唯一键（DATE_C + MAT_CODE）查询是否存在数据（沿用原有逻辑的唯一键设计）
			if (tqmtscb00_dr.QueryCount("DATE_C,MAT_CODE") > 0) {
				// 存在相同数据：先删后插（更新逻辑）
				tqmtscb00_dr_copy.CopyFrom(tqmtscb00_dr);
				tqmtscb00_dr.Delete("DATE_C,MAT_CODE");  // 删除原数据
				// 填充审计字段（沿用原有审计字段命名规范）
				tqmtscb00_dr_copy["REC_REVISE_TIME"] = datetime;
				tqmtscb00_dr_copy["REC_REVISOR"] = s.userid;
				tqmtscb00_dr_copy.Insert();  // 插入新数据（更新）
			}
			else {
				// 无相同数据：直接插入
				tqmtscb00_dr["REC_CREATE_TIME"] = datetime;
				tqmtscb00_dr["REC_CREATOR"] = s.userid;
				tqmtscb00_dr.Insert();  // 插入新数据
			}

		}
		Log::Trace("", "", "镍单价unit_ni = {0}", unit_ni.ToString());
		// ===================== 新增：效益价格计算&更新逻辑 =====================
		if (date_c_value!=" " && bcls_rec->Tables[0].Rows.get_Count() > 0)
		{
			// 1. 计算效益铬钢单价（铬）
			sqlstr = " select decode(USE_CR,0,0,round(UNIT_PRICE/USE_CR,3)) from tqmtscb00_dr"
				" where 1=1"
				" and PRICE_TYPE = ' '"
				" and mat_code = 'AT000329'"  //铬
				" and date_c = @date_c";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("date_c", date_c_value);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				unit_cr = cmd_inq.GetDecimal(1); 
			}
			cmd_inq.Close();
			Log::Trace("", "", "铬单价unit_cr = {0}", unit_cr.ToString());

			// 2. 计算效益镍钢单价（镍）
			sqlstr = " select decode(USE_NI,0,0,round(UNIT_PRICE/USE_NI,3)) from tqmtscb00_dr"
				" where 1=1"
				" and PRICE_TYPE = ' '"
				" and mat_code = 'AT000385'"  //镍
				" and date_c = @date_c";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("date_c", date_c_value);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				unit_ni = cmd_inq.GetDecimal(1); 
			}
			cmd_inq.Close();
			Log::Trace("", "", "镍单价unit_ni = {0}", unit_ni.ToString());

			// 3. 计算钼单价
			sqlstr = " select decode(USE_MO,0,0,round(UNIT_PRICE/USE_MO,3)) from tqmtscb00_dr"
				" where 1=1"
				" and PRICE_TYPE = ' '"
				" and mat_code = 'AT000331'"  //钼
				" and date_c = @date_c";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("date_c", date_c_value);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				unit_mo = cmd_inq.GetDecimal(1); 
			}
			cmd_inq.Close();
			Log::Trace("", "", "钼单价unit_mo = {0}", unit_mo.ToString());

			// 4. 计算铁水单价
			sqlstr = " select round(UNIT_PRICE/100,3) from tqmtscb00_dr"
				" where 1=1"
				" and PRICE_TYPE = ' '"
				" and mat_code = 'TS0000'"  //铁水
				" and date_c = @date_c";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("date_c", date_c_value);
			cmd_inq.ExecuteReader();
			if (cmd_inq.Read())
			{
				unit_fe = cmd_inq.GetDecimal(1);  
			}
			cmd_inq.Close();
			Log::Trace("", "", "铁水单价unit_fe = {0}", unit_fe.ToString());

			// 5. 更新效益价格（UNIT_PRICE_NI/UNIT_PRICE_CR）
			Log::Trace("", "", "linke = {0} | 开始更新TQMTSCB00_DR表效益价格", __LINE__);
			sqlstr = " update tqmtscb00_dr set UNIT_PRICE_NI = round(USE_CR*@unit_cr+USE_NI*@unit_ni+USE_MO*@unit_mo-UNIT_PRICE,2)"
				",UNIT_PRICE_CR = round(USE_CR*@unit_cr+USE_NI*@unit_ni+USE_MO*@unit_mo+(99-USE_CR-USE_NI-USE_MO)*@unit_fe-UNIT_PRICE,2)"
				" where 1=1"
				" and PRICE_TYPE = ' '"
				" and XY_FLAG = '是'"
				" and date_c = @date_c";
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("date_c", date_c_value);
			cmd_inq.Parameters.Set("unit_cr", unit_cr);
			cmd_inq.Parameters.Set("unit_ni", unit_ni);
			cmd_inq.Parameters.Set("unit_mo", unit_mo);
			cmd_inq.Parameters.Set("unit_fe", unit_fe);
			int updateCount = cmd_inq.ExecuteNonQuery();  // 执行更新
			cmd_inq.Close();
			Log::Trace("", "", "linke = {0} | TQMTSCB00_DR表效益价格更新完成，影响行数={1}", __LINE__, updateCount);
		}

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
