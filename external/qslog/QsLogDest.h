// Copyright (c) 2013, Razvan Petru
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or other
//   materials provided with the distribution.
// * The name of the contributors may not be used to endorse or promote products
//   derived from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef QSLOGDEST_H
#define QSLOGDEST_H

#include "QsLogLevel.h"
#include <QSharedPointer>
#include <QtGlobal>
class QString;
class QObject;

#ifdef QSLOG_IS_SHARED_LIBRARY
#define QSLOG_SHARED_OBJECT Q_DECL_EXPORT
#elif QSLOG_IS_SHARED_LIBRARY_IMPORT
#define QSLOG_SHARED_OBJECT Q_DECL_IMPORT
#else
#define QSLOG_SHARED_OBJECT
#endif

namespace QsLogging
{

class QSLOG_SHARED_OBJECT Destination
{
public:
    typedef void (*LogFunction)(const QString &message, Level level);
    virtual void rotate() = 0;

public:
    virtual ~Destination();
    virtual void write(const QString& message, Level level) = 0;
    virtual bool isValid() = 0; // returns whether the destination was created correctly
};
typedef QSharedPointer<Destination> DestinationPtr;


// a series of "named" paramaters, to make the file destination creation more readable
enum LogRotationOption
{
    DisableLogRotation = 0,
    EnableLogRotation  = 1,
    EnableLogRotationOnOpen = 2,
};

struct QSLOG_SHARED_OBJECT MaxSizeBytes
{
    MaxSizeBytes() : size(0) {}
    explicit MaxSizeBytes(qint64 size_) : size(size_) {}
    qint64 size;
};

struct QSLOG_SHARED_OBJECT MaxOldLogCount
{
    MaxOldLogCount() : count(0) {}
    explicit MaxOldLogCount(int count_) : count(count_) {}
    int count;
};


//! Creates logging destinations/sinks. The caller shares ownership of the destinations with the logger.
//! After being added to a logger, the caller can discard the pointers.
class QSLOG_SHARED_OBJECT DestinationFactory
{
public:
    static DestinationPtr MakeFileDestination(const QString& filePath,
        LogRotationOption rotation = DisableLogRotation,
        const MaxSizeBytes &sizeInBytesToRotateAfter = MaxSizeBytes(),
        const MaxOldLogCount &oldLogsToKeep = MaxOldLogCount());
    static DestinationPtr MakeDebugOutputDestination();
    // takes a pointer to a function
    static DestinationPtr MakeFunctorDestination(Destination::LogFunction f);
    // takes a QObject + signal/slot
    static DestinationPtr MakeFunctorDestination(QObject *receiver, const char *member);
};

} // end namespace

#endif // QSLOGDEST_H
