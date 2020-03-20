cmake_minimum_required(VERSION 3.10...3.17)

project(swedish)

find_package(Wt CONFIG REQUIRED COMPONENTS Wt HTTP Dbo DboPostgres)

add_executable(
    swedish.wt
    main.cpp
)

target_link_libraries(swedish.wt PRIVATE Wt::Wt Wt::HTTP Wt::Dbo Wt::DboPostgres)
