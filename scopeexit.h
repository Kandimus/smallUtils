// Copyright (C) Nikita Shipovalov

#pragma once

#include <utility>

namespace su
{
// The purpose of this thing is to ensure code execution on scope exit
// The preferred usage is with macro below (less code needed):
//
// SU_ON_SCOPE_EXIT( do_Some_Stuff )

template <typename F>
class ScopeExit
{
public:
    ScopeExit( ScopeExit&& ) = default;

    ScopeExit( F&& f ) : m_f( std::move(f) ) {}
    ~ScopeExit() { m_f(); }
 
private:
    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit& operator=(ScopeExit&&) = delete;

    F m_f;
};

template <typename F>
ScopeExit<F> makeScopeExit( F&& f )
{
    return ScopeExit<F>( std::move(f) );
};


} //namespace su

/*
    Utility macro to help adding several scope exiters in the same scope:

    {
        Class* var = new Class();
        SU_ON_SCOPE_EXIT( delete var; )
        stuff();
        SU_ON_SCOPE_EXIT( anotherStuffOnScopeExit(); )
    } // <- execution of scope exiters is here in the same order as they were declared
*/
#define SU_STRING_JOIN2(arg1, arg2)    SU_DO_STRING_JOIN2(arg1, arg2)
#define SU_DO_STRING_JOIN2(arg1, arg2) arg1 ## arg2
#define SU_ON_SCOPE_EXIT(code)         const auto SU_STRING_JOIN2(scope_exit_, __LINE__) = su::makeScopeExit( [&](){ code; } )

