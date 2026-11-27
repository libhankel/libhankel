// clang-format off
#include "utils_for_tests/unity_config.h"
#include "unity.h"
// clang-format on
#include <math.h>

#include "interp_loglinear.h"

void setUp(void) {}
void tearDown(void) {}

void test_create_validation(void) {
    /* Passes non-positive and non-finite y values to create; expects NULL back.
     * Log-linear interpolation is only defined for strictly positive ordinates. */
    double x_ok[3] = {0.0, 1.0, 2.0};
    double y_ok[3] = {1.0, 10.0, 100.0};
    double y_zero[3] = {1.0, 0.0, 100.0};
    double y_neg[3] = {1.0, -10.0, 100.0};
    double y_nan[3] = {1.0, NAN, 100.0};
    double y_inf[3] = {1.0, INFINITY, 100.0};

    TEST_ASSERT_NULL_MESSAGE(loglinear_interp_create(x_ok, y_zero, 3), "reject y = 0");
    TEST_ASSERT_NULL_MESSAGE(loglinear_interp_create(x_ok, y_neg, 3), "reject y < 0");
    TEST_ASSERT_NULL_MESSAGE(loglinear_interp_create(x_ok, y_nan, 3), "reject NaN ordinate");
    TEST_ASSERT_NULL_MESSAGE(loglinear_interp_create(x_ok, y_inf, 3), "reject infinite ordinate");

    /* Passes non-increasing and non-finite x values, plus NULL pointers; expects NULL back.*/
    double x_dup[3] = {0.0, 1.0, 1.0};
    double x_nan[3] = {NAN, 1.0, 2.0};

    TEST_ASSERT_NULL_MESSAGE(loglinear_interp_create(x_dup, y_ok, 3), "reject non-increasing x");
    TEST_ASSERT_NULL_MESSAGE(loglinear_interp_create(x_nan, y_ok, 3), "reject NaN abscissa");
    TEST_ASSERT_NULL_MESSAGE(loglinear_interp_create(x_ok, y_ok, 1), "reject n < 2");
    TEST_ASSERT_NULL_MESSAGE(loglinear_interp_create(NULL, y_ok, 3), "reject NULL x");
    TEST_ASSERT_NULL_MESSAGE(loglinear_interp_create(x_ok, NULL, 3), "reject NULL y");

    loglinear_interp_t *h = loglinear_interp_create(x_ok, y_ok, 3);
    TEST_ASSERT_NOT_NULL_MESSAGE(h, "accept valid positive data");
    loglinear_interp_destroy(h);
}

void test_exponential_interpolation(void) {
    /* Evaluates the midpoint and a quarter point and checks the expected power-law values.
     * Log-linear interpolation computes exp(linear(log y)), so the midpoint is the geometric
     * mean and the quarter point follows y0 * (y1/y0)^t, here 100^0.25 ≈ 3.162. */
    double x[2] = {0.0, 1.0};
    double y[2] = {1.0, 100.0};

    loglinear_interp_t *h = loglinear_interp_create(x, y, 2);
    TEST_ASSERT_NOT_NULL_MESSAGE(h, "create should succeed");

    /* Midpoint: geometric mean of 1 and 100 is 10. */
    double mid = loglinear_interp_eval(h, 0.5);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(1e-14, 10.0, mid, "midpoint should be geometric mean");

    /* Quarter point: exp(0.25 * log(100)) = 100^0.25 ≈ 3.162. */
    double quarter = loglinear_interp_eval(h, 0.25);
    double expected_quarter = pow(100.0, 0.25);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(1e-14, expected_quarter, quarter,
                                      "quarter point should follow power law");

    loglinear_interp_destroy(h);
}

void test_eval_error_handling(void) {
    /* Calls eval with a NULL handle, below-range, above-range, and NaN inputs; expects NaN back.
     * NaN on bad input lets callers detect failures without a separate error channel. */
    TEST_ASSERT_DOUBLE_IS_NAN_MESSAGE(loglinear_interp_eval(NULL, 0.5), "NULL handle");

    double x[3] = {1.0, 2.0, 3.0};
    double y[3] = {1.0, 10.0, 100.0};

    loglinear_interp_t *h = loglinear_interp_create(x, y, 3);
    TEST_ASSERT_NOT_NULL_MESSAGE(h, "create should succeed");

    TEST_ASSERT_DOUBLE_IS_NAN_MESSAGE(loglinear_interp_eval(h, 0.0), "below range");
    TEST_ASSERT_DOUBLE_IS_NAN_MESSAGE(loglinear_interp_eval(h, 4.0), "above range");
    TEST_ASSERT_DOUBLE_IS_NAN_MESSAGE(loglinear_interp_eval(h, NAN), "NaN input");

    loglinear_interp_destroy(h);

    /* Calls destroy(NULL) to confirm it is a no-op.
     * Standard C convention for free-like functions; callers often pass handles that may be NULL. */
    loglinear_interp_destroy(NULL);
}

void test_monotonicity(void) {
    /* Densely samples a monotone-decreasing table and checks every consecutive pair is non-increasing.
     * Log-linear interpolation preserves monotonicity because exp is strictly monotone. */
    enum { N = 64 };
    double x[N], y[N];

    for (int i = 0; i < N; i++) {
        x[i] = pow(10.0, -3.0 + 5.0 * i / (N - 1));
        y[i] = 1.0 / (1.0 + x[i] * x[i] * x[i] * x[i]);
    }

    loglinear_interp_t *h = loglinear_interp_create(x, y, N);
    TEST_ASSERT_NOT_NULL_MESSAGE(h, "create should succeed");

    double prev = INFINITY;
    for (int i = 0; i < 2001; i++) {
        double xi = x[0] + (x[N - 1] - x[0]) * i / 2000.0;
        double v = loglinear_interp_eval(h, xi);
        char msg[64];

        snprintf(msg, sizeof msg, "non-monotone at xi = %g", xi);
        TEST_ASSERT_TRUE_MESSAGE(v <= prev && v > 0.0, msg);

        prev = v;
    }

    loglinear_interp_destroy(h);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_create_validation);
    RUN_TEST(test_exponential_interpolation);
    RUN_TEST(test_eval_error_handling);
    RUN_TEST(test_monotonicity);
    return UNITY_END();
}
