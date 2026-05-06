print("--- CREATE ---");

// Вставка одного документа
const newTask = db.tasks.insertOne({
  goal_id:     "goal_query_test",
  assignee_id: "user_query_test",
  status:      "pending",
  text:        "Тестовая задача для демонстрации запросов"
});
const testId = newTask.insertedId;
print("Создана задача с _id: " + testId);

// Вставка нескольких документов
db.tasks.insertMany([
  { goal_id: "goal_query_test", assignee_id: "user_001", status: "in_progress", text: "Задача 2" },
  { goal_id: "goal_query_test", assignee_id: "user_002", status: "done",        text: "Задача 3" },
  { goal_id: "goal_query_test", assignee_id: "user_001", status: "cancelled",   text: "Задача 4" }
]);
print("Дополнительные тестовые задачи вставлены\n");

print("--- READ ---");

// Найти все задачи
print("Все задачи (первые 3):");
db.tasks.find().limit(3).forEach(doc => printjson(doc));

// Найти задачу по _id
print("\nПоиск по _id:");
printjson(db.tasks.findOne({ _id: testId }));

// Найти все задачи с определённым статусом
print("\nЗадачи со статусом 'pending':");
db.tasks.find({ status: "pending" }).forEach(doc => printjson(doc));

// Найти задачи конкретного исполнителя
print("\nЗадачи исполнителя 'user_001':");
db.tasks.find({ assignee_id: "user_001" }).forEach(doc => printjson(doc));

// Найти задачи по цели
print("\nЗадачи цели 'goal_query_test':");
db.tasks.find({ goal_id: "goal_query_test" }).forEach(doc => printjson(doc));

// Комбинированный фильтр: исполнитель + статус
print("\nЗадачи user_001 со статусом in_progress:");
db.tasks.find({ assignee_id: "user_001", status: "in_progress" }).forEach(doc => printjson(doc));

// Проекция — только нужные поля
print("\nТолько поля _id, status, text:");
db.tasks.find({ goal_id: "goal_query_test" }, { status: 1, text: 1 }).forEach(doc => printjson(doc));

// Поиск по нескольким статусам ($in)
print("\nЗадачи со статусом pending или done:");
db.tasks.find({ status: { $in: ["pending", "done"] } }).limit(5).forEach(doc => printjson(doc));

// Подсчёт документов
print("\nКоличество задач по статусам:");
const statuses = ["pending", "in_progress", "done", "cancelled"];
statuses.forEach(s => {
  print("  " + s + ": " + db.tasks.countDocuments({ status: s }));
});

// UPDATE

print("\n--- UPDATE ---");

// Обновить статус одной задачи по _id
const updateResult = db.tasks.updateOne(
  { _id: testId },
  { $set: { status: "in_progress" } }
);
print("Обновлено документов: " + updateResult.modifiedCount);
print("Статус после обновления:");
printjson(db.tasks.findOne({ _id: testId }));

// Массовое обновление: перевести все pending задачи цели в in_progress
const bulkUpdate = db.tasks.updateMany(
  { goal_id: "goal_query_test", status: "pending" },
  { $set: { status: "in_progress" } }
);
print("\nМассовое обновление pending -> in_progress: " + bulkUpdate.modifiedCount + " документов");

// Upsert: создать если не существует
const upsertResult = db.tasks.updateOne(
  { goal_id: "goal_upsert", assignee_id: "user_upsert" },
  { $set: { status: "pending", text: "Создано через upsert" } },
  { upsert: true }
);
print("\nUpsert: upsertedId = " + upsertResult.upsertedId);

// DELETE

print("\n--- DELETE ---");

// Удалить одну задачу по _id
const deleteOne = db.tasks.deleteOne({ _id: testId });
print("Удалено документов (deleteOne): " + deleteOne.deletedCount);

// Удалить все тестовые задачи
const deleteMany = db.tasks.deleteMany({
  goal_id: { $in: ["goal_query_test", "goal_upsert"] }
});
print("Удалено тестовых документов (deleteMany): " + deleteMany.deletedCount);

// AGGREGATION

print("\n--- AGGREGATION ---");

// Количество задач по статусам
print("Группировка по статусам:");
db.tasks.aggregate([
  { $group: { _id: "$status", count: { $sum: 1 } } },
  { $sort: { count: -1 } }
]).forEach(doc => printjson(doc));

// Количество задач на каждого исполнителя
print("\nЗадач на исполнителя:");
db.tasks.aggregate([
  { $group: { _id: "$assignee_id", total: { $sum: 1 } } },
  { $sort: { total: -1 } },
  { $limit: 5 }
]).forEach(doc => printjson(doc));

// Задачи по цели с подсчётом по статусам
print("\nЗадачи по goal_id с разбивкой по статусам:");
db.tasks.aggregate([
  {
    $group: {
      _id: { goal: "$goal_id", status: "$status" },
      count: { $sum: 1 }
    }
  },
  { $sort: { "_id.goal": 1, "_id.status": 1 } }
]).forEach(doc => printjson(doc));

// INDEXES

print("\n--- INDEXES ---");

db.tasks.createIndex({ assignee_id: 1 });
db.tasks.createIndex({ goal_id: 1 });
db.tasks.createIndex({ status: 1 });
db.tasks.createIndex({ assignee_id: 1, status: 1 });

print("Индексы созданы:");
db.tasks.getIndexes().forEach(idx => printjson(idx));

// Проверка использования индекса (explain)
print("\nexplain для поиска по assignee_id:");
printjson(db.tasks.find({ assignee_id: "user_001" }).explain("queryPlanner").queryPlanner.winningPlan);

print("\n=== Запросы выполнены ===");
