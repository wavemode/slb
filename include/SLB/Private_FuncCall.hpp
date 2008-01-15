/*
    SLB - Simple Lua Binder
    Copyright (C) 2007 Jose L. Hidalgo Valiño (PpluX)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

	Jose L. Hidalgo (www.pplux.com)
	pplux@pplux.com
*/

#ifndef __SLB_PRIVATE_FUNC_CALL__
#define __SLB_PRIVATE_FUNC_CALL__

#include "SPP.hpp"
#include "PushGet.hpp"
#include "ClassInfo.hpp"
#include "Manager.hpp"
#include "lua.hpp"
#include <typeinfo>

namespace SLB {
namespace Private {

//----------------------------------------------------------------------------
//-- FuncCall Implementations ------------------------------------------------
//----------------------------------------------------------------------------
	template<class T>
	class FC_Function; //> FuncCall to call functions (C static functions)

	template<class C, class T>
	class FC_Method; //> FuncCall to call Class methods

	template<class C, class T>
	class FC_ConstMethod; //> FuncCall to call Class const methods

	template<class> 
	class FC_ClassConstructor;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

	// SLB_INFO: Collects info of the arguments
	#define SLB_INFO_PARAMS(N) _Targs.push_back(&typeid(T##N));
	#define SLB_INFO(RETURN, N) \
		_Treturn = &typeid(RETURN);\
		SPP_REPEAT(N,SLB_INFO_PARAMS ) \

	// SLB_GET: Generates Code to get N parameters 
	//
	//    N       --> Number of parameters
	//    START   --> where to start getting parameters
	//                n=0   means start from the top
	//                n>0   start at top+n (i.e. with objects first parameter is the object)
	//
	//    For each paramter a param_n variable is generated with type Tn
	#define SLB_GET_PARAMS(N, START) T##N param_##N = SLB::Private::Type<T##N>::get(L,N + (START) );
	#define SLB_GET(N,START) \
		if (lua_gettop(L) != N + (START) ) \
		{ \
			lua_pushfstring(L, "Error number of arguments (given %d -> expected %d)", \
					lua_gettop(L)-(START), N); \
			lua_error(L);\
		}\
		SPP_REPEAT_BASE(N,SLB_GET_PARAMS, (START) ) \
		

	// FC_Method (BODY) 
	//    N       --> Number of parameters
	//
	// ( if is a const method )
	//    NAME    --> FC_Method   |  FC_ConstMethod
	//    CONST   --> /*nothing*/ |  const 
	// 
	// ( if returns or not a value)
	//    HEADER  --> class R    |  /*nothing*/
	//    RETURN  --> R          |  void 
	//
	//    ...     --> implementation of call function
	#define SLB_FC_METHOD_BODY(N,NAME, CONST, HEADER,RETURN, ...) \
	\
		template<class C HEADER SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
		class NAME <C,RETURN (SPP_ENUM_D(N,T))> : public FuncCall { \
		public: \
			NAME( RETURN (C::*func)(SPP_ENUM_D(N,T)) CONST ) : _func(func) \
			{\
			}\
		protected: \
			int call(lua_State *L) \
			{ \
				__VA_ARGS__ \
			} \
		private: \
			RETURN (C::*_func)(SPP_ENUM_D(N,T)) CONST; \
		}; \


	// FC_Method (implementation with return or not a value):
	// ( if is a const method )
	//    NAME    --> FC_Method   |  FC_ConstMethod
	//    CONST   --> /*nothing*/ |  const 
	#define SLB_FC_METHOD(N, NAME, CONST) \
		SLB_FC_METHOD_BODY(N, NAME, CONST, SPP_COMMA class R ,R, \
				CONST C *obj = SLB::get<CONST C*>(L,1); \
				if (obj == 0) luaL_error(L, "Invalid object for this method");\
				SLB_GET(N,1) \
				R value = (obj->*_func)(SPP_ENUM_D(N,param_)); \
				SLB::push<R>(L, value); \
				return 1; \
			) \
		SLB_FC_METHOD_BODY(N, NAME, CONST, /*nothing*/ , void,	\
				CONST C *obj = SLB::get<CONST C*>(L,1); \
				if (obj == 0) luaL_error(L, "Invalid object for this method");\
				SLB_GET(N,1) \
				(obj->*_func)(SPP_ENUM_D(N,param_)); \
				return 0; \
			)

	// FC_Method (with const methods or not)
	#define SLB_REPEAT(N) \
		SLB_FC_METHOD(N, FC_ConstMethod,  const) /* With const functions */ \
		SLB_FC_METHOD(N, FC_Method, /* nothing */ ) /* with non const functions */

	SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
	#undef SLB_REPEAT


	// FC_Function (Body)
	//    N       --> Number of parameters
	//
	// ( if returns or not a value)
	//    HEADER  --> class R    |  /*nothing*/
	//    RETURN  --> R          |  void 
	//
	//    ...     --> implementation of call function
	#define SLB_FC_FUNCTION_BODY(N, HEADER, RETURN, ...) \
	\
		template< HEADER  SPP_ENUM_D(N, class T)> \
		class FC_Function< RETURN (SPP_ENUM_D(N,T))> : public FuncCall { \
		public: \
			FC_Function( RETURN (*func)(SPP_ENUM_D(N,T)) ) : _func(func) {\
				SLB_INFO(RETURN, N) \
			} \
		protected: \
			virtual ~FC_Function() {} \
			int call(lua_State *L) \
			{ \
				__VA_ARGS__ \
			} \
		private: \
			RETURN (*_func)(SPP_ENUM_D(N,T)); \
		}; 
	
	#define SLB_FC_FUNCTION(N) \
		SLB_FC_FUNCTION_BODY( N, class R SPP_COMMA_IF(N), R, \
				SLB_GET(N,0) \
				R value = (_func)(SPP_ENUM_D(N,param_)); \
				SLB::push<R>(L, value); \
				return 1; \
		)\
		SLB_FC_FUNCTION_BODY( N, /* nothing */ , void, \
				SLB_GET(N,0) \
				(_func)(SPP_ENUM_D(N,param_)); \
				return 0; \
		)

	SPP_MAIN_REPEAT_Z(MAX,SLB_FC_FUNCTION)
	#undef SLB_FC_METHOD
	#undef SLB_FC_METHOD_BODY
	#undef SLB_FC_FUNCTION
	#undef SLB_FC_FUNCTION_BODY

	#define SLB_REPEAT(N) \
		template<class C SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
		struct FC_ClassConstructor<C(SPP_ENUM_D(N,T))> : public FuncCall\
		{\
		public:\
			FC_ClassConstructor() {} \
		protected: \
			int call(lua_State *L) \
			{ \
				ClassInfo *c = Manager::getInstance().getClass(typeid(C)); \
				if (c == 0) luaL_error(L, "Class %s is not avaliable! ", typeid(C).name()); \
				SLB_GET(N, 1); \
				Private::Type<C*>::push(L, new C( SPP_ENUM_D(N,param_) ), true ); \
				return 1; \
			} \
		}; \

	SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
	#undef SLB_REPEAT

	#undef SLB_GET
	#undef SLB_GET_PARAMS


} // end of SLB::Private namespace	

#endif
