@echo off

rem
rem Usage - Sign.cmd <target file>
rem

setlocal

echo --- BEGIN: sign.bat -------------------------------------

set TargetFile=%~1
if not exist "%TargetFile%" (
  echo FATAL ERROR - %TargetFile% does not exist and cannot be signed
  echo --- END: sign.bat -------------------------------------
  exit /b 1
)
for %%i in ("%TargetFile%") do set TargetFileExtension="%%~xi"

if "%SigningCertSha256%" == "" (
  set SigningCertSha256=%~dp0PlexOfficialSPC_sha256.pfx
)
if not exist "%SigningCertSha256%" (
  set SigningCertSha256=%~dp0PlexTestSPC.pfx
)
echo Signing with %SigningCertSha256%
set TimestampErrors=0

rem Create timestamp server lists... All servers on this list support both RFC 3161 and non-RFC variants
set ServerListRfc3161=(http://timestamp.digicert.com,http://timestamp.globalsign.com/scripts/timestamp.dll,http://timestamp.comodoca.com)

rem Signing files SHA256 with the SHA256 cert
echo Adding SHA256 signature to %TargetFile%...
call :SignFile "%SigningCertSha256%" "%SigningCertPasswordSha256%" sha256 0 "%TargetFile%" "%ServerListRfc3161%" sha256
if errorlevel 1 goto SignFailed

echo Verifying signature...
signtool.exe verify /pa "%TargetFile%"
if errorlevel 1 (
  echo FATAL ERROR - could not verify signature for %TargetFile%. There were %TimestampErrors% timestamping errors.
  echo --- END: sign.bat ------------------------------------------------------------
  exit /b 1
) else (
  echo --- END: sign.bat ------------------------------------------------------------
  exit /b 0
)

:SignFailed
  REM return an error code...
  echo FAILED: FATAL ERROR - signing %TargetFile% failed. There were %TimestampErrors% timestamping errors.
  echo --- END: sign.bat -------------------------------------------------------------
  exit /b 1

rem When timestamping a file, signtool will fail when the timestamp server doesn't respond. So we retry in a loop, in an attempt to reduce spurious failures.
rem When running signtool, we redirect output to null because signtool.exe may inadvertently output the word "error", causing msbuild to fail the build.
:SignFile

  setlocal
  set CertFilePath=%1
  set CertPassword=%~2
  set SignatureHashAlgorithm=%3
  set AppendSignature=%4
  set TargetFilePath=%5
  set TimestampServerList=%~6
  set Rfc3161HashAlgorithm=%7

  rem Compute password args
  if "%CertPassword%" neq "" (
    set PasswordArgs=/p %CertPassword%
  ) else (
    set PasswordArgs=
  )

  rem Compute append args
  if "%AppendSignature%" == "1" (
    set AppendSignatureArgs=/as
  ) else (
    set AppendSignatureArgs=
  )

  rem Compute timestamp server args
  set TimestampArg1=/tr
  set TimestampArg2=/td %Rfc3161HashAlgorithm%

  for /L %%a in (1,1,300) do (
    for %%s in %TimestampServerList% do (

      rem echo signtool.exe sign %AppendSignatureArgs% /fd %SignatureHashAlgorithm% %TimestampArg1% %%s %TimestampArg2% /f %CertFilePath% %PasswordArgs% %TargetFilePath%
      signtool.exe sign %AppendSignatureArgs% /fd %SignatureHashAlgorithm% %TimestampArg1% %%s %TimestampArg2% /f %CertFilePath% %PasswordArgs% %TargetFilePath% > NUL 2>&1
      if errorlevel 0 if not errorlevel 1 goto SignFileSuccess

      echo Signing attempt %%a failed. Probably cannot find the timestamp server at %%s
      set /a TimestampErrors+=1
    )

    echo Waiting 1 second...
    choice /N /T:1 /D:Y >NUL
  )

  endlocal
  exit /b 1

:SignFileSuccess
  echo Signing succeeded
  endlocal
  exit /b 0
