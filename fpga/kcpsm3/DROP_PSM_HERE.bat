echo off
echo Assemble and convert Picoblaze Source (*.psm)

if EXIST %~n1.psm (
	pico -i%~n1.psm -o%~n1.hex
	if errorlevel 1 (
  	echo #########################################
	  echo Assembly errors found!
  	echo #########################################
	  pause
	) else (
		del %~n1.bin
		mem2dat %~n1.hex
		del %~n1.hex
	  echo Picoblaze file "%~n1.bin" ready for upload.
	)	
) else (
  echo #########################################
  echo ERROR: file "%~n1.psm" not found!
  echo #########################################
	pause
  exit 1
)
