
-- Таблица пользователей
CREATE TABLE users (
    id          SERIAL PRIMARY KEY,
    login       VARCHAR(64)  NOT NULL UNIQUE,
    first_name  VARCHAR(128) NOT NULL,
    last_name   VARCHAR(128) NOT NULL
);

-- Таблица целей
CREATE TABLE goals (
    id          SERIAL PRIMARY KEY,
    title       VARCHAR(256) NOT NULL,
    owner_id    INTEGER      NOT NULL REFERENCES users(id) ON DELETE CASCADE
);

-- Таблица задач (будет на mongodb дальше преобразую)
-- на данный момент из условий ЛР 2 - не нужна таблица
CREATE TABLE tasks (
    id          SERIAL PRIMARY KEY,
    goal_id     INTEGER      NOT NULL REFERENCES goals(id) ON DELETE CASCADE,
    assignee_id INTEGER      REFERENCES users(id) ON DELETE SET NULL,
    title       VARCHAR(256) NOT NULL,
    description TEXT,
    status      VARCHAR(32)  NOT NULL DEFAULT 'TODO'
                    CHECK (status IN ('TODO', 'IN_PROGRESS', 'DONE', 'CANCELLED')),
    due_date    DATE,
    created_at  TIMESTAMP    NOT NULL DEFAULT NOW(),
    updated_at  TIMESTAMP    NOT NULL DEFAULT NOW(),
    CONSTRAINT chk_task_title_nonempty CHECK (title <> '')
);