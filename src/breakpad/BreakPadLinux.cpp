#include <client/linux/handler/exception_handler.h>

#include "BreakPad.h"

void installBreakPadHandler(const QString& name, const QString& destPath)
{
  google_breakpad::MinidumpDescriptor desc(destPath.toStdString());
  new google_breakpad::ExceptionHandler(desc, 0, 0, 0, true, -1);
}
