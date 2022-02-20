/**
 * \file Ifx_Cf32.h
 * \brief Floating point signal, vector, and matrix library
 *
 *
 * \version disabled
 * \copyright Copyright (c) 2013 Infineon Technologies AG. All rights reserved.
 *
 *
 *                                 IMPORTANT NOTICE
 *
 *
 * Use of this file is subject to the terms of use agreed between (i) you or 
 * the company in which ordinary course of business you are acting and (ii) 
 * Infineon Technologies AG or its licensees. If and as long as no such 
 * terms of use are agreed, use of this file is subject to following:


 * Boost Software License - Version 1.0 - August 17th, 2003

 * Permission is hereby granted, free of charge, to any person or 
 * organization obtaining a copy of the software and accompanying 
 * documentation covered by this license (the "Software") to use, reproduce,
 * display, distribute, execute, and transmit the Software, and to prepare
 * derivative works of the Software, and to permit third-parties to whom the 
 * Software is furnished to do so, all subject to the following:

 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer, must
 * be included in all copies of the Software, in whole or in part, and all
 * derivative works of the Software, unless such copies or derivative works are
 * solely in the form of machine-executable object code generated by a source
 * language processor.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE 
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.

 *
 * \defgroup library_srvsw_sysse_math_cf32 Floating point signal, vector, and matrix library
 * \ingroup library_srvsw_sysse_math_f32
 */

#ifndef IFX_CF32_H
#define IFX_CF32_H

#include <math.h>
#include "Cpu/Std/IfxCpu_Intrinsics.h"

#define _DATAF(val) ((float32)(val))

/* Complex Arithmetic --------------------------------------------------------*/
IFX_INLINE cfloat32 IFX_Cf32_exp(const cfloat32 *c)
{
    float32  f = (float32)expf(c->real);
    cfloat32 R;
    R.real = f * (float32)cosf(c->imag);
    R.imag = f * (float32)sinf(c->imag);
    return R;
}


IFX_INLINE cfloat32 IFX_Cf32_mul(const cfloat32 *a, const cfloat32 *b)
{
    cfloat32 R;
    R.real = (a->real * b->real) - (a->imag * b->imag);
    R.imag = (a->imag * b->real) + (a->real * b->imag);
    return R;
}


IFX_INLINE cfloat32 IFX_Cf32_amp(const cfloat32 *a, float32 gain)
{
    cfloat32 R;
    R.real = a->real * gain;
    R.imag = a->imag * gain;
    return R;
}


IFX_INLINE float32 IFX_Cf32_dot(const cfloat32 *b)
{
    return (b->real * b->real) + (b->imag * b->imag);
}


IFX_INLINE float32 IFX_Cf32_mag(const cfloat32 *c)
{
    return (float32)sqrtf(IFX_Cf32_dot(c));
}


IFX_INLINE cfloat32 IFX_Cf32_div(const cfloat32 *a, const cfloat32 *b)
{
    float32  denom = IFX_Cf32_dot(b);
    cfloat32 R;
    R.real = ((a->real * b->real) + (a->imag * b->imag)) / denom;
    R.imag = ((a->imag * b->real) - (a->real * b->imag)) / denom;
    return R;
}


IFX_INLINE cfloat32 IFX_Cf32_add(const cfloat32 *a, const cfloat32 *b)
{
    cfloat32 R;
    R.real = (a->real + b->real);
    R.imag = (a->imag + b->imag);
    return R;
}


IFX_INLINE cfloat32 IFX_Cf32_sub(const cfloat32 *a, const cfloat32 *b)
{
    cfloat32 R;
    R.real = (a->real - b->real);
    R.imag = (a->imag - b->imag);
    return R;
}


IFX_INLINE void IFX_Cf32_set(cfloat32 *a, float32 re, float32 im)
{
    a->real = re;
    a->imag = im;
}


IFX_INLINE void IFX_Cf32_reset(cfloat32 *a)
{
    IFX_Cf32_set(a, 0.0, 0.0);
}


IFX_INLINE cfloat32 IFX_Cf32_saturate(cfloat32 *a, float32 *ampl, float32 limit)
{
    cfloat32 R = *a;
    *ampl = IFX_Cf32_mag(a);

    if (*ampl > limit)
    {
        float32 scale = limit / *ampl;
        R.imag = R.imag * scale;
        R.real = R.real * scale;
    }

    return R;
}


IFX_EXTERN void     CplxVecCpy_f32S(cfloat32 *X, short *S, short nS, short incrS);
IFX_EXTERN void     CplxVecRst_f32(cfloat32 *X, short nX);
IFX_EXTERN void     CplxVecCpy_f32(cfloat32 *X, cfloat32 *S, short nS);
IFX_EXTERN float32 *CplxVecPwr_f32(cfloat32 *X, short nX);
IFX_EXTERN float32 *CplxVecMag_f32(cfloat32 *X, short nX);
IFX_EXTERN void     CplxVecMul_f32(cfloat32 *X, const cfloat32 *mul, short nX);

/* Vector Operation ----------------------------------------------------------*/

IFX_EXTERN void    VecWin_f32(float32 *X, const float32 *W, short nX, short nW, short incrX, short symW);
IFX_EXTERN void    VecPwrdB_f32(float32 *X, short nX);
IFX_EXTERN void    VecPwrdB_SF(sint16 *R, float32 *X, short nX);
IFX_EXTERN void    VecGain_f32(float32 *X, float32 gain, short nX);
IFX_EXTERN void    VecOfs_f32(float32 *X, float32 offset, short nX);
IFX_EXTERN float32 VecSum_f32(float32 *X, short nX);
IFX_EXTERN float32 VecAvg_f32(float32 *X, short nX);
IFX_EXTERN float32 VecMax_f32(float32 *X, short nX);
IFX_EXTERN float32 VecMin_f32(float32 *X, short nX);
IFX_EXTERN float32 VecMinIdx_f32(float32 *X, short nX, sint16 *minIdx, sint16 *maxIdx);
IFX_EXTERN float32 VecMaxIdx_f32(float32 *X, short nX, sint16 *minIdx, sint16 *maxIdx);
IFX_EXTERN void    VecHalfSwap_f32(float32 *X, short nX);

/* Helper functions ----------------------------------------------------------*/
#ifdef __WIN32__
#include <stdio.h>
void Cplx_f32_printf(FILE *fp, pchar fileName, cfloat32 *data, long nX, int encloseData);
#else
#define Cplx_f32_printf(...)
#endif

#endif /* IFX_CF32_H */