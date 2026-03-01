workspace {
    name "E-tasktracker"
    description "Сервис для работы с целями и задачами пользователя"

    # включаем режим с иерархической системой идентификаторов
    !identifiers hierarchical

    model {
        properties { 
            structurizr.groupSeparator "/"
            workspace_cmdb "cmdb_mnemonic"
            architect "Дмитрий Овчинниов"
        }

        my_user = person "Обычный пользователь"
        

        my_system = softwareSystem "Ядро системы"{
            
            user_service = container "Сервис работы с пользователями" {
                technology "Yandex Userver"
                tags "user"
            }
            user_database = container "UserPostgreSQLTable" "Хранение информации о аккаунтах пользователей" {
                technology "PostgreSQL 14"
                tags "database" "user"
            }

            user_service -> user_database "Запрос/изменение данных о пользователях" "SQL TCP:5432"

            task_service = container "Сервис работы с задачами" {
                technology "Yandex Userver"
                tags "task"
            }
            task_database = container "TaskPostgreSQLTable" "Хранение информации о задачах" {
                technology "PostgreSQL 14"
                tags "database" "task"
            }

            task_service -> task_database "Запрос/изменение данных о задачах" "SQL TCP:5433"

            goal_service = container "Сервис работы с целями" {
                technology "Yandex Userver"
                tags "goal"
            }
            goal_database = container "GoalPostgreSQLTable" "Хранение информации о целях" {
                technology "PostgreSQL 14"
                tags "database" "goal"
            }

            goal_service -> goal_database "Запрос/изменение данных о целях" "SQL TCP:5434"

            main_service = container "Сервис оркестратор" {
                technology "Yandex Userver"
            }

            main_service -> user_service "Работа с пользовательсики данными" "REST HTTP:1000"
            main_service -> task_service "Работа с задачами" "REST HTTP:1001"
            main_service -> goal_service "Работа с целями" "REST HTTP:1002"
        


            
        }

        my_user -> my_system "Получение услуг по управлению задачами"
        my_user -> my_system.main_service "Работа с задачами, целями, аккаунтами" "REST HTTP:1003"
    
    }

    views {
        # Конфигурируем настройки отображения plant uml
        properties {
            plantuml.format     "svg"
            kroki.format        "svg"
            structurizr.sort created
            structurizr.tooltips true
        }

        # Задаем стили для отображения
        themes default


        # Диаграмма контекста
        systemContext my_system {
            include *
            autoLayout
        }

        container my_system {
            include *
            autoLayout 
        }

        dynamic my_system {
            title "Создание новой задачи на пути к цели"
            my_user -> my_system.main_service "Запрос на новую задачу к цели для пользователя id=1"
            my_system.main_service -> my_system.user_service "Нахождение всех целей пользователя"
            my_system.user_service -> my_system.user_database "Найди все цели для id=1"
            my_system.user_database -> my_system.user_service "Список целей человека с id=1"
            my_system.user_service -> my_system.main_service "Список целей"
            my_system.main_service -> my_system.goal_service "Найди id задачи пользователя из списка"
            my_system.goal_service -> my_system.goal_database "Получение данных по целям"
            my_system.goal_database -> my_system.goal_service "Название целей"
            my_system.goal_service -> my_system.main_service "Получил id цели"
            my_system.main_service -> my_system.task_service "Сохрани новую задачу цели"
            my_system.task_service -> my_system.task_database "Сохрани новую задачу в бд"
            my_system.task_database -> my_system.task_service "Задача сохранена"
            my_system.task_service -> my_system.main_service "Задача сохранена"
            my_system.main_service -> my_user "Успех сохранения задачи для заданной цели"
            autoLayout rl
        }

        styles {
            element "database" {
                shape cylinder
            }
            element "goal" {
                color #FFFFFF
                stroke #297ABD
                background #94AFC4
            }
            element "task" {
                color #FFFFFF
                stroke #B83030
                background #BC6262
            }
            element "user" {
                color #FFFFFF
                stroke #19F21C
                background #64D266
            }
        }
    }
}