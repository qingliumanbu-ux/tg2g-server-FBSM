/*************************************************
Copyright: Baosight Software LTD.co Copyright (c) 2021
Author:      XXX
Version:     1.0
Date:        2026/3/5
Description: 配料单的物料代码和库存信息（配料后查询）
**************************************************/
#include "stdafx.h"
BM2F_ENTERACE(fbsm41_material_inq)

int f_fbsm41_material_inq(EIClass *bcls_rec, EIClass *bcls_ret, CDbConnection *conn)
{
	CTracer log(__FUNCTION__);
	int doFlag = 0;
	CString sqlstr = " ";
	CDbCommand cmd_inq_q(conn);
	CString compose_list_no = "";

	try
	{
		compose_list_no = bcls_rec->Tables[0].Rows[0]["COMPOSE_LIST_NO"].ToString().Trim();
		Log::Trace("", __FUNCTION__, "compose_list_no = [{0}]", compose_list_no);

		sqlstr = " WITH "
			" raw_component_81 AS ( "
			"   SELECT * FROM ( "
			"     SELECT QUALITY_BATCH_NO, MAT_CODE, "
			"       MAX(DECODE(ELM_NAME, 'Mn', ELM_VALUE)) AS Mn, "
			"       MAX(DECODE(ELM_NAME, 'Ni', ELM_VALUE)) AS Ni, "
			"       MAX(DECODE(ELM_NAME, 'P', ELM_VALUE)) AS P, "
			"       MAX(DECODE(ELM_NAME, 'C', ELM_VALUE)) AS C, "
			"       MAX(DECODE(ELM_NAME, 'Si', ELM_VALUE)) AS Si, "
			"       MAX(DECODE(ELM_NAME, 'S', ELM_VALUE)) AS S, "
			"       MAX(DECODE(ELM_NAME, 'Cr', ELM_VALUE)) AS Cr, "
			"       MAX(DECODE(ELM_NAME, 'Mo', ELM_VALUE)) AS Mo, "
			"       MAX(DECODE(ELM_NAME, 'Cu', ELM_VALUE)) AS Cu, "
			"       MAX(DECODE(ELM_NAME, 'Co', ELM_VALUE)) AS CO, "
			"       MAX(DECODE(ELM_NAME, 'Ti', ELM_VALUE)) AS TI, "
			"       MAX(DECODE(ELM_NAME, 'Nb', ELM_VALUE)) AS NB, "
			"       MAX(DECODE(ELM_NAME, 'Al', ELM_VALUE)) AS AL, "
			"       MAX(DECODE(ELM_NAME, 'V', ELM_VALUE)) AS V, "
			"       MAX(DECODE(ELM_NAME, 'B', ELM_VALUE)) AS B, "
			"       MAX(DECODE(ELM_NAME, 'N', ELM_VALUE)) AS N, "
			"       MAX(DECODE(ELM_NAME, 'Ca', ELM_VALUE)) AS Ca, "
			"       1 AS RN "
			"     FROM tmmsm81al "
			"     GROUP BY QUALITY_BATCH_NO, MAT_CODE "
			"   ) "
			" ), "
			" raw_component_zj AS ( "
			"   SELECT COALESCE(b.MAT_CODE, a.MAT_CODE) AS MAT_CODE, "
			"     a.Mn AS Mn, "
			"     DECODE(b.Ni, 0, a.Ni, NULL, a.Ni, b.Ni) AS Ni, "
			"     a.P AS P, "
			"     a.C AS C, "
			"     a.Si AS Si, "
			"     a.S AS S, "
			"     DECODE(b.Cr, 0, a.Cr, NULL, a.Cr, b.Cr) AS Cr, "
			"     DECODE(b.Mo, 0, a.Mo, NULL, a.Mo, b.Mo) AS Mo, "
			"     a.Cu AS Cu, "
			"     a.CO AS CO, "
			"     a.TI AS TI, "
			"     a.NB AS NB, "
			"     a.AL AS AL, "
			"     a.V AS V, "
			"     a.B AS B, "
			"     a.N AS N, "
			"     a.Ca AS Ca "
			"   FROM ( "
			"     SELECT * FROM ( "
			"       SELECT ' ' AS QUALITY_BATCH_NO, MAT_CODE, Mn, Ni, P, C, Si, S, Cr, Mo, Cu, CO, TI, NB, AL, V, B, N, Ca, "
			"         ROW_NUMBER() OVER(PARTITION BY MAT_CODE ORDER BY REC_CREATE_TIME DESC) as rn "
			"       FROM ( "
			"         SELECT QUALITY_BATCH_NO, MAT_CODE, REC_CREATE_TIME, "
			"           MAX(DECODE(ELM_NAME, 'Mn', ELM_VALUE)) AS Mn, "
			"           MAX(DECODE(ELM_NAME, 'Ni', ELM_VALUE)) AS Ni, "
			"           MAX(DECODE(ELM_NAME, 'P', ELM_VALUE)) AS P, "
			"           MAX(DECODE(ELM_NAME, 'C', ELM_VALUE)) AS C, "
			"           MAX(DECODE(ELM_NAME, 'Si', ELM_VALUE)) AS Si, "
			"           MAX(DECODE(ELM_NAME, 'S', ELM_VALUE)) AS S, "
			"           MAX(DECODE(ELM_NAME, 'Cr', ELM_VALUE)) AS Cr, "
			"           MAX(DECODE(ELM_NAME, 'Mo', ELM_VALUE)) AS Mo, "
			"           MAX(DECODE(ELM_NAME, 'Cu', ELM_VALUE)) AS Cu, "
			"           MAX(DECODE(ELM_NAME, 'Co', ELM_VALUE)) AS CO, "
			"           MAX(DECODE(ELM_NAME, 'Ti', ELM_VALUE)) AS TI, "
			"           MAX(DECODE(ELM_NAME, 'Nb', ELM_VALUE)) AS NB, "
			"           MAX(DECODE(ELM_NAME, 'Al', ELM_VALUE)) AS AL, "
			"           MAX(DECODE(ELM_NAME, 'V', ELM_VALUE)) AS V, "
			"           MAX(DECODE(ELM_NAME, 'B', ELM_VALUE)) AS B, "
			"           MAX(DECODE(ELM_NAME, 'N', ELM_VALUE)) AS N, "
			"           MAX(DECODE(ELM_NAME, 'Ca', ELM_VALUE)) AS Ca "
			"         FROM tmmsm81al "
			"         GROUP BY QUALITY_BATCH_NO, MAT_CODE, REC_CREATE_TIME "
			"       ) t "
			"     ) sub "
			"     WHERE sub.rn = 1 "
			"   ) a "
			"   FULL OUTER JOIN ( "
			"     SELECT * FROM ( "
			"       SELECT MAT_ID as MAT_CODE, '' AS Mn, Ni, '' AS P, '' AS C, '' AS Si, '' AS S, "
			"         Cr, Mo, '' AS Cu, '' AS CO, '' AS TI, '' AS NB, '' AS AL, '' AS V, '' AS B, '' AS N, '' AS Ca, 1 as rn "
			"       FROM ZJ_MAT_ELEMENT "
			"     ) "
			"   ) b ON a.MAT_CODE = b.MAT_CODE "
			" ), "
			" raw_component_wq08 AS ( "
			"   SELECT COALESCE(d.MAT_CODE, b.MAT_CODE, a.MAT_CODE) AS MAT_CODE, "
			"     COALESCE(b.Mn, a.Mn) AS Mn, "
			"     COALESCE(d.Ni, a.Ni, b.Ni) AS Ni, "
			"     a.P AS P, "
			"     a.C AS C, "
			"     a.Si AS Si, "
			"     a.S AS S, "
			"     COALESCE(d.Cr, a.Cr, b.Cr) AS Cr, "
			"     COALESCE(d.Mo, a.Mo, b.Mo) AS Mo, "
			"     COALESCE(b.Cu, a.Cu) AS Cu, "
			"     a.CO AS CO, "
			"     a.TI AS TI, "
			"     a.NB AS NB, "
			"     a.AL AS AL, "
			"     a.V AS V, "
			"     a.B AS B, "
			"     a.N AS N, "
			"     a.Ca AS Ca "
			"   FROM ( "
			"     SELECT * FROM ( "
			"       SELECT ' ' AS QUALITY_BATCH_NO, MAT_CODE, Mn, Ni, P, C, Si, S, Cr, Mo, Cu, CO, TI, NB, AL, V, B, N, Ca, "
			"         ROW_NUMBER() OVER(PARTITION BY MAT_CODE ORDER BY REC_CREATE_TIME DESC) as rn "
			"       FROM ( "
			"         SELECT QUALITY_BATCH_NO, MAT_CODE, REC_CREATE_TIME, "
			"           MAX(DECODE(ELM_NAME, 'Mn', ELM_VALUE)) AS Mn, "
			"           MAX(DECODE(ELM_NAME, 'Ni', ELM_VALUE)) AS Ni, "
			"           MAX(DECODE(ELM_NAME, 'P', ELM_VALUE)) AS P, "
			"           MAX(DECODE(ELM_NAME, 'C', ELM_VALUE)) AS C, "
			"           MAX(DECODE(ELM_NAME, 'Si', ELM_VALUE)) AS Si, "
			"           MAX(DECODE(ELM_NAME, 'S', ELM_VALUE)) AS S, "
			"           MAX(DECODE(ELM_NAME, 'Cr', ELM_VALUE)) AS Cr, "
			"           MAX(DECODE(ELM_NAME, 'Mo', ELM_VALUE)) AS Mo, "
			"           MAX(DECODE(ELM_NAME, 'Cu', ELM_VALUE)) AS Cu, "
			"           MAX(DECODE(ELM_NAME, 'Co', ELM_VALUE)) AS CO, "
			"           MAX(DECODE(ELM_NAME, 'Ti', ELM_VALUE)) AS TI, "
			"           MAX(DECODE(ELM_NAME, 'Nb', ELM_VALUE)) AS NB, "
			"           MAX(DECODE(ELM_NAME, 'Al', ELM_VALUE)) AS AL, "
			"           MAX(DECODE(ELM_NAME, 'V', ELM_VALUE)) AS V, "
			"           MAX(DECODE(ELM_NAME, 'B', ELM_VALUE)) AS B, "
			"           MAX(DECODE(ELM_NAME, 'N', ELM_VALUE)) AS N, "
			"           MAX(DECODE(ELM_NAME, 'Ca', ELM_VALUE)) AS Ca "
			"         FROM tmmsm81al "
			"         WHERE MAT_CODE LIKE 'F%' "
			"         GROUP BY QUALITY_BATCH_NO, MAT_CODE, REC_CREATE_TIME "
			"       ) t "
			"     ) sub "
			"     WHERE sub.rn = 1 "
			"   ) a "
			"   FULL OUTER JOIN ( "
			"     SELECT * FROM ( "
			"       SELECT MAT_CODE, '' AS Mn, Ni, '' AS P, '' AS C, '' AS Si, '' AS S, "
			"         Cr, Mo, '' AS Cu, '' AS CO, '' AS TI, '' AS NB, '' AS AL, '' AS V, '' AS B, '' AS N, '' AS Ca, 1 as rn "
			"       FROM ( "
			"         SELECT MAT_CODE, NI_VALUE AS Ni, CR_VALUE AS Cr, MO_VALUE AS Mo, "
			"           ROW_NUMBER() OVER(PARTITION BY MAT_CODE ORDER BY REC_CREATE_TIME DESC) AS rn "
			"         FROM TMMSMWQ "
			"         WHERE MAT_CODE LIKE 'F%' "
			"       ) "
			"       WHERE rn = 1 "
			"     ) "
			"   ) d ON a.MAT_CODE = d.MAT_CODE "
			"   FULL OUTER JOIN ( "
			"     SELECT * FROM ( "
			"       SELECT MAT_CODE, MN_VALUE AS Mn, NI_VALUE as Ni, '' AS P, '' AS C, '' AS Si, '' AS S, "
			"         Cr_VALUE as Cr, Mo_VALUE as Mo, Cu_VALUE AS Cu, '' AS CO, '' AS TI, '' AS NB, '' AS AL, '' AS V, '' AS B, '' AS N, '' AS Ca, 1 as rn "
			"       FROM tfbsm08 "
			"       WHERE MAT_CODE LIKE 'F%' "
			"     ) t "
			"   ) b ON COALESCE(a.MAT_CODE, d.MAT_CODE) = b.MAT_CODE "
			" ), "
			" base_inv14 AS ( "
			"   SELECT MAT_CODE, STOCK_WT, BUNKER_NO AS STOCK_NAME, '2-NORTH-SCRAP' AS SOURCE_TYPE, "
			"     LOT_NO, QUALITY_BATCH_NO, BUNKER_NO AS BUNKER_INFO "
			"   FROM ( "
			"     SELECT A.MAT_CODE, A.BUNKER_NO, A.LOT_NO, A.QUALITY_BATCH_NO, SUM(A.STOCK_WT) STOCK_WT "
			"     FROM tmmsm85 A "
			"     JOIN ( "
			"       SELECT t60.MAT_CODE, t60.BUNKER_NO FROM tmmsm60 t60 "
			"       WHERE (t60.BUNKER_NO LIKE 'VS%' AND t60.BUNKER_FLAG NOT IN ('4') OR t60.BUNKER_FLAG = '3') "
			"     ) cfg ON A.MAT_CODE = cfg.MAT_CODE AND A.BUNKER_NO = cfg.BUNKER_NO "
			"     WHERE A.RECV_DEPT_CODE = '624002048' "
			"     GROUP BY A.MAT_CODE, A.BUNKER_NO, A.LOT_NO, A.QUALITY_BATCH_NO "
			"   ) "
			" ), "
			" base_inv57 AS ( "
			"   SELECT A.MAT_CODE, SUM(A.STOCK_WT) AS STOCK_WT, '1' AS STOCK_NAME, '7-RECYCLE-SCRAP' AS SOURCE_TYPE, "
			"     NULL AS LOT_NO, NULL AS QUALITY_BATCH_NO, LISTAGG(B.BUNKER_NO, ', ') WITHIN GROUP (ORDER BY B.BUNKER_NO) AS BUNKER_INFO "
			"   FROM tmmsm85 A "
			"   JOIN ( "
			"     SELECT MAT_CODE, BUNKER_NO FROM tmmsm60 "
			"     WHERE (BUNKER_NO LIKE 'VS%' OR BUNKER_FLAG = '11') "
			"       AND BUNKER_FLAG NOT IN ('12') "
			"       AND BUNKER_NO NOT IN ('VS04','VS11') "
			"   ) B ON A.MAT_CODE = B.MAT_CODE AND A.BUNKER_NO = B.BUNKER_NO "
			"   WHERE A.UNLOAD_AREA_CODE<>'624002048' AND A.RECV_DEPT_CODE<>'624002048' "
			"   GROUP BY A.MAT_CODE "
			" ), "
			" base_inv6 AS ( "
			"   SELECT MAT_CODE, SUM(STOCK_WT * 1000) AS STOCK_WT, '1' AS STOCK_NAME, '6-SCRAP-YARD-1' AS SOURCE_TYPE, "
			"     NULL AS LOT_NO, NULL AS QUALITY_BATCH_NO, NULL AS BUNKER_INFO "
			"   FROM TFBSM57C "
			"   WHERE MAT_CODE LIKE 'F%' "
			"   GROUP BY MAT_CODE "
			" ) "
			" SELECT rownum AS seq_id, "
			"   m50.MAT_NAME, "
			"   src.MAT_CODE, "
			"   m50.MAT_TYPE_PL1 AS BACK_C2, "
			"   src.STOCK_NAME, "
			"   src.LOT_NO, "
			"   NVL(src.C, 0) AS C_VALUE, "
			"   NVL(src.SI, 0) AS SI_VALUE, "
			"   NVL(src.MN, 0) AS MN_VALUE, "
			"   NVL(src.P, 0) AS P_VALUE, "
			"   NVL(src.S, 0) AS S_VALUE, "
			"   NVL(src.CR, 0) AS CR_VALUE, "
			"   NVL(src.NI, 0) AS NI_VALUE, "
			"   NVL(src.MO, 0) AS MO_VALUE, "
			"   NVL(src.CU, 0) AS CU_VALUE, "
			"   NVL(src.CO, 0) AS CO_VALUE, "
			"   NVL(src.TI, 0) AS TI_VALUE, "
			"   NVL(src.NB, 0) AS NB_VALUE, "
			"   NVL(src.AL, 0) AS AL_VALUE, "
			"   NVL(src.B, 0) AS B_VALUE, "
			"   NVL(src.N, 0) AS N_VALUE, "
			"   NVL(src.CA, 0) AS CA_VALUE, "
			"   src.WEIGHT "
			" FROM ( "
			"   SELECT e.MAT_CODE, e.STOCK_NAME, e.QUALITY_BATCH_NO, e.LOT_NO, e.STOCK_WT AS WEIGHT, "
			"     p.Mn, p.Ni, p.P, p.C, p.Si, p.S, p.Cr, p.Mo, p.Cu, p.CO, p.TI, p.NB, p.AL, p.V, p.B, p.N, p.Ca "
			"   FROM base_inv14 e "
			"   LEFT JOIN raw_component_81 p ON e.MAT_CODE = p.MAT_CODE AND e.QUALITY_BATCH_NO = p.QUALITY_BATCH_NO "
			"   UNION ALL "
			"   SELECT e.MAT_CODE, e.STOCK_NAME, e.QUALITY_BATCH_NO, e.LOT_NO, e.STOCK_WT AS WEIGHT, "
			"     p.Mn, p.Ni, p.P, p.C, p.Si, p.S, p.Cr, p.Mo, p.Cu, p.CO, p.TI, p.NB, p.AL, p.V, p.B, p.N, p.Ca "
			"   FROM base_inv57 e "
			"   LEFT JOIN raw_component_zj p ON e.MAT_CODE = p.MAT_CODE "
			"   UNION ALL "
			"   SELECT e.MAT_CODE, e.STOCK_NAME, e.QUALITY_BATCH_NO, e.LOT_NO, e.STOCK_WT AS WEIGHT, "
			"     p.Mn, p.Ni, p.P, p.C, p.Si, p.S, p.Cr, p.Mo, p.Cu, p.CO, p.TI, p.NB, p.AL, p.V, p.B, p.N, p.Ca "
			"   FROM base_inv6 e "
			"   LEFT JOIN raw_component_wq08 p ON e.MAT_CODE = p.MAT_CODE "
			" ) src "
			" LEFT JOIN TMMSM50 m50 ON src.MAT_CODE = m50.MAT_CODE "
			" ORDER BY src.MAT_CODE ";
		
		Log::Trace("", __FUNCTION__, "SQL为[{0}]", sqlstr);
		cmd_inq_q.SetCommandText(sqlstr);
		cmd_inq_q.ExecuteQuery(bcls_ret->Tables[0]);
		cmd_inq_q.Close();

		Log::Trace("", __FUNCTION__, "物料编码执行结束 ");

		bcls_ret->Tables.Add();
        sqlstr = " SELECT  BUNKER_NO FROM TMMSM60 WHERE BUNKER_TYPE = 'EAFBOX' "
					 " ORDER BY BUNKER_NO "
            ;
		cmd_inq_q.SetCommandText(sqlstr);
        Log::Trace("", "", "sqlstr = {0}", sqlstr);
		cmd_inq_q.ExecuteQuery(bcls_ret->Tables[1]);
		cmd_inq_q.Close();

		// CString str1 = "";
        // bcls_ret->WriteHTML(str1);
        // Log::Trace("", __FUNCTION__, "str=[{0}]", str1);
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
