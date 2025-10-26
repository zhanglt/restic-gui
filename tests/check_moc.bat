@echo off
echo ============================================
echo Checking MOC Files Status
echo ============================================
echo.

set MISSING=0

echo Checking required moc files...
echo.

if exist build\moc\moc_ResticWrapperTest.cpp (
    echo [OK] moc_ResticWrapperTest.cpp exists
) else (
    echo [MISSING] moc_ResticWrapperTest.cpp
    set MISSING=1
)

if exist build\moc\moc_BackupManagerTest.cpp (
    echo [OK] moc_BackupManagerTest.cpp exists
) else (
    echo [MISSING] moc_BackupManagerTest.cpp
    set MISSING=1
)

if exist build\moc\moc_DatabaseManagerTest.cpp (
    echo [OK] moc_DatabaseManagerTest.cpp exists
) else (
    echo [MISSING] moc_DatabaseManagerTest.cpp
    set MISSING=1
)

echo.
echo ============================================

if %MISSING%==1 (
    echo STATUS: Some moc files are missing!
    echo.
    echo SOLUTION: Run rebuild.bat to fix this issue.
    echo.
) else (
    echo STATUS: All required moc files exist!
    echo.
    echo You can now build the project.
    echo.
)

echo ============================================
pause
