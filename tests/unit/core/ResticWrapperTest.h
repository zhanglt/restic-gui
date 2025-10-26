#ifndef RESTICWRAPPERTEST_H
#define RESTICWRAPPERTEST_H

#include "common/TestBase.h"
#include "core/ResticWrapper.h"
#include "models/Repository.h"

namespace ResticGUI {
namespace Test {

/**
 * @brief ResticWrapper Test Class
 *
 * Tests for restic CLI wrapper including:
 * - restic path validation
 * - Repository operations
 * - JSON parsing
 * - Progress signals
 * - Error handling
 * - Password security
 * - Operation cancellation
 *
 * Note: Most tests require actual restic binary
 * Tests will skip if restic is not available
 */
class ResticWrapperTest : public TestBase
{
    Q_OBJECT

private slots:
    void initTestCase() override;
    void cleanupTestCase() override;

    // Path validation tests
    void testResticPathValidation();
    void testGetVersion();
    void testResticNotFound();

    // Repository operation tests (require restic)
    void testInitRepository();
    void testRepositoryPasswordHandling();

    // JSON parsing tests
    void testJsonOutputParsing();
    void testInvalidJsonHandling();

    // Signal tests
    void testProgressSignals();

    // Error handling tests
    void testErrorHandling();
    void testCancelOperation();

    // Security tests
    void testPasswordNotInCommandLine();

private:
    Core::ResticWrapper* m_wrapper;
    QString m_testRepoPath;
    QString m_testPassword;
    bool m_resticAvailable;

    // Helper methods
    bool isResticAvailable();
    Models::Repository createTestRepository();
};

} // namespace Test
} // namespace ResticGUI

#endif // RESTICWRAPPERTEST_H