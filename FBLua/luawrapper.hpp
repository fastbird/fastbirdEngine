#pragma once

/*
 * Copyright (c) 2010-2013 Alexander Ames
 * Alexander.Ames@gmail.com
 * See Copyright Notice at the end of this file
 */

// API Summary:
//
// LuaWrapper is a library designed to help bridge the gab between Lua and
// C++. It is designed to be small (a single header file), simple, fast,
// and typesafe. It has no external dependencies, and does not need to be
// precompiled; the header can simply be dropped into a project and used
// immediately. It even supports class inheritance to a certain degree. Objects
// can be created in either Lua or C++, and passed back and forth.
//
// The main functions of interest are the following:
//  luaW_is<T>
//  luaW_to<T>
//  luaW_check<T>
//  luaW_push<T>
//  luaW_register<T>
//  luaW_setfuncs<T>
//  luaW_extend<T, U>
//  luaW_hold<T>
//  luaW_release<T>
//
// These functions allow you to manipulate arbitrary classes just like you
// would the primitive types (e.g. numbers or strings). If you are familiar
// with the normal Lua API the behavior of these functions should be very
// intuative.
//
// For more information see the README and the comments below

#ifndef LUA_WRAPPER_H_
#define LUA_WRAPPER_H_

// If you are linking against Lua compiled in C++, define LUAW_NO_EXTERN_C
//#ifndef LUAW_NO_EXTERN_C
//extern "C"
//{
//#endif // LUAW_NO_EXTERN_C
//
//#include "lua/lua.h"
//#include "lua/lauxlib.h"
//    
//#ifndef LUAW_NO_EXTERN_C
//}
//#endif // LUAW_NO_EXTERN_C

#define LUAW_POSTCTOR_KEY "__postctor"
#define LUAW_EXTENDS_KEY "__extends"
#define LUAW_STORAGE_KEY "storage"
#define LUAW_CACHE_KEY "cache"
#define LUAW_CACHE_METATABLE_KEY "cachemetatable"
#define LUAW_HOLDS_KEY "holds"
#define LUAW_WRAPPER_KEY "LuaWrapper"

// A simple utility function to adjust a given index
// Useful for when a parameter index needs to be adjusted
// after pushing or popping things off the stack
inline int luaW_correctindex(lua_State* L, int index, int correction)
{
	L;
    return index < 0 ? index - correction : index;
}

// These are the default allocator and deallocator. If you would prefer an
// alternative option, you may select a different function when registering
// your class.
template <typename T>
T* luaW_defaultallocator(lua_State*)
{
    return new T();
}
template <typename T>
void luaW_defaultdeallocator(lua_State*, T* obj)
{
    delete obj;
}

// The identifier function is responsible for pushing a value unique to each
// object on to the stack. Most of the time, this can simply be the address
// of the pointer, but sometimes that is not adaquate. For example, if you
// are using shared_ptr you would need to push the address of the object the
// shared_ptr represents, rather than the address of the shared_ptr itself.
template <typename T>
void luaW_defaultidentifier(lua_State* L, T* obj)
{
    LuaUtils::pushlightuserdata(L, obj);
}

// This class is what is used by LuaWrapper to contain the userdata. data
// stores a pointer to the object itself, and cast is used to cast toward the
// base class if there is one and it is necessary. Rather than use RTTI and
// typid to compare types, I use the clever trick of using the cast to compare
// types. Because there is at most one cast per type, I can use it to identify
// when and object is the type I want. This is only used internally.
struct luaW_Userdata
{
    luaW_Userdata(void* vptr = NULL, luaW_Userdata (*udcast)(const luaW_Userdata&) = NULL)
        : data(vptr), cast(udcast) {}
    void* data;
    luaW_Userdata (*cast)(const luaW_Userdata&);
};

// This class cannot actually to be instantiated. It is used only hold the
// table name and other information.
template <typename T>
class LuaWrapper
{
public:
    static const char* classname;
    static void (*identifier)(lua_State*, T*);
    static T* (*allocator)(lua_State*);
    static void (*deallocator)(lua_State*, T*);
    static luaW_Userdata (*cast)(const luaW_Userdata&);
private:
    LuaWrapper();
};
template <typename T> const char* LuaWrapper<T>::classname;
template <typename T> void (*LuaWrapper<T>::identifier)(lua_State*, T*);
template <typename T> T* (*LuaWrapper<T>::allocator)(lua_State*);
template <typename T> void (*LuaWrapper<T>::deallocator)(lua_State*, T*);
template <typename T> luaW_Userdata (*LuaWrapper<T>::cast)(const luaW_Userdata&);

// Cast from an object of type T to an object of type U. This template
// function is instantiated by calling luaW_extend<T, U>(L). This is only used
// internally.
template <typename T, typename U>
luaW_Userdata luaW_cast(const luaW_Userdata& obj)
{
    return luaW_Userdata(static_cast<U*>(static_cast<T*>(obj.data)), LuaWrapper<U>::cast);
}

template <typename T, typename U>
void luaW_identify(lua_State* L, T* obj)
{
    LuaWrapper<U>::identifier(L, static_cast<U*>(obj));
}

template <typename T>
inline void luaW_wrapperfield(lua_State* L, const char* field)
{
    LuaUtils::getfield(L, LUA_REGISTRYINDEX, LUAW_WRAPPER_KEY); // ... LuaWrapper
    LuaUtils::getfield(L, -1, field); // ... LuaWrapper LuaWrapper.field
    LuaUtils::getfield(L, -1, LuaWrapper<T>::classname); // ... LuaWrapper LuaWrapper.field LuaWrapper.field.class
    LuaUtils::replace(L, -3); // ... LuaWrapper.field.class LuaWrapper.field
    LuaUtils::pop(L, 1); // ... LuaWrapper.field.class
}

// Analogous to lua_is(boolean|string|*)
//
// Returns 1 if the value at the given acceptable index is of type T (or if
// strict is false, convertable to type T) and 0 otherwise.
template <typename T>
bool luaW_is(lua_State *L, int index, bool strict = false)
{
    bool equal = false;// LuaUtils::isnil(L, index);
    if (!equal && LuaUtils::isuserdata(L, index) && LuaUtils::getmetatable(L, index))
    {
        // ... ud ... udmt
        LuaUtils::Lgetmetatable(L, LuaWrapper<T>::classname); // ... ud ... udmt Tmt
        equal = LuaUtils::rawequal(L, -1, -2) != 0;
        if (!equal && !strict)
        {
            LuaUtils::getfield(L, -2, LUAW_EXTENDS_KEY); // ... ud ... udmt Tmt udmt.extends
            for (LuaUtils::pushnil(L); LuaUtils::next(L, -2); )
            {
                // ... ud ... udmt Tmt udmt.extends k v
                equal = LuaUtils::rawequal(L, -1, -4) != 0;
                if (equal)
                {
                    LuaUtils::pop(L, 2); // ... ud ... udmt Tmt udmt.extends
                    break;
                }
				LuaUtils::pop(L, 1);
            }
            LuaUtils::pop(L, 1); // ... ud ... udmt Tmt
        }
        LuaUtils::pop(L, 2); // ... ud ...
    }
    return equal;
}

// Analogous to lua_to(boolean|string|*)
//
// Converts the given acceptable index to a T*. That value must be of (or
// convertable to) type T; otherwise, returns NULL.
template <typename T>
T* luaW_to(lua_State* L, int index, bool strict = false)
{
    if (luaW_is<T>(L, index, strict))
    {
        luaW_Userdata* pud = static_cast<luaW_Userdata*>(LuaUtils::touserdata(L, index));
        luaW_Userdata ud;
        while (!strict && LuaWrapper<T>::cast != pud->cast)
        {
            ud = pud->cast(*pud);
            pud = &ud;
        }
        return static_cast<T*>(pud->data);
    }
    return NULL;
}

// Analogous to luaL_check(boolean|string|*)
//
// Converts the given acceptable index to a T*. That value must be of (or
// convertable to) type T; otherwise, an error is raised.
template <typename T>
T* luaW_check(lua_State* L, int index, bool strict = false)
{
    T* obj = NULL;
    if (luaW_is<T>(L, index, strict))
    {
        luaW_Userdata* pud = (luaW_Userdata*)LuaUtils::touserdata(L, index);
        luaW_Userdata ud;
        while (!strict && LuaWrapper<T>::cast != pud->cast)
        {
            ud = pud->cast(*pud);
            pud = &ud;
        }
        obj = (T*)pud->data;
    }
    else
    {
		LuaUtils::argerror(L, index, FormatString("%s expected, got %s", LuaWrapper<T>::classname, LuaUtils::luatypename(L, index)).c_str());
    }
    return obj;
}

template <typename T>
T* luaW_opt(lua_State* L, int index, T* fallback = NULL, bool strict = false)
{
    if (LuaUtils::isnil(L, index))
        return fallback;
    else
        return luaW_check<T>(L, index, strict);
}

// Analogous to lua_push(boolean|string|*)
//
// Pushes a userdata of type T onto the stack. If this object already exists in
// the Lua environment, it will assign the existing storage table to it.
// Otherwise, a new storage table will be created for it.
template <typename T>
void luaW_push(lua_State* L, T* obj)
{
    if (obj)
    {
        LuaWrapper<T>::identifier(L, obj); // ... id
        luaW_wrapperfield<T>(L, LUAW_CACHE_KEY); // ... id cache
        LuaUtils::pushvalue(L, -2); // ... id cache id
        LuaUtils::gettable(L, -2); // ... id cache obj
        if (LuaUtils::isnil(L, -1))
        {
            // Create the new luaW_userdata and place it in the cache
            LuaUtils::pop(L, 1); // ... id cache
            LuaUtils::insert(L, -2); // ... cache id
            luaW_Userdata* ud = static_cast<luaW_Userdata*>(LuaUtils::newuserdata(L, sizeof(luaW_Userdata))); // ... cache id obj
            ud->data = obj;
            ud->cast = LuaWrapper<T>::cast;
			
			LuaUtils::Lgetmetatable(L, LuaWrapper<T>::classname); // ... cache id obj mt
			LuaUtils::setmetatable(L, -2); // ... cache id obj

            LuaUtils::pushvalue(L, -1); // ... cache id obj obj
            LuaUtils::insert(L, -4); // ... obj cache id obj
            LuaUtils::settable(L, -3); // ... obj cache           

            LuaUtils::pop(L, 1); // ... obj
        }
        else
        {
			//fb::Log("LuaWrapper pushing cached one for typeid : %s", typeid(T).name());
            LuaUtils::replace(L, -3); // ... obj cache
            LuaUtils::pop(L, 1); // ... obj
			luaW_Userdata* ud = static_cast<luaW_Userdata*>(LuaUtils::touserdata(L, -1));
			ud->cast = LuaWrapper<T>::cast;
			LuaUtils::Lgetmetatable(L, LuaWrapper<T>::classname); // ... obj mt
			LuaUtils::setmetatable(L, -2); // ... obj
        }
    }
    else
    {
        LuaUtils::pushnil(L);
    }
}

// Instructs LuaWrapper that it owns the userdata, and can manage its memory.
// When all references to the object are removed, Lua is free to garbage
// collect it and delete the object.
//
// Returns true if luaW_hold took hold of the object, and false if it was
// already held
template <typename T>
bool luaW_hold(lua_State* L, T* obj)
{
    luaW_wrapperfield<T>(L, LUAW_HOLDS_KEY); // ... holds
    LuaWrapper<T>::identifier(L, obj); // ... holds id
    LuaUtils::pushvalue(L, -1); // ... holds id id
    LuaUtils::gettable(L, -3); // ... holds id hold
    // If it's not held, hold it
    if (!LuaUtils::toboolean(L, -1))
    {
        // Apply hold boolean
        LuaUtils::pop(L, 1); // ... holds id
        LuaUtils::pushboolean(L, true); // ... holds id true
        LuaUtils::settable(L, -3); // ... holds
        LuaUtils::pop(L, 1); // ...
        return true;
    }
    LuaUtils::pop(L, 3); // ...
    return false;
}

// Releases LuaWrapper's hold on an object. This allows the user to remove
// all references to an object in Lua and ensure that Lua will not attempt to
// garbage collect it.
//
// This function takes the index of the identifier for an object rather than
// the object itself. This is because needs to be able to run after the object
// has already been deallocated. A wrapper is provided for when it is more
// convenient to pass in the object directly.
template <typename T>
void luaW_release(lua_State* L, int index)
{
    luaW_wrapperfield<T>(L, LUAW_HOLDS_KEY); // ... id ... holds
    LuaUtils::pushvalue(L, luaW_correctindex(L, index, 1)); // ... id ... holds id
    LuaUtils::pushnil(L); // ... id ... holds id nil
    LuaUtils::settable(L, -3); // ... id ... holds
    LuaUtils::pop(L, 1); // ... id ...
}

template <typename T>
void luaW_release(lua_State* L, T* obj)
{
    LuaWrapper<T>::identifier(L, obj); // ... id
    luaW_release<T>(L, -1); // ... id
    LuaUtils::pop(L, 1); // ...
}

template <typename T>
void luaW_clearstore(lua_State* L, T* obj)
{
	luaW_wrapperfield<T>(L, LUAW_STORAGE_KEY); // storage
	LuaWrapper<T>::identifier(L, obj); // storage id
	LuaUtils::pushnil(L); // storage id nil
	LuaUtils::settable(L, -3); // storage
	LuaUtils::pop(L, 1); // 
}
// This function is called from Lua, not C++
//
// Calls the lua post-constructor (LUAW_POSTCTOR_KEY or "__postctor") on a
// userdata. Assumes the userdata is on top of the stack, and numargs arguments
// are below it. This runs the LUAW_POSTCTOR_KEY function on T's metatable,
// using the object as the first argument and whatever else is below it as
// the rest of the arguments This exists to allow types to adjust values in
// thier storage table, which can not be created until after the constructor is
// called.
template <typename T>
void luaW_postconstructor(lua_State* L, int numargs)
{
    // ... args... ud
    LuaUtils::getfield(L, -1, LUAW_POSTCTOR_KEY); // ... args... ud ud.__postctor
    if (LuaUtils::type(L, -1) == LUA_TFUNCTION)
    {
        LuaUtils::pushvalue(L, -2); // ... args... ud ud.__postctor ud
        LuaUtils::insert(L, -3 - numargs); // ... ud args... ud ud.__postctor
        LuaUtils::insert(L, -3 - numargs); // ... ud.__postctor ud args... ud
        LuaUtils::insert(L, -3 - numargs); // ... ud ud.__postctor ud args...
        LuaUtils::call(L, numargs + 1, 0); // ... ud
    }
    else
    {
        LuaUtils::pop(L, 1); // ... ud
    }
}

// This function is generally called from Lua, not C++
//
// Creates an object of type T using the constructor and subsequently calls the
// post-constructor on it.
template <typename T>
inline int luaW_new(lua_State* L, int args)
{
    T* obj = LuaWrapper<T>::allocator(L);
    luaW_push<T>(L, obj);
    luaW_hold<T>(L, obj);
    luaW_postconstructor<T>(L, args);
    return 1;
}

template <typename T>
int luaW_new(lua_State* L)
{
    return luaW_new<T>(L, LuaUtils::gettop(L));
}

// This function is called from Lua, not C++
//
// The default metamethod to call when indexing into lua userdata representing
// an object of type T. This will first check the userdata's environment table
// and if it's not found there it will check the metatable. This is done so
// individual userdata can be treated as a table, and can hold thier own
// values.
template <typename T>
int luaW_index(lua_State* L)
{
    // obj key
    T* obj = luaW_to<T>(L, 1);
    luaW_wrapperfield<T>(L, LUAW_STORAGE_KEY); // obj key storage
    LuaWrapper<T>::identifier(L, obj); // obj key storage id
    LuaUtils::gettable(L, -2); // obj key storage store

    // Check if storage table exists
    if (!LuaUtils::isnil(L, -1))
    {
        LuaUtils::pushvalue(L, -3); // obj key storage store key
        LuaUtils::gettable(L, -2); // obj key storage store store[k]
    }

    // If either there is no storage table or the key wasn't found
    // then fall back to the metatable
    if (LuaUtils::isnil(L, -1))
    {
        LuaUtils::settop(L, 2); // obj key
        LuaUtils::getmetatable(L, -2); // obj key mt
        LuaUtils::pushvalue(L, -2); // obj key mt k
        LuaUtils::gettable(L, -2); // obj key mt mt[k]
    }
    return 1;
}

// This function is called from Lua, not C++
//
// The default metamethod to call when creating a new index on lua userdata
// representing an object of type T. This will index into the the userdata's
// environment table that it keeps for personal storage. This is done so
// individual userdata can be treated as a table, and can hold thier own
// values.
template <typename T>
int luaW_newindex(lua_State* L)
{
    // obj key value
    T* obj = luaW_check<T>(L, 1);	
    luaW_wrapperfield<T>(L, LUAW_STORAGE_KEY); // obj key value storage
    LuaWrapper<T>::identifier(L, obj); // obj key value storage id
    LuaUtils::pushvalue(L, -1); // obj key value storage id id
    LuaUtils::gettable(L, -3); // obj key value storage id store

    // Add the storage table if there isn't one already
    if (LuaUtils::isnil(L, -1))
    {
        LuaUtils::pop(L, 1); // obj key value storage id
        LuaUtils::newtable(L); // obj key value storage id store
        LuaUtils::pushvalue(L, -1); // obj key value storage id store store
        LuaUtils::insert(L, -3); // obj key value storage store id store
        LuaUtils::settable(L, -4); // obj key value storage store
    }

    LuaUtils::pushvalue(L, 2); // obj key value ... store key
    LuaUtils::pushvalue(L, 3); // obj key value ... store key value
    LuaUtils::settable(L, -3); // obj key value ... store

    return 0;
}

// This function is called from Lua, not C++
//
template <typename T>
int luaW_pairs(lua_State* L)
{
	// obj
	T* obj = luaW_check<T>(L, 1);
	luaW_wrapperfield<T>(L, LUAW_STORAGE_KEY); // obj storage
	LuaWrapper<T>::identifier(L, obj); // obj storage id
	LuaUtils::pushvalue(L, -1); // obj storage id id
	LuaUtils::gettable(L, -3); // obj storage id store

	// Add the storage table if there isn't one already
	if (LuaUtils::isnil(L, -1))
	{
		LuaUtils::pop(L, 1); // obj storage id
		LuaUtils::newtable(L); // obj storage id store
		LuaUtils::pushvalue(L, -1); // obj storage id store store
		LuaUtils::insert(L, -3); // obj storage store id store
		LuaUtils::settable(L, -4); // obj storage store
	}


	LuaUtils::getglobal(L, "next"); // obj storage id store next
	LuaUtils::insert(L, -2); // obj storage id next store 	
	LuaUtils::pushnil(L);// obj storage id next store nil
	return 3;	
}

// This function is called from Lua, not C++
//
// The __gc metamethod handles cleaning up userdata. The userdata's reference
// count is decremented and if this is the final reference to the userdata its
// environment table is nil'd and pointer deleted with the destructor callback.
template <typename T>
int luaW_gc(lua_State* L)
{
    // obj
    T* obj = luaW_to<T>(L, 1);
    LuaWrapper<T>::identifier(L, obj); // obj key value storage id
    luaW_wrapperfield<T>(L, LUAW_HOLDS_KEY); // obj id counts count holds
    LuaUtils::pushvalue(L, 2); // obj id counts count holds id
    LuaUtils::gettable(L, -2); // obj id counts count holds hold
    if (LuaUtils::toboolean(L, -1) && LuaWrapper<T>::deallocator)
    {
        LuaWrapper<T>::deallocator(L, obj);
    }

    luaW_wrapperfield<T>(L, LUAW_STORAGE_KEY); // obj id counts count holds hold storage
    LuaUtils::pushvalue(L, 2); // obj id counts count holds hold storage id
    LuaUtils::pushnil(L); // obj id counts count holds hold storage id nil
    LuaUtils::settable(L, -3); // obj id counts count holds hold storage

    luaW_release<T>(L, 2);
    return 0;
}

// Thakes two tables and registers them with Lua to the table on the top of the
// stack. 
//
// This function is only called from LuaWrapper internally. 
FB_DLL_LUA void luaW_registerfuncs(lua_State* L, const luaL_Reg defaulttable[], const luaL_Reg table[]);

// Initializes the LuaWrapper tables used to track internal state. 
//
// This function is only called from LuaWrapper internally. 
FB_DLL_LUA void luaW_initialize(lua_State* L);

// Run luaW_register or luaW_setfuncs to create a table and metatable for your
// class.  These functions create a table with filled with the function from
// the table argument in addition to the functions new and build (This is
// generally for things you think of as static methods in C++). The given
// metatable argument becomes a metatable for each object of your class. These
// can be thought of as member functions or methods.
//
// You may also supply constructors and destructors for classes that do not
// have a default constructor or that require special set up or tear down. You
// may specify NULL as the constructor, which means that you will not be able
// to call the new function on your class table. You will need to manually push
// objects from C++. By default, the default constructor is used to create
// objects and a simple call to delete is used to destroy them.
//
// By default LuaWrapper uses the address of C++ object to identify unique
// objects. In some cases this is not desired, such as in the case of
// shared_ptrs. Two shared_ptrs may themselves have unique locations in memory
// but still represent the same object. For cases like that, you may specify an
// identifier function which is responsible for pushing a key representing your
// object on to the stack.
//
// luaW_register will set table as the new value of the global of the given
// name. luaW_setfuncs is identical to luaW_register, but it does not set the
// table globally.  As with luaL_register and luaL_setfuncs, both funcstions
// leave the new table on the top of the stack.
template <typename T>
void luaW_setfuncs(lua_State* L, const char* classname, const luaL_Reg* table, const luaL_Reg* metatable, T* (*allocator)(lua_State*) = luaW_defaultallocator<T>, void (*deallocator)(lua_State*, T*) = luaW_defaultdeallocator<T>, void (*identifier)(lua_State*, T*) = luaW_defaultidentifier<T>)
{
    luaW_initialize(L);

    LuaWrapper<T>::classname = classname;
    LuaWrapper<T>::identifier = identifier;
    LuaWrapper<T>::allocator = allocator;
    LuaWrapper<T>::deallocator = deallocator;

    const luaL_Reg defaulttable[] =
    {
        { "new", luaW_new<T> },
        { NULL, NULL }
    };
    const luaL_Reg defaultmetatable[] = 
    { 
        { "__index", luaW_index<T> }, 
        { "__newindex", luaW_newindex<T> }, 
		{ "__pairs", luaW_pairs<T>},
        { "__gc", luaW_gc<T> }, 
        { NULL, NULL } 
    };

    // Set up per-type tables
    LuaUtils::getfield(L, LUA_REGISTRYINDEX, LUAW_WRAPPER_KEY); // ... LuaWrapper
    
    LuaUtils::getfield(L, -1, LUAW_STORAGE_KEY); // ... LuaWrapper LuaWrapper.storage
    LuaUtils::newtable(L); // ... LuaWrapper LuaWrapper.storage {}
    LuaUtils::setfield(L, -2, LuaWrapper<T>::classname); // ... LuaWrapper LuaWrapper.storage
    LuaUtils::pop(L, 1); // ... LuaWrapper

    LuaUtils::getfield(L, -1, LUAW_HOLDS_KEY); // ... LuaWrapper LuaWrapper.holds
    LuaUtils::newtable(L); // ... LuaWrapper LuaWrapper.holds {}
    LuaUtils::setfield(L, -2, LuaWrapper<T>::classname); // ... LuaWrapper LuaWrapper.holds
    LuaUtils::pop(L, 1); // ... LuaWrapper

    LuaUtils::getfield(L, -1, LUAW_CACHE_KEY); // ... LuaWrapper LuaWrapper.cache
    LuaUtils::newtable(L); // ... LuaWrapper LuaWrapper.cache {}
    luaW_wrapperfield<T>(L, LUAW_CACHE_METATABLE_KEY); // ... LuaWrapper LuaWrapper.cache {} cmt
    LuaUtils::setmetatable(L, -2); // ... LuaWrapper LuaWrapper.cache {}
    LuaUtils::setfield(L, -2, LuaWrapper<T>::classname); // ... LuaWrapper LuaWrapper.cache
        
    LuaUtils::pop(L, 2); // ...
    
    // Open table
    LuaUtils::newtable(L); // ... T
    luaW_registerfuncs(L, allocator ? defaulttable : NULL, table); // ... T

    // Open metatable, set up extends table
    int result = LuaUtils::Lnewmetatable(L, classname); // ... T mt
	if (!result)
	{
		fb::Log("Metatable %s already exists.", classname);
	}
    LuaUtils::newtable(L); // ... T mt {}
    LuaUtils::setfield(L, -2, LUAW_EXTENDS_KEY); // ... T mt
    luaW_registerfuncs(L, defaultmetatable, metatable); // ... T mt
    LuaUtils::setfield(L, -2, "metatable"); // ... T
}

template <typename T>
void luaW_register(lua_State* L, const char* classname, const luaL_Reg* table, const luaL_Reg* metatable, T* (*allocator)(lua_State*) = luaW_defaultallocator<T>, void (*deallocator)(lua_State*, T*) = luaW_defaultdeallocator<T>, void (*identifier)(lua_State*, T*) = luaW_defaultidentifier<T>)
{
    luaW_setfuncs(L, classname, table, metatable, allocator, deallocator, identifier); // ... T
    LuaUtils::pushvalue(L, -1); // ... T T
    LuaUtils::setglobal(L, classname); // ... T
}

// luaW_extend is used to declare that class T inherits from class U. All
// functions in the base class will be available to the derived class (except
// when they share a function name, in which case the derived class's function
// wins). This also allows luaW_to<T> to cast your object apropriately, as
// casts straight through a void pointer do not work.
template <typename T, typename U>
void luaW_extend(lua_State* L)
{
    if(!LuaWrapper<T>::classname)
		LuaUtils::error(L, "attempting to call extend on a type that has not been registered");

    if(!LuaWrapper<U>::classname)
		LuaUtils::error(L, FormatString("attempting to extend %s by a type that has not been registered", LuaWrapper<T>::classname).c_str());

    LuaWrapper<T>::cast = luaW_cast<T, U>;
    LuaWrapper<T>::identifier = luaW_identify<T, U>;

    LuaUtils::Lgetmetatable(L, LuaWrapper<T>::classname); // mt
    LuaUtils::Lgetmetatable(L, LuaWrapper<U>::classname); // mt emt

    // Point T's metatable __index at U's metatable for inheritance
    LuaUtils::newtable(L); // mt emt {}
    LuaUtils::pushvalue(L, -2); // mt emt {} emt
    LuaUtils::setfield(L, -2, "__index"); // mt emt {}
    LuaUtils::setmetatable(L, -3); // mt emt

    // Set up per-type tables to point at parent type
    LuaUtils::getfield(L, LUA_REGISTRYINDEX, LUAW_WRAPPER_KEY); // ... LuaWrapper
    
    LuaUtils::getfield(L, -1, LUAW_STORAGE_KEY); // ... LuaWrapper LuaWrapper.storage
    LuaUtils::getfield(L, -1, LuaWrapper<U>::classname); // ... LuaWrapper LuaWrapper.storage U
    LuaUtils::setfield(L, -2, LuaWrapper<T>::classname); // ... LuaWrapper LuaWrapper.storage
    LuaUtils::pop(L, 1); // ... LuaWrapper

    LuaUtils::getfield(L, -1, LUAW_HOLDS_KEY); // ... LuaWrapper LuaWrapper.holds
    LuaUtils::getfield(L, -1, LuaWrapper<U>::classname); // ... LuaWrapper LuaWrapper.holds U
    LuaUtils::setfield(L, -2, LuaWrapper<T>::classname); // ... LuaWrapper LuaWrapper.holds
    LuaUtils::pop(L, 1); // ... LuaWrapper

    LuaUtils::getfield(L, -1, LUAW_CACHE_KEY); // ... LuaWrapper LuaWrapper.cache
    LuaUtils::getfield(L, -1, LuaWrapper<U>::classname); // ... LuaWrapper LuaWrapper.cache U
    LuaUtils::setfield(L, -2, LuaWrapper<T>::classname); // ... LuaWrapper LuaWrapper.cache
        
    LuaUtils::pop(L, 2); // ...
    
    // Make a list of all types that inherit from U, for type checking
    LuaUtils::getfield(L, -2, LUAW_EXTENDS_KEY); // mt emt mt.extends
    LuaUtils::pushvalue(L, -2); // mt emt mt.extends emt
    LuaUtils::setfield(L, -2, LuaWrapper<U>::classname); // mt emt mt.extends
    LuaUtils::getfield(L, -2, LUAW_EXTENDS_KEY); // mt emt mt.extends emt.extends
    for (LuaUtils::pushnil(L); LuaUtils::next(L, -2); )
    {
        // mt emt mt.extends emt.extends k v
        LuaUtils::pushvalue(L, -2); // mt emt mt.extends emt.extends k v k
        LuaUtils::pushvalue(L, -2); // mt emt mt.extends emt.extends k v k v
        LuaUtils::rawset(L, -6); // mt emt mt.extends emt.extends k v
		LuaUtils::pop(L, 1);
    }

    LuaUtils::pop(L, 4); // mt emt
}

/*
 * Copyright (c) 2010-2013 Alexander Ames
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#define CHECK_NUM_LUA_ARGS(x) \
	int numLuaArgs = LuaUtils::gettop(L); \
	if (numLuaArgs != (x)) \
	{\
		assert(0); \
	}

#endif // LUA_WRAPPER_H_
