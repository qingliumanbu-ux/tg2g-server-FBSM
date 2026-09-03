/// <summary>
/// 功能说明: 低代码通用保存服务-XX模块使用
/// </summary>
/// Copyright: Baosight Software LTD.co Copyright (c) 2010
/// Company: 上海宝信软件股份有限公司
/// Author:   项目组
/// Version:  1.0
/// History:  yyyy-MM-dd XXX [创建]

#include "stdafx.h"
#include "./Be2UserModel/SI/CFormDevConfig.h"

// Service 入口
BM2F_ENTERACE(fbsm_inq)
int f_fbsm_inq(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	int doFlag = 0;
	try
	{
		Log::Trace("", "", "st_to ={0}", __LINE__);
		doFlag = BE2::CFormDevConfig::SaveUtility(bcls_rec, bcls_ret, conn);
	}
	catch (CException& ex)
	{
		strcpy(s.msg, ex.GetMsg());
		s.flag = -1;
		doFlag = -1;
	}
	return doFlag;
}