/*
    Copyright 2016, Felspar Co Ltd. http://support.felspar.com/
    Distributed under the Boost Software License, Version 1.0.
    See accompanying file LICENSE_1_0.txt or copy at
        http://www.boost.org/LICENSE_1_0.txt
*/


#pragma once


#include <fost/core>


namespace fostgres {


    struct match {
        fostlib::json configuration;
        std::vector<fostlib::string> arguments;
    };


    fostlib::nullable<match> matcher(
        const fostlib::json &config,
        const fostlib::string &path);


}

