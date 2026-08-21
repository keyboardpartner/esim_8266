echo off
echo Assemble and convert Picoblaze Source (*.psm) with KCPSM6

if exist %~n1.psm (
	kcpsm6 %~n1.psm
	if errorlevel 1 (
  	echo #########################################
	  echo Assembly errors found!
  	echo #########################################
	  pause
	) else (
		del %~n1.bin
		hex2dat6 %~n1.hex 2
	  echo Picoblaze file "%~n1.dat" ready for upload.
	  if exist %~n1.fmt (
			copy %~n1.fmt %~n1.psm
			del *.fmt
	  )
		del *.log
		del *.hex
		del KCPSM6_session_log.txt
	)	
) else (
  echo #########################################
  echo ERROR: file "%~n1.psm" not found!
  echo #########################################
	pause
  exit 1
)

