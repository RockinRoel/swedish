cmake_minimum_required(VERSION 3.10...3.17)

project(swedish)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(Wt CONFIG REQUIRED COMPONENTS Wt HTTP Dbo DboPostgres)

add_executable(
  swedish.wt
  src/main.cpp
  src/model/User.h src/model/User.cpp
  src/model/Puzzle.h src/model/Puzzle.cpp
  src/model/Session.h src/model/Session.cpp
  src/model/Square.h src/model/Square.cpp
  src/widgets/ColorPicker.h src/widgets/ColorPicker.cpp
  src/widgets/PuzzleUploader.h src/widgets/PuzzleUploader.cpp
)

target_link_libraries(
  swedish.wt
  PRIVATE
  Wt::Wt
  Wt::HTTP
  Wt::Dbo
  Wt::DboPostgres
)

if(TARGET Boost::headers)
  target_link_libraries(swedish.wt PRIVATE Boost::headers)
else()
  target_link_libraries(swedish.wt PRIVATE Boost::boost)
endif()

target_link_libraries(swedish.wt PRIVATE ${BOOST_FILESYSTEM_LIBRARY})
