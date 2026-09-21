@echo off
REM ================================================================
REM test_os_emulator.bat - automated smoke test for os_emulator.exe
REM
REM Feeds every console command (valid inputs, edge cases, and error
REM paths) into the plain-text CSOPESY shell and prints the results,
REM so a full manual test pass is one double-click. Also serves as
REM living documentation of each command's expected output.
REM
REM Usage:
REM   test_os_emulator.bat
REM
REM Requires: os_emulator.exe in this folder (built automatically
REM from os_emulator.cpp with g++ if it is missing).
REM ================================================================
setlocal
set "EXE=os_emulator.exe"
set "SRC=os_emulator.cpp"

if not exist "%EXE%" (
  echo [%EXE%] not found - building from %SRC% ...
  g++ -std=c++17 "%SRC%" -o "%EXE%"
  if errorlevel 1 (
    echo BUILD FAILED - install g++ with C++17 support and retry.
    endlocal
    exit /b 1
  )
  echo Build OK.
  echo.
)

echo ================== os_emulator smoke test ==================
echo.

echo [TEST 1] help - list all commands
(echo help ^& echo exit) | "%EXE%"
echo.

echo [TEST 2] set_text - plain words, multi-word, punctuation, numbers
(echo set_text Hello ^& echo exit) | "%EXE%"
echo.
(echo set_text Operating Systems are fun! ^& echo exit) | "%EXE%"
echo.
REM NOTE: a literal double-quote char cannot go inside a piped (echo ...)
REM block - an unbalanced quote swallows the closing paren/pipe and the
REM rest of this script stops running. Type quote test input manually.
(echo set_text Hello, World! Test 123 @#$-_=+;:',.?/ ^& echo exit) | "%EXE%"
echo.
(echo set_text Fish ^& Chips ^& echo exit) | "%EXE%"
echo.

echo [TEST 3] set_text with no argument - expect error, then spaces only
(echo set_text ^& echo exit) | "%EXE%"
echo.
(echo set_text    ^& echo exit) | "%EXE%"
echo.

echo [TEST 4] set_speed valid - expect confirmation
(echo set_speed 150 ^& echo exit) | "%EXE%"
echo.
(echo set_speed 1 ^& echo exit) | "%EXE%"
echo.

echo [TEST 5] set_speed invalid - expect error each time
(echo set_speed ^& echo exit) | "%EXE%"
echo.
(echo set_speed 0 ^& echo exit) | "%EXE%"
echo.
(echo set_speed -5 ^& echo exit) | "%EXE%"
echo.
(echo set_speed abc ^& echo exit) | "%EXE%"
echo.
(echo set_speed 12abc ^& echo exit) | "%EXE%"
echo.
(echo set_speed 1.5 ^& echo exit) | "%EXE%"
echo.
(echo set_speed 9999999999999999999 ^& echo exit) | "%EXE%"
echo.

echo [TEST 6] start_marquee / stop_marquee stubs
(echo start_marquee ^& echo stop_marquee ^& echo exit) | "%EXE%"
echo.

echo [TEST 7] unknown command, case sensitivity, padded whitespace
(echo bad_command ^& echo exit) | "%EXE%"
echo.
(echo HELP ^& echo exit) | "%EXE%"
echo.
(echo.    help    ^& echo exit) | "%EXE%"
echo.
(echo Set_Text hello ^& echo exit) | "%EXE%"
echo.

echo [TEST 8] input closed without exit (EOF) - expect graceful terminate
(echo help) | "%EXE%"
echo.

echo [TEST 9] full session - help, text, speed, start, stop, bad cmd, exit
(
  echo help
  echo set_text Operating Systems are fun!
  echo set_speed 150
  echo start_marquee
  echo stop_marquee
  echo bad_command
  echo exit
) | "%EXE%"
echo.

echo ================== all tests finished ==================
endlocal
exit /b 0
