/*
    Copyright 2016, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#include <fost/log>
#include <fost/pg/stored-procedure.hpp>
#include <fostgres/fostgres.hpp>
#include <fostgres/sql.hpp>


namespace {
    template<typename RS>
    std::vector<fostlib::string> columns(const RS &rs) {
        std::vector<fostlib::string> cols;
        std::size_t number{0};
        for ( const auto &c : rs.columns() ) {
            cols.push_back(c.value("un-named." + std::to_string(++number)));
        }
        return cols;
    }
}


std::pair<std::vector<fostlib::string>, fostlib::pg::recordset> fostgres::column_names(
    fostlib::pg::recordset && rs
) {
    return std::make_pair(columns(rs), std::move(rs));
}


std::pair<std::vector<fostlib::string>, fostlib::pg::recordset> fostgres::sql(
    fostlib::pg::connection &cnx, const fostlib::string &cmd
) {
    auto logger = fostlib::log::debug(c_fostgres);
    logger("", "Executing SQL command")
        ("dsn", cnx.configuration())
        ("command", cmd);

    /// Execute the SQL we've been given
    auto rs = cnx.exec(fostlib::coerce<fostlib::utf8_string>(cmd));

    /// Return data suitable for sending to the browser
    return std::make_pair(columns(rs), std::move(rs));
}


std::pair<std::vector<fostlib::string>, fostlib::pg::recordset> fostgres::sql(
    const fostlib::json &dsn, const fostlib::string &cmd
) {
    auto logger = fostlib::log::debug(c_fostgres);
    logger("", "Executing SQL command")
        ("command", cmd);

    /// Execute the SQL we've been given
    fostlib::pg::connection cnx(dsn);
    logger("dsn", cnx.configuration());
    auto rs = cnx.exec(fostlib::coerce<fostlib::utf8_string>(cmd));

    /// Return data suitable for sending to the browser
    return std::make_pair(columns(rs), std::move(rs));
}


std::pair<std::vector<fostlib::string>, fostlib::pg::recordset> fostgres::sql(
    fostlib::pg::connection &cnx,
    const fostlib::string &cmd, const std::vector<fostlib::string> &args
) {
    auto logger = fostlib::log::debug(c_fostgres);
    logger("", "Executing SQL command")
        ("dsn", cnx.configuration())
        ("command", cmd)
        ("args", args);

    /// Execute the SQL we've been given
    auto sp = cnx.procedure(fostlib::coerce<fostlib::utf8_string>(cmd));
    auto rs = sp.exec(args);

    return std::make_pair(columns(rs), std::move(rs));
}


std::pair<std::vector<fostlib::string>, fostlib::pg::recordset> fostgres::sql(
    const fostlib::json &dsn,
    const fostlib::string &cmd,
    const std::vector<fostlib::string> &args
) {
    auto logger = fostlib::log::debug(c_fostgres);
    logger("", "Executing SQL command")
        ("command", cmd)
        ("args", args);

    /// Execute the SQL we've been given
    fostlib::pg::connection cnx(dsn);
    logger("dsn", cnx.configuration());
    auto sp = cnx.procedure(fostlib::coerce<fostlib::utf8_string>(cmd));
    auto rs = sp.exec(args);

    return std::make_pair(columns(rs), std::move(rs));
}
