//
//  Mach7: Pattern Matching Library for C++
//
//  Copyright 2011-2013, Texas A&M University.
//  Copyright 2014 Yuriy Solodkyy.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//      * Redistributions of source code must retain the above copyright
//        notice, this list of conditions and the following disclaimer.
//
//      * Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//
//      * Neither the names of Mach7 project nor the names of its contributors
//        may be used to endorse or promote products derived from this software
//        without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY
//  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

///
/// \file
///
/// This file is a part of Mach7 library test suite.
///
/// \author Yuriy Solodkyy <yuriy.solodkyy@gmail.com>
///
/// \see https://parasol.tamu.edu/mach7/
/// \see https://github.com/solodon4/Mach7
/// \see https://github.com/solodon4/SELL
///

#include "testshape.hpp"
#include "testutils.hpp"
#include "type_switchN.hpp"
#include "testrepeat.hpp"

#undef  NUMBER_OF_DERIVED
#define NUMBER_OF_DERIVED 10

//------------------------------------------------------------------------------

template <size_t N>
struct shape_kind : OtherBase, Shape
{
    typedef Shape base_class;
    shape_kind(size_t n = N) : base_class(n) {}
    void accept(ShapeVisitor&) const;
};

//------------------------------------------------------------------------------

struct ShapeVisitor
{
    #define FOR_EACH_MAX NUMBER_OF_DERIVED-1
    #define FOR_EACH_N(N) virtual void visit(const shape_kind<N>&) {}
    #include "loop_over_numbers.hpp"
    #undef  FOR_EACH_N
    #undef  FOR_EACH_MAX
};

//------------------------------------------------------------------------------

template <size_t N> void shape_kind<N>::accept(ShapeVisitor& v) const { v.visit(*this); }

#if 1

#define MY_CASE_N_M_K(K,...) Case(shape_kind<__VA_ARGS__>,shape_kind<K/10>,shape_kind<K%10>) return (__VA_ARGS__)*100 + (K/10)*10 + K%10;

XTL_TIMED_FUNC_BEGIN
size_t do_match(const Shape& s1, const Shape& s2, const Shape& s3, size_t)
{
    Match(s1,s2,s3)
    {
        #define FOR_EACH_MAX  NUMBER_OF_DERIVED-1
        #define FOR_EACH_N(N) XTL_TEST_REPEAT(100, MY_CASE_N_M_K, N)
        #include "loop_over_numbers.hpp"
        #undef  FOR_EACH_N
        #undef  FOR_EACH_MAX
    }
    EndMatch
    return invalid;
}
XTL_TIMED_FUNC_END
#else
#error No preprocessed version provided
#endif

//------------------------------------------------------------------------------

template <size_t N, size_t M>
struct VisitorFor2 : ShapeVisitor
{
    VisitorFor2(const shape_kind<N>& s1, const shape_kind<M>& s2) : first(s1), second(s2), result(invalid) {}

    #define FOR_EACH_MAX  NUMBER_OF_DERIVED-1
    #define FOR_EACH_N(K) virtual void visit(const shape_kind<K>&) { result = N*100 + M*10 + K; }
    #include "loop_over_numbers.hpp"
    #undef  FOR_EACH_N
    #undef  FOR_EACH_MAX

    const shape_kind<N>& first;
    const shape_kind<M>& second;
    int                  result; 

private:
    VisitorFor2& operator=(const VisitorFor2&); ///< No assignment operator
};

//------------------------------------------------------------------------------

template <size_t N>
struct VisitorFor : ShapeVisitor
{
    VisitorFor(const shape_kind<N>& s1, const Shape& s3) : first(s1), third(s3), result(invalid) {}

    #define FOR_EACH_MAX  NUMBER_OF_DERIVED-1
    #define FOR_EACH_N(M) virtual void visit(const shape_kind<M>& second) { VisitorFor2<N,M> v(first,second); third.accept(v); result = v.result; }
    #include "loop_over_numbers.hpp"
    #undef  FOR_EACH_N
    #undef  FOR_EACH_MAX

    const shape_kind<N>& first;
    const Shape&         third;
    int                  result; 

private:
    VisitorFor& operator=(const VisitorFor&); ///< No assignment operator
};

//------------------------------------------------------------------------------

struct Visitor : ShapeVisitor
{
    Visitor(const Shape& s2, const Shape& s3) : second(s2), third(s3), result(invalid) {}

    #define FOR_EACH_MAX  NUMBER_OF_DERIVED-1
    #define FOR_EACH_N(N) virtual void visit(const shape_kind<N>& first) { VisitorFor<N> v(first,third); second.accept(v); result = v.result; }
    #include "loop_over_numbers.hpp"
    #undef  FOR_EACH_N
    #undef  FOR_EACH_MAX

    const Shape& second;
    const Shape& third;
    int          result;

private:
    Visitor& operator=(const Visitor&); ///< No assignment operator
};

//------------------------------------------------------------------------------

XTL_TIMED_FUNC_BEGIN
size_t do_visit(const Shape& s1, const Shape& s2, const Shape& s3, size_t)
{
    Visitor v(s2,s3);
    s1.accept(v);
    return v.result;
}
XTL_TIMED_FUNC_END

//------------------------------------------------------------------------------

Shape* make_shape(size_t i)
{
    switch (i % NUMBER_OF_DERIVED)
    {
        #define FOR_EACH_MAX  NUMBER_OF_DERIVED-1
        #define FOR_EACH_N(N) case N: return new shape_kind<N>;
        #include "loop_over_numbers.hpp"
        #undef  FOR_EACH_N
        #undef  FOR_EACH_MAX
    }
    return 0;
}

//------------------------------------------------------------------------------

#include "testvismat3.hpp"    // Utilities for timing tests

//------------------------------------------------------------------------------

int main()
{
    using namespace mch; // Mach7's library namespace

    //verdict pp = test_repetitive();
    //verdict ps = test_sequential();
    verdict pr = test_randomized();
    std::cout << "OVERALL: "
              //<< "Repetitive: " << pp << "; "
              //<< "Sequential: " << ps << "; "
              << "Random: "     << pr 
              << std::endl; 
}

//------------------------------------------------------------------------------