/****************************************************************************
 Copyright (c) 2011 cocos2d-x.org

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "LuaEngine.h"
#include <stdio.h>

extern "C" {
#include "lualib.h"
#include "lauxlib.h"
#include <android/log.h>
}


#define MAX_LEN 1024

void Log(const char * pszFormat, ...)
{
	char buf[MAX_LEN];
	va_list args;
	va_start(args, pszFormat);
	vsprintf(buf, pszFormat, args);
	va_end(args);	

	__android_log_print(ANDROID_LOG_DEBUG, "Pure Lua debug info", buf);
}

LuaEngine* LuaEngine::m_defaultEngine = NULL;

LuaEngine* LuaEngine::defaultEngine(void)
{
    if (!m_defaultEngine)
    {
        m_defaultEngine = LuaEngine::create();
    }
    return m_defaultEngine;
}

LuaEngine* LuaEngine::create(void)
{
    LuaEngine* pEngine = new LuaEngine();
    pEngine->init();
    return pEngine;
}

LuaEngine::~LuaEngine(void)
{
    lua_close(m_state);
    if (this == m_defaultEngine)
    {
        m_defaultEngine = NULL;
    }
}

bool LuaEngine::init(void)
{
    m_state = lua_open();
    luaL_openlibs(m_state);
    return true;
}

void LuaEngine::addSearchPath(const char* path)
{
    lua_getglobal(m_state, "package");                                  /* stack: package */
    lua_getfield(m_state, -1, "path");                /* get package.path, stack: package path */
    const char* cur_path =  lua_tostring(m_state, -1);
    lua_pop(m_state, 1);                                                /* stack: package */
    lua_pushfstring(m_state, "%s;%s/?.lua", cur_path, path);            /* stack: package newpath */
    lua_setfield(m_state, -2, "path");          /* package.path = newpath, stack: package */
    lua_pop(m_state, 1);                                                /* stack: - */
}

int LuaEngine::executeString(const char *codes)
{
    int nRet =    luaL_dostring(m_state, codes);
    lua_gc(m_state, LUA_GCCOLLECT, 0);

    if (nRet != 0)
    {
        Log("[LUA ERROR] %s", lua_tostring(m_state, -1));
        lua_pop(m_state, 1);
        return nRet;
    }
    return 0;
}

int LuaEngine::executeScriptFile(const char* filename)
{
    int nRet = luaL_dofile(m_state, filename);
    // lua_gc(m_state, LUA_GCCOLLECT, 0);

    if (nRet != 0)
    {
        Log("[LUA ERROR] %s", lua_tostring(m_state, -1));
        lua_pop(m_state, 1);
        return nRet;
    }
    return 0;
}

int LuaEngine::executeGlobalFunction(const char* functionName)
{
    lua_getglobal(m_state, functionName);       /* query function by name, stack: function */
    if (!lua_isfunction(m_state, -1))
    {
        Log("[LUA ERROR] name '%s' does not represent a Lua function", functionName);
        lua_pop(m_state, 1);
        return 0;
    }

    int error = lua_pcall(m_state, 0, 1, 0);             /* call function, stack: ret */
    // lua_gc(m_state, LUA_GCCOLLECT, 0);

    if (error)
    {
        Log("[LUA ERROR] %s", lua_tostring(m_state, - 1));
        lua_pop(m_state, 1); // clean error message
        return 0;
    }

    // get return value
    if (!lua_isnumber(m_state, -1))
    {
        lua_pop(m_state, 1);
        return 0;
    }

    int ret = lua_tointeger(m_state, -1);
    lua_pop(m_state, 1);                                                /* stack: - */
    return ret;
}

int LuaEngine::executeFunctionByHandler(int nHandler, int numArgs)
{
    if (pushFunction(nHandler))                                         /* stack: ... arg1 arg2 ... func */
    {
        if (numArgs > 0)
        {
            lua_insert(m_state, -(numArgs + 1));                        /* stack: ... func arg1 arg2 ... */
        }
        
        int traceback = 0;
        lua_getglobal(m_state, "__G__TRACKBACK__");                     /* stack: ... func arg1 arg2 ... G */
        if (!lua_isfunction(m_state, -1))
        {
            lua_pop(m_state, 1);                                        /* stack: ... func arg1 arg2 ... */
        }
        else
        {
            traceback = -(numArgs + 2);
            lua_insert(m_state, traceback);                             /* stack: ... G func arg1 arg2 ... */
        }
        
        int error = 0;
        error = lua_pcall(m_state, numArgs, 1, traceback);              /* stack: ... ret */
        if (error)
        {
            if (traceback == 0)
            {
                Log("[LUA ERROR] %s", lua_tostring(m_state, - 1));    /* stack: ... error */
                lua_pop(m_state, 1); // remove error message from stack
            }
            return 0;
        }
        
        // get return value
        int ret = 0;
        if (lua_isnumber(m_state, -1))
        {
            ret = lua_tointeger(m_state, -1);
        }
        else if (lua_isboolean(m_state, -1))
        {
            ret = lua_toboolean(m_state, -1);
        }
        
        lua_pop(m_state, 1); // remove return value from stack
        return ret;
    }
    else
    {
        lua_pop(m_state, numArgs); // remove args from stack
        return 0;
    }
}

void LuaEngine::cleanStack(void)
{
    lua_settop(m_state, 0);
}

void LuaEngine::addLuaLoader(lua_CFunction func)
{
    if (!func) return;

    // stack content after the invoking of the function
    // get loader table
    lua_getglobal(m_state, "package");                     // package
    lua_getfield(m_state, -1, "loaders");                  // package, loaders

    // insert loader into index 2
    lua_pushcfunction(m_state, func);                      // package, loaders, func
    for (int i = lua_objlen(m_state, -2) + 1; i > 2; --i)
    {
        lua_rawgeti(m_state, -2, i - 1);                   // package, loaders, func, function
                                                           // we call lua_rawgeti, so the loader table now is at -3
        lua_rawseti(m_state, -3, i);                       // package, loaders, func
    }
    lua_rawseti(m_state, -2, 2);                           // package, loaders

    // set loaders into package
    lua_setfield(m_state, -2, "loaders");                  // package

    lua_pop(m_state, 1);
}

bool LuaEngine::pushFunction(int nHandler)
{
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, nHandler);  /* stack: ... func */
    if (!lua_isfunction(m_state, -1))
    {
        Log("[LUA ERROR] function refid '%d' does not reference a Lua function", nHandler);
        lua_pop(m_state, 1);
        return false;
    }
    return true;
}

