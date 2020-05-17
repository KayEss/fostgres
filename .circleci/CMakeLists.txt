cmake_minimum_required(VERSION 3.10)
project(fostgres-circleci)

## Opt in to tests
add_custom_target(check)
add_custom_target(pgtest)

add_subdirectory(boost)
add_subdirectory(crypto)
add_subdirectory(cord)
add_subdirectory(fost-base)
add_subdirectory(fost-beanbag)
add_subdirectory(fost-internet)
add_subdirectory(fost-postgres)
add_subdirectory(fost-web)
add_subdirectory(.. fostgres)
add_subdirectory(json-schema)
add_subdirectory(libpqxx)
add_subdirectory(threading)
