#include <client/mac/handler/exception_handler.h>

#include "BreakPad.h"

/////////////////////////////////////////////////////////////////////////////////////////
static inline bool BreakPad_MinidumpCallback(const char *dump_dir, const char *minidump_id, void *context, bool succeeded)
{
  fprintf(stderr, "****** Plex Media Player CRASHED, CRASH REPORT WRITTEN: %s\n", dump_dir);
  return succeeded;
}

/////////////////////////////////////////////////////////////////////////////////////////
void installBreakPadHandler(const QString& name, const QString& destPath)
{
  Q_UNUSED(name);
  new google_breakpad::ExceptionHandler(destPath.toStdString(), NULL, BreakPad_MinidumpCallback, NULL, true, NULL);
}
