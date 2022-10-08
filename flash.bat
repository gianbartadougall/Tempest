:: This batch script uses ST-LINK_CLI.exe to program STM32 microcontroller automatically.
:: The ST-LINK_CLI.exe file must be in your environment PATH for this to work
::
:: To configure this file, set the path of the .bin file to be uploaded to the microcontroller
@echo off

:: Spaces before and after the equal sign is included in the name so make sure there are no spaces
:: before or after equal sign when setting variables
set STLINK="ST-LINK_CLI.exe"
set LOG_FILE="log.txt"
set BIN_FILE_PATH="build/Tempest.bin"

:: Set colours used for printing
set RED="RED"
set GREEN="GREEN"

set ERR_MSG_BIN_FILE_NOT_FOUND="The specified binary file to flash onto the STM32 could not be found. Please update the BIN_FILE_PATH variable in 'flash.bat'."
set ERR_MSG_FLASH_FAILED="Flashing the STM32 failed. The verification code could not be verified"
set ERR_MSG_RESET_FAILED="STM32 could not be reset"

:: List of verification coes
set CMD_FLASH_VCODE="Verification...OK"
set CMD_RESET_VCODE="MCU Reset"

:: List of ST-LINK_CLI commands used
set CMD_RESET="-Rst"
set CMD_FLASH="-P"
set CMD_VERIFY="-V"

:: List of addresss
set FLASH_ADDRESS=0x08000000

:: Confirm the binary file exists
if not exist %BIN_FILE_PATH% (
    call :Print %ERR_MSG_BIN_FILE_NOT_FOUND% %RED%

    PAUSE
    EXIT /B
)

:: Flash the STM32 with binary file
call :STCommand %CMD_FLASH% %FLASH_ADDRESS% %CMD_VERIFY% %ERR_MSG_FLASH_FAILED% %CMD_FLASH_VCODE%

:: Restart the STM32 so it begins running the program
call :STCommand %CMD_RESET% %FLASH_ADDRESS% %CMD_VERIFY% %ERR_MSG_RESET_FAILED% %CMD_RESET_VCODE%

call :Print "Flashed STM32 succesfully" %GREEN% 

:: Exit with no errors
call :Exit

:: FUNCTIONS

:: **************************************************************************************************************
:: @brief: Prints a message to the console in a given colour
:Print

set argC=0
for %%x in (%*) do (
   set /A argC+=1
)

:: If a second argument is specified it should be the colour
if "%argC%" == "2" (

    if "%~2"=="RED" (
        echo [31m%~1[0m
    )

    if "%~2"=="GREEN" (
        echo [32m%~1[0m
    )

) else (
    echo %~1
)

:: This command makes the code return to where this function was called
goto :eof
:: **************************************************************************************************************



:: **************************************************************************************************************
:: @brief: Executes an ST-LINK command
:STCommand

:: Copying the arguments into new pararameters
set command=%~1
set address=%~2
set optionalCommand=%~3
set errorMsg=%~4
set verificationCode=%~5

:: Executing the ST-LINK_CLI command. Redirecting output to the log file
%STLINK% %command% %BIN_FILE_PATH% %address% %optionalCommand% > %LOG_FILE%

:: Confirm the verification code is present in the log file. If the verification
:: code can not be found then an error occured.
>nul find "%verificationCode%" %LOG_FILE% && (
    
    :: Command was sucessful. Deleting the log file
    del %LOG_FILE%

    :: Returning to function call
    goto :eof

) || (
    :: Print user defined error
    call :Print "%errorMsg%" "RED"

    :: Change colour of the CMD text to red
    echo [31m

    :: Echo contents of log file
    type %LOG_FILE%

    :: Change colour back to white
    echo [0m

    :: Exit
    call :Exit
)

:: **************************************************************************************************************

:Exit

EXIT /B