/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2026/3/5
Description: 配料单的物料代码和库存信息
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm15z_material_inq)

int f_fbsm15z_material_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
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

		Log::Trace("", __FUNCTION__, "st_no = [{0}],station_id = [{1}]", st_no, station_id);
		
		//查询对应的st_no 是否有配料
		sqlstr = "select count(1) "
			" from tfbsm01"
			" where st_no = @st_no"
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("st_no", st_no);
		v_count = cmd_inq.ExecuteScalar().ToInt32();
		
		if (v_count == 0)
		{
			//取中类代表或是大类代表
			//获取需要对应取的钢种
			sqlstr = " select st_no,material_code,comm_fmly_code"
				" from tfbsm11 t1"
				" where exists( select 1 from tfbsm11 t2 where t1.MATERIAL_CODE=t2.MATERIAL_CODE and t2.st_no= @st_no)  "
				" and MARK_POS_CODE = '1'"
				" union "
				" select st_no,material_code,comm_fmly_code"
				" from tfbsm11 t1"
				" where exists( select 1 from tfbsm11 t2 where t1.COMM_FMLY_CODE=t2.COMM_FMLY_CODE and t2.st_no= @st_no)  "
				" and MARK_POS_CODE = '2'"
				;
			cmd.SetCommandText(sqlstr);
			cmd.Parameters.Set("st_no", st_no);
			cmd.ExecuteReader();
			if (cmd.Read())
			{
				st_no = cmd.GetString(1);
			}
			cmd.Close();
		}			
		cmd_inq.Close();

		Log::Trace("", __FUNCTION__, "st_no = [{0}]", st_no);

		//bcls_ret->Tables.SetTableName(0,"IF");
		sqlstr = " select  t3.mat_name ,t2.mat_code ,t2.Weight"
			" ,nvl(t3.MAT_TYPE_PL1,' ') as BACK_C2"
			" ,nvl(t3.MAT_TYPE_PL1 ,' ') category"
			",nvl(t4.code_desc, ' ') STOCK_NAME"
			",nvl(t2.LOT_NO,' ') LOT_NO"
			" ,t2.c_VALUE,t2.si_VALUE,t2.mn_VALUE,t2.p_VALUE,t2.s_VALUE,t2.cr_VALUE,t2.ni_VALUE "
			" ,t2.mo_VALUE,t2.cu_VALUE,t2.co_VALUE,t2.ti_VALUE,t2.nb_VALUE,t2.al_VALUE "
			", t2.b_VALUE, t2.n_VALUE, t2.ca_VALUE "
			" ,case when t3.YEILD_MINUS =0 then 100 else t3.YEILD_MINUS end MAT_YEILD"
			" from ("
			" select mat_code,LOT_NO,STOCK_NAME"
			", nvl(c, 0) c_VALUE, nvl(si, 0) si_VALUE, nvl(mn, 0) mn_VALUE, nvl(p, 0) p_VALUE, nvl(s, 0) s_VALUE, nvl(cr, 0) cr_VALUE, nvl(ni, 0) ni_VALUE "
			" ,nvl(mo,0) mo_VALUE,nvl(cu,0) cu_VALUE,nvl(co,0) co_VALUE,nvl(ti,0) ti_VALUE,nvl(nb,0) nb_VALUE,nvl(al,0) al_VALUE "
			", nvl(b, 0) b_VALUE, nvl(n, 0) n_VALUE, nvl(ca, 0) ca_VALUE "
			", case when STOCK_NAME = '5' then WEIGHT when  STOCK_NAME like 'VS%' and STOCK_NAME not in(select BUNKER_NO from tmmsm60 where CO_BUNKER = '1') then(WEIGHT / 1000) - 100 else  WEIGHT / 1000 END Weight"
			" from FBSM_13_AI  "
			" union "
			" select mat_code,' ' LOT_NO,'5' STOCK_NAME"
			", 0 c_VALUE, 0 si_VALUE, 0 mn_VALUE, 0 p_VALUE, 0 s_VALUE, 0 cr_VALUE, 0 ni_VALUE "
			" ,0 mo_VALUE,0 cu_VALUE,0 co_VALUE,0 ti_VALUE,0 nb_VALUE,0 al_VALUE "
			", 0 b_VALUE, 0 n_VALUE, 0 ca_VALUE "
			",0 Weight"
			" from tfbsm01 t1 where not exists(select 1 from FBSM_13_AI t13 where t1.mat_code = t13.mat_code)  and st_no=@st_no"
			" ) t2"
			" left join tmmsm50 t3 on t2.mat_code = t3.mat_code"
			" left join (select CODE, CODE_DESC_1_CONTENT as code_desc from tep0002 where code_class = 'FBSM13' union select BUNKER_NO as CODE, MAT_NAME as code_desc  from tmmsm60  where  BUNKER_NO like 'VS%') t4 on t2.STOCK_NAME = t4.code"
			" where 1=1"			
			" ORDER BY  CASE  WHEN t2.STOCK_NAME = '2' THEN 1 WHEN t2.STOCK_NAME = '3' THEN 2 WHEN t2.STOCK_NAME = '4' THEN 3 WHEN t2.STOCK_NAME = '1' THEN 4 WHEN t2.STOCK_NAME LIKE 'VS%' THEN 4 WHEN t2.STOCK_NAME = '5' THEN 6 ELSE 7 END "
			",t2.MAT_CODE"
			;
		cmd_inq_q.SetCommandText(sqlstr);
		cmd_inq_q.Parameters.Set("st_no", st_no);
		cmd_inq_q.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq_q.Close();

		Log::Trace("", __FUNCTION__, "物料编码执行结束 ");

		

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
