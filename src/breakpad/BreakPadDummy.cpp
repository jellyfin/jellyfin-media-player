#include "QsLog.h"
#include "BreakPad.h"

void installBreakPadHandler(const QString& name, const QString& destPath)
{
  QLOG_WARN() << "No crash reporting compiled.";
}
