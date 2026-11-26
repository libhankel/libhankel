// clang-format off
#include "utils_for_tests/unity_config.h"
#include "unity.h"
// clang-format on
#include <math.h>
#include <stdio.h>

/*
Tests for tabulated_ff. Most tests check refusal: bad inputs that reach
hankel_transform would produce NaN or a wrong answer under status 0.
*/
#include "tabulated_ff.h"

/* Large enough that the fit window hits the 40-point cap, as real datasets do. */
#define N_PTS 200

/* Exponent of the default dataset: Porod's value for a sharp interface, and
 * comfortably above the 3/2 convergence threshold. */
#define TRUE_P 4.0

#define Q_FIRST 1.0e-3
#define Q_LAST 1.0e1

static double q_data[N_PTS];
static double f_data[N_PTS];

/* A pure power law, log-spaced. Pure so the fitted exponent has an exact known
 * answer: anything but TRUE_P is the fit being wrong, not the data curving. */
void setUp(void) {
    for (int i = 0; i < N_PTS; i++) {
        q_data[i] = Q_FIRST * pow(Q_LAST / Q_FIRST, (double)i / (N_PTS - 1));
        f_data[i] = pow(q_data[i], -TRUE_P);
    }
}

void tearDown(void) {}

/* Build a handle from the default dataset; asserts that create succeeds.
 * Evaluation tests use this so they can focus on eval, not setup. */
static tabulated_ff_t *make(tabulated_tail_t tail, double exponent) {
    tabulated_ff_t *t = NULL;
    TEST_ASSERT_EQUAL_INT(0, tabulated_ff_create(q_data, f_data, N_PTS, TABULATED_INTERP_CUBIC, tail, exponent, &t));
    return t;
}

/* Attempts create with minimal fixed config; returns the status code.
 * Rejection tests assert this returns -13 or -14. */
static int try_create(const double *q, const double *f) {
    tabulated_ff_t *t = NULL;
    // 5-point size is the smallest that passes the minimum-points check
    return tabulated_ff_create(q, f, 5, TABULATED_INTERP_CUBIC, TABULATED_TAIL_ZERO, 0.0, &t);
}

/* The fitted exponent is not exposed, so it is read back off the tail: at twice
 * q_hi a power law has fallen by 2^-p, one sixteenth for p = 4. That ratio is
 * the only observable the fit produces, and exactly what the transform depends
 * on. q_hi comes off the data array, not from Q_LAST: log-spacing leaves them a
 * couple of ULPs apart, enough to land on the wrong side of the join. */
static double tail_drop_at_double(tabulated_ff_t *t) {
    const double q_hi = q_data[N_PTS - 1];
    return tabulated_ff_eval(2.0 * q_hi, t) / tabulated_ff_eval(q_hi, t);
}

/* --------------------------------------------------------------------------
 * Rejecting invalid inputs
 * ----------------------------------------------------------------------- */

void test_create_rejects_null_arguments(void) {
    /* Passes NULL for q, f, and out in turn; each must return -13.
     * A null deref would be a segfault rather than a detectable bad answer. */
    tabulated_ff_t *t = NULL;

    TEST_ASSERT_EQUAL_INT_MESSAGE(
        -13, tabulated_ff_create(NULL, f_data, N_PTS, TABULATED_INTERP_CUBIC, TABULATED_TAIL_ZERO, 0.0, &t),
        "NULL q must be rejected");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        -13, tabulated_ff_create(q_data, NULL, N_PTS, TABULATED_INTERP_CUBIC, TABULATED_TAIL_ZERO, 0.0, &t),
        "NULL f must be rejected");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        -13, tabulated_ff_create(q_data, f_data, N_PTS, TABULATED_INTERP_CUBIC, TABULATED_TAIL_ZERO, 0.0, NULL),
        "NULL out must be rejected");
}

void test_create_leaves_out_null_on_failure(void) {
    /* create must write NULL into *out on failure so callers can safely call
     * destroy without checking the return code. t starts as a garbage pointer
     * so a failure path that skips the write does not accidentally look like NULL. */
    tabulated_ff_t *t = (tabulated_ff_t *)0xDEADBEEF;

    TEST_ASSERT_EQUAL_INT(-13,
                          tabulated_ff_create(q_data, f_data, 2, TABULATED_INTERP_CUBIC, TABULATED_TAIL_ZERO, 0.0, &t));
    TEST_ASSERT_NULL_MESSAGE(t, "out must be cleared even when create fails");
}

void test_create_rejects_too_few_points(void) {
    /* Passes 3 points when the minimum is 4; must return -13.
     * Caught early so the caller gets a point-count message rather than
     * the -3 that a NULL spline would become. */
    tabulated_ff_t *t = NULL;

    TEST_ASSERT_EQUAL_INT(-13,
                          tabulated_ff_create(q_data, f_data, 3, TABULATED_INTERP_CUBIC, TABULATED_TAIL_ZERO, 0.0, &t));
}

void test_create_rejects_non_increasing_q(void) {
    /* Passes q with two equal adjacent values; must return -13.
     * A zero-width segment gives an infinite slope. */

    double q[5] = {1.0, 2.0, 2.0, 4.0, 5.0};
    double f[5] = {1.0, 0.5, 0.4, 0.2, 0.1};

    TEST_ASSERT_EQUAL_INT(-13, try_create(q, f));
}

void test_create_rejects_non_positive_q(void) {
    /* Passes q[0] = 0; must return -13.
     * log(q) is taken when fitting the tail, and q <= 0 has no physical meaning.
     * Only q[0] is checked explicitly; strict increase covers the rest. */

    double q[5] = {0.0, 1.0, 2.0, 3.0, 4.0};
    double f[5] = {1.0, 0.5, 0.4, 0.2, 0.1};

    TEST_ASSERT_EQUAL_INT(-13, try_create(q, f));
}

void test_create_rejects_nan_in_q(void) {
    // Passes NaN in q; must return -13.

    double q[5] = {1.0, 2.0, NAN, 4.0, 5.0};
    double f[5] = {1.0, 0.5, 0.4, 0.2, 0.1};

    TEST_ASSERT_EQUAL_INT(-13, try_create(q, f));
}

void test_create_rejects_infinite_q(void) {
    /* Passes INFINITY as the last q value; must return -13.
     * INFINITY passes the strict-increase test, so it needs its own isfinite
     * check. */
    double q[5] = {1.0, 2.0, 3.0, 4.0, INFINITY};
    double f[5] = {1.0, 0.5, 0.4, 0.2, 0.1};

    TEST_ASSERT_EQUAL_INT(-13, try_create(q, f));
}

void test_create_rejects_non_finite_f(void) {
    /* Passes NaN and INFINITY in f; each must return -13.
     * f is never compared, so it gets no free NaN check the way q does.
     * Boost builds a spline through either value and returns NaN from every
     * eval that touches the affected segment. */
    double q[5] = {1.0, 2.0, 3.0, 4.0, 5.0};
    double f_nan[5] = {1.0, 0.5, NAN, 0.2, 0.1};
    double f_inf[5] = {1.0, 0.5, 0.4, INFINITY, 0.1};

    TEST_ASSERT_EQUAL_INT_MESSAGE(-13, try_create(q, f_nan), "NaN f rejected");
    TEST_ASSERT_EQUAL_INT_MESSAGE(-13, try_create(q, f_inf), "infinite f rejected");
}

void test_create_rejects_unknown_tail(void) {
    /* Passes an out-of-range tail value (99); must return -13.
     * The enum arrives as an integer from the Python binding, so an
     * out-of-range value is reachable. Unchecked it would fall through
     * eval's tail == ZERO test and silently behave as a power law. */
    tabulated_ff_t *t = NULL;

    TEST_ASSERT_EQUAL_INT(
        -13, tabulated_ff_create(q_data, f_data, N_PTS, TABULATED_INTERP_CUBIC, (tabulated_tail_t)99, 4.0, &t));
}

/* --------------------------------------------------------------------------
 * Rejecting tails that would not converge
 * ----------------------------------------------------------------------- */

void test_exponent_at_convergence_threshold_is_rejected(void) {
    /* p = 1.5 and NaN must be rejected; p = 1.51 must be accepted.
     * The integrand goes like q^(1/2 - p); at p = 3/2 exactly it diverges
     * logarithmically, so the threshold must be exclusive (strict >).
     * NaN fails the same comparison since any > with NaN is false.
     *
     * The reject cases come first: create clears *out, so a failure after
     * the success below would drop the handle it had just written. */
    tabulated_ff_t *t = NULL;

    TEST_ASSERT_EQUAL_INT_MESSAGE(
        -14, tabulated_ff_create(q_data, f_data, N_PTS, TABULATED_INTERP_CUBIC, TABULATED_TAIL_POWER_LAW, 1.5, &t),
        "p = 3/2 itself must be rejected");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        -14, tabulated_ff_create(q_data, f_data, N_PTS, TABULATED_INTERP_CUBIC, TABULATED_TAIL_POWER_LAW, NAN, &t),
        "NaN p must not slip through the comparison");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        0, tabulated_ff_create(q_data, f_data, N_PTS, TABULATED_INTERP_CUBIC, TABULATED_TAIL_POWER_LAW, 1.51, &t),
        "just above 3/2 must be accepted");
    tabulated_ff_destroy(t);
}


void test_create_rejects_exponent_from_data_not_at_tail(void) {
    /* Fits p from a DAB curve sampled before its power-law regime; must return -14.
     * True asymptote is p = 4, but data only reaches q * xi = 0.5 where the
     * curve is still bending. The fit sees a shallow local slope below 1.5,
     * which the convergence guard rejects. Without the guard the transform
     * would return plausible but wrong numbers. */
    const double xi = 10.0;
    double q[N_PTS];
    double f[N_PTS];
    tabulated_ff_t *t = NULL;

    for (int i = 0; i < N_PTS; i++) {
        q[i] = 1.0e-3 * pow(0.05 / 1.0e-3, (double)i / (N_PTS - 1));
        double u = q[i] * xi;
        f[i] = 1.0 / ((1.0 + u * u) * (1.0 + u * u));
    }

    TEST_ASSERT_EQUAL_INT_MESSAGE(
        -14, tabulated_ff_create(q, f, N_PTS, TABULATED_INTERP_CUBIC, TABULATED_TAIL_POWER_LAW, 0.0, &t),
        "data that does not reach the power-law tail must not fit a usable exponent");
}

/* --------------------------------------------------------------------------
 * The fit itself
 * ----------------------------------------------------------------------- */

void test_fitted_exponent_recovers_a_known_power_law(void) {
    /* Checks the fitted exponent by measuring how much the tail drops on doubling q. */
    tabulated_ff_t *t = make(TABULATED_TAIL_POWER_LAW, 0.0);

    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(1e-9, pow(2.0, -TRUE_P), tail_drop_at_double(t),
                                      "fitted exponent must reproduce the data's own power law");

    tabulated_ff_destroy(t);
}

void test_fit_skips_non_positive_values(void) {
    /* Corrupts the last 3 high-q points with negative/zero values; the
     * fitted exponent must still match the known power law.
     * log() of a non-positive is NaN and would propagate through all four
     * running sums, so the fit must skip non-positive points entirely. */
    f_data[N_PTS - 1] = -1.0e-5;
    f_data[N_PTS - 2] = -2.0e-5;
    f_data[N_PTS - 3] = 0.0;

    tabulated_ff_t *t = make(TABULATED_TAIL_POWER_LAW, 0.0);

    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(1e-9, pow(2.0, -TRUE_P), tail_drop_at_double(t),
                                      "the surviving points still describe the same power law");

    tabulated_ff_destroy(t);
}

/* --------------------------------------------------------------------------
 * Evaluation
 * ----------------------------------------------------------------------- */

void test_eval_reproduces_the_tabulated_values(void) {
    /* Evaluates at the first point, an interior point, and the last point;
     * each must match the original f value to within floating-point rounding.
     * Both endpoints are included because eval serves them from cached values,
     * a different branch from the interior. */
    tabulated_ff_t *t = make(TABULATED_TAIL_POWER_LAW, 0.0);

    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(1e-12 * f_data[0], f_data[0],
                                      tabulated_ff_eval(q_data[0], t), "at the first point");
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(1e-12 * f_data[100], f_data[100],
                                      tabulated_ff_eval(q_data[100], t), "at an interior point");
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(1e-12 * f_data[N_PTS - 1], f_data[N_PTS - 1],
                                      tabulated_ff_eval(q_data[N_PTS - 1], t),
                                      "at the last point");

    tabulated_ff_destroy(t);
}

void test_eval_below_the_data_is_flat(void) {
    /* Evaluates at two q values below the data range; both must return f_data[0].
     * An accidental extrapolation would separate them. */
    tabulated_ff_t *t = make(TABULATED_TAIL_POWER_LAW, 0.0);

    TEST_ASSERT_EQUAL_DOUBLE(f_data[0], tabulated_ff_eval(Q_FIRST * 0.5, t));
    TEST_ASSERT_EQUAL_DOUBLE(f_data[0], tabulated_ff_eval(1.0e-30, t));

    tabulated_ff_destroy(t);
}

void test_eval_above_the_data_follows_the_power_law(void) {
    /* Evaluates three decades above q_hi and checks the power-law decay.
     * The exponent is supplied (not fitted) so the expected value is pure
     * arithmetic and does not depend on the fit being correct. */
    tabulated_ff_t *t = make(TABULATED_TAIL_POWER_LAW, 3.0);

    double expected = f_data[N_PTS - 1] * pow(1000.0, -3.0);
    TEST_ASSERT_DOUBLE_WITHIN(1e-12 * expected, expected,
                              tabulated_ff_eval(1000.0 * q_data[N_PTS - 1], t));

    tabulated_ff_destroy(t);
}

void test_eval_above_the_data_is_zero_for_a_truncated_tail(void) {
    /* Checks that eval above the last data point returns exactly zero. */
    tabulated_ff_t *t = make(TABULATED_TAIL_ZERO, 0.0);
    const double q_hi = q_data[N_PTS - 1];

    TEST_ASSERT_EQUAL_DOUBLE(0.0, tabulated_ff_eval(q_hi * 1.001, t));
    TEST_ASSERT_EQUAL_DOUBLE(0.0, tabulated_ff_eval(1.0e20, t));

    /* Still the measured value exactly at q_hi: the zero cliff starts above it,
     * not at it. Use q_data[N_PTS-1], not Q_LAST — log-spacing leaves them just
     * barely different, which is the difference between f_hi and zero here. */
    TEST_ASSERT_EQUAL_DOUBLE(f_data[N_PTS - 1], tabulated_ff_eval(q_hi, t));

    tabulated_ff_destroy(t);
}

void test_power_law_tail_joins_continuously(void) {
    /* Checks that eval just above q_hi matches eval at q_hi; no step at the join.
     * The tail uses a ratio form so it starts exactly at f_hi. A * pow(q, -p)
     * would introduce a rounding gap, and a step in the integrand causes
     * ringing across the whole transform. */
    tabulated_ff_t *t = make(TABULATED_TAIL_POWER_LAW, 0.0);

    /* q_hi = last q value in the data — the upper boundary of the measured range.
     * Use q_data[N_PTS-1], not Q_LAST: log-spacing leaves them just barely
     * different, enough to put both samples in the tail and miss the join. */
    const double q_hi = q_data[N_PTS - 1];
    double at_join = tabulated_ff_eval(q_hi, t);
    double just_above = tabulated_ff_eval(q_hi * (1.0 + 1.0e-10), t);

    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(1e-8 * at_join, at_join, just_above,
                                      "the tail must not step away from the last measured value");

    tabulated_ff_destroy(t);
}

void test_eval_is_finite_across_the_whole_strategy_range(void) {
    /* Evaluates at 41 q values from 1e-20 to 1e+20; each must be finite.
     * The DHT strategy spans ~35 decades, and a single NaN anywhere poisons
     * the whole transform while hankel_transform still reports success. */
    tabulated_ff_t *t = make(TABULATED_TAIL_POWER_LAW, 0.0);

    for (int i = -20; i <= 20; i++) {
        double q = pow(10.0, (double)i);
        char msg[64];

        /* Include the exponent in the message: all iterations share the same
         * line number, so without it a failure gives no clue which q failed. */
        snprintf(msg, sizeof msg, "not finite at q = 1e%d", i);
        TEST_ASSERT_DOUBLE_IS_DETERMINATE_MESSAGE(tabulated_ff_eval(q, t), msg);
    }

    tabulated_ff_destroy(t);
}

void test_eval_returns_nan_for_null_handle(void) {
    /* Passes a NULL handle to eval; must return NaN rather than crash.
     * A caller who ignores a failed create should get a bad result, 
     * not a segfault. */
    TEST_ASSERT_DOUBLE_IS_NAN(tabulated_ff_eval(1.0, NULL));
}



int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_create_rejects_null_arguments);
    RUN_TEST(test_create_leaves_out_null_on_failure);
    RUN_TEST(test_create_rejects_too_few_points);
    RUN_TEST(test_create_rejects_non_increasing_q);
    RUN_TEST(test_create_rejects_non_positive_q);
    RUN_TEST(test_create_rejects_nan_in_q);
    RUN_TEST(test_create_rejects_infinite_q);
    RUN_TEST(test_create_rejects_non_finite_f);
    RUN_TEST(test_create_rejects_unknown_tail);

    RUN_TEST(test_exponent_at_convergence_threshold_is_rejected);
    RUN_TEST(test_create_rejects_exponent_from_data_not_at_tail);

    RUN_TEST(test_fitted_exponent_recovers_a_known_power_law);
    RUN_TEST(test_fit_skips_non_positive_values);

    RUN_TEST(test_eval_reproduces_the_tabulated_values);
    RUN_TEST(test_eval_below_the_data_is_flat);
    RUN_TEST(test_eval_above_the_data_follows_the_power_law);
    RUN_TEST(test_eval_above_the_data_is_zero_for_a_truncated_tail);
    RUN_TEST(test_power_law_tail_joins_continuously);
    RUN_TEST(test_eval_is_finite_across_the_whole_strategy_range);
    RUN_TEST(test_eval_returns_nan_for_null_handle);

    RUN_TEST(test_destroy_null_is_safe);

    return UNITY_END();
}
