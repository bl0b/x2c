/*
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef _X2C_BASE_H_
#define _X2C_BASE_H_

struct ordered_sequence {};
struct unordered_sequence {};
struct alternative {};
struct single {};
struct multiple {};
template <typename X> struct optional;
template <> struct optional<single> {};
template <> struct optional<multiple> {};


#if defined(DEBUG_X2C) && !defined(NDEBUG)
#include <iostream>
#include <iomanip>

struct scope_debug {
    const char* F;
    int L;
    static int& indent() {
        static int _ = 0;
        return _;
    }
    std::ostream& output(const char* x) const { return std::cout << std::setw(indent() * 3) << "" << F << ':' << L << ' ' << x << std::endl; }
    scope_debug(const char* f, int l)
        : F(f), L(l)
    { output("enter"); ++indent(); }
    ~scope_debug() { --indent(); output("leave"); }
};

#define debug_log std::cout << std::setw(scope_debug::indent() * 3) << ""
#define debug_endl std::endl
#define DEBUG scope_debug _____debug_____(__FILE__, __LINE__)

#else

#define DEBUG
struct debug_logger {
    static debug_logger& _() { static debug_logger x; return x; }
};

template <typename X> debug_logger& operator << (debug_logger&d, const X& x) { return d; }

#define debug_endl (0)
#define debug_log debug_logger::_()
#endif

#endif

