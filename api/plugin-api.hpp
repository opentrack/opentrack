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

#include "compat/simple-mat.hpp"
#include "export.hpp"

using Pose = Mat<double, 6, 1>;

enum Axis {
    TX, TY, TZ, Yaw, Pitch, Roll,

    NonAxis = -1,
};

namespace plugin_api {
namespace detail {

class OTR_API_EXPORT BaseDialog : public QDialog
{
    Q_OBJECT
protected:
    BaseDialog();
public:
    void closeEvent(QCloseEvent *) override;
signals:
    void closing();
private slots:
    void done(int) override;
};

} // ns
} // ns

#define OTR_PLUGIN_EXPORT OTR_GENERIC_EXPORT

#define OPENTRACK_DECLARE_PLUGIN_INTERNAL(ctor_class, ctor_ret_class, metadata_class, dialog_class, dialog_ret_class) \
    extern "C" OTR_PLUGIN_EXPORT ctor_ret_class* GetConstructor(); \
    extern "C" OTR_PLUGIN_EXPORT Metadata* GetMetadata(); \
    extern "C" OTR_PLUGIN_EXPORT dialog_ret_class* GetDialog(); \
    \
    extern "C" OTR_PLUGIN_EXPORT ctor_ret_class* GetConstructor() \
    { \
        return new ctor_class; \
    } \
    extern "C" OTR_PLUGIN_EXPORT Metadata* GetMetadata() \
    { \
        return new metadata_class; \
    } \
    extern "C" OTR_PLUGIN_EXPORT dialog_ret_class* GetDialog() \
    { \
        return new dialog_class; \
    }

// implement this in all plugins
// also you must link against "opentrack-api" in CMakeLists.txt to avoid vtable link errors
struct OTR_API_EXPORT Metadata
{
    Metadata(const Metadata&) = delete;
    Metadata(Metadata&&) = delete;
    Metadata& operator=(const Metadata&) = delete;
    Metadata();

    // plugin name to be displayed in the interface
    virtual QString name() = 0;
    // plugin icon, you can return an empty QIcon()
    virtual QIcon icon() = 0;
    // optional destructor
    virtual ~Metadata();
};

struct OTR_API_EXPORT module_status final
{
    QString error;

    bool is_ok() const;
    module_status(const QString& error = QString());
};

struct OTR_API_EXPORT module_status_mixin
{
    static module_status status_ok();
    static module_status error(const QString& error);

    virtual module_status initialize() = 0;
};

// implement this in filters
struct OTR_API_EXPORT IFilter : module_status_mixin
{
    IFilter(const IFilter&) = delete;
    IFilter(IFilter&&) = delete;
    IFilter& operator=(const IFilter&) = delete;
    IFilter();

    // optional destructor
    virtual ~IFilter();
    // perform filtering step.
    // you have to take care of dt on your own, try "opentrack-compat/timer.hpp"
    virtual void filter(const double *input, double *output) = 0;
    // optionally reset the filter when centering
    virtual void center() {}
};

struct OTR_API_EXPORT IFilterDialog : public plugin_api::detail::BaseDialog
{
    IFilterDialog();

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

    IProtocol(const IProtocol&) = delete;
    IProtocol(IProtocol&&) = delete;
    IProtocol& operator=(const IProtocol&) = delete;

    // optional destructor
    virtual ~IProtocol();
    // called 250 times a second with XYZ yaw pitch roll pose
    // try not to perform intense computation here. use a thread.
    virtual void pose(const double* headpose) = 0;
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
    ITracker(ITracker&&) = delete;
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
};

// call once with your chosen class names in the plugin
#define OPENTRACK_DECLARE_TRACKER(tracker_class, dialog_class, metadata_class) \
    OPENTRACK_DECLARE_PLUGIN_INTERNAL(tracker_class, ITracker, metadata_class, dialog_class, ITrackerDialog)

struct OTR_API_EXPORT IExtension : module_status_mixin
{
    enum event_mask : unsigned
    {
        none = 0u,
        on_raw              = 1 << 0,
        on_before_filter    = 1 << 1,
        on_before_mapping   = 1 << 2,
        on_finished         = 1 << 3,
    };

    enum event_ordinal : unsigned
    {
        ev_raw                 = 0,
        ev_before_filter       = 1,
        ev_before_mapping      = 2,
        ev_finished            = 3,

        event_count = 4,
    };

    IExtension() = default;
    virtual ~IExtension();

    virtual event_mask hook_types() = 0;

    virtual void process_raw(Pose&) {}
    virtual void process_before_filter(Pose&) {}
    virtual void process_before_mapping(Pose&) {}
    virtual void process_finished(Pose&) {}

    IExtension(const IExtension&) = delete;
    IExtension(IExtension&&) = delete;
    IExtension& operator=(const IExtension&) = delete;
};

struct OTR_API_EXPORT IExtensionDialog : public plugin_api::detail::BaseDialog
{
    ~IExtensionDialog() override;

    virtual void register_extension(IExtension& ext) = 0;
    virtual void unregister_extension() = 0;
};

#define OPENTRACK_DECLARE_EXTENSION(ext_class, dialog_class, metadata_class) \
    OPENTRACK_DECLARE_PLUGIN_INTERNAL(ext_class, IExtension, metadata_class, dialog_class, IExtensionDialog)
