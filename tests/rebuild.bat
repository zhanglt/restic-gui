@echo off
echo ============================================
echo Rebuilding Restic GUI Tests
echo ============================================
echo.

echo [1/4] Cleaning old build files...
if exist build\moc\moc_ResticWrapperTest.cpp del build\moc\moc_ResticWrapperTest.cpp
if exist build\moc\moc_BackupManagerTest.cpp del build\moc\moc_BackupManagerTest.cpp
if exist build\moc\moc_DatabaseManagerTest.cpp del build\moc\moc_DatabaseManagerTest.cpp
if exist build\obj\*.obj del /Q build\obj\*.obj
echo Done.
echo.

echo [2/4] Running qmake to regenerate makefiles...
qmake tests.pro
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: qmake failed!
    pause
    exit /b 1
)
echo Done.
echo.

echo [3/4] Running nmake clean...
nmake clean
echo Done.
echo.

echo [4/4] Building tests...
nmake
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    pause
    exit /b 1
)
echo.

echo ============================================
echo Build completed successfully!
echo ============================================
echo.
echo You can now run the tests with:
echo   ..\bin\restic-gui-tests.exe
echo.
pause
