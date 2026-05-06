Оптимизировать получилось поиск по имени и фамилии.
Другие запросы оптимизированы т.к схема определяет связи либо Unique создает индекс
ы
Для теста создаем данные чтобы использовался индекс
```sql
INSERT INTO users (login, first_name, last_name)
SELECT 
    'user_' || i,
    'First_' || i,
    'Last_' || i
FROM generate_series(1, 10000) i;

INSERT INTO goals (title, owner_id)
SELECT 
    'Goal ' || i,
    (SELECT id FROM users ORDER BY random() LIMIT 1)
FROM generate_series(1, 50000) i;

-- Обновить статистику
ANALYZE goals;
ANALYZE users;

DROP TABLE IF EXISTS tasks;

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

SELECT * FROM tasks;

DO $$
DECLARE
    user_ids  INT[];
    goal_ids  INT[];
    u_len     INT;
    g_len     INT;
BEGIN
    SELECT ARRAY(SELECT id FROM users ORDER BY id) INTO user_ids;
    SELECT ARRAY(SELECT id FROM goals ORDER BY id) INTO goal_ids;
    u_len := array_length(user_ids, 1);
    g_len := array_length(goal_ids, 1);

    INSERT INTO tasks (goal_id, assignee_id, title, description, status, due_date, created_at, updated_at)
    SELECT
        goal_ids[(random() * (g_len - 1))::int + 1],
        CASE WHEN random() < 0.1
             THEN NULL
             ELSE user_ids[(random() * (u_len - 1))::int + 1]
        END,
        'Task ' || i,
        CASE WHEN random() < 0.3
             THEN NULL
             ELSE 'Description for task ' || i
        END,
        (ARRAY['TODO','IN_PROGRESS','DONE','CANCELLED'])[floor(random() * 4 + 1)::int],
        CASE WHEN random() < 0.2
             THEN NULL
             ELSE CURRENT_DATE + (round(random() * 180 - 30))::int
        END,
        '2026-08-28',
        '2026-08-28'
    FROM generate_series(1, 200000) i;
END;
$$;

ANALYZE tasks;

```

>Отлично, попробуем использовать индексы

```sql
-- Попробуем ускорить для точного совпадения

EXPLAIN (ANALYZE, BUFFERS, VERBOSE)
SELECT id, login, first_name, last_name
FROM   users
WHERE  first_name = 'John'
  AND  last_name  = 'Doe';

-- Seq Scan on public.users  (cost=0.00..234.54 rows=1 width=36) (actual time=0.717..0.718 rows=0 loops=1)
--   Output: id, login, first_name, last_name
--   Filter: ((users.first_name = 'John'::text) AND (users.last_name = 'Doe'::text))
--   Rows Removed by Filter: 10036
--   Buffers: shared hit=84
-- Planning:
--   Buffers: shared hit=72
-- Planning Time: 0.817 ms
-- Execution Time: 0.729 ms

CREATE INDEX idx_users_full_name ON users(first_name, last_name);

-- Index Scan using idx_users_full_name on public.users  (cost=0.29..8.30 rows=1 width=36) (actual time=0.043..0.043 rows=0 loops=1)
--   Output: id, login, first_name, last_name
--   Index Cond: ((users.first_name = 'John'::text) AND (users.last_name = 'Doe'::text))
--   Buffers: shared hit=2
-- Planning:
--   Buffers: shared hit=91
-- Planning Time: 0.989 ms
-- Execution Time: 0.058 ms

-- Как видим запросы стали идти быстрее
```


```sql
--- Рассмотрим другие запросы задания

--- Соединение таблиц
EXPLAIN (ANALYZE, BUFFERS, VERBOSE)
SELECT g.id,
       g.title,
       u.login         AS owner_login,
       u.first_name    AS owner_first_name,
       u.last_name     AS owner_last_name
FROM   goals g
JOIN   users u ON u.id = g.owner_id
ORDER  BY g.id DESC;

-- Nested Loop  (cost=0.58..2885.79 rows=50071 width=42) (actual time=0.046..16.517 rows=50071 loops=1)
--   Output: g.id, g.title, u.login, u.first_name, u.last_name
--   Inner Unique: true
--   Buffers: shared hit=631
--   ->  Index Scan Backward using goals_pkey on public.goals g  (cost=0.29..1630.36 rows=50071 width=18) (actual time=0.016..4.408 rows=50071 loops=1)
--         Output: g.id, g.owner_id, g.title
--         Buffers: shared hit=595
--   ->  Memoize  (cost=0.30..0.32 rows=1 width=36) (actual time=0.000..0.000 rows=1 loops=50071)
--         Output: u.login, u.first_name, u.last_name, u.id
--         Cache Key: g.owner_id
--         Cache Mode: logical
--         Hits: 50060  Misses: 11  Evictions: 0  Overflows: 0  Memory Usage: 2kB
--         Buffers: shared hit=36
--         ->  Index Scan using users_pkey on public.users u  (cost=0.29..0.31 rows=1 width=36) (actual time=0.003..0.003 rows=1 loops=11)
--               Output: u.login, u.first_name, u.last_name, u.id
--               Index Cond: (u.id = g.owner_id)
--               Buffers: shared hit=36
-- Planning:
--   Buffers: shared hit=357
-- Planning Time: 1.769 ms
-- Execution Time: 17.997 ms

EXPLAIN (ANALYZE, BUFFERS, VERBOSE)
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

-- Gather Merge  (cost=5080.53..5080.88 rows=3 width=98) (actual time=8.734..10.248 rows=3 loops=1)
--   Output: t.id, t.title, t.description, t.status, t.due_date, t.created_at, t.updated_at, u.login, u.first_name, u.last_name
--   Workers Planned: 1
--   Workers Launched: 1
--   Buffers: shared hit=2612
--   ->  Sort  (cost=4080.52..4080.53 rows=3 width=98) (actual time=6.126..6.127 rows=2 loops=2)
--         Output: t.id, t.title, t.description, t.status, t.due_date, t.created_at, t.updated_at, u.login, u.first_name, u.last_name
--         Sort Key: t.created_at DESC, t.id
--         Sort Method: quicksort  Memory: 25kB
--         Buffers: shared hit=2612
--         Worker 0:  actual time=3.750..3.751 rows=1 loops=1
--           Sort Method: quicksort  Memory: 25kB
--           Buffers: shared hit=778
--         ->  Nested Loop Left Join  (cost=0.29..4080.50 rows=3 width=98) (actual time=1.062..6.106 rows=2 loops=2)
--               Output: t.id, t.title, t.description, t.status, t.due_date, t.created_at, t.updated_at, u.login, u.first_name, u.last_name
--               Inner Unique: true
--               Buffers: shared hit=2598
--               Worker 0:  actual time=1.982..3.717 rows=1 loops=1
--                 Buffers: shared hit=764
--               ->  Parallel Seq Scan on public.tasks t  (cost=0.00..4055.59 rows=3 width=74) (actual time=1.033..6.073 rows=2 loops=2)
--                     Output: t.id, t.goal_id, t.assignee_id, t.title, t.description, t.status, t.due_date, t.created_at, t.updated_at
--                     Filter: (t.goal_id = 1)
--                     Rows Removed by Filter: 99998
--                     Buffers: shared hit=2585
--                     Worker 0:  actual time=1.946..3.680 rows=1 loops=1
--                       Buffers: shared hit=757
--               ->  Index Scan using users_pkey on public.users u  (cost=0.29..8.30 rows=1 width=36) (actual time=0.018..0.018 rows=1 loops=3)
--                     Output: u.id, u.first_name, u.last_name, u.login
--                     Index Cond: (u.id = t.assignee_id)
--                     Buffers: shared hit=13
--                     Worker 0:  actual time=0.033..0.033 rows=1 loops=1
--                       Buffers: shared hit=7
-- Planning:
--   Buffers: shared hit=296
-- Planning Time: 1.973 ms
-- Execution Time: 10.286 ms

```
Как видно из запросов - используется primary индекс. 


Индексы для таблиц

>Таблица users
```sql
-- 1. Индекс на первичный ключ - создается автоматом
CREATE UNIQUE INDEX idx_users_id ON users(id);

-- 2. Индекс на колонку login - создается и так т.к unique
CREATE UNIQUE INDEX users_login_key ON users(login);

-- 3. Индекс для поиска по имени и фамилии отдельно
CREATE INDEX idx_users_first_name ON users(first_name);
CREATE INDEX idx_users_last_name ON users(last_name);

-- 4. Составной индекс для поиска по полному имени
CREATE INDEX idx_users_full_name ON users(first_name, last_name);
```

>Таблица goals
```sql
-- 1. Индекс на первичный ключ - создается автоматом
CREATE UNIQUE INDEX idx_goals_id ON goals(id);

-- 2. Индекс на внешний ключ owner_id  
CREATE INDEX idx_goals_owner_id ON goals(owner_id);
-- Поможет для SELECT * FROM goals WHERE owner_id = 5
-- Или SELECT g.*, u.first_name FROM goals g JOIN users u ON g.owner_id = u.id

-- 3. Индекс для поиска целей по названию
CREATE INDEX idx_goals_title ON goals(title);

-- 4. Индекс составной
CREATE INDEX idx_goals_owner_title ON goals(owner_id, title);
```