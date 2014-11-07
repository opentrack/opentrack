#pragma once

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
};

template<int h, int w> using dmat = Mat<double, h, w>;

