/* Copyright (c) 2012-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include "ui_ftnoir_accela_filtercontrols.h"
#include "api/plugin-api.hpp"
#include "spline-widget/spline.hpp"
#include <atomic>
#include <QMutex>
#include <QTimer>

#include "options/options.hpp"
using namespace options;
#include "compat/timer.hpp"

struct settings_accela : opts
{
    static constexpr double rot_gains[16][2] =
    {
        { 6, 200 },
        { 2.66, 50 },
        { 1.66, 17 },
        { 1, 4 },
        { .5, .53 },
        { 0, 0 },
        { -1, 0 }
    };

    static constexpr double trans_gains[16][2] =
    {
        { 2.33, 40 },
        { 1.66, 13 },
        { 1.33, 5 },
        { .66, 1 },
        { .33, .5 },
        { 0, 0 },
        { -1, 0 }
    };

    static void make_splines(spline& rot, spline& trans);

    value<int> rot_threshold, trans_threshold, ewma, rot_deadzone, trans_deadzone;
    value<slider_value> rot_nonlinearity;
    static constexpr double mult_rot = 4. / 100.;
    static constexpr double mult_trans = 4. / 100.;
    static constexpr double mult_rot_dz = 2. / 100.;
    static constexpr double mult_trans_dz = 2. / 100.;
    static constexpr double mult_ewma = 1.25;
    static constexpr double max_rot_nl = 1.33;
    settings_accela() :
        opts("Accela"),
        rot_threshold(b, "rotation-threshold", 45),
        trans_threshold(b, "translation-threshold", 50),
        ewma(b, "ewma", 2),
        rot_deadzone(b, "rotation-deadzone", 0),
        trans_deadzone(b, "translation-deadzone", 0),
        rot_nonlinearity(b, "rotation-nonlinearity", slider_value(1, 1, 2))
    {}
};

class FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void filter(const double* input, double *output) override;
    void center() override { first_run = true; }
    spline rot, trans;
private:
    settings_accela s;
    bool first_run;
    double last_output[6];
    double smoothed_input[6];
    Timer t;

    template <typename T>
    static inline int signum(T x, std::false_type)
    {
        return T(0) < x;
    }

    template <typename T>
    static inline int signum(T x, std::true_type)
    {
        return (T(0) < x) - (x < T(0));
    }

    template <typename T>
    static inline int signum(T x)
    {
        return signum(x, std::is_signed<T>());
    }
};

class FilterControls: public IFilterDialog
{
    Q_OBJECT
public:
    FilterControls();
    void register_filter(IFilter*) override {}
    void unregister_filter() override {}
private:
    Ui::AccelaUICFilterControls ui;
    void save();
    settings_accela s;
private slots:
    void doOK();
    void doCancel();
    void update_ewma_display(int value);
    void update_rot_display(int value);
    void update_trans_display(int value);
    void update_rot_dz_display(int value);
    void update_trans_dz_display(int value);
    void update_rot_nl_slider(const slider_value& sl);
};

class FTNoIR_FilterDll : public Metadata
{
public:
    QString name() { return QString("Accela"); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};
