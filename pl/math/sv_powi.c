/*
 * Double-precision SVE powi(x, n) function.
 *
 * Copyright (c) 2020-2023, Arm Limited.
 * SPDX-License-Identifier: MIT OR Apache-2.0 WITH LLVM-exception
 */

#include "sv_math.h"
#if SV_SUPPORTED

/* Optimized double-precision vector powi (double base, long integer power).
   powi is developed for environments in which accuracy is of much less
   importance than performance, hence we provide no estimate for worst-case
   error.  */
svfloat64_t
__sv_powi_x (svfloat64_t as, svint64_t ns, svbool_t p)
{
  /* Compute powi by successive squaring, right to left.  */
  svfloat64_t acc = svdup_n_f64 (1.0);
  svbool_t want_recip = svcmplt_n_s64 (p, ns, 0);
  svuint64_t ns_abs = svreinterpret_u64_s64 (svabs_s64_x (p, ns));

  /* We use a max to avoid needing to check whether any lane != 0 on each
     iteration.  */
  uint64_t max_n = svmaxv_u64 (p, ns_abs);

  svfloat64_t c = as;
  /* Successively square c, and use merging predication (_m) to determine
     whether or not to perform the multiplication or keep the previous
     iteration.  */
  while (true)
    {
      svbool_t px = svcmpeq_n_u64 (p, svand_n_u64_x (p, ns_abs, 1ull), 1ull);
      acc = svmul_f64_m (px, acc, c);
      max_n >>= 1;
      if (max_n == 0)
	break;

      ns_abs = svlsr_n_u64_x (p, ns_abs, 1);
      c = svmul_f64_x (p, c, c);
    }

  /* Negative powers are handled by computing the abs(n) version and then
     taking the reciprocal.  */
  if (svptest_any (want_recip, want_recip))
    acc = svdivr_n_f64_m (want_recip, acc, 1.0);

  return acc;
}

strong_alias (__sv_powi_x, _ZGVsMxvv_powk)

#endif // SV_SUPPORTED
