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

#include "QsLogDest.h"
#include "QsLogDestConsole.h"
#include "QsLogDestFile.h"
#include "QsLogDestFunctor.h"
#include <QString>

namespace QsLogging
{

Destination::~Destination()
{
}

//! destination factory
DestinationPtr DestinationFactory::MakeFileDestination(const QString& filePath,
    LogRotationOption rotation, const MaxSizeBytes &sizeInBytesToRotateAfter,
    const MaxOldLogCount &oldLogsToKeep)
{
    if (EnableLogRotation == rotation || EnableLogRotationOnOpen == rotation) {
        QScopedPointer<SizeRotationStrategy> logRotation(new SizeRotationStrategy);
        logRotation->setMaximumSizeInBytes(sizeInBytesToRotateAfter.size);
        logRotation->setBackupCount(oldLogsToKeep.count);

        FileDestination *dest = new FileDestination(filePath, RotationStrategyPtr(logRotation.take()));
        if (EnableLogRotationOnOpen == rotation && dest->rotationStrategy()->currentSizeInBytes() > 0)
          dest->rotate();
        return DestinationPtr(dest);
    }

    return DestinationPtr(new FileDestination(filePath, RotationStrategyPtr(new NullRotationStrategy)));
}

DestinationPtr DestinationFactory::MakeDebugOutputDestination()
{
    return DestinationPtr(new DebugOutputDestination);
}

DestinationPtr DestinationFactory::MakeFunctorDestination(QsLogging::Destination::LogFunction f)
{
    return DestinationPtr(new FunctorDestination(f));
}

DestinationPtr DestinationFactory::MakeFunctorDestination(QObject *receiver, const char *member)
{
    return DestinationPtr(new FunctorDestination(receiver, member));
}

} // end namespace
