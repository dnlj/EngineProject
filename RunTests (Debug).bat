@echo off

call "./bin/Debug_Windows_x64/DungeonGameTest"

if NOT "%~1" == "--nopause" (
	if NOT "%~1" == "-np" (
		pause
	)
)