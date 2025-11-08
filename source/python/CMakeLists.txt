# smoldyn python binding.

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)
# find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module Development.Embed VERSION 3.8.15...3.12)

# recompile libsmoldyn for PYTHON.
add_library(_pysmoldyn STATIC ${SRC_FILES}
    $<TARGET_OBJECTS:Steve>
    $<TARGET_OBJECTS:nsv>)

# Enables PYTHON_CALLBACK extention.
target_compile_options(_pysmoldyn PRIVATE -DENABLE_PYTHON_CALLBACK)
# COMPILE_AS_PY_MODULE changes the behaviour of libSteve at a few places
# especially where scanf is used for user input.
target_compile_options(_pysmoldyn PRIVATE -DCOMPILE_AS_PY_MODULE)

target_include_directories(_pysmoldyn PRIVATE ${Python3_INCLUDE_DIRS} ${PYBIND11_SOURCE_DIR}/include)

set_target_properties(_pysmoldyn PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(_pysmoldyn PRIVATE ${DEP_LIBS})
target_link_libraries(_pysmoldyn PUBLIC ${PYTHON_LIBRARIES})

# A custom target to install dependencies. Useful on various CI
add_custom_target(install_deps
    COMMAND ${Python3_EXECUTABLE} -m pip install wheel numpy setuptools --user
    COMMENT "Installing Python modules for testing"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM)

pybind11_add_module(_smoldyn MODULE
    CallbackFunc.cpp
    Command.cpp
    Simulation.cpp
    module.cpp)

target_compile_options(_smoldyn PRIVATE
    -DSMOLDYN_VERSION=${SMOLDYN_VERSION}
    -DENABLE_PYTHON_CALLBACK
    # https://pybind11.readthedocs.io/en/stable/faq.html#someclass-declared-with-greater-visibility-than-the-type-of-its-field-someclass-member-wattributes
    -fvisibility=hidden)

target_include_directories(_smoldyn PRIVATE ${PYTHON_INCLUDE_DIRS} ${PYBIND11_SOURCE_DIR}/include)


# If generator pattern is not used here, cmake produces module in Release/Debug folders
# which will break the wheels.
set_target_properties(_smoldyn PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_SOURCE_DIR}/smoldyn/>)

target_link_libraries(_smoldyn PRIVATE _pysmoldyn)

# Run static type checking using mypy (developer only). Works best with
# python3.7+
add_custom_target(lint
    COMMAND mypy ${CMAKE_SOURCE_DIR}/source/python/smoldyn/*.py
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_target(lint_strict
    COMMAND mypy --strict ${CMAKE_SOURCE_DIR}/source/python/smoldyn/*.py
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

###  python tests ###
enable_testing()

# NOTE: We do not use pytest because pytest doesn't handle global variables very
# well.
file(GLOB TEST_SCRIPTS "${CMAKE_SOURCE_DIR}/tests/test_*.py")
list(LENGTH TEST_SCRIPTS TOTAL_TESTS)
message(STATUS "[TEST] Found ${TOTAL_TESTS} test scripts")

foreach(_pyscript ${TEST_SCRIPTS})
    # message(STATUS " [TEST] Adding test ${_pyscript}")
    get_filename_component(_test_name ${_pyscript} NAME_WE)
    get_filename_component(_test_dir ${_pyscript} DIRECTORY)
    add_test(NAME ${_test_name} COMMAND
        ${Python3_EXECUTABLE} ${_pyscript}
        WORKING_DIRECTORY ${_test_dir})
    set_tests_properties(${_test_name} PROPERTIES
        ENVIRONMENT "PYTHONPATH=${PY_SMOLDYN_OUTDIR};SMOLDYN_NO_PROMPT=1")
endforeach()
