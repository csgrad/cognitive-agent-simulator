:: Batch file to create experiments
:: for creature simulation
@echo off

:: horizontal movement
setlocal ENABLEDELAYEDEXPANSION
for %%H in (0 1) do (
	:: random decel
	for %%R in (0 1) do (
		:: kb preservation
		for %%K in (0 1) do (			
			set DIR=kbpreserve%%Krandomdecel%%RhorizontalCreature%%H
			echo !DIR!
			@rmdir !DIR! /S /Q > nul 2>&1
			@mkdir !DIR!
			cd !DIR!

			call:generateLanes %%H %%R %%K
			
			cd ..
		)
	)
)
endlocal
goto:eof

::----------------------------------------------------------------------
:: Generates all the lanes required for the simulation
:: -param1: creature Horizontal Movement (true/false)
:: -param2: random deceleration (true/false)
:: -param3: kb preserve (true/false)
::----------------------------------------------------------------------
:generateLanes
setlocal ENABLEDELAYEDEXPANSION
for %%L in (1 2 4) do (
::  echo %%L lanes
for %%B in (0 1) do (
    if %%B EQU 1 (
      set /a S=%%L/2
	  set /a E=%%L-1
	  if !S! NEQ 0 (
	    set DIR=%%Llanes!S!backward
        echo !DIR!
        @rmdir !DIR! /S /Q > nul 2>&1
		@mkdir !DIR!
		cd !DIR!
		call:generateBackLanes !S! !E! %~1 %~2 %~3
		cd ..
      )
    ) else (
      set DIR=%%Llanes
      echo !DIR!
      @rmdir !DIR! /S /Q > nul 2>&1
	  @mkdir !DIR!
	  cd !DIR!
	  call:allFearDesire %~1 %~2 %~3 %%L
	  cd ..
    )
  )
)
endlocal
goto:eof

::----------------------------------------------------------------------
:: Generates the backlanes for the current lane
:: - param1 backStart 
:: - param2 backEnd
:: -param3: creature Horizontal Movement (true/false)
:: -param4: random deceleration (true/false)
:: -param5: kb preserve (true/false)
::----------------------------------------------------------------------
:generateBackLanes
setlocal ENABLEDELAYEDEXPANSION

::total lanes
set /a T=%~2+1

set B=LANE_BACKWARDS 
:loopBack
set B=!B!%~1 
if %~1 LSS %~2 (
  set /a N=%~1+1
  call:loopBack !N! %~2 %~3 %~4 %~5
) else ( 
  call:allFearDesire %~3 %~4 %~5 !T! "!B!"
)

endlocal
goto:eof


::----------------------------------------------------------------------
::Loops through all possible fear and desire and sets up experiment
:: -param1: creature Horizontal Movement (true/false)
:: -param2: random deceleration (true/false)
:: -param3: kb preserve (true/false)
:: -param4: number lanes
:: -param5: backwards lanes (list of backward lanes starting at 0) or nothing if not set
::----------------------------------------------------------------------
:allFearDesire
echo p1 %~1 p2 %~2 p3 %~3 p4 %~4 p5 %~5
setlocal ENABLEDELAYEDEXPANSION
call:createFearDesire 0.0 0.0 %~1 %~2 %~3 %~4 "%~5"
:: loop through fear from 0 to 1
for %%F in (0 25 50 75 100) do (
    set /a TEMP=%%F/100
    if !TEMP! EQU 1 (
      set FEAR=1.0
    ) else (
      set FEAR=0.%%F
    )
	set /a R=100-%%F
	set /a TEMP=!R!/100
	if !TEMP! EQU 1 (
      set DESIRE=1.0
    ) else (
      set DESIRE=0.!R!
    )
    call:createFearDesire !FEAR! !DESIRE! %~1 %~2 %~3 %~4 "%~5"
)
endlocal

goto:eof

::----------------------------------------------------------------------
::Creates a directories for a particular fear and desire
:: - param1: fear
:: - param2: desire
:: - param3: creature Horizontal Movement (true/false)
:: - param4: random deceleration (true/false)
:: - param5: kb preserve (true/false)
:: - param6: number lanes
:: - param7: backwards lanes (list of backward lanes starting at 0) or nothing if not set
::----------------------------------------------------------------------
:createFearDesire
echo p1 %~1 p2 %~2 p3 %~3 p4 %~4 p5 %~5 p6 %~6 p7 %~7
set DIR=fear%~1desire%~2
echo %DIR%
@rmdir %DIR% /S /Q > nul 2>&1
@mkdir %DIR%
cd %DIR%
call:createCPs %~1 %~2 %~3 %~4 %~5 %~6 "%~7"
cd ..
goto:eof

::----------------------------------------------------------------------
::Creates a directories for all crosspoints
:: - param1 fear
:: - param2 desire
:: - param3: creature Horizontal Movement (true/false)
:: - param4: random deceleration (true/false)
:: - param5: kb preserve (true/false)
:: - param6: number lanes
:: - param7: backwards lanes (list of backward lanes starting at 0) or nothing if not set
::----------------------------------------------------------------------
:createCPs
 echo p1 %~1 p2 %~2 p3 %~3 p4 %~4 p5 %~5 p6 %~6 p7 %~7

:: loop through cross points
for %%C in (15 45 90) do (  
  @rmdir cp%%C /S /Q > nul 2>&1
  @mkdir cp%%C
  call:generateConfig %%C %~1 %~2 %~3 %~4 %~5 %~6 "%~7"
  @copy ..\..\..\creatureSimulation.exe . > nul 2>&1
  @creatureSimulation.exe cp%%C.txt > nul 2>&1
  @move i*.txt cp%%C > nul 2>&1
  @copy ..\..\..\bestfiterrorgraph.m cp%%C > nul 2>&1
  @copy ..\..\..\errorbargraph.m cp%%C > nul 2>&1
  @copy ..\..\..\stdshade.m cp%%C > nul 2>&1
  @copy ..\..\..\tp.m cp%%C > nul 2>&1
  cd cp%%C
  call:graphResults
  cd ..
)
goto:eof


::----------------------------------------------------------------------
::Generates a config file for a particular simulation run
:: - param1: crosspoint
:: - param2: fear
:: - param3: desire
:: - param4: creature Horizontal Movement (true/false)
:: - param5: random deceleration (true/false)
:: - param6: kb preserve (true/false)
:: - param7: number lanes
:: - param8: backwards lanes (list of backward lanes starting at 0) or nothing if not set
::----------------------------------------------------------------------
:generateConfig

::  echo p1 %~1 p2 %~2 p3 %~3 p4 %~4 p5 %~5 p6 %~6 p7 %~7 p8 %~8

  @del cp%~1.txt /Q > nul 2>&1
  echo CROSS_POINT %~1 >> cp%~1.txt
  echo FIXED_FEAR %~2			// 0 disabled >> cp%~1.txt
  echo FIXED_DESIRE %~3 >> cp%~1.txt
  echo CELL_NUM 101 >> cp%~1.txt
  echo LANE_NUM %~7 >> cp%~1.txt
  echo PROPENSITY_RIGHT 0.5	// likelyhood that a car will return towards the right lane rather than staying in passing lane >> cp%~1.txt
  echo MAX_TIME 1511 >> cp%~1.txt
  echo MAX_CREATURES 0			// limit the number of generated creatures (for testing) 0 = disabled >> cp%~1.txt
  echo MAX_CARS 0				// limit the number of generated cars (for testing) 0 = disabled >> cp%~1.txt
  echo MAX_SPEED 11 >> cp%~1.txt
  echo ENTRY_CAR_PROB 0.1 >> cp%~1.txt
  echo ENTRY_CAR_PROB_MAX 1.1 >> cp%~1.txt
  echo ENTRY_CAR_PROB_INC	0.1 >> cp%~1.txt
  echo ENTRY_CREATURE_PROB 1 >> cp%~1.txt
  echo INTELLIGENCE 1			// 0 = naiive, 1 = fd >> cp%~1.txt
  echo DISTANCE 3 5 7			// we can now add up to 10 values (but should be from low to high) >> cp%~1.txt
  echo VELOCITY 3 5 7			// we can now add up to 10 values (but should be from low to high) >> cp%~1.txt
  echo REPEATS 5 >> cp%~1.txt
  echo KB_OUTPUT TRUE			//output knowledge base at each timestep in trace >> cp%~1.txt
  
if %~6 EQU 1 (
  echo KB_EXPERIMENT TRUE		//preserve knowledge to next experiment >> cp%~1.txt
) else (
  echo KB_EXPERIMENT FALSE		//preserve knowledge to next experiment >> cp%~1.txt
)

if %~5 EQU 1 (
  echo RANDOM_DECEL TRUE		// nagel-shrek random deceleration >> cp%~1.txt
) else (
  echo RANDOM_DECEL FALSE		// nagel-shrek random deceleration >> cp%~1.txt
)

if %~4 EQU 1 (
  echo HORIZONTAL_HOP 1		//max number of cells left or right in one timestep >> cp%~1.txt
  echo HORIZONTAL_MAX 5		//max number of cells left or right a creature can move from originalcrosspoint >> cp%~1.txt
) else (
  echo HORIZONTAL_HOP 0		//max number of cells left or right in one timestep >> cp%~1.txt
  echo HORIZONTAL_MAX 0		//max number of cells left or right a creature can move from originalcrosspoint >> cp%~1.txt
)

echo %~8	//lane #s of backwards lanes >> cp%~1.txt
goto:eof

:graphResults
for %%x in (*stats.txt) do (
   @more +1 %%x > %%x.tmp
   @move %%x.tmp %%x > nul 2>&1
)


@del *configdependent.txt
@del *kb.txt
@del *trace.txt
@del *car0.2*
@del *car0.4*
@del *car0.6*
@del *car0.8*
@del *car1cre*
@del *.pdf
@del *.png
@del *.jpg
::RScript boxplot.R
matlab -r tp -nowindow
matlab -r bestfiterrorgraph -nowindow
matlab -r errorbargraph -nowindow
goto:eof
