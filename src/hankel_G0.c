#include "libhankel.h"
#include "src/utils/sasfit_integrate.h"
#include <math.h>

static double q_times_f(double q, hankel_inputs *inputs) {
    return q * inputs->function(q, inputs->f_params);
}

/**
 * @brief Computes G(0), the Hankel transform evaluated at r = 0.
 *
 * Computes \f$ G(0) = \int_0^\infty q \, f(q) \, dq \f$ using tanh-sinh quadrature.
 *
 * @param f       Form factor function to integrate.
 * @param f_ctx   Context pointer passed to @p f on every evaluation.
 * @param epsrel  Relative error tolerance (e.g. 1e-6).
 *
 * @return The value of G(0).
 */
double hankel_G0(form_factor_f f, void *f_ctx, double epsrel) {
    hankel_inputs inputs = {0};
    inputs.function = f;
    inputs.f_params = f_ctx;
    return sasfit_integrate_ctm(0.0, INFINITY, q_times_f, &inputs, epsrel);
}
