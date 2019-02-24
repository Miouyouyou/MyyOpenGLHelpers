#ifndef MYY_C_TYPES_H
#define MYY_C_TYPES_H

#include <stdint.h>


#ifndef __cplusplus

enum bool_type { false, true };
typedef uint_fast8_t bool;

#endif /* __cplusplus */

#if defined(__amd64) || defined(_M_AMD64) || (defined(__ARM_ARCH__) && __ARM_ARCH__ == 8)
typedef int64_t intreg_t;
typedef uint64_t uintreg_t;
#else
typedef int32_t intreg_t;
typedef uint32_t uintreg_t;
#endif /* *intreg_t */

#endif /* MYY_C_TYPES_H */
