#pragma once
#include <initializer_list>

template<typename num, int h, int w>
struct Mat
{
    num data[h][w];

    template<int p>
    Mat<num, w, p> operator*(const Mat<num, w, p>& other) const
    {
        Mat<num, w, p> ret;
        for (int j = 0; j < w; j++)
            for (int i = 0; i < p; i++)
            {
                num sum = num(0);

                for (int k = 0; k < h; k++)
                    sum += data[j][k]*other.data[k][i];

                ret.data[j][i] = sum;
            }

        return ret;
    }

    num operator()(int j, int i) const { return data[j][i]; }
    num& operator()(int j, int i) { return data[j][i]; }

    Mat(std::initializer_list<num>&& list)
    {
        auto iter = list.begin();
        for (int i = 0; i < h; i++)
            for (int j = 0; j < w; j++)
                data[i][j] = *iter++;
    }

    Mat()
    {
        for (int j = 0; j < h; j++)
            for (int i = 0; i < w; i++)
                data[j][i] = 0;
    }

    Mat(const num* mem)
    {
        for (int j = 0; j < h; j++)
            for (int i = 0; i < w; i++)
                data[j][i] = mem[i*h+j];
    }

    // XXX add more operators as needed, third-party dependencies mostly
    // not needed merely for matrix algebra -sh 20141030

    static Mat<num, h, h> eye()
    {
        Mat<num, h, h> ret;
        for (int j = 0; j < h; j++)
            for (int i = 0; i < w; i++)
                ret.data[j][i] = 0;

        for (int i = 0; i < h; i++)
            ret.data[i][i] = 1;

        return ret;
    }

    Mat<num, w, h> t()
    {
        Mat<num, w, h> ret;

        for (int j = 0; j < h; j++)
            for (int i = 0; i < w; i++)
                ret.data[i][j] = data[j][i];

        return ret;
    }
    
    template<int h_, int w_> using dmat = Mat<double, h_, w_>;
    
    // http://stackoverflow.com/a/18436193
    static dmat<3, 1> rmat_to_euler(const dmat<3, 3>& R)
    {
        static constexpr double pi = 3.141592653;
        const double up = 90 * pi / 180.;
        static constexpr double bound = 1. - 2e-4;
        if (R(0, 2) > bound)
        {
            double roll = atan(R(1, 0) / R(2, 0));
            return dmat<3, 1>({0., up, roll});
        }
        if (R(0, 2) < -bound)
        {
            double roll = atan(R(1, 0) / R(2, 0));
            return dmat<3, 1>({0., -up, roll});
        }
        double pitch = asin(-R(0, 2));
        double roll = atan2(R(1, 2), R(2, 2));
        double yaw = atan2(R(0, 1), R(0, 0));
        return dmat<3, 1>({yaw, pitch, roll});
    }
    
    // tait-bryan angles, not euler
    static dmat<3, 3> euler_to_rmat(const double* input)
    {
        static constexpr double pi = 3.141592653;
        auto H = input[0] * pi / 180;
        auto P = input[1] * pi / 180;
        auto B = input[2] * pi / 180;
    
        const auto c1 = cos(H);
        const auto s1 = sin(H);
        const auto c2 = cos(P);
        const auto s2 = sin(P);
        const auto c3 = cos(B);
        const auto s3 = sin(B);
    
        double foo[] = {
            // z
            c1 * c2,
            c1 * s2 * s3 - c3 * s1,
            s1 * s3 + c1 * c3 * s2,
            // y
            c2 * s1,
            c1 * c3 + s1 * s2 * s3,
            c3 * s1 * s2 - c1 * s3,
            // x
            -s2,
            c2 * s3,
            c2 * c3
        };
    
        return dmat<3, 3>(foo);
    }
};

template<int h, int w> using dmat = Mat<double, h, w>;
