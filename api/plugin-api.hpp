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

#include "export.hpp"

#ifndef OPENTRACK_PLUGIN_EXPORT
#   ifdef _WIN32
#       define OPENTRACK_PLUGIN_LINKAGE __declspec(dllexport)
#   else
#       define OPENTRACK_PLUGIN_LINKAGE
#   endif
#   ifndef _MSC_VER
#       define OPENTRACK_PLUGIN_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_PLUGIN_LINKAGE
#   else
#       define OPENTRACK_PLUGIN_EXPORT OPENTRACK_PLUGIN_LINKAGE
#   endif
#endif

enum Axis {
    TX = 0, TY = 1, TZ = 2, Yaw = 3, Pitch = 4, Roll = 5,
    // for indexing in general
    rYaw = 0, rPitch = 1, rRoll = 2,
};

namespace plugin_api {
namespace detail {

class OPENTRACK_API_EXPORT BaseDialog : public QWidget
{
    Q_OBJECT
protected:
    BaseDialog();
public:
    void closeEvent(QCloseEvent *) override;
signals:
    void closing();
};

} // ns
} // ns

#define OPENTRACK_DECLARE_PLUGIN_INTERNAL(ctor_class, ctor_ret_class, metadata_class, dialog_class, dialog_ret_class) \
    extern "C" OPENTRACK_PLUGIN_EXPORT ctor_ret_class* GetConstructor(); \
    extern "C" OPENTRACK_PLUGIN_EXPORT Metadata* GetMetadata(); \
    extern "C" OPENTRACK_PLUGIN_EXPORT dialog_ret_class* GetDialog(); \
    \
    extern "C" OPENTRACK_PLUGIN_EXPORT ctor_ret_class* GetConstructor() \
    { \
        return new ctor_class; \
    } \
    extern "C" OPENTRACK_PLUGIN_EXPORT Metadata* GetMetadata() \
    { \
        return new metadata_class; \
    } \
    extern "C" OPENTRACK_PLUGIN_EXPORT dialog_ret_class* GetDialog() \
    { \
        return new dialog_class; \
    }

// implement this in all plugins
// also you must link against "opentrack-api" in CMakeLists.txt to avoid vtable link errors
struct OPENTRACK_API_EXPORT Metadata
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

// implement this in filters
struct OPENTRACK_API_EXPORT IFilter
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

struct OPENTRACK_API_EXPORT IFilterDialog : public plugin_api::detail::BaseDialog
{
    IFilterDialog();

    // optional destructor
    virtual ~IFilterDialog();
    // receive a pointer to the filter from ui thread
    virtual void register_filter(IFilter* filter) = 0;
    // received filter pointer is about to get deleted
    virtual void unregister_filter() = 0;
};

// call once with your chosen class names in the plugin
#define OPENTRACK_DECLARE_FILTER(filter_class, dialog_class, metadata_class) \
    OPENTRACK_DECLARE_PLUGIN_INTERNAL(filter_class, IFilter, metadata_class, dialog_class, IFilterDialog)

// implement this in protocols
struct OPENTRACK_API_EXPORT IProtocol
{
    IProtocol();

    IProtocol(const IProtocol&) = delete;
    IProtocol(IProtocol&&) = delete;
    IProtocol& operator=(const IProtocol&) = delete;

    // optional destructor
    virtual ~IProtocol();
    // return true if protocol was properly initialized
    virtual bool correct() = 0;
    // called 250 times a second with XYZ yaw pitch roll pose
    // try not to perform intense computation here. if you must, use a thread.
    virtual void pose(const double* headpose) = 0;
    // return game name or placeholder text
    virtual QString game_name() = 0;
};

struct OPENTRACK_API_EXPORT IProtocolDialog : public plugin_api::detail::BaseDialog
{
    // optional destructor
    virtual ~IProtocolDialog();
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
struct OPENTRACK_API_EXPORT ITracker
{
    ITracker(const ITracker&) = delete;
    ITracker(ITracker&&) = delete;
    ITracker& operator=(const ITracker&) = delete;
    ITracker();

    // optional destructor
    virtual ~ITracker();
    // start tracking, and grab a frame to display webcam video in, optionally
    virtual void start_tracker(QFrame* frame) = 0;
    // return XYZ yaw pitch roll data. don't block here, use a separate thread for computation.
    virtual void data(double *data) = 0;
    // tracker notified of centering
    // returning true makes identity the center pose
    virtual bool center() { return false; }
};

struct OPENTRACK_API_EXPORT ITrackerDialog : public plugin_api::detail::BaseDialog
{
    // optional destructor
    virtual ~ITrackerDialog();
    // receive a pointer to the tracker from ui thread
    virtual void register_tracker(ITracker *tracker) = 0;
    // received tracker pointer is about to get deleted
    virtual void unregister_tracker() = 0;

    ITrackerDialog();
};

// call once with your chosen class names in the plugin
#define OPENTRACK_DECLARE_TRACKER(tracker_class, dialog_class, metadata_class) \
    OPENTRACK_DECLARE_PLUGIN_INTERNAL(tracker_class, ITracker, metadata_class, dialog_class, ITrackerDialog)
