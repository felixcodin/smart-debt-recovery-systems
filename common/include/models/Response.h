// Response.h - Generic HTTP response wrapper template

#ifndef SDRS_RESPONSE_H
#define SDRS_RESPONSE_H

#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include "../utils/Constants.h"

namespace sdrs::models
{

template<typename T>
class Response
{
private:
    bool _success;
    std::string _message;
    std::optional<T> _data;
    int _statusCode;

public:
    Response(bool success, const std::string& message, int statusCode = sdrs::constants::status_codes::OK, std::optional<T> data = std::nullopt)
        : _success(success), _message(message), _data(data), _statusCode(statusCode)
    {
        // Do nothing
    }

    // Static factory methods for common responses
    static Response<T> success(const T& data, const std::string& message = "Success", int statusCode = sdrs::constants::status_codes::OK)
    {
        return Response<T>(true, message, statusCode, data);
    }

    static Response<T> error(const std::string& message, int statusCode = sdrs::constants::status_codes::INTERNAL_SEVER_ERROR)
    {
        return Response<T>(false, message, statusCode, std::nullopt);
    }

    static Response<T> notFound(const std::string& message = "Resource not found")
    {
        return Response<T>(false, message, sdrs::constants::status_codes::NOT_FOUND, std::nullopt);
    }

    static Response<T> badRequest(const std::string& message = "Bad request")
    {
        return Response<T>(false, message, sdrs::constants::status_codes::BAD_REQUEST, std::nullopt);
    }

    // Getters
    bool isSuccess() const { return _success; }
    const std::string& getMessage() const { return _message; }
    const std::optional<T>& getData() const { return _data; }
    int getStatusCode() const { return _statusCode; }

    // JSON serialization
    std::string toJson() const
    {
        nlohmann::json j;
        j["success"] = _success;
        j["message"] = _message;
        j["status_code"] = _statusCode;
        
        if (_data.has_value())
        {
            // If T has toJson() method, use it
            if constexpr (requires { _data.value().toJson(); })
            {
                j["data"] = nlohmann::json::parse(_data.value().toJson());
            }
            else
            {
                // Otherwise, try direct JSON conversion
                j["data"] = _data.value();
            }
        }
        else
        {
            j["data"] = nullptr;
        }
        
        return j.dump();
    }
};

// Specialization for void (no data)
template<>
class Response<void>
{
private:
    bool _success;
    std::string _message;
    int _statusCode;

public:
    Response(bool success, const std::string& message, int statusCode = sdrs::constants::status_codes::OK)
        : _success(success), _message(message), _statusCode(statusCode)
    {
        // Do nothing
    }

    static Response<void> success(const std::string& message = "Success", int statusCode = sdrs::constants::status_codes::OK)
    {
        return Response<void>(true, message, statusCode);
    }

    static Response<void> error(const std::string& message, int statusCode = sdrs::constants::status_codes::INTERNAL_SEVER_ERROR)
    {
        return Response<void>(false, message, statusCode);
    }

    static Response<void> notFound(const std::string& message = "Resource not found")
    {
        return Response<void>(false, message, sdrs::constants::status_codes::NOT_FOUND);
    }

    static Response<void> badRequest(const std::string& message = "Bad request")
    {
        return Response<void>(false, message, sdrs::constants::status_codes::BAD_REQUEST);
    }

    bool isSuccess() const { return _success; }
    const std::string& getMessage() const { return _message; }
    int getStatusCode() const { return _statusCode; }

    std::string toJson() const
    {
        nlohmann::json j;
        j["success"] = _success;
        j["message"] = _message;
        j["status_code"] = _statusCode;
        j["data"] = nullptr;
        return j.dump();
    }
};

} // namespace sdrs::models

#endif // SDRS_RESPONSE_H
