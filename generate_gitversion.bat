@echo off
setlocal enabledelayedexpansion

REM Check if the directory argument is provided
if "%~1"=="" (
    echo Error: No directory specified for gitparams.h.
    exit /b 1
)

REM Store the directory argument
set OUTPUT_DIR=%~1

REM Check if the specified directory exists
if not exist "!OUTPUT_DIR!" (
    echo Error: The specified directory "!OUTPUT_DIR!" does not exist.
    exit /b 1
)

REM Change to the specified directory
pushd "!OUTPUT_DIR!" || (
    echo Error: Unable to change to directory "!OUTPUT_DIR!".
    exit /b 1
)

set filename=__gitversion_autogen.hpp

REM Create %filename% with an auto-generated file comment
echo // Auto-generated file. Do not edit manually. > %filename%
echo // This file contains the current Git commit and branch information. >> %filename%

REM Get the short Git commit hash
for /f "delims=" %%i in ('git rev-parse --verify HEAD') do set GIT_CUR_COMMIT=%%i
set GIT_CUR_COMMIT=!GIT_CUR_COMMIT:~0,8!

REM Get the current Git branch name
for /f "delims=" %%i in ('git rev-parse --abbrev-ref HEAD') do set GIT_BRANCH=%%i

REM Write the macros to %filename% with CRLF line endings
echo #define GIT_CUR_COMMIT "!GIT_CUR_COMMIT!" >> %filename%
echo #define GIT_BRANCH "!GIT_BRANCH!" >> %filename%

echo Generated %filename% with GIT_CUR_COMMIT=!GIT_CUR_COMMIT! and GIT_BRANCH=!GIT_BRANCH!

endlocal