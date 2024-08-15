#include "mmintrin.h"

#ifndef __INTRINSICS_H__


// Template for type conversion and discrimination
template <class T>
union __m64t
{
    T       t;
    __m64   m64;

    __m64t (__m64 _m) : m64 (_m) { }        // Initialization from __m64
    __m64t (T _t)     : t (_t)   { }        // Initialization from T
    operator __m64 ()       { return m64; } // Implicit conversion back to __m64
    operator __m64 () const { return m64; } // Implicit conversion back to __m64
};


// Overloads for 3DNow!
typedef __m64t<float[2]>   m3DNow;
inline  m3DNow operator + (m3DNow a, m3DNow b) { return _m_pfadd (a, b); }
inline  m3DNow operator - (m3DNow a, m3DNow b) { return _m_pfsub (a, b); }
inline  m3DNow operator * (m3DNow a, m3DNow b) { return _m_pfmul (a, b); }
inline  m3DNow operator / (m3DNow a, m3DNow b)
{
    __m64 r = _m_punpckldq (_m_pfrcp (b),
                            _m_pfrcp (_m_punpckhdq (b, b)));
    return _m_pfmul (a, _m_pfrcpit2 (_m_pfrcpit1 (b, r), r));
}

// Some functions defined over 3DNow! data types
inline m3DNow rsqrt (m3DNow a) { return _m_pfrsqrt (a);    }
inline m3DNow sqrt  (m3DNow a) { return _m_pfmul   (a, _m_pfrsqrt (a)); }
inline m3DNow rcp   (m3DNow a) { return _m_pfrcp   (a);    }
inline m3DNow acc   (m3DNow a) { return _m_pfacc   (a, a); }
inline m3DNow acc   (m3DNow a, m3DNow b) { return _m_pfacc   (a, b); }
inline m3DNow rsqrt24 (m3DNow a)
{
    __m64 r = _m_punpckldq (_m_pfrsqrt (a),
                            _m_pfrsqrt (_m_punpckhdq (a, a)));
    return _m_pfrcpit2 (_m_pfrsqit1 (a, r), r);
}

inline m3DNow sqrt24 (m3DNow a)
{
    __m64 r = _m_punpckldq (_m_pfrsqrt (a),
                            _m_pfrsqrt (_m_punpckhdq (a, a)));
    return _m_pfmul (a, _m_pfrcpit2 (_m_pfrsqit1 (a, r), r));
}


//typedef __m64t<int[2]>      __m64i;
//inline __m64i operator + (__m64i a, __m64i b) { return _m_paddd (a, b); }
//inline __m64i operator - (__m64i a, __m64i b) { return _m_psubd (a, b); }


#endif

// eof
