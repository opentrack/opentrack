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

#define BUILD_compat
#include "compat/export.hpp"

enum Axis {
    TX = 0, TY, TZ, Yaw, Pitch, Roll
};

typedef struct PortableLockedShm
{
    void* mem;
    int fd, size;
} PortableLockedShm;

typedef struct WineSHM
{
    double data[6];
    int gameid, gameid2;
    unsigned char table[8];
    bool stop;
} WineSHM;

static PortableLockedShm* lck_posix = NULL;
static WineSHM* shm_posix = NULL;
static WineSHM* data_last = NULL;
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

#ifdef __GNUC__
#   define unused(varname) varname __attribute__((__unused__))
#else
#   define unused(varname) varname
#endif

PortableLockedShm* PortableLockedShm_init(const char *shmName, const char *unused(mutexName), int mapSize)
{
    PortableLockedShm* self = malloc(sizeof(PortableLockedShm));
    char shm_filename[NAME_MAX];
    shm_filename[0] = '/';
    strncpy(shm_filename+1, shmName, NAME_MAX-2);
    shm_filename[NAME_MAX-1] = '\0';
    /* (void) shm_unlink(shm_filename); */

    self->fd = shm_open(shm_filename, O_RDWR | O_CREAT, 0600);
    (void) ftruncate(self->fd, mapSize);
    self->mem = mmap(NULL, mapSize, PROT_READ|PROT_WRITE, MAP_SHARED, self->fd, (off_t)0);
    return self;
}

void PortableLockedShm_free(PortableLockedShm* self)
{
    /*(void) shm_unlink(shm_filename);*/
    (void) munmap(self->mem, self->size);
    (void) close(self->fd);
    free(self);
}

void PortableLockedShm_lock(PortableLockedShm* self)
{
    flock(self->fd, LOCK_SH);
}

void PortableLockedShm_unlock(PortableLockedShm* self)
{
    flock(self->fd, LOCK_UN);
}

float write_head_position(
        float                unused(inElapsedSinceLastCall),
        float                unused(inElapsedTimeSinceLastFlightLoop),
        int                  unused(inCounter),
        void *               unused(inRefcon) )
{
    if (lck_posix != NULL && shm_posix != NULL) {
        if(data_last == NULL){
            data_last = calloc(1, sizeof(WineSHM));
        }

        //only set the view if tracking is running
        if(memcmp(shm_posix, data_last, sizeof(shm_posix->data)) != 0){
            PortableLockedShm_lock(lck_posix);
            if (!translation_disabled)
            {
                XPLMSetDataf(view_x, shm_posix->data[TX] * 1e-3 + offset_x);
                XPLMSetDataf(view_y, shm_posix->data[TY] * 1e-3 + offset_y);
                XPLMSetDataf(view_z, shm_posix->data[TZ] * 1e-3 + offset_z);
            }
            XPLMSetDataf(view_heading, shm_posix->data[Yaw] * 180 / M_PI);
            XPLMSetDataf(view_pitch, shm_posix->data[Pitch] * 180 / M_PI);
            XPLMSetDataf(view_roll, shm_posix->data[Roll] * 180 / M_PI);
        } else {
            //reset roll, otherwise it would be stuck at last angle
            XPLMSetDataf(view_roll, 0);
        }

        memcpy(&data_last, &shm_posix, sizeof(WineSHM));

        PortableLockedShm_unlock(lck_posix);
    }
    return -1.0;
}

static int TrackToggleHandler( XPLMCommandRef inCommand,
                               XPLMCommandPhase inPhase,
                               void * inRefCon )
{
    if ( track_disabled )
    {
        //Enable
        XPLMRegisterFlightLoopCallback(write_head_position, -1.0, NULL);

        // Reinit the offsets when we re-enable the plugin
        if ( !translation_disabled )
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

static int TranslationToggleHandler( XPLMCommandRef inCommand,
                                     XPLMCommandPhase inPhase,
                                     void * inRefCon )
{
    translation_disabled = !translation_disabled;
    if (!translation_disabled)
    {
        // Reinit the offsets when we re-enable the translations so that we can "move around"
        reinit_offset();
    }
    return 0;
}

PLUGIN_API OTR_COMPAT_EXPORT int XPluginStart ( char * outName, char * outSignature, char * outDescription ) {
    view_x = XPLMFindDataRef("sim/aircraft/view/acf_peX");
    view_y = XPLMFindDataRef("sim/aircraft/view/acf_peY");
    view_z = XPLMFindDataRef("sim/aircraft/view/acf_peZ");
    view_heading = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
    view_pitch = XPLMFindDataRef("sim/graphics/view/pilots_head_the");
    view_roll = XPLMFindDataRef("sim/graphics/view/field_of_view_roll_deg");

    track_toggle = XPLMCreateCommand("opentrack/toggle", "Disable/Enable head tracking");
    translation_disable_toggle = XPLMCreateCommand("opentrack/toggle_translation", "Disable/Enable input translation from opentrack");

    XPLMRegisterCommandHandler( track_toggle,
                                TrackToggleHandler,
                                1,
                                (void*)0);

    XPLMRegisterCommandHandler( translation_disable_toggle,
                                TranslationToggleHandler,
                                1,
                                (void*)0);


    if (view_x && view_y && view_z && view_heading && view_pitch && track_toggle && translation_disable_toggle) {
        lck_posix = PortableLockedShm_init(WINE_SHM_NAME, WINE_MTX_NAME, sizeof(WineSHM));
        if (lck_posix->mem == (void*)-1) {
            fprintf(stderr, "opentrack failed to init SHM!\n");
            return 0;
        }
        shm_posix = (WineSHM*) lck_posix->mem;
        memset(shm_posix, 0, sizeof(WineSHM));
        strcpy(outName, "opentrack");
        strcpy(outSignature, "opentrack - freetrack lives!");
        strcpy(outDescription, "head tracking view control");
        fprintf(stderr, "opentrack init complete\n");
        return 1;
    }
    return 0;
}

PLUGIN_API OTR_COMPAT_EXPORT void XPluginStop ( void ) {
    if (lck_posix)
    {
        PortableLockedShm_free(lck_posix);
        lck_posix = NULL;
        shm_posix = NULL;
    }
}

PLUGIN_API OTR_COMPAT_EXPORT void XPluginEnable ( void ) {
    XPLMRegisterFlightLoopCallback(write_head_position, -1.0, NULL);
    track_disabled = 0;
}

PLUGIN_API OTR_COMPAT_EXPORT void XPluginDisable ( void ) {
    XPLMUnregisterFlightLoopCallback(write_head_position, NULL);
    track_disabled = 1;
}

PLUGIN_API OTR_COMPAT_EXPORT void XPluginReceiveMessage(
        XPLMPluginID    unused(inFromWho),
        int             unused(inMessage),
        void *          unused(inParam))
{
    if (inMessage == XPLM_MSG_AIRPORT_LOADED)
        reinit_offset();
}
