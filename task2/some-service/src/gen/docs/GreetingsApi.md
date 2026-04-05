# GreetingsApi

All URIs are relative to *https://api.example.com/v1*

| Method | HTTP request | Description |
|------------- | ------------- | -------------|
| [**getHello**](GreetingsApi.md#getHello) | **GET** /hello | Возвращает приветствие |


<a id="getHello"></a>
# **getHello**
> GetHello200Response getHello(name)

Возвращает приветствие

Простой эндпоинт, который возвращает приветственное сообщение

### Example
```java
// Import classes:
import org.openapitools.client.ApiClient;
import org.openapitools.client.ApiException;
import org.openapitools.client.Configuration;
import org.openapitools.client.models.*;
import org.openapitools.client.api.GreetingsApi;

public class Example {
  public static void main(String[] args) {
    ApiClient defaultClient = Configuration.getDefaultApiClient();
    defaultClient.setBasePath("https://api.example.com/v1");

    GreetingsApi apiInstance = new GreetingsApi(defaultClient);
    String name = "Мир"; // String | Имя для персонализированного приветствия (опционально)
    try {
      GetHello200Response result = apiInstance.getHello(name);
      System.out.println(result);
    } catch (ApiException e) {
      System.err.println("Exception when calling GreetingsApi#getHello");
      System.err.println("Status code: " + e.getCode());
      System.err.println("Reason: " + e.getResponseBody());
      System.err.println("Response headers: " + e.getResponseHeaders());
      e.printStackTrace();
    }
  }
}
```

### Parameters

| Name | Type | Description  | Notes |
|------------- | ------------- | ------------- | -------------|
| **name** | **String**| Имя для персонализированного приветствия (опционально) | [optional] |

### Return type

[**GetHello200Response**](GetHello200Response.md)

### Authorization

No authorization required

### HTTP request headers

 - **Content-Type**: Not defined
 - **Accept**: application/json, text/plain

### HTTP response details
| Status code | Description | Response headers |
|-------------|-------------|------------------|
| **200** | Успешный ответ с приветствием |  -  |
| **400** | Ошибка в запросе |  -  |
| **500** | Внутренняя ошибка сервера |  -  |

