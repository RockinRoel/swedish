cmake_minimum_required(VERSION 3.10...3.17)

project(swedish)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(Wt CONFIG REQUIRED COMPONENTS Wt HTTP Dbo DboPostgres)

add_executable(
  swedish.wt
  src/main.cpp
  src/Application.h src/Application.cpp
  src/Direction.h
  src/Dispatcher.h src/Dispatcher.cpp
  src/GlobalSession.h src/GlobalSession.cpp
  src/Layout.h src/Layout.cpp
  src/Rotation.h
  src/UserCopy.h
  src/jobs/SquareFinder.h src/jobs/SquareFinder.cpp
  src/model/User.h src/model/User.cpp
  src/model/Puzzle.h src/model/Puzzle.cpp
  src/model/Session.h src/model/Session.cpp
  src/widgets/ColorPicker.h src/widgets/ColorPicker.cpp
  src/widgets/PuzzleUploader.h src/widgets/PuzzleUploader.cpp
  src/widgets/PuzzleView.h src/widgets/PuzzleView.cpp
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
