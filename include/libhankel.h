
#ifndef LIBHANKEL_H
#define LIBHANKEL_H
#include <stddef.h>
// Provides LIBHANKEL_API: dllexport when building the DLL, dllimport when consuming it (no-op on non-Windows).
#include "libhankel_export.h"

/**
 * @struct strategy_params
 * @brief Parameters required by the selected Hankel strategy.
 *        This structure groups together parameters that influence
 *        the behavior and accuracy of the algorithm.
 */
typedef struct {

    /**
     * @brief Index of the Bessel zero used to split the integral at the
     *        boundary between the finite and oscillatory parts (capped at 10).
     *        Used by ``Adaptive_DE_Ooura`` only.
     *        Corresponds to ``N_ogata`` in SASfit.
     */
    int bessel_zero_idx;

    /**
     * @brief Number of quadrature nodes.
     *        Used by ``Fixed_DE_Ogata`` only.
     *        Corresponds to ``N_ogata`` in SASfit.
     */
    int n_nodes;

    /**
     * @brief Maximum number of QWE extrapolation iterations before the loop
     *        stops.
     *        Used by ``QWE_Key`` and ``QWE_Chave`` only.
     */
    int max_iters;

    /**
     * @brief Tolerance parameter or relative error allowed.
     *
     * Determines the numerical precision or convergence threshold
     * of the computation. Smaller values lead to higher accuracy
     * at the cost of increased computation time.
     * Corresponds to ``eps_nriq`` in SASfit.
     *
     * @note Only used by adaptive strategies: ``Adaptive_DE_Ooura``, ``QWE_Key``,
     *       and ``QWE_Chave``. Setting this for ``DHT_*``, ``Fixed_DE_Ogata``, or
     *       ``FFTLog`` has no effect and will produce a warning on stderr.
     */
    double eps_rel;

    /**
     * @brief Starting guess for the maximum in the function \f$ x \cdot formFactor \f$.
     *        Used by ``Fixed_DE_Ogata`` only.
     *        Corresponds to ``h_ogata`` in SASfit.
     */
    double f_max;
} strategy_params;

/**
 * @typedef form_factor_f
 * @brief Function pointer type for a form factor callback.
 *
 * This callback is used to evaluate the form factor at a given input value.
 *
 * @param x   Input variable .
 * @param ctx User-provided context or data pointer (may be NULL).
 *
 * @return The value of the form factor evaluated at @p x.
 */
typedef double (*form_factor_f)(double x, void *ctx);

/**
 * @def LIBHANKEL_ALL_STRATEGIES
 * @brief Every strategy name accepted by @ref hankel_transform, as a quoted,
 *        comma-separated list.
 *
 * Defined here so the C and the Python error messages are built from the same
 * literal and cannot drift apart when a strategy is added or renamed.
 */
#define LIBHANKEL_ALL_STRATEGIES                                                                   \
    "'DHT_Guptasarma', 'DHT_Guptasarma_Fast', 'DHT_Key_51', 'DHT_Key_101', "                       \
    "'DHT_Key_201', 'DHT_Anderson_801', 'Fixed_DE_Ogata', 'Adaptive_DE_Ooura', "                   \
    "'QWE_Chave', 'QWE_Key', 'FFTLog'"

/** @brief Computes Hankel transform using the method specified by the user. */
LIBHANKEL_API int hankel_transform(int nu, form_factor_f f, double *x, size_t len_x, void *f_ctx,
                                   double *output, const char *strategy_name,
                                   strategy_params strategy_params);

/** @brief Computes Hankel transform, using digital filters. */
LIBHANKEL_API int hankel_transform_DHT(int nu, form_factor_f f, double x, void *f_ctx,
                                       double *output, int n_strategy);

/** @brief Computes the Hankel transform of order @p nu using Ooura's
 *         double-exponential (DE) quadrature. */
LIBHANKEL_API int hankel_transform_DE_Ooura(int nu, form_factor_f f, const double *x, size_t len_x,
                                            void *f_ctx, double *output, int bessel_zero_idx, double eps_rel);

/** @brief Computes the Hankel transform of order @p nu using Ogata's (2005)
 *         DE quadrature rule. */
LIBHANKEL_API int hankel_transform_DE_Ogata(int nu, form_factor_f f, const double *x, size_t len_x,
                                            void *f_ctx, double *output, int n_nodes, double f_max);

/** @brief Computes Hankel transform using the Quadrature With Extrapolation method by Key. */
LIBHANKEL_API int hankel_transform_QWE_Key(int nu, form_factor_f f, double x, void *f_ctx,
                                           double *output, int max_iters, double eps_rel);

/** @brief Computes Hankel transform using the Quadrature With Extrapolation method by Chave. */
LIBHANKEL_API int hankel_transform_QWE_Chave(int nu, form_factor_f f, double x, void *f_ctx,
                                             double *output, int max_iters, double eps_rel);

/** @brief Computes G(0), the Hankel transform evaluated at r = 0. */
LIBHANKEL_API double hankel_G0(form_factor_f f, void *f_ctx, double epsrel);

/** @brief Computes the Hankel transform using the FFTLog algorithm. */
#ifdef __cplusplus
extern "C"
#endif
LIBHANKEL_API int hankel_transform_fftlog(int nu, form_factor_f f, const double *x, size_t len_x,
                                          void *f_ctx, double *output);

#endif // LIBHANKEL_H
