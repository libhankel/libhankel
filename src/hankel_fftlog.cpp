#include "../external_libs/fft_log/fft_log.h"
#include <math.h>

extern "C" {
#include "libhankel.h"
#include "utils/validate_x.h"
}

/**
 * @internal
 * @brief Computes the Hankel transform using the FFTLog algorithm.
 *
 * Computes the whole transformed curve in one O(N log N) pass and returns
 * values at individual x points via a cached spline.  Efficient when the
 * same form factor is evaluated at many x values in sequence.
 *
 * @param nu     order of Bessel function — must be 0 or 1
 * @param f      form factor callback
 * @param x      array of x values at which to evaluate the transform
 * @param len_x  number of entries in @p x and @p output
 * @param f_ctx  context pointer passed through to @p f
 * @param output result array
 *
 * @return 0 on success, or a negative status code.
 */
extern "C" int hankel_transform_fftlog(int nu, form_factor_f f, const double *x, size_t len_x,
                             void *f_ctx, double *output) {
    int status = validate_x_array(x, len_x);
    if (status != 0) {
        return status;
    }

    for (size_t j = 0; j < len_x; j++) {
        const double v = sasfit_fftlog((double)nu, f, x[j], f_ctx);
        if (isnan(v)) {
            return -1;
        }
        output[j] = v;
    }
    return 0;
}
