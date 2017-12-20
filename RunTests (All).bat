@echo off
call "RunTests (Debug).bat" --nopause
call "RunTests (Release).bat" --nopause

if NOT "%~1" == "--nopause" (
	if NOT "%~1" == "-np" (
		pause
	)
)