# cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_FRAMEWORK_PATH=/Users/timothy.banks/Work/issues/develop/eos/release -DCMAKE_FRAMEWORK_PATH=/Users/timothy.banks/Work/issues/develop/eosio.cdt/release -DEOSIO_ROOT=/Users/timothy.banks/Work/issues/develop/eos/release -DEOSIO_CDT_ROOT=/Users/timothy.banks/Work/issues/develop/eosio.cdt/release ..

project(session_example)

cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ExternalProject)

find_package(eosio REQUIRED)
find_package(Boost 1.66 REQUIRED COMPONENTS
    date_time
    filesystem
    chrono
    unit_test_framework
    iostreams)

add_executable(session_example session_example.cpp)
target_link_libraries( session_example 
  ${EOSIO_ROOT}/libraries/fc/libfc.a 
  ${EOSIO_ROOT}/libraries/rocksdb/librocksdb.a 
  ${EOSIO_ROOT}/libraries/softfloat/libsoftfloat.a 
  ${EOSIO_ROOT}/libraries/chain/libeosio_chain.a 
  ${Boost_CHRONO_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

set_target_properties(session_example PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(session_example
	PUBLIC 
	${CMAKE_SOURCE_DIR}/include
  ${EOSIO_ROOT}/../libraries/chain_kv/include/b1
  ${EOSIO_ROOT}/../libraries/chain_kv/include
	${EOSIO_ROOT}/../libraries/fc/include
  ${EOSIO_ROOT}/../libraries/rocksdb/include
  /usr/local/include/
)

set_target_properties(session_example
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)