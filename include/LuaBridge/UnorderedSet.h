// https://github.com/vinniefalco/LuaBridge
// Copyright 2018, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#pragma once

#include <LuaBridge/detail/Stack.h>

#include <unordered_set>

namespace luabridge {

template<class T>
struct Stack<std::unordered_set<T>>
{
    static void push(lua_State* L, std::unordered_set<T> const& set)
    {
        lua_createtable(L, static_cast<int>(set.size()), 0);
        for (std::size_t i = 0; i < set.size(); ++i)
        {
            lua_pushinteger(L, static_cast<lua_Integer>(i + 1));
            Stack<T>::push(L, *std::next(set.begin(), i));
            lua_settable(L, -3);
        }
    }

    static std::unordered_set<T> get(lua_State* L, int index)
    {
        if (!lua_istable(L, index))
        {
            luaL_error(L, "#%d argument must be a table", index);
        }

        std::unordered_set<T> set;
        set.reserve(static_cast<std::size_t>(get_length(L, index)));

        int const absindex = lua_absindex(L, index);
        lua_pushnil(L);
        while (lua_next(L, absindex) != 0)
        {
            set.emplace(Stack<T>::get(L, -1));
            lua_pop(L, 1);
        }
        return set;
    }

    static bool isInstance(lua_State* L, int index) { return lua_istable(L, index); }
};

} // namespace luabridge
