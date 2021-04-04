//
// Created by Tobias Hieta on 26/08/15.
//

#ifndef KONVERGO_CRASHDUMPS_H
#define KONVERGO_CRASHDUMPS_H

#include "BreakPad.h"
#include "Paths.h"
#include "Version.h"

static void setupCrashDumper()
{
  QDir dir(Paths::cacheDir("crashdumps/incoming/" + Version::GetCanonicalVersionString()));
  dir.mkpath(dir.absolutePath());

#ifdef NDEBUG
  installBreakPadHandler("Jellyfin Media Player", dir.path());
#else
  QLOG_INFO() << "This is a debug build. Not installing crash handler.";
#endif
}

#endif //KONVERGO_CRASHDUMPS_H
