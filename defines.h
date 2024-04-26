
#ifndef _SMALLUTILL_ASSERT_H_
#define _SMALLUTILL_ASSERT_H_
#pragma once

#ifndef SU_ASSERT
#ifdef _DEBUG
#define SU_ASSERT(exp)      (void)( (!!(exp)) || (__debugbreak(), 1) )
#define SU_BREAKPOINT()     (__debugbreak())
#else
#define SU_ASSERT(x)        ((void)0)
#define SU_BREAKPOINT()     ((void)0)
#endif
#endif

#endif
