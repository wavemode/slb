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

#ifndef __SLB_ERROR__
#define __SLB_ERROR__

#include "Export.hpp"
#include "lua.hpp"
#include <sstream>

namespace SLB {

	/** This class handles any error raised while lua is working.
	  You can subclass it and set it as default on SLB::Manager::setErrorHandler*/
	class SLB_EXPORT ErrorHandler
	{
	public:
		ErrorHandler() {}
		virtual ~ErrorHandler() {}

		// performs the lua_pcall using this errorHandler
		int lua_pcall(lua_State *L, int nargs, int nresults);

		/// first function to be called with the main error
		virtual void begin(const char *error) {}

		/** Last function that will be called at the end, should
		    return the string of the computed error. */
		virtual const char* end() = 0;

		/// Called on each stack element, here you can call SE_*
		/// functions. 
		virtual void stackElement(int level) {}

	protected:

		/// StackElement function
		/// returns the possible name of the function (can be null)
		const char *SE_name();

		/// StackElement function
		/// returns the "global", "local", "method", "field", "upvalue", or ""
		const char *SE_nameWhat();

		/// StackElement function
		/// returns "Lua", "C", "main", "tail"
		const char *SE_what();

		/// StackElement function
		/// returns If the function was defined in a string, then source is that string.
		/// If the function was defined in a file, then source starts with a '@' followed by the file name
		const char *SE_source();

		/// StackElement function
		/// shorten version of source
		const char *SE_shortSource(); 

		/// StackElement function
		/// number of line, or -1 if there is no info available
		int SE_currentLine();

		/// StackElement function
		/// number of upvalues of the function
		int SE_numberOfUpvalues();

		/// StackElement function
		/// the line number where the definition of the funciton starts
		int SE_lineDefined();

		/// StackElement function
		/// the last line where the definition of the function ends
		int SE_lastLineDefined();

		/// This function will be called by the handler to start the process of
		/// retrieving info from the lua failure. If you reinterpret this function
		/// Call to the parent's implementation.
		virtual void process(lua_State *L);

	private:
		static int _slb_stackHandler(lua_State *L);
		lua_Debug _debug;
		lua_State *_L;
	};


	class SLB_EXPORT DefaultErrorHandler : public ErrorHandler
	{
	public:
		virtual void begin(const char *error);
		virtual const char* end();
		virtual void stackElement(int level);
	private:
		std::ostringstream _out;
	};


#ifdef __SLB_TODO__
	class SLB_EXPORT ThreadSafeErrorHandler : public ErrorHandler
	{
	public:
		/** Will call EH but locking at the begin, and at the end. 
		    The ErrorHandler passed will be freed be owned by this
			object */
		ThreadSafeErrorHandler( ErrorHandler *EH );
		virtual ~ThreadSafeErrorHandler();
		virtual void begin(const char *error);
		virtual const char* end();

	private:
		ErrorHandler *_EH;
	};
#endif

} /* SLB */


#endif /* EOF */
