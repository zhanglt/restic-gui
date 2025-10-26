#!/bin/bash
# ========================================
# Restic GUI - 测试运行脚本 (Linux/macOS)
# ========================================

set -e  # 遇到错误立即退出

echo "========================================"
echo "Restic GUI - Test Suite Builder"
echo "========================================"
echo ""

# 检查 qmake 是否可用
if ! command -v qmake &> /dev/null; then
    echo "[错误] 找不到 qmake。请确保 Qt 已安装并添加到 PATH 中。"
    echo ""
    echo "提示："
    echo "  - Ubuntu/Debian: sudo apt install qt5-default qttools5-dev"
    echo "  - macOS: brew install qt@5"
    echo "  - 或手动设置 PATH: export PATH=/path/to/qt/bin:\$PATH"
    echo ""
    exit 1
fi

echo "[步骤 1/4] 清理旧的构建文件..."
rm -rf build
rm -f Makefile
echo "完成"
echo ""

echo "[步骤 2/4] 运行 qmake 生成 Makefile..."
qmake tests.pro
echo "完成"
echo ""

echo "[步骤 3/4] 编译测试项目..."
echo "注意：这可能需要几分钟时间..."
echo ""

# 使用多核编译加速
if command -v nproc &> /dev/null; then
    JOBS=$(nproc)
else
    JOBS=4
fi

make -j$JOBS

echo ""
echo "编译完成"
echo ""

echo "[步骤 4/4] 运行测试..."
echo "========================================"
echo ""

# 运行测试可执行文件
if [ -f "../bin/restic-gui-tests" ]; then
    ../bin/restic-gui-tests "$@"
    TEST_RESULT=$?
elif [ -f "../bin/debug/restic-gui-tests" ]; then
    ../bin/debug/restic-gui-tests "$@"
    TEST_RESULT=$?
elif [ -f "../bin/release/restic-gui-tests" ]; then
    ../bin/release/restic-gui-tests "$@"
    TEST_RESULT=$?
else
    echo "[错误] 找不到测试可执行文件"
    echo "预期位置："
    echo "  - ../bin/restic-gui-tests"
    echo "  - ../bin/debug/restic-gui-tests"
    echo "  - ../bin/release/restic-gui-tests"
    exit 1
fi

echo ""
echo "========================================"
if [ $TEST_RESULT -eq 0 ]; then
    echo "测试结果: 全部通过 ✓"
else
    echo "测试结果: 有测试失败 ✗"
fi
echo "========================================"
echo ""

exit $TEST_RESULT
