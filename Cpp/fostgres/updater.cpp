/**
    Copyright 2016-2019, Felspar Co Ltd. <https://support.felspar.com/>

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
*/


#include "updater.hpp"

#include <f5/json/schema.cache.hpp>
#include <fost/insert>


/**
 * ## `fostgres::updater`
 */


fostgres::updater::updater(
        fostlib::json mconf,
        fostlib::pg::connection &cnx,
        const fostgres::match &m,
        fostlib::http::server::request &req)
: relation(fostlib::coerce<fostlib::string>(mconf["table"])),
  deduced_action(action::do_default),
  config(m.configuration),
  col_config(mconf["columns"]),
  cnx(cnx),
  m(m),
  req(req) {
    if (mconf.has_key("returning")) {
        const fostlib::json &ret_cols = mconf["returning"];
        std::transform(
                ret_cols.begin(), ret_cols.end(),
                std::back_inserter(returning_cols), [](const auto &s) {
                    return fostlib::coerce<fostlib::string>(s);
                });
    }
}


std::pair<fostlib::json, fostlib::json>
        fostgres::updater::data(const fostlib::json &body) {
    deduced_action = action::do_default;
    fostlib::json keys, values;
    for (const auto &col_def : col_config.object()) {
        const auto &key = col_def.first;
        auto data =
                fostgres::datum(key, col_def.second, m.arguments, body, req);
        if (col_def.second["key"].get(false)) {
            /// Key column. We must have data for this.
            if (data) {
                fostlib::insert(keys, key, data.value());
            } else {
                throw fostlib::exceptions::not_implemented(
                        __PRETTY_FUNCTION__, "Key column doesn't have a value",
                        key);
            }
        } else if (data) {
            /// Value column with data
            fostlib::insert(values, key, data.value());
        } else {
            /// There is no data for this column. We will skip it in the
            /// update/insert.
            if (col_def.second["insert"] == "required") {
                /// Because there is no value for this column, and it
                /// is marked as required for insert we suggest a forced
                /// update for this.
                deduced_action = action::updateable;
            }
        }
    }
    return std::make_pair(keys, values);
}


std::pair<boost::shared_ptr<fostlib::mime>, int> fostgres::updater::insert(
        fostgres::updater::intermediate_data d,
        std::optional<std::size_t> row) {
    for (const auto &col_def : col_config.object()) {
        const bool allow_schema = fostlib::coerce<std::optional<bool>>(
                                          col_def.second["allow$schema"])
                                          .value_or(false);
        auto instance =
                (col_def.second["key"].get(false) ? d.first
                                                  : d.second)[col_def.first];
        auto error = schema_check(
                cnx, config, m, req, col_def.second, instance,
                (row ? fostlib::jcursor{*row} : fostlib::jcursor{})
                        / col_def.first);
        if (error.first) return error;
    }
    auto rel = relation;
    if (returning_cols.size()) {
        auto rs = cnx.upsert(
                rel.shrink_to_fit(), d.first, d.second, returning_cols);
        auto result = fostgres::column_names(std::move(rs));
        return response_object(std::move(result), config);
    } else {
        cnx.upsert(rel.shrink_to_fit(), d.first, d.second);
    }
    return {nullptr, 0};
}


std::pair<boost::shared_ptr<fostlib::mime>, int> fostgres::updater::update(
        fostgres::updater::intermediate_data d,
        std::optional<std::size_t> row) {
    auto rel = relation;
    std::vector<fostlib::string> returning = {"*"};
    auto rs = cnx.update(rel.shrink_to_fit(), d.first, d.second, returning);
    auto pos = rs.begin();
    if (pos == rs.end()) {
        return {nullptr, 404};
    } else if (++pos != rs.end()) {
        throw fostlib::exceptions::not_implemented(
                __PRETTY_FUNCTION__,
                "Update for multiple rows not supported by Fostgres");
    }
    return {nullptr, 0};
}


std::pair<
        std::pair<boost::shared_ptr<fostlib::mime>, int>,
        std::pair<fostlib::json, fostlib::json>>
        fostgres::updater::upsert(
                const fostlib::json &body, std::optional<std::size_t> row) {
    auto d = data(body);
    return {insert(d, row), d};
}
std::pair<fostlib::json, fostlib::json>
        fostgres::updater::update(const fostlib::json &body) {
    auto d = data(body);
    update(d);
    return d;
}


/**
 * ## Schema validation
 */


std::pair<boost::shared_ptr<fostlib::mime>, int> fostgres::schema_check(
        fostlib::pg::connection &cnx,
        const fostlib::json &config,
        const fostgres::match &m,
        fostlib::http::server::request &req,
        const fostlib::json &s_config,
        const fostlib::json &body,
        fostlib::jcursor dpos) {
    const std::pair<boost::shared_ptr<fostlib::mime>, int> ok{nullptr, 0};
    const auto validate = [&](const f5::json::schema &s) {
        if (auto valid = s.validate(body); not valid) {
            const bool pretty =
                    fostlib::coerce<fostlib::nullable<bool>>(config["pretty"])
                            .value_or(true);
            fostlib::json result;
            fostlib::insert(result, "schema", s_config["schema"]);
            auto e{(f5::json::validation::result::error)std::move(valid)};
            fostlib::insert(result, "error", "assertion", e.assertion);
            fostlib::insert(result, "error", "in-schema", e.spos);
            fostlib::insert(result, "error", "in-data", dpos / e.dpos);
            boost::shared_ptr<fostlib::mime> response(new fostlib::text_body(
                    fostlib::json::unparse(result, pretty),
                    fostlib::mime::mime_headers(), L"application/json"));
            return std::make_pair(response, 422);
        } else {
            return ok;
        }
    };

    const bool allow_schema =
            fostlib::coerce<std::optional<bool>>(s_config["allow$schema"])
                    .value_or(false);
    if (allow_schema && body.has_key("$schema")) {
        return validate((*f5::json::schema_cache::root_cache())
                                [fostlib::coerce<f5::u8view>(body["$schema"])]);
    } else if (s_config.has_key("schema")) {
        f5::json::schema s{fostlib::url{}, s_config["schema"]};
        return validate(s);
    } else {
        return ok;
    }
}
