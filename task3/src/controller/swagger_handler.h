#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Logger.h>
#include <Poco/Timestamp.h>

#include <string>

namespace handlers {

inline const std::string SWAGGER_YAML = R"""(
openapi: 3.0.3
info:
  title: Goals & Users API
  description: REST API for managing users and goals with JWT authentication
  version: 1.0.0

servers:
  - url: http://127.0.0.1:8080
    description: Адрес для проверки

security: []

paths:

  /api/v1/auth:
    post:
      summary: Получить JWT bearer токен
      description: |
        Подпись будет работать только 24 часа. Use the returned token as `Bearer <token>` in
        the Authorization header for protected endpoints.
      operationId: acquireBearer
      tags: [Auth]
      security: []
      requestBody:
        required: false
        description: No request body needed — credentials are passed via Basic Auth header.
        content: {}
      parameters:
        - in: header
          name: Authorization
          required: true
          schema:
            type: string
            example: "Basic YWRtaW46c2VjdXJlZA=="
          description: Логин и пароль `admin:secured`. Может выйти всплывающее окно
      responses:
        "200":
          description: Successfully authenticated — returns signed JWT
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/TokenResponse"
              examples:
                success:
                  summary: Success
                  value:
                    token: "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiJhZG1pbiIsImlhdCI6MTcxMDAwMDAwMCwiZXhwIjoxNzEwMDg2NDAwLCJ1c2VybmFtZSI6ImFkbWluIn0.SIGNATURE"
        "400":
          description: Missing or wrong credentials
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/ErrorResponse"
              examples:
                wrong_credentials:
                  summary: Wrong login/password
                  value:
                    error: "I dont recognize you, your ip adress send to our security team"
                missing_username:
                  summary: Empty username
                  value:
                    error: "username is required"
                missing_password:
                  summary: Empty password
                  value:
                    error: "password is required"
        "401":
          description: No Basic Auth header provided
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/ErrorResponse"
              examples:
                no_auth:
                  summary: Missing Authorization header
                  value:
                    error: "Basic authentication required"
        "500":
          description: JWT secret not configured on server
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/ErrorResponse"
              examples:
                no_secret:
                  summary: JWT_SECRET env var missing
                  value:
                    error: "JWT_SECRET not configured"

  /api/v1/users:
    post:
      summary: Add a new user
      description: Creates and stores a new user. Requires a valid JWT bearer token.
      operationId: addUser
      tags: [Users]
      security:
        - bearerAuth: []
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: "#/components/schemas/UserRequest"
            examples:
              valid:
                summary: Valid user payload
                value:
                  login: "jdoe"
                  name: "John"
                  familyName: "Doe"
      responses:
        "200":
          description: User saved successfully
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/MessageResponse"
              examples:
                success:
                  summary: Success
                  value:
                    message: "Success on saving user"
        "401":
          description: Missing or invalid JWT token
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/ErrorResponse"
              examples:
                no_token:
                  summary: No Bearer token
                  value:
                    error: "Bearer token required"
                invalid_token:
                  summary: Bad or expired token
                  value:
                    error: "Your ip address send to out security team. Wrong Auth"
        "500":
          description: Failed to parse request body
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/MessageResponse"
              examples:
                parse_error:
                  summary: Malformed JSON
                  value:
                    message: "Cant parse request"

  /api/v1/users/login:
    get:
      summary: Find user by login
      description: Looks up a user by their exact login string. Returns full user details if found.
      operationId: findUserByLogin
      tags: [Users]
      parameters:
        - in: query
          name: name
          required: true
          schema:
            type: string
            example: "admin"
          description: The exact login of the user to search for
      responses:
        "200":
          description: |
            Always returns 200. Check `status` field to distinguish found vs not found.
          content:
            application/json:
              schema:
                oneOf:
                  - $ref: "#/components/schemas/UserFoundResponse"
                  - $ref: "#/components/schemas/UserNotFoundResponse"
              examples:
                found:
                  summary: User exists
                  value:
                    status: "success"
                    user:
                      login: "admin"
                      firstName: "Admin"
                      lastName: "Adminov"
                      fullName: "Admin Adminov"
                not_found:
                  summary: User does not exist
                  value:
                    user: "not exist"

  /api/v1/users/fullname:
    get:
      summary: Check user existence by full name
      description: Returns whether a user with the given first and last name exists.
      operationId: findUserByFullName
      tags: [Users]
      parameters:
        - in: query
          name: name
          required: true
          schema:
            type: string
            example: "Admin"
          description: First name
        - in: query
          name: lastName
          required: true
          schema:
            type: string
            example: "Adminov"
          description: Last name
      responses:
        "200":
          description: Existence check result
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/UserExistenceResponse"
              examples:
                exists:
                  summary: User found
                  value:
                    user: "exist"
                not_exists:
                  summary: User not found
                  value:
                    user: "not exist"

  /api/v1/goals:
    get:
      summary: Get all goals
      description: Returns a list of all goals stored in memory. No authentication required.
      operationId: getAllGoals
      tags: [Goals]
      responses:
        "200":
          description: List of all goals
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/GoalsListResponse"
              examples:
                success:
                  summary: Goals returned
                  value:
                    status: "success"
                    goals:
                      - login: "admin"
                        name: "make money"
                      - login: "default"
                        name: "pass tests"

    post:
      summary: Add a new goal
      description: Creates and stores a new goal. Requires a valid JWT bearer token.
      operationId: addGoal
      tags: [Goals]
      security:
        - bearerAuth: []
      requestBody:
        required: true
        content:
          application/json:
            schema:
              $ref: "#/components/schemas/GoalRequest"
            examples:
              valid:
                summary: Valid goal payload
                value:
                  login: "jdoe"
                  name: "learn C++"
      responses:
        "200":
          description: Goal saved successfully
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/MessageResponse"
              examples:
                success:
                  summary: Success
                  value:
                    message: "Success on saving goal"
        "401":
          description: Missing or invalid JWT token
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/ErrorResponse"
              examples:
                no_token:
                  summary: No Bearer token
                  value:
                    error: "Bearer token required"
                invalid_token:
                  summary: Bad or expired token
                  value:
                    error: "Your ip address send to out security team. Wrong Auth"
        "500":
          description: Failed to parse request body
          content:
            application/json:
              schema:
                $ref: "#/components/schemas/MessageResponse"
              examples:
                parse_error:
                  summary: Malformed JSON
                  value:
                    message: "Cant parse request"

  /swagger.yaml:
    get:
      summary: Serve OpenAPI spec (YAML)
      operationId: getSwaggerYaml
      tags: [Meta]
      responses:
        "200":
          description: YAML spec file
          content:
            text/yaml: {}

  /swaggerUI:
    get:
      summary: Serve Swagger UI
      operationId: getSwaggerUI
      tags: [Meta]
      responses:
        "200":
          description: HTML Swagger UI page
          content:
            text/html: {}

components:
  securitySchemes:
    bearerAuth:
      type: http
      scheme: bearer
      bearerFormat: JWT
      description: |
        Obtain a token from `POST /api/v1/auth` and pass it as:
        `Authorization: Bearer <token>`

  schemas:
    TokenResponse:
      type: object
      properties:
        token:
          type: string
          description: Signed HS256 JWT, valid for 24 hours
      required: [token]

    ErrorResponse:
      type: object
      properties:
        error:
          type: string
      required: [error]

    MessageResponse:
      type: object
      properties:
        message:
          type: string
      required: [message]

    UserRequest:
      type: object
      properties:
        login:
          type: string
          example: "jdoe"
        name:
          type: string
          example: "John"
        familyName:
          type: string
          example: "Doe"
      required: [login, name, familyName]

    GoalRequest:
      type: object
      properties:
        login:
          type: string
          example: "jdoe"
          description: Login of the user this goal belongs to
        name:
          type: string
          example: "learn C++"
          description: Goal description
      required: [login, name]

    UserFoundResponse:
      type: object
      properties:
        status:
          type: string
          example: "success"
        user:
          type: object
          properties:
            login:
              type: string
            firstName:
              type: string
            lastName:
              type: string
            fullName:
              type: string
          required: [login, firstName, lastName, fullName]
      required: [status, user]

    UserNotFoundResponse:
      type: object
      properties:
        user:
          type: string
          example: "not exist"
      required: [user]

    UserExistenceResponse:
      type: object
      properties:
        user:
          type: string
          enum: [exist, "not exist"]
      required: [user]

    GoalObject:
      type: object
      properties:
        login:
          type: string
        name:
          type: string
      required: [login, name]

    GoalsListResponse:
      type: object
      properties:
        status:
          type: string
          example: "success"
        goals:
          type: array
          items:
            $ref: "#/components/schemas/GoalObject"
      required: [status, goals]
)""";

// Swagger UI HTML страница
inline const std::string SWAGGER_UI_HTML = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Poco Server API Documentation</title>
    <link rel="stylesheet" type="text/css" href="https://unpkg.com/swagger-ui-dist@5.10.5/swagger-ui.css">
    <style>
        html {
            box-sizing: border-box;
            overflow: -moz-scrollbars-vertical;
            overflow-y: scroll;
        }
        *,
        *:before,
        *:after {
            box-sizing: inherit;
        }
        body {
            margin: 0;
            background: #fafafa;
        }
        .topbar {
            background-color: #1b2a4e;
            padding: 10px;
            color: white;
            text-align: center;
        }
        .topbar h1 {
            margin: 0;
            font-size: 24px;
        }
        .topbar p {
            margin: 5px 0 0;
            font-size: 14px;
            opacity: 0.8;
        }
    </style>
</head>
<body>
    <div class="topbar">
        <h1>Poco Template Server API</h1>
        <p>Interactive API Documentation</p>
    </div>
    <div id="swagger-ui"></div>
    <script src="https://unpkg.com/swagger-ui-dist@5.10.5/swagger-ui-bundle.js"></script>
    <script src="https://unpkg.com/swagger-ui-dist@5.10.5/swagger-ui-standalone-preset.js"></script>
    <script>
        window.onload = function() {
            const ui = SwaggerUIBundle({
                url: "http://127.0.0.1:8080/swagger.yaml",  // или "/swagger.yaml" в зависимости от вашего эндпоинта
                dom_id: '#swagger-ui',
                deepLinking: true,
                presets: [
                    SwaggerUIBundle.presets.apis,
                    SwaggerUIStandalonePreset
                ],
                plugins: [
                    SwaggerUIBundle.plugins.DownloadUrl
                ],
                layout: "BaseLayout",
                validatorUrl: null,  // Отключаем валидатор для локальной разработки
                docExpansion: "list",  // Раскрывать все операции
                defaultModelsExpandDepth: 1,
                defaultModelExpandDepth: 1,
                displayRequestDuration: true,
                filter: true,
                showExtensions: true,
                showCommonExtensions: true,
                supportedSubmitMethods: ['get', 'post', 'put', 'delete', 'patch'],
                tryItOutEnabled: true  // Включаем Try it out по умолчанию
            });
            window.ui = ui;
        };
    </script>
</body>
</html>
)";

// Обработчик для Swagger UI страницы
class SwaggerUIHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override {
        
        auto& logger = Poco::Logger::get("Server");
        logger.information("GET /swagger - Serving Swagger UI");
        
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentType("text/html; charset=utf-8");
        response.setContentLength(static_cast<std::streamsize>(SWAGGER_UI_HTML.size()));
        
        std::ostream& ostr = response.send();
        ostr << SWAGGER_UI_HTML;
    }
};

class SwaggerHandler : public Poco::Net::HTTPRequestHandler {
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request,
                       Poco::Net::HTTPServerResponse& response) override {
        // Poco::Timestamp start;
        // if (g_httpRequests) g_httpRequests->inc();

        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        response.setContentType("application/x-yaml");
        response.setContentLength(static_cast<std::streamsize>(SWAGGER_YAML.size()));
        std::ostream& ostr = response.send();
        ostr << SWAGGER_YAML;

        // Poco::Timespan elapsed = Poco::Timestamp() - start;
        // double seconds = static_cast<double>(elapsed.totalMicroseconds()) / 1000000.0;
        // if (g_httpDuration) g_httpDuration->observe(seconds);
        auto& logger = Poco::Logger::get("Server");
        // logger.information("200 GET /swagger.yaml from %s, %.2f ms",
                          // request.clientAddress().toString(), seconds * 1000.0);
    }
};

} // namespace handlers
