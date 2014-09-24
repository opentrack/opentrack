#pragma once

#include <algorithm>
#undef NDEBUG
#include <cassert>
#include <iterator>
#include <tuple>
#include <deque>
#include <vector>

#include "timer.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QDebug>

namespace detail {
    template<typename t1, typename t2, typename t, typename m = t>
    class zip_iterator : public std::iterator<std::forward_iterator_tag, t>
    {
    private:
        using self = zip_iterator<t1, t2, t, m>;
        t1 x1, z1;
        t2 x2, z2;
        void maybe_end() { if (x1 == z1 || x2 == z2) *this = end(); }
    public:
        zip_iterator(const t1& it1, const t1& end1, const t2& it2, const t2& end2)
            : x1(it1), z1(end1), x2(it2), z2(end2) { maybe_end(); }
        constexpr zip_iterator() {}
        
        static constexpr self end() { return self(); }
        
        self operator++() { x1++; x2++; self tmp = *this; maybe_end(); return tmp; }
        self operator++(int) { self tmp(*this); x1++; x2++; maybe_end(); return tmp; }
        bool operator==(const self& rhs) const { return x1 == rhs.x1 && x2 == rhs.x2; }
        bool operator!=(const self& rhs) const { return !this->operator ==(rhs); }
        t operator*() { return m(*x1, *x2); }
    };
}

class Gain {
private:
    static constexpr bool use_box_filter = true;
    static constexpr int box_size = 20 / 640.;
    static constexpr double control_upper_bound = 1.0; // XXX FIXME implement for logitech crapola
    static constexpr int GAIN_HISTORY_COUNT = 15, GAIN_HISTORY_EVERY_MS = 200;
    
    int control;
    double step, eps;
    
    std::deque<double> means_history;
    
    Timer debug_timer, history_timer;
    
    typedef unsigned char px;
    template<typename t1, typename t2, typename t, typename m = t>
    using zip_iterator = detail::zip_iterator<t1, t2, t, m>;
    
    static double mean(const cv::Mat& frame)
    {
        // grayscale only
        assert(frame.channels() == 1);
        assert(frame.elemSize() == 1);
        assert(!frame.empty());
        
        return std::accumulate(frame.begin<px>(), frame.end<px>(), 0.) / (frame.rows * frame.cols);
    }
    
    static double get_variance(const cv::Mat& frame, double mean)
    {
        struct variance {
        private:
            double mu;
        public:
            variance(double mu) : mu(mu) {}
            double operator()(double seed, px p)
            {
                double tmp = p - mu;
                return seed + tmp * tmp;
            }
        } logic(mean);
        
        return std::accumulate(frame.begin<unsigned char>(), frame.end<unsigned char>(), 0., logic) / (frame.rows * frame.cols);
    }
    
    static double get_covariance(const cv::Mat& frame, double mean, double prev_mean)
    {
        struct covariance {
        public:
            using pair = std::tuple<px, px>;
        private:
            double mu_0, mu_1;
            
            inline double Cov(double seed, const pair& t)
            {
                px p0 = std::get<0>(t);
                px p1 = std::get<1>(t);
                return seed + (p0 - mu_0) * (p1 - mu_1);
            }
        public:
            covariance(double mu_0, double mu_1) : mu_0(mu_0), mu_1(mu_1) {}
            
            double operator()(double seed, const pair& t)
            {
                return Cov(seed, t);
            }
        } logic(mean, prev_mean);
        
        const double N = frame.rows * frame.cols;
        
        using zipper = zip_iterator<cv::MatConstIterator_<px>,
        cv::MatConstIterator_<px>,
        std::tuple<px, px>>;
        
        zipper zip(frame.begin<px>(),
                                             frame.end<px>(),
                                             frame.begin<px>(),
                                             frame.end<px>());
        std::vector<covariance::pair> values(zip, zipper::end());
        
        return std::accumulate(values.begin(), values.end(), 0., logic) / N;
    }
    
#pragma GCC diagnostic ignored "-Wsign-compare"
    
public:
    Gain(int control = CV_CAP_PROP_GAIN, double step = 0.3, double eps = 0.02) :
        control(control), step(step), eps(eps)
    {
    }
    
    void tick(cv::VideoCapture&, const cv::Mat& frame_)
    {
        cv::Mat frame;
        
        if (use_box_filter)
        {
            cv::Mat tmp(frame_);
            static constexpr int min_box = 3;
            static constexpr int box = 2 * box_size;
            cv::blur(frame_, tmp, cv::Size(min_box + box * frame_.cols, min_box + box * frame_.rows));
            frame = tmp;
        }
        else
            frame = frame_;

        const double mu = mean(frame);
        const double var = get_variance(frame, mu);
        
        if (debug_timer.elapsed_ms() > 500)
        {
            debug_timer.start();
            qDebug() << "gain:" << "mean" << mu << "variance" << var;
        }
        
        const int sz = means_history.size();
        
        for (int i = 0; i < sz; i++)
        {
            const double cov = get_covariance(frame, mu, means_history[i]);
            
            qDebug() << "cov" << i << cov;
        }
        
        if (GAIN_HISTORY_COUNT > means_history.size() && history_timer.elapsed_ms() > GAIN_HISTORY_EVERY_MS)
        {
            means_history.push_front(mu);
            
            if (GAIN_HISTORY_COUNT == means_history.size())
                means_history.pop_back();
        }
    }
};
