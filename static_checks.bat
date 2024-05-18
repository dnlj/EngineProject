@echo off

set "RED=[31m"
set "GREEN=[32m"
set "RESET=[0m"
set "IGNORE=%~0"
set "COLORS=--colors ""path:none"" --colors ""line:none"" --colors ""column:none"" --colors ""match:none"" --colors ""match:fg:red"" --colors ""path:fg:red"""

rem ################################################################################
rem # Rules
rem ################################################################################

rem Don't ever use checks like `#ifdef ENGINE_SERVER`. ENGINE_SERVER is always defined and is either true or false. Use `#if ENGINE_SERVER`.
call :check "ifdef.*SERVER"
call :check "ifdef.*CLIENT"

rem Bad brace formatting.
call :check -U "\)\s*[\r\n]+\s*\{"

rem Use left const.
call :check "^\s*auto const\s|\(\s*auto const\s"

rem ################################################################################


exit /B %ERRORLEVEL%

:check
	rg %* %COLORS% -g "!%IGNORE%" || echo %GREEN%PASS:%RESET% %*
	if %ERRORLEVEL% neq 1 goto :fail %1
exit /B 0

:fail
	echo %RED%FAIL:%RESET% %1
	exit /B 1
