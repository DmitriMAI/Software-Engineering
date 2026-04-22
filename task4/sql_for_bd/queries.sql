-- ── 1. Создание пользователя ─────────────────────────────────────
INSERT INTO users (login, first_name, last_name)
VALUES ('some', 'some', 'some')
RETURNING id, login, first_name, last_name;

-- ── 2. Найти по логину пользователя ────────────────────────────────────
SELECT id, login, first_name, last_name
FROM   users
WHERE  login = 'some';

-- ── 3. Найти по точному совпадения фио ──────────────────────
SELECT id, login, first_name, last_name
FROM   users
WHERE  first_name = 'some'
  AND  last_name  = 'some';

-- ── 4. Создать цель ─────────────────────────────────────
INSERT INTO goals (title, owner_id)
VALUES ('goals', 1)
RETURNING id, title, owner_id;

-- ── 5. Список целей ─────────────────────────────────
SELECT g.id,
       g.title,
       u.login         AS owner_login,
       u.first_name    AS owner_first_name,
       u.last_name     AS owner_last_name
FROM   goals g
JOIN   users u ON u.id = g.owner_id
ORDER  BY g.id DESC;

-- ── 6. Новая задача ───────────────────────
INSERT INTO tasks (goal_id, assignee_id, title, description, due_date)
VALUES (1, 1, 'coolTask', 'coolTask', '2026-08-28')
RETURNING id, goal_id, assignee_id, title, description, status, due_date, created_at, updated_at;

-- ── 7. Список задач ──────────────────────────────
SELECT t.id,
       t.title,
       t.description,
       t.status,
       t.due_date,
       t.created_at,
       t.updated_at,
       u.login         AS assignee_login,
       u.first_name    AS assignee_first_name,
       u.last_name     AS assignee_last_name
FROM   tasks t
LEFT   JOIN users u ON u.id = t.assignee_id
WHERE  t.goal_id = '1'
ORDER  BY t.created_at DESC, t.id ASC;

-- ── 8. Поменять статус задачи ─────────────────────────────────────
UPDATE tasks
SET    status     = 'DONE',
       updated_at = NOW()
WHERE  id = 1
RETURNING id, status, updated_at;