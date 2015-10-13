#include <client/windows/handler/exception_handler.h>

#include "BreakPad.h"

/////////////////////////////////////////////////////////////////////////////////////////
bool BreakPad_MinidumpCallback(const wchar_t* dump_path,
                               const wchar_t* minidump_id,
                               void* context,
                               EXCEPTION_POINTERS* exinfo,
                               MDRawAssertionInfo* assertion,
                               bool succeeded)
{
  return succeeded;
}

/////////////////////////////////////////////////////////////////////////////////////////
void installBreakPadHandler(const QString& name, const QString& destPath)
{
  new google_breakpad::ExceptionHandler(destPath.toStdWString().c_str(), NULL, BreakPad_MinidumpCallback, NULL,
                                        google_breakpad::ExceptionHandler::HANDLER_EXCEPTION |
                                        google_breakpad::ExceptionHandler::HANDLER_PURECALL);
}
