const dbName = "poco_template";
db = db.getSiblingDB(dbName);

// СХЕМА ВАЛИДАЦИИ

const taskSchema = {
  $jsonSchema: {
    bsonType: "object",
    title: "Task",
    required: ["goal_id", "assignee_id", "status", "text"],
    properties: {
      _id: {
        bsonType: "objectId"
      },
      goal_id: {
        bsonType: "string",
        minLength: 1,
        maxLength: 128
      },
      assignee_id: {
        bsonType: "string",
        minLength: 1,
        maxLength: 128
      },
      status: {
        bsonType: "string",
        enum: ["pending", "in_progress", "done", "cancelled"]
      },
      text: {
        bsonType: "string",
        minLength: 1,
        maxLength: 2000
      }
    }
  }
};

// Создать коллекцию с валидацией
db.createCollection("tasks", {
  validator: taskSchema,
  validationLevel: "strict",
  validationAction: "error"
});
print("[mongo-init] Коллекция tasks создана со схемой валидации");

// ИНДЕКСЫ

db.tasks.createIndex({ assignee_id: 1 });
db.tasks.createIndex({ goal_id: 1 });
db.tasks.createIndex({ status: 1 });
db.tasks.createIndex({ assignee_id: 1, status: 1 });
print("[mongo-init] Индексы созданы");

// ТЕСТОВЫЕ ДАННЫЕ
db.tasks.insertMany([
  {
    goal_id:     "goal_001",
    assignee_id: "user_001",
    status:      "pending",
    text:        "Написать техническое задание для нового модуля"
  },
  {
    goal_id:     "goal_001",
    assignee_id: "user_002",
    status:      "in_progress",
    text:        "Реализовать REST API для управления задачами"
  },
  {
    goal_id:     "goal_002",
    assignee_id: "user_001",
    status:      "done",
    text:        "Настроить CI/CD pipeline"
  },
  {
    goal_id:     "goal_002",
    assignee_id: "user_003",
    status:      "pending",
    text:        "Написать unit-тесты для сервиса пользователей"
  },
  {
    goal_id:     "goal_003",
    assignee_id: "user_002",
    status:      "cancelled",
    text:        "Провести рефакторинг модуля авторизации"
  },
  {
    goal_id:     "goal_003",
    assignee_id: "user_003",
    status:      "in_progress",
    text:        "Добавить логирование запросов в формате JSON"
  },
  {
    goal_id:     "goal_004",
    assignee_id: "user_001",
    status:      "pending",
    text:        "Обновить документацию API"
  },
  {
    goal_id:     "goal_004",
    assignee_id: "user_004",
    status:      "done",
    text:        "Настроить мониторинг метрик через Prometheus"
  },
  {
    goal_id:     "goal_005",
    assignee_id: "user_004",
    status:      "done",
    text:        "Выполнить задания"
  },
  {
    goal_id:     "goal_006",
    assignee_id: "user_004",
    status:      "done",
    text:        "Получить хорошую оценку за предмет"
  }
]);