:: This script creates a new .c and .h file and fills both files out
:: with generic skeleton code.

@echo OFF

set PATH=Core/
set h=
set c=

:: Confirm an argument was given
setlocal enabledelayedexpansion

set argCount=0
for %%x in (%*) do (
   set /A argCount+=1
   set "argVec[!argCount!]=%%~x"
)

if %argCount% == 1 (
	set c=%PATH%/Src/%1.c
	set h=%PATH%/Inc/%1.h
	set fileName=%1
) else (
	echo [31mMissing file name[0m
	exit
)

:: Convert filename to upper case
set fileNameUpper=!fileName!

for %%A in (A B C D E F G H I J K L M N O P Q R S T U V W X Y Z) do (
    set "fileNameUpper=!fileNameUpper:%%A=%%A!"
)

:: Extract the date from the computer
for /f "tokens=2 delims==" %%a in ('wmic OS Get localdatetime /value') do set "dt=%%a"
set "YY=%dt:~2,2%" & set "YYYY=%dt:~0,4%" & set "MM=%dt:~4,2%" & set "DD=%dt:~6,2%"
set "HH=%dt:~8,2%" & set "Min=%dt:~10,2%" & set "Sec=%dt:~12,2%"

set "datestamp=%YYYY%-%MM%-%DD%"
@REM echo datestamp: "%datestamp%"

:: Create new .h file	
call :printHFile

:: Create new .c file
call :printCFile

:: Exit
EXIT

:bs
	echo.>>%1%
	goto :eof

:text
	:: Send text to given file
	echo %~2>>%~1
	goto :eof

:printHeader
	:: Print header file
	call :text %~1 "/**"
	call :text %~1 " * @file %~2.%~3"
	call :text %~1 " * @author Gian Barta-Dougall"
	call :text %~1 " * @brief System file for !fileName!"
	call :text %~1 " * @version 0.1"
	call :text %~1 " * @date !datestamp!"
	call :text %~1 " * "
	call :text %~1 " * @copyright Copyright (c) !YYYY!"
	call :text %~1 " * "
	call :text %~1 " */"

	goto :eof

:printHFile

	:: Print header
	call :printHeader %h% !fileName! h

	:: Print header guards
	call :text %h% "#ifndef !fileNameUpper!_H"
	call :text %h% "#define !fileNameUpper!_H"
	call :bs %h%

	:: Print includes. By default include stm32l4xx include
	call :text %h% "/* Public Includes */"
	call :bs %h%
	call :text %h% "/* Public STM Includes */"
	call :text %h% "#include "stm32l4xx.h""
	call :bs %h%
	call :text %h% "/* Public #defines */"
	call :bs %h%
	call :text %h% "/* Public Structures and Enumerations */"
	call :bs %h%
	call :text %h% "/* Public Variable Declarations */"
	call :bs %h%
	call :text %h% "/* Public Function Prototypes */"
	call :bs %h%
	call :text %h% "/**"
	call :text %h% " * @brief Initialise the system library."
	call :text %h% " */"
	call :text %h% "void !fileName!_init(void);"
	call :bs %h%
	call :text %h% "#endif // !fileNameUpper!_H"

	goto :eof

:printCFile

	:: Print header
	call :printHeader %c% !fileName! c

	:: Print includes 
	call :text %c% "/* Public Includes */"
	call :bs %c%
	call :text %c% "/* Private Includes */"
	call :text %c% "#include "!filename!.h""
	call :bs %c%
	call :text %c% "/* Private STM Includes */"
	call :bs %c%
	call :text %c% "/* Private #defines */"
	call :bs %c%
	call :text %c% "/* Private Structures and Enumerations */"
	call :bs %c%
	call :text %c% "/* Private Variable Declarations */"
	call :bs %c%
	call :text %c% "/* Private Function Prototypes */"
	call :text %c% "void !filename!_hardware_init(void);"
	call :bs %c%
	call :text %c% "/* Public Functions */"
	call :bs %c%
	call :text %c% "void !fileName!_init(void) {"
	call :bs %c%
	call :text %c% "	// Initialise hardware "
	call :text %c% "	!fileName!_hardware_init();"
	call :bs %c%
	call :text %c% "}"
	call :bs %c%
	call :text %c% "/* Private Functions */"
	call :bs %c%
	call :text %c% "/**"
	call :text %c% " * @brief Initialise the hardware for the library."
	call :text %c% " */"
	call :text %c% "void !fileName!_hardware_init(void) {"
	call :bs %c%
	call :text %c% "}"
	goto :eof