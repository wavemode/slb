#ifndef __SLB_CLASS__
#define __SLB_CLASS__

#include "SPP.hpp"
#include "Export.hpp"
#include "Debug.hpp"
#include "ClassInfo.hpp"
#include "ClassHelpers.hpp"
#include "Manager.hpp"
#include "FuncCall.hpp"
#include "Value.hpp"
#include "Instance.hpp"
#include <typeinfo>
#include <map>
#include <vector>
#include <string>

struct lua_State;

namespace SLB {

	template< typename T, typename W = Instance::Default >
	class Class {
	public:
		typedef Class<T,W> __Self;

		Class(const char *name);

		__Self &rawSet(const char *name, Object *obj);

		template<typename TValue>
		__Self &set(const char *name, const TValue &obj)
		{ return rawSet(name, (Object*) Value::copy(obj)); }

		template<typename TValue>
		__Self &set_ref(const char *name, TValue obj)
		{ return rawSet(name, Value::ref(obj)); }

		template<typename TValue>
		__Self &set_autoDelete(const char *name, TValue *obj)
		{ return rawSet(name, Value::autoDelete(obj)); }

		__Self &constructor();

		template<typename TBase>
		__Self &inherits()
		{ _class->inheritsFrom<T,TBase>(); return *this;}
		
		__Self &__add()
		{ SLB_DEBUG(0, "NOT IMPLEMENTED!"); return *this; }

		__Self &__mult()
		{ SLB_DEBUG(0, "NOT IMPLEMENTED!"); return *this; }

		#define SLB_REPEAT(N) \
		\
			/* Methods */ \
			template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
			__Self &set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) ); \
		\
			/* CONST Methods */ \
			template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
			__Self &set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) const ); \
		\
			/* C-functions  */ \
			template<class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
			__Self &set(const char *name, R (func)(SPP_ENUM_D(N,T)) ); \
		\
			/* constructors */ \
			template<class T0 SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
			__Self &constructor(); \

		SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
		#undef SLB_REPEAT

	protected:
		ClassInfo *_class;

	};
	
	template<typename T, typename W>
	Class<T,W>::Class(const char *name)
	{
		// we expect to have a template "Implementation" inside W
		typedef typename W::template Implementation<T> Adapter;
		_class = Manager::getInstance().getOrCreateClass( typeid(T) );
		_class->setName( name );
		_class->setInstanceFactory(new InstanceFactoryAdapter< T, Adapter >() );
		SLB_DEBUG(1, "Class declaration for %s[%s]", name, typeid(T).name());
	}
	
	template<typename T, typename W>
	inline Class<T,W> &Class<T,W>::rawSet(const char *name, Object *obj)
	{
		_class->set(name, obj);
		return *this;
	}
	
	template<typename T,  typename W>
	inline Class<T,W> &Class<T,W>::constructor()
	{
		_class->setConstructor( FuncCall::classConstructor<T>() );
		return *this;
	}

	#define SLB_REPEAT(N) \
	\
		/* Methods */ \
		template<typename T, typename W>\
		template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
		inline Class<T,W> &Class<T,W>::set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) ){ \
			if (typeid(T) != typeid(C)) inherits<C>();\
			return rawSet(name, FuncCall::create(func)); \
		} \
	\
		/* CONST Methods */ \
		template<typename T, typename W>\
		template<class C, class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
		inline Class<T,W> &Class<T,W>::set(const char *name, R (C::*func)(SPP_ENUM_D(N,T)) const ){ \
			if (typeid(T) != typeid(C)) inherits<C>();\
			return rawSet(name, FuncCall::create(func)); \
		} \
	\
		/* C-functions  */ \
		template<typename T, typename W> \
		template<class R SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
		inline Class<T,W> &Class<T,W>::set(const char *name, R (func)(SPP_ENUM_D(N,T)) ){ \
			return rawSet(name, FuncCall::create(func)); \
		} \
	\
		/* constructor  */ \
		template<typename T, typename W> \
		template<class T0 SPP_COMMA_IF(N) SPP_ENUM_D(N, class T)> \
		inline Class<T,W> &Class<T,W>::constructor(){ \
			FuncCall *fc = FuncCall::classConstructor<T,T0 SPP_COMMA_IF(N) SPP_ENUM_D(N,T)>();\
			_class->setConstructor( fc );\
			return *this; \
		} \

	SPP_MAIN_REPEAT_Z(MAX,SLB_REPEAT)
	#undef SLB_REPEAT
}


#endif
