#!/bin/bash
echo "============================================"
echo "Rebuilding Restic GUI Tests"
echo "============================================"
echo

echo "[1/4] Cleaning old build files..."
rm -f build/moc/moc_ResticWrapperTest.cpp
rm -f build/moc/moc_BackupManagerTest.cpp
rm -f build/moc/moc_DatabaseManagerTest.cpp
rm -f build/obj/*.o
echo "Done."
echo

echo "[2/4] Running qmake to regenerate makefiles..."
qmake tests.pro
if [ $? -ne 0 ]; then
    echo "ERROR: qmake failed!"
    exit 1
fi
echo "Done."
echo

echo "[3/4] Running make clean..."
make clean
echo "Done."
echo

echo "[4/4] Building tests..."
make
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed!"
    exit 1
fi
echo

echo "============================================"
echo "Build completed successfully!"
echo "============================================"
echo
echo "You can now run the tests with:"
echo "  ../bin/restic-gui-tests"
echo
