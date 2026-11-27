// clang-format off
#include "utils_for_tests/unity_config.h"
#include "unity.h"
// clang-format on
#include <math.h>
#include <stdio.h>

/*
Tests for the cubic (PCHIP) interpolator wrapper in src/utils/interp_cubic.cpp.

Includes the header. cubic_interp_t is an opaque type that cannot be
usefully redeclared here, and including the header also exercises its
__cplusplus linkage guard from a C translation unit.
*/
#include "interp_cubic.h"

void setUp(void) {}
void tearDown(void) {}

void test_eval_at_node_returns_exact_value(void) {
    /* Evaluates the spline at a data point and checks the result matches the input y value.
     * Also a canary for an off-by-one in the vector copy inside cubic_interp_create. */
     
    double x[5] = {0.0, 1.0, 2.0, 3.0, 4.0};
    double y[5] = {0.0, 1.0, 4.0, 9.0, 16.0};

    cubic_interp_t *h = cubic_interp_create(x, y, 5);
    TEST_ASSERT_NOT_NULL_MESSAGE(h, "create should succeed for 5 valid increasing points");

    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(1e-12, 4.0, cubic_interp_eval(h, 2.0),
                                      "spline must pass through the data point at x = 2");

    cubic_interp_destroy(h);
}

void test_create_validation(void) {
    /* Passes non-increasing x, too few points, and NULL pointers to create; expects NULL back.
     * Each check is independent so a missing guard for one class still fails here. */

    double x_ok[5] = {0.0, 1.0, 2.0, 3.0, 4.0};
    double y_ok[5] = {0.0, 1.0, 4.0, 9.0, 16.0};
    double x_dup[5] = {0.0, 1.0, 1.0, 3.0, 4.0};
    double x_few[3] = {0.0, 1.0, 2.0};
    double y_few[3] = {0.0, 1.0, 4.0};

    TEST_ASSERT_NULL_MESSAGE(cubic_interp_create(x_dup, y_ok, 5), "reject non-increasing x");
    TEST_ASSERT_NULL_MESSAGE(cubic_interp_create(x_few, y_few, 3), "reject n < 4");
    TEST_ASSERT_NULL_MESSAGE(cubic_interp_create(NULL, y_ok, 5), "reject NULL x");
    TEST_ASSERT_NULL_MESSAGE(cubic_interp_create(x_ok, NULL, 5), "reject NULL y");
}

void test_eval_returns_nan_for_null_handle(void) {
    /* Calls eval with a NULL handle; expects NaN back; also confirms destroy(NULL) is a no-op.
     * Null dereferences are segfaults, not exceptions — the wrapper must guard eval explicitly. */
    double xi = 2.0;

    TEST_ASSERT_DOUBLE_IS_NAN_MESSAGE(cubic_interp_eval(NULL, xi),
                                      "eval should return NaN for NULL handle");

    /* Standard C convention for free-like functions; callers often pass handles that may be NULL. */
    cubic_interp_destroy(NULL);
}

void test_eval_returns_nan_outside_range(void) {
    /* Calls eval below and above the data range; expects NaN back.
     * Boost throws on extrapolation and the catch returns NaN; both ends are tested independently 
     * in case one is broken. */

    double x[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double y[5] = {1.0, 4.0, 9.0, 16.0, 25.0};

    cubic_interp_t *h = cubic_interp_create(x, y, 5);
    TEST_ASSERT_NOT_NULL_MESSAGE(h, "create should succeed for 5 valid increasing points");

    TEST_ASSERT_DOUBLE_IS_NAN_MESSAGE(cubic_interp_eval(h, 0.0),
                                      "eval should return NaN below the bottom of the range");
    TEST_ASSERT_DOUBLE_IS_NAN_MESSAGE(cubic_interp_eval(h, 6.0),
                                      "eval should return NaN above the top of the range");

    cubic_interp_destroy(h);
}

void test_eval_at_range_endpoints_is_finite(void) {
    /* checks that evaluating exactly at x[0] and x[n-1] — the first and last data points — 
     * returns a finite number rather than NaN. The range check is inclusive; 
     * a < vs <= bug breaks one endpoint without affecting the out-of-range test.*/

    double x[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double y[5] = {1.0, 4.0, 9.0, 16.0, 25.0};

    cubic_interp_t *h = cubic_interp_create(x, y, 5);
    TEST_ASSERT_NOT_NULL_MESSAGE(h, "create should succeed for 5 valid increasing points");

    TEST_ASSERT_DOUBLE_IS_DETERMINATE_MESSAGE(cubic_interp_eval(h, 1.0),
                                              "eval must be finite at the bottom of the range");
    TEST_ASSERT_DOUBLE_IS_DETERMINATE_MESSAGE(cubic_interp_eval(h, 5.0),
                                              "eval must be finite at the top of the range");

    cubic_interp_destroy(h);
}

void test_no_overshoot_on_step_data(void) {
    /* Fits a step function and sweeps 101 points, checking every result stays in [0, 1].
     * A plain C² spline rings on step data; PCHIP's monotonicity preservation prevents overshooting. */

    double x[6] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0};
    double y[6] = {0.0, 0.0, 0.0, 1.0, 1.0, 1.0};

    const double eps = 1e-12;
    const int nsamples = 101;
    int i;

    cubic_interp_t *h = cubic_interp_create(x, y, 6);
    TEST_ASSERT_NOT_NULL_MESSAGE(h, "create should succeed for 6 valid increasing points");

    for (i = 0; i < nsamples; i++) {
        double xi = 5.0 * i / (nsamples - 1);
        double v = cubic_interp_eval(h, xi);
        char msg[64];

        snprintf(msg, sizeof msg, "value left [0, 1] at xi = %g", xi);

        TEST_ASSERT_GREATER_OR_EQUAL_DOUBLE_MESSAGE(0.0 - eps, v, msg);
        TEST_ASSERT_LESS_OR_EQUAL_DOUBLE_MESSAGE(1.0 + eps, v, msg);
    }

    cubic_interp_destroy(h);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_eval_at_node_returns_exact_value);
    RUN_TEST(test_create_validation);
    RUN_TEST(test_eval_returns_nan_for_null_handle);
    RUN_TEST(test_eval_returns_nan_outside_range);
    RUN_TEST(test_eval_at_range_endpoints_is_finite);
    RUN_TEST(test_no_overshoot_on_step_data);
    return UNITY_END();
}
