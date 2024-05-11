@echo off

set "RED=[31m"
set "GREEN=[32m"
set "RESET=[0m"
set "IGNORE=%~0"


rem ################################################################################
rem # Rules
rem ################################################################################

rem Don't ever use checks like `#ifdef ENGINE_SERVER`. ENGINE_SERVER is always defined and is either true or false. Use `#if ENGINE_SERVER`.
call :check "ifdef.*SERVER"
call :check "ifdef.*CLIENT"

rem Use left const.
call :check "auto const"

rem ################################################################################


exit /B %ERRORLEVEL%

:check
	rg %1 -g "!%IGNORE%" >nul || echo %GREEN%PASS:%RESET% %1
	if %ERRORLEVEL% neq 1 goto :fail %1
exit /B 0

:fail
	echo %RED%FAIL:%RESET% %1
	exit /B 1
