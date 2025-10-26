@echo off
REM ========================================
REM Restic GUI - 测试运行脚本 (Windows)
REM ========================================

echo ========================================
echo Restic GUI - Test Suite Builder
echo ========================================
echo.

REM 检查 qmake 是否在 PATH 中
where qmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [错误] 找不到 qmake。请确保 Qt 已安装并添加到 PATH 中。
    echo.
    echo 提示：您可能需要运行 Qt 的命令行工具，例如：
    echo   - Qt 5.14 for Desktop (MSVC 2017 64-bit)
    echo   - 或设置环境变量指向 qmake 路径
    echo.
    pause
    exit /b 1
)

echo [步骤 1/4] 清理旧的构建文件...
if exist build rmdir /s /q build
if exist Makefile del /q Makefile
if exist Makefile.Debug del /q Makefile.Debug
if exist Makefile.Release del /q Makefile.Release
echo 完成
echo.

echo [步骤 2/4] 运行 qmake 生成 Makefile...
qmake tests.pro
if %ERRORLEVEL% neq 0 (
    echo [错误] qmake 失败
    pause
    exit /b 1
)
echo 完成
echo.

echo [步骤 3/4] 编译测试项目...
echo 注意：这可能需要几分钟时间...
echo.

REM 检测使用的编译器
where nmake >nul 2>nul
if %ERRORLEVEL% equ 0 (
    echo 使用 MSVC 编译器 (nmake)...
    nmake
    set MAKE_RESULT=%ERRORLEVEL%
) else (
    where mingw32-make >nul 2>nul
    if %ERRORLEVEL% equ 0 (
        echo 使用 MinGW 编译器 (mingw32-make)...
        mingw32-make
        set MAKE_RESULT=%ERRORLEVEL%
    ) else (
        echo [错误] 找不到编译器 (nmake 或 mingw32-make)
        pause
        exit /b 1
    )
)

if %MAKE_RESULT% neq 0 (
    echo.
    echo [错误] 编译失败
    pause
    exit /b 1
)
echo.
echo 编译完成
echo.

echo [步骤 4/4] 运行测试...
echo ========================================
echo.

REM 运行测试可执行文件
if exist ..\bin\restic-gui-tests.exe (
    ..\bin\restic-gui-tests.exe %*
    set TEST_RESULT=%ERRORLEVEL%
) else if exist ..\bin\debug\restic-gui-tests.exe (
    ..\bin\debug\restic-gui-tests.exe %*
    set TEST_RESULT=%ERRORLEVEL%
) else if exist ..\bin\release\restic-gui-tests.exe (
    ..\bin\release\restic-gui-tests.exe %*
    set TEST_RESULT=%ERRORLEVEL%
) else (
    echo [错误] 找不到测试可执行文件
    echo 预期位置：
    echo   - ..\bin\restic-gui-tests.exe
    echo   - ..\bin\debug\restic-gui-tests.exe
    echo   - ..\bin\release\restic-gui-tests.exe
    pause
    exit /b 1
)

echo.
echo ========================================
if %TEST_RESULT% equ 0 (
    echo 测试结果: 全部通过 ✓
) else (
    echo 测试结果: 有测试失败 ✗
)
echo ========================================
echo.

pause
exit /b %TEST_RESULT%
