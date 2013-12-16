#ifdef THORIN_P_TYPE
#define THORIN_PS_TYPE(T) THORIN_P_TYPE(T)
#define THORIN_PU_TYPE(T) THORIN_P_TYPE(T)
#define THORIN_PF_TYPE(T) THORIN_P_TYPE(T)
#endif

#ifdef THORIN_Q_TYPE
#define THORIN_QS_TYPE(T) THORIN_Q_TYPE(T)
#define THORIN_QU_TYPE(T) THORIN_Q_TYPE(T)
#define THORIN_QF_TYPE(T) THORIN_Q_TYPE(T)
#endif

#ifdef THORIN_ALL_TYPE
#define THORIN_PS_TYPE(T) THORIN_ALL_TYPE(T)
#define THORIN_PU_TYPE(T) THORIN_ALL_TYPE(T)
#define THORIN_QS_TYPE(T) THORIN_ALL_TYPE(T)
#define THORIN_QU_TYPE(T) THORIN_ALL_TYPE(T)
#define THORIN_PF_TYPE(T) THORIN_ALL_TYPE(T)
#define THORIN_QF_TYPE(T) THORIN_ALL_TYPE(T)
#endif

#ifndef THORIN_PS_TYPE
#define THORIN_PS_TYPE(T)
#endif

THORIN_PS_TYPE(ps1)
THORIN_PS_TYPE(ps8)
THORIN_PS_TYPE(ps16)
THORIN_PS_TYPE(ps32)
THORIN_PS_TYPE(ps64)

#ifndef THORIN_PU_TYPE
#define THORIN_PU_TYPE(T)
#endif

THORIN_PU_TYPE(pu1)
THORIN_PU_TYPE(pu8)
THORIN_PU_TYPE(pu16)
THORIN_PU_TYPE(pu32)
THORIN_PU_TYPE(pu64)

#ifndef THORIN_QS_TYPE
#define THORIN_QS_TYPE(T)
#endif

THORIN_QS_TYPE(qs1)
THORIN_QS_TYPE(qs8)
THORIN_QS_TYPE(qs16)
THORIN_QS_TYPE(qs32)
THORIN_QS_TYPE(qs64)

#ifndef THORIN_QU_TYPE
#define THORIN_QU_TYPE(T)
#endif

THORIN_QU_TYPE(qu1)
THORIN_QU_TYPE(qu8)
THORIN_QU_TYPE(qu16)
THORIN_QU_TYPE(qu32)
THORIN_QU_TYPE(qu64)

#ifndef THORIN_PF_TYPE
#define THORIN_PF_TYPE(T)
#endif

THORIN_PF_TYPE(pf32)
THORIN_PF_TYPE(pf64)

#ifndef THORIN_QF_TYPE
#define THORIN_QF_TYPE(T)
#endif

THORIN_QF_TYPE(qf32)
THORIN_QF_TYPE(qf64)

#undef THORIN_PS_TYPE
#undef THORIN_PU_TYPE
#undef THORIN_QS_TYPE
#undef THORIN_QU_TYPE
#undef THORIN_PF_TYPE
#undef THORIN_QF_TYPE
#undef THORIN_P_TYPE
#undef THORIN_Q_TYPE
#undef THORIN_ALL_TYPE
