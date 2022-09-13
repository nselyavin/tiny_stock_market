#include <string>
#include <utility>
#include <iostream>
#include <RequestHandler.h>
#include <nlohmann/json.hpp>
#include <OrderHandler.h>
#include <UserHandler.h>

using namespace httpparser;
using namespace nlohmann;
using std::string;

std::string RequestHandler::get_response(Request request) {
    return route(request).serialize();
}

Response RequestHandler::route(Request request) {
    std::tuple url(request.method, request.uri);

    if (url == std::tuple("POST", "/api/add_user")) {
        return add_user(request);
    }
    if (url == std::tuple("POST", "/api/add_order")) {
        return add_order(request);
    }
    if (url == std::tuple("POST", "/api/get_orders")) {
        return get_orders(request);
    }
    if (url == std::tuple("POST", "/api/get_userdetail")) {
        return get_userdetail(request);
    } else {
        Response resp;
        resp.status     = "Not Found";
        resp.statusCode = 404;
        return resp;
    }
}

/// @example
/// {
///     "user_id": "123123123"
/// }
httpparser::Response RequestHandler::add_user(httpparser::Request req) {
    string user_id;
    json   data = json::parse(req.content_as_str());
    try {
        user_id = data.at("user_id");
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        Response resp;
        resp.statusCode = 400;
        resp.status     = "Bad Request";
        return resp;
    }

    UserHandler::get_instance()->add_user(user_id);

    Response resp;
    resp.statusCode = 200;
    resp.status     = "Ok";
    return resp;
}

/// @example
/// {
///     "user_id": "123123123",
///     "source": "RUB",
///     "target": "USD",
///     "value": 20,
///     "price": 61
/// }
Response RequestHandler::add_order(Request req) {

    Order order;
    json  data = json::parse(req.content_as_str());
    try {
        order.user_id    = data.at("user_id");
        order.order_pair = {data.at("source"), data.at("target")};
        order.value      = data.at("value");
        order.price      = data.at("price");
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        Response resp;
        resp.statusCode = 400;
        resp.status     = "Bad Request";
        return resp;
    }

    if (!UserHandler::get_instance()->verify_user(order.user_id)) {
        Response resp;
        resp.statusCode = 403;
        resp.status     = "Unauthorized";
        return resp;
    }

    OrderHandler::get_instance().get()->add_order(order);

    Response resp;
    resp.statusCode = 200;
    resp.status     = "Ok";

    return resp;
}

Response RequestHandler::get_orders(Request req) {
    string user_id;
    json   data = json::parse(req.content_as_str());
    try {
        user_id = data.at("user_id");
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        Response resp;
        resp.statusCode = 400;
        resp.status     = "Bad Request";
        return resp;
    }
    if (!UserHandler::get_instance()->verify_user(user_id)) {
        Response resp;
        resp.statusCode = 403;
        resp.status     = "Unauthorized";
        return resp;
    }

    auto opt_orders =
        OrderHandler::get_instance().get()->get_orders_by_user(user_id);

    std::stringstream sstr_body;
    sstr_body << "{\"orders\":[";

    if (opt_orders.has_value()) {
        auto orders = opt_orders.value();
        for (const auto& order : orders) {
            sstr_body << order.serialize() << ", ";
        }
    }

    sstr_body << "]}";

    Response resp;
    resp.statusCode = 200;
    resp.status     = "Ok";
    Response::HeaderItem cont_type{"Content-Type", "application/json"};
    Response::HeaderItem cont_len{"Content-Length",
                                  std::to_string(sstr_body.str().length())};
    resp.headers.push_back(cont_type);
    resp.headers.push_back(cont_len);
    resp.str_to_content(sstr_body.str());

    return resp;
}

httpparser::Response RequestHandler::get_userdetail(httpparser::Request req) {
    string user_id;
    json   data = json::parse(req.content_as_str());
    try {
        user_id = data.at("user_id");
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        Response resp;
        resp.statusCode = 400;
        resp.status     = "Bad Request";
        return resp;
    }
    if (!UserHandler::get_instance()->verify_user(user_id)) {
        Response resp;
        resp.statusCode = 403;
        resp.status     = "Unauthorized";
        return resp;
    }

    auto user = UserHandler::get_instance()->get_user(user_id);

    json body = {
        {"user_id", user_id},
    };
    json balance_arr = json::array();

    for (const auto& bal: user->get().get_balance()) {
        balance_arr.push_back({bal.first, bal.second});
    }

    body["balance"] = balance_arr;

    Response resp;
    resp.statusCode = 200;
    resp.status     = "Ok";
    Response::HeaderItem cont_type{"Content-Type", "application/json"};
    Response::HeaderItem cont_len{"Content-Length",
                                  std::to_string(body.dump().length())};
    resp.headers.push_back(cont_type);
    resp.headers.push_back(cont_len);
    resp.str_to_content(body.dump());
    return resp;
}
