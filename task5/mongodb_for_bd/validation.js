const taskSchema = {
  $jsonSchema: {
    bsonType: "object",
    title: "Task",
    description: "Задача, привязанная к цели и исполнителю",
    required: ["goal_id", "assignee_id", "status", "text"],
    additionalProperties: false, 
    properties: {
      _id: {
        bsonType: "objectId",
        description: "Уникальный идентификатор, генерируется MongoDB"
      },
      goal_id: {
        bsonType: "string",
        minLength: 1,
        maxLength: 128,
        description: "Идентификатор цели (обязательное, 1-128 символов)"
      },
      assignee_id: {
        bsonType: "string",
        minLength: 1,
        maxLength: 128,
        description: "Идентификатор исполнителя (обязательное, 1-128 символов)"
      },
      status: {
        bsonType: "string",
        enum: ["pending", "in_progress", "done", "cancelled"],
        description: "Статус задачи (обязательное, только допустимые значения)"
      },
      text: {
        bsonType: "string",
        minLength: 1,
        maxLength: 2000,
        description: "Текст задачи (обязательное, 1-2000 символов)"
      }
    }
  }
};