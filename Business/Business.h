#pragma once

/*************************************************
  Copyright (C), 2000-2007, Tianxiang Invest. Co., Ltd.
  File name:	Business.h
  Author:       赵宏俊
  Version:		1.0
  Date:			2007-09-04
  
  Description:
				业务层环境定义

*************************************************/

//////////////////////////////////////////////////////////////////////////
// 环境定义
//////////////////////////////////////////////////////////////////////////
#ifdef BUSINESS_INTERNAL
# define BUSINESS_EXT __declspec(dllexport)
#elif defined BUSINESS_STATIC
# define BUSINESS_EXT
#else
# define BUSINESS_EXT __declspec(dllimport)
#endif

