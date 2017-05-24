#pragma once

#include <cmath>
#include <cinttypes>

// no copyright information other than the attribution below.

// code shared as an example/tutorial of running variance
// written by John D. Cook on the website <http://www.johndcook.com/blog/standard_deviation>

// following references in the site's article:

// Chan, Tony F.; Golub, Gene H.; LeVeque, Randall J. (1983).
// Algorithms for Computing the Sample Variance: Analysis and Recommendations.
// The American Statistician 37, 242-247.

// Ling, Robert F. (1974).
// Comparison of Several Algorithms for Computing Sample Means and Variances.
// Journal of the American Statistical Association, Vol. 69, No. 348, 859-866.

class variance
{
    double m_old, m_new, s_old, s_new;
    std::uintptr_t cnt;

public:
    using size_type = std::uintptr_t;

    variance() : cnt(0) {}

    void clear() { *this = variance(); }

    size_type count() { return cnt; }

    void input(double x)
    {
        cnt++;

        if (cnt == 1)
        {
            m_old = m_new = x;
            s_old = 0;
        }
        else
        {
            m_new = m_old + (x - m_old)/cnt;
            s_new = s_old + (x - m_old)*(x - m_new);

            m_old = m_new;
            s_old = s_new;
        }
    }

    double avg() const { return cnt > 0 ? m_new : 0; }
    double Var() const { return cnt > 1 ? s_new/(cnt - 1) : 0; }
    double stddev() const { return std::sqrt(Var()); }
};
