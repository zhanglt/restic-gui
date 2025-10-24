-- ========================================
-- Restic GUI 数据库初始化脚本
-- 版本：v1
-- ========================================

PRAGMA foreign_keys = ON;

-- 版本管理表
CREATE TABLE IF NOT EXISTS schema_version (
    version INTEGER PRIMARY KEY,
    applied_at TEXT NOT NULL
);

-- 仓库表
CREATE TABLE IF NOT EXISTS repositories (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    type TEXT NOT NULL,
    path TEXT NOT NULL,
    config TEXT,
    password_hash TEXT,
    created_at TEXT NOT NULL,
    last_backup TEXT,
    is_default INTEGER DEFAULT 0,
    CHECK (type IN ('local', 'sftp', 's3', 'rest', 'azure', 'gs', 'b2', 'rclone')),
    CHECK (is_default IN (0, 1))
);

CREATE INDEX IF NOT EXISTS idx_repositories_type ON repositories(type);
CREATE INDEX IF NOT EXISTS idx_repositories_is_default ON repositories(is_default);

-- 备份任务表
CREATE TABLE IF NOT EXISTS backup_tasks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    repository_id INTEGER NOT NULL,
    source_paths TEXT NOT NULL,
    exclude_patterns TEXT,
    tags TEXT,
    hostname TEXT,
    options TEXT,
    schedule_type INTEGER DEFAULT 0,
    schedule_config TEXT,
    enabled INTEGER DEFAULT 1,
    last_run TEXT,
    next_run TEXT,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL,
    FOREIGN KEY (repository_id) REFERENCES repositories(id) ON DELETE CASCADE,
    CHECK (schedule_type BETWEEN 0 AND 5),
    CHECK (enabled IN (0, 1))
);

CREATE INDEX IF NOT EXISTS idx_backup_tasks_repository ON backup_tasks(repository_id);
CREATE INDEX IF NOT EXISTS idx_backup_tasks_enabled ON backup_tasks(enabled);

-- 备份历史表
CREATE TABLE IF NOT EXISTS backup_history (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    task_id INTEGER NOT NULL,
    snapshot_id TEXT,
    start_time TEXT NOT NULL,
    end_time TEXT NOT NULL,
    success INTEGER NOT NULL,
    status INTEGER DEFAULT 1,
    files_new INTEGER DEFAULT 0,
    files_changed INTEGER DEFAULT 0,
    files_unmodified INTEGER DEFAULT 0,
    dirs_new INTEGER DEFAULT 0,
    dirs_changed INTEGER DEFAULT 0,
    dirs_unmodified INTEGER DEFAULT 0,
    data_added INTEGER DEFAULT 0,
    data_processed INTEGER DEFAULT 0,
    total_files INTEGER DEFAULT 0,
    total_bytes INTEGER DEFAULT 0,
    duration INTEGER DEFAULT 0,
    error_message TEXT,
    FOREIGN KEY (task_id) REFERENCES backup_tasks(id) ON DELETE CASCADE,
    CHECK (success IN (0, 1)),
    CHECK (status BETWEEN 0 AND 3)
);

CREATE INDEX IF NOT EXISTS idx_backup_history_task ON backup_history(task_id);
CREATE INDEX IF NOT EXISTS idx_backup_history_start_time ON backup_history(start_time);

-- 设置表
CREATE TABLE IF NOT EXISTS settings (
    key TEXT PRIMARY KEY,
    value TEXT,
    updated_at TEXT NOT NULL
);

-- 密码存储表
CREATE TABLE IF NOT EXISTS password_store (
    repository_id INTEGER PRIMARY KEY,
    encrypted_password TEXT NOT NULL,
    iv TEXT NOT NULL,
    salt TEXT NOT NULL,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL,
    FOREIGN KEY (repository_id) REFERENCES repositories(id) ON DELETE CASCADE
);

-- 快照缓存表
CREATE TABLE IF NOT EXISTS snapshots_cache (
    id TEXT PRIMARY KEY,
    repository_id INTEGER NOT NULL,
    snapshot_time TEXT NOT NULL,
    hostname TEXT,
    username TEXT,
    paths TEXT NOT NULL,
    tags TEXT,
    parent_id TEXT,
    size INTEGER,
    file_count INTEGER,
    dir_count INTEGER,
    cached_at TEXT NOT NULL,
    FOREIGN KEY (repository_id) REFERENCES repositories(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_snapshots_cache_repository ON snapshots_cache(repository_id);

-- 插入默认设置
INSERT OR IGNORE INTO settings (key, value, updated_at) VALUES
    ('app.version', '1.0.0', datetime('now')),
    ('app.theme', 'light', datetime('now')),
    ('app.language', 'zh_CN', datetime('now')),
    ('backup.threads', '4', datetime('now')),
    ('backup.retry_count', '3', datetime('now')),
    ('network.timeout', '30', datetime('now')),
    ('log.level', '1', datetime('now')),
    ('password.cache_duration', '60', datetime('now')),
    ('restic.path', 'restic', datetime('now'));

-- 记录schema版本
INSERT OR REPLACE INTO schema_version (version, applied_at) VALUES (1, datetime('now'));
