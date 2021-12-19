/* Copyright (c) 2013-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QString>
#include <QWidget>
#include <QFrame>
#include <QIcon>
#include <QWidget>
#include <QDialog>
#include <QCoreApplication>

#include "../compat/simple-mat.hpp"
#include "../compat/tr.hpp"
#include "export.hpp"

using Pose = Mat<double, 6, 1>;

enum Axis : int
{
    NonAxis = -1,
    TX = 0, TY = 1, TZ = 2,

    Yaw = 3, Pitch = 4, Roll = 5,
    Axis_MIN = TX, Axis_MAX = 5,

    Axis_COUNT = 6,
};

namespace plugin_api::detail {

class OTR_API_EXPORT BaseDialog : public QDialog
{
    Q_OBJECT
protected:
    BaseDialog();
public:
    void closeEvent(QCloseEvent *) override;
    virtual bool embeddable() noexcept;
    virtual void set_buttons_visible(bool x);   // XXX TODO remove it once all modules are converted
    virtual void save();                        // XXX HACK should be pure virtual
    virtual void reload();                      // XXX HACK should be pure virtual -sh 20211214
signals:
    void closing();
private slots:
    void done(int) override;
};

} // ns plugin_api::detail

#define OTR_PLUGIN_EXPORT OTR_GENERIC_EXPORT

#define OPENTRACK_DECLARE_PLUGIN_INTERNAL(ctor_class, ctor_ret_class, metadata_class, dialog_class, dialog_ret_class) \
    extern "C"                                                  \
    {                                                           \
        OTR_PLUGIN_EXPORT ctor_ret_class* GetConstructor(void); \
        ctor_ret_class* GetConstructor(void)                    \
        {                                                       \
            return new ctor_class;                              \
        }                                                       \
        OTR_PLUGIN_EXPORT Metadata_* GetMetadata(void);         \
        Metadata_* GetMetadata(void)                            \
        {                                                       \
            return new metadata_class;                          \
        }                                                       \
        OTR_PLUGIN_EXPORT dialog_ret_class* GetDialog(void);    \
        dialog_ret_class* GetDialog(void)                       \
        {                                                       \
            return new dialog_class;                            \
        }                                                       \
    }

// implement this in all plugins
// also you must link against "opentrack-api" in CMakeLists.txt to avoid vtable link errors
class OTR_API_EXPORT Metadata_
{
public:
    Metadata_();

    // plugin name to be displayed in the interface
    virtual QString name() = 0;
    // plugin icon, you can return an empty QIcon()
    virtual QIcon icon() = 0;
    // optional destructor
    virtual ~Metadata_();
};

class OTR_API_EXPORT Metadata : public TR, public Metadata_
{
    Q_OBJECT

public:
    Metadata();
    ~Metadata() override;
};

struct OTR_API_EXPORT module_status final
{
    QString error;

    bool is_ok() const;
    module_status();
    module_status(const QString& error);
};

/*
 * implement in all module types
 */
struct OTR_API_EXPORT module_status_mixin
{
    static module_status status_ok(); // return from initialize() if ok
    static module_status error(const QString& error); // return error message on init failure

    virtual module_status initialize() = 0; // where to return from
    virtual ~module_status_mixin();

    Q_DECLARE_TR_FUNCTIONS(module_status_mixin)
};

// implement this in filters
struct OTR_API_EXPORT IFilter : module_status_mixin
{
    IFilter(const IFilter&) = delete;
    IFilter& operator=(const IFilter&) = delete;
    IFilter();

    // optional destructor
    ~IFilter() override;
    // perform filtering step.
    // you have to take care of dt on your own, try "opentrack-compat/timer.hpp"
    virtual void filter(const double *input, double *output) = 0;
    // optionally reset the filter when centering
    virtual void center() {}
};

struct OTR_API_EXPORT IFilterDialog : public plugin_api::detail::BaseDialog
{
    IFilterDialog();
    ~IFilterDialog() override;

    // optional destructor
    //~IFilterDialog() override;
    // receive a pointer to the filter from ui thread
    virtual void register_filter(IFilter* filter) = 0;
    // received filter pointer is about to get deleted
    virtual void unregister_filter() = 0;
};

// call once with your chosen class names in the plugin
#define OPENTRACK_DECLARE_FILTER(filter_class, dialog_class, metadata_class) \
    OPENTRACK_DECLARE_PLUGIN_INTERNAL(filter_class, IFilter, metadata_class, dialog_class, IFilterDialog)

// implement this in protocols
struct OTR_API_EXPORT IProtocol : module_status_mixin
{
    IProtocol();
    ~IProtocol() override;

    IProtocol(const IProtocol&) = delete;
    IProtocol& operator=(const IProtocol&) = delete;

    // called 250 times a second with XYZ yaw pitch roll pose
    // try not to perform intense computation here. use a thread.
    virtual void pose(const double* pose, const double* raw) = 0;
    // return game name or placeholder text
    virtual QString game_name() = 0;
};

struct OTR_API_EXPORT IProtocolDialog : public plugin_api::detail::BaseDialog
{
    // optional destructor
    // ~IProtocolDialog() override;
    // receive a pointer to the protocol from ui thread
    virtual void register_protocol(IProtocol *protocol) = 0;
    // received protocol pointer is about to get deleted
    virtual void unregister_protocol() = 0;

    IProtocolDialog();
    ~IProtocolDialog() override;
};

// call once with your chosen class names in the plugin
#define OPENTRACK_DECLARE_PROTOCOL(protocol_class, dialog_class, metadata_class) \
    OPENTRACK_DECLARE_PLUGIN_INTERNAL(protocol_class, IProtocol, metadata_class, dialog_class, IProtocolDialog)

// implement this in trackers
struct OTR_API_EXPORT ITracker
{
    ITracker();

    // optional destructor
    virtual ~ITracker();
    // start tracking, and grab a frame to display webcam video in, optionally
    virtual module_status start_tracker(QFrame* frame) = 0;
    // return XYZ yaw pitch roll data. don't block here, use a separate thread for computation.
    virtual void data(double *data) = 0;
    // tracker notified of centering
    // returning true makes identity the center pose
    virtual bool center();

    static module_status status_ok();
    static module_status error(const QString& error);

    ITracker(const ITracker&) = delete;
    ITracker& operator=(const ITracker&) = delete;
};

struct OTR_API_EXPORT ITrackerDialog : public plugin_api::detail::BaseDialog
{
    // optional destructor
    //~ITrackerDialog() override;
    // receive a pointer to the tracker from ui thread
    virtual void register_tracker(ITracker *tracker);
    // received tracker pointer is about to get deleted
    virtual void unregister_tracker();

    ITrackerDialog();
    ~ITrackerDialog() override;
};

// call once with your chosen class names in the plugin
#define OPENTRACK_DECLARE_TRACKER(tracker_class, dialog_class, metadata_class) \
    OPENTRACK_DECLARE_PLUGIN_INTERNAL(tracker_class, ITracker, metadata_class, dialog_class, ITrackerDialog)

