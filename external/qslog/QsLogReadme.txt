QsLog - the simple Qt logger
-------------------------------------------------------------------------------
QsLog is an easy to use logger that is based on Qt's QDebug class.

Features
-------------------------------------------------------------------------------
    * Six logging levels (from trace to fatal)
    * Logging level threshold configurable at runtime.
    * Minimum overhead when logging is turned off.
    * Multiple destinations, comes with file and debug destinations.
    * Thread-safe
    * Logging of common Qt types out of the box.
    * Immediate logging or queueing messages in a separate thread.
    * Small dependency: just drop it in your project directly.

Usage
-------------------------------------------------------------------------------
By directly including QsLog in your project:
    1. Include QsLog.pri in your pro file
    2. Include QsLog.h in your C++ files. Include QsLogDest.h only where you create/add destinations.
    3. Get the instance of the logger by calling QsLogging::Logger::instance();
    4. Optionally set the logging level. Info is default.
    5. Create as many destinations as you want by using the QsLogging::DestinationFactory.
    6. Add the destinations to the logger instance by calling addDestination.
    7. Start logging!
    Note: when you want to use QsLog both from an executable and a shared library you have to
          link dynamically with QsLog due to a limitation with static variables.

By linking to QsLog dynamically:
    1. Build QsLog using the QsLogSharedLibrary.pro.
    2. Add the QsLog shared library to your LIBS project dependencies.
    3. Follow the steps in "directly including QsLog in your project" starting with step 2.

Configuration
-------------------------------------------------------------------------------
QsLog has several configurable parameters:
    * defining QS_LOG_LINE_NUMBERS in the .pri file enables writing the file and line number
      automatically for each logging call
    * defining QS_LOG_SEPARATE_THREAD will route all log messages to a separate thread.

Sometimes it's necessary to turn off logging. This can be done in several ways:
    * globally, at compile time, by enabling the QS_LOG_DISABLE macro in the .pri file.
    * globally, at run time, by setting the log level to "OffLevel".
    * per file, at compile time, by including QsLogDisableForThisFile.h in the target file.

Thread safety
-------------------------------------------------------------------------------
The Qt docs say: A thread-safe function can be called simultaneously from multiple threads,
even when the invocations use shared data, because all references to the shared data are serialized.
A reentrant function can also be called simultaneously from multiple threads, but only if each
invocation uses its own data.

Since sending the log message to the destinations is protected by a mutex, the logging macros are
thread-safe provided that the log has been initialized - i.e: instance() has been called.
The instance function and the setup functions (e.g: setLoggingLevel, addDestination) are NOT
thread-safe and are NOT reentrant.

IMPORTANT: when using a separate thread for logging, your program might crash at exit time on some
           operating systems if you won't call Logger::destroyInstance() before your program exits.
           This function can be called either before returning from main in a console app or
           inside QCoreApplication::aboutToQuit in a Qt GUI app.
           The reason is that the logging thread is still running as some objects are destroyed by
           the OS. Calling destroyInstance will wait for the thread to finish.
           Nothing will happen if you forget to call the function when not using a separate thread
           for logging.
