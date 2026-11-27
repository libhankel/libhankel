#include "../external_libs/qwe_Chave/qwe_Chave.h"
#include "../external_libs/qwe_Key/qwe_Key.h"
#include "libhankel.h"

// Standard library headers
#include <float.h>

// Project / local headers
#include "../src/utils/validate_x.h"

/*
This file contains functions corresponding to strategies 12 and 13 in SASfit.
They have been grouped together as these are QWE transforms, that is Quadrature
With Extrapolation. Specifically:
- HANKEL_QWE 12
- HANKEL_CHAVE 13
*/

/* Both routines below place their quadrature intervals at the Bessel zeros
 * divided by x, so a non-positive or non-finite x would return DBL_MAX, Inf or
 * - for x < 0 - a plausible-looking finite number, all reported as a success. */

/**
 * @internal
 * @brief Computes Hankel transform using the Quadrature With Extrapolation method by Key.
 * @note Corresponds to strategy 12 in SASfit.
 *
 * @param nu         order of bessel function - must be 0 or 1
 * @param f          pointer to function to transform
 * @param x          value at which to compute the transform
 * @param f_ctx      pointer to struct containing inputs for f
 * @param output     pointer to var containing output from transform
 * @param max_iters  maximum number of QWE iterations before the extrapolation loop stops
 * @param eps_rel    relative error allowed e.g. 1e-9 (``eps_nriq`` in SASfit);
 *                   passed as eps_rel * 10 to qwe_Key, which divides rtol by 10
 *                   when sizing r_min, so the factors cancel to give
 *                   r_min = r_max * eps_rel.
 *
 * @return 0 on success, or a negative status code (see the
 *         <a href="../usage/status_codes.html">Status Codes</a> page). @p x
 *         must be finite and greater than zero.
 */
int hankel_transform_QWE_Key(int nu, form_factor_f f, const double x, void *f_ctx, double *output,
                             int max_iters, double eps_rel) {

    int status;

    status = validate_x(x);
    if (status != 0) {
        return status;
    }

    status = (int)qwe_Key(nu, f, x, f_ctx, output, max_iters, eps_rel * 10, DBL_MIN);

    return status;
}

/**
 * @internal
 * @brief Computes Hankel transform using the Quadrature With Extrapolation method by Chave.
 * @note Corresponds to strategy 13 in SASfit.
 *
 * @param nu         order of bessel function - must be 0 or 1
 * @param f          pointer to function to transform
 * @param x          value at which to compute the transform
 * @param f_ctx      pointer to struct containing inputs for f
 * @param output     pointer to var containing output from transform
 * @param max_iters  maximum number of QWE iterations before the extrapolation loop stops
 * @param eps_rel    relative error allowed e.g. 1e-9 (``eps_nriq`` in SASfit)
 *
 * @return 0 on success, or a negative status code (see the
 *         <a href="../usage/status_codes.html">Status Codes</a> page). @p x
 *         must be finite and greater than zero.
 */
int hankel_transform_QWE_Chave(int nu, form_factor_f f, const double x, void *f_ctx, double *output,
                               int max_iters, double eps_rel) {
    int status;

    status = validate_x(x);
    if (status != 0) {
        return status;
    }

    status = (int)qwe_Chave(nu, f, x, f_ctx, output, max_iters, eps_rel * 10, DBL_MIN);

    return status;
}
