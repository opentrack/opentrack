/* Copyright (c) 2013, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <limits.h>
#include <unistd.h>
#include <math.h>

#include <XPLMPlugin.h>
#include <XPLMDataAccess.h>
#include <XPLMProcessing.h>
#include <XPLMUtilities.h>

#ifndef PLUGIN_API
#define PLUGIN_API
#endif

#pragma GCC diagnostic ignored "-Wunused-parameter"

/* using Wine name to ease things */
#define WINE_SHM_NAME "facetracknoir-wine-shm"
#define WINE_MTX_NAME "facetracknoir-wine-mtx"

#include "compat/linkage-macros.hpp"

#ifndef MAP_FAILED
#   define MAP_FAILED ((void*)-1)
#endif

#ifdef __GNUC__
#   pragma GCC diagnostic ignored "-Wimplicit-float-conversion"
#   pragma GCC diagnostic ignored "-Wdouble-promotion"
#   pragma GCC diagnostic ignored "-Wlanguage-extension-token"
#endif

enum Axis {
    TX = 0, TY, TZ, Yaw, Pitch, Roll
};

typedef struct shm_wrapper
{
    void* mem;
    int fd, size;
} shm_wrapper;

typedef struct WineSHM
{
    double data[6];
    int gameid, gameid2;
    unsigned char table[8];
    bool stop;
} volatile WineSHM;

static shm_wrapper* lck_posix = NULL;
static WineSHM* shm_posix = NULL;
static void *view_x, *view_y, *view_z, *view_heading, *view_pitch, *view_roll;
static float offset_x, offset_y, offset_z;
static XPLMCommandRef track_toggle = NULL, translation_disable_toggle = NULL;
static int track_disabled = 1;
static int translation_disabled;

static void reinit_offset() {
    offset_x = XPLMGetDataf(view_x);
    offset_y = XPLMGetDataf(view_y);
    offset_z = XPLMGetDataf(view_z);
}

shm_wrapper* shm_wrapper_init(const char *shm_name, const char *mutex_name, int mapSize)
{
    (void)mutex_name;
    shm_wrapper* self = malloc(sizeof(shm_wrapper));
    char shm_filename[NAME_MAX];
    shm_filename[0] = '/';
    strncpy(shm_filename+1, shm_name, NAME_MAX-2);
    shm_filename[NAME_MAX-1] = '\0';
    /* (void) shm_unlink(shm_filename); */

    self->fd = shm_open(shm_filename, O_RDWR | O_CREAT, 0600);
    (void) ftruncate(self->fd, mapSize);
    self->mem = mmap(NULL, mapSize, PROT_READ|PROT_WRITE, MAP_SHARED, self->fd, (off_t)0);
    return self;
}

void shm_wrapper_free(shm_wrapper* self)
{
    /*(void) shm_unlink(shm_filename);*/
    (void) munmap(self->mem, self->size);
    (void) close(self->fd);
    free(self);
}

void shm_wrapper_lock(shm_wrapper* self)
{
    flock(self->fd, LOCK_SH);
}

void shm_wrapper_unlock(shm_wrapper* self)
{
    flock(self->fd, LOCK_UN);
}

float write_head_position(float inElapsedSinceLastCall,
                          float inElapsedTimeSinceLastFlightLoop,
                          int   inCounter,
                          void* inRefcon)
{
    if (lck_posix != NULL && shm_posix != NULL) {
        shm_wrapper_lock(lck_posix);
        if (!translation_disabled)
        {
            XPLMSetDataf(view_x, shm_posix->data[TX] * 1e-3 + offset_x);
            XPLMSetDataf(view_y, shm_posix->data[TY] * 1e-3 + offset_y);
            XPLMSetDataf(view_z, shm_posix->data[TZ] * 1e-3 + offset_z);
        }
        XPLMSetDataf(view_heading, shm_posix->data[Yaw] * 180 / M_PI);
        XPLMSetDataf(view_pitch, shm_posix->data[Pitch] * 180 / M_PI);
        XPLMSetDataf(view_roll, shm_posix->data[Roll] * 180 / M_PI);
        shm_wrapper_unlock(lck_posix);
    }
    return -1.0;
}

static int TrackToggleHandler(XPLMCommandRef inCommand,
                              XPLMCommandPhase inPhase,
                              void* inRefCon)
{
    if (inPhase != xplm_CommandBegin) return 0;

    if (track_disabled)
    {
        //Enable
        XPLMRegisterFlightLoopCallback(write_head_position, -1, NULL);

        // Reinit the offsets when we re-enable the plugin
        if (!translation_disabled)
            reinit_offset();
    }
    else
    {
        //Disable
        XPLMUnregisterFlightLoopCallback(write_head_position, NULL);
    }
    track_disabled = !track_disabled;
    return 0;
}

static int TranslationToggleHandler(XPLMCommandRef inCommand,
                                    XPLMCommandPhase inPhase,
                                    void* inRefCon)
{
    if (inPhase != xplm_CommandBegin) return 0;
    
    translation_disabled = !translation_disabled;
    if (!translation_disabled)
    {
        // Reinit the offsets when we re-enable the translations so that we can "move around"
        reinit_offset();
    }
    return 0;
}

static inline
void volatile_explicit_bzero(void volatile* restrict ptr, size_t len)
{
    for (size_t i = 0; i < len; i++)
        *((char volatile* restrict)ptr + i) = 0;

    asm volatile("" ::: "memory");
}

PLUGIN_API OTR_GENERIC_EXPORT
int XPluginStart (char* outName, char* outSignature, char* outDescription) {
    // view_x = XPLMFindDataRef("sim/aircraft/view/acf_peX");
    // view_y = XPLMFindDataRef("sim/aircraft/view/acf_peY");
    // view_z = XPLMFindDataRef("sim/aircraft/view/acf_peZ");
    // view_heading = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
    // view_pitch = XPLMFindDataRef("sim/graphics/view/pilots_head_the");
    // view_roll = XPLMFindDataRef("sim/graphics/view/field_of_view_roll_deg");

    view_x = XPLMFindDataRef("sim/graphics/view/pilots_head_x");
    view_y = XPLMFindDataRef("sim/graphics/view/pilots_head_y");
    view_z = XPLMFindDataRef("sim/graphics/view/pilots_head_z");
    view_heading = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
    view_pitch = XPLMFindDataRef("sim/graphics/view/pilots_head_the");
    view_roll = XPLMFindDataRef("sim/graphics/view/pilots_head_phi");

    track_toggle = XPLMCreateCommand("opentrack/toggle", "Disable/Enable head tracking");
    translation_disable_toggle = XPLMCreateCommand("opentrack/toggle_translation", "Disable/Enable input translation from opentrack");

    XPLMRegisterCommandHandler(track_toggle,
                               TrackToggleHandler,
                               1,
                               NULL);

    XPLMRegisterCommandHandler(translation_disable_toggle,
                               TranslationToggleHandler,
                               1,
                               NULL);


    if (view_x && view_y && view_z && view_heading && view_pitch && track_toggle && translation_disable_toggle) {
        lck_posix = shm_wrapper_init(WINE_SHM_NAME, WINE_MTX_NAME, sizeof(WineSHM));
        if (lck_posix->mem == MAP_FAILED) {
            fprintf(stderr, "opentrack failed to init SHM!\n");
            return 0;
        }
        shm_posix = lck_posix->mem;
        volatile_explicit_bzero(shm_posix, sizeof(WineSHM));
        strcpy(outName, "opentrack");
        strcpy(outSignature, "opentrack - freetrack lives!");
        strcpy(outDescription, "head tracking view control");
        fprintf(stderr, "opentrack init complete\n");
        return 1;
    }
    return 0;
}

PLUGIN_API OTR_GENERIC_EXPORT
void XPluginStop (void) {
    if (lck_posix)
    {
        shm_wrapper_free(lck_posix);
        lck_posix = NULL;
        shm_posix = NULL;
    }
}

PLUGIN_API OTR_GENERIC_EXPORT
int XPluginEnable (void) {
    XPLMRegisterFlightLoopCallback(write_head_position, -1.0, NULL);
    track_disabled = 0;
    return 1;
}

PLUGIN_API OTR_GENERIC_EXPORT
void XPluginDisable (void) {
    XPLMUnregisterFlightLoopCallback(write_head_position, NULL);
    track_disabled = 1;
}

PLUGIN_API OTR_GENERIC_EXPORT
void XPluginReceiveMessage(XPLMPluginID    inFromWho,
                           int             inMessage,
                           void *          inParam)
{
    if (inMessage == XPLM_MSG_AIRPORT_LOADED)
        reinit_offset();
}
