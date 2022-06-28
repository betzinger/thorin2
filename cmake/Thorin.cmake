# clear globals
SET(THORIN_DIALECT_LIST    "" CACHE INTERNAL "THORIN_DIALECT_LIST")
SET(THORIN_DIALECT_LAYOUT  "" CACHE INTERNAL "THORIN_DIALECT_LAYOUT")

if(NOT THORIN_TARGET_NAMESPACE)
    set(THORIN_TARGET_NAMESPACE "")
endif()

#[=======================================================================[
add_thorin_dialect
-------------------

Registers a new Thorin dialect.

```
add_thorin_dialect(<name>
    [SOURCES <source>...]
    [DEPENDS <other_dialect_name>...]
    [HEADER_DEPENDS <other_dialect_name>...]
    [INSTALL])
```

The `<name>` is expected to be the name of the dialect. This means, there
should be (relative to your CMakeLists.txt) a file `<name>/<name>.thorin`
containing the axiom declarations.
This will generate a header `dialects/<name>/autogen.h` that can be used in normalizers
and passes to identify the axioms.

- `SOURCES`: The values to the `SOURCES` argument are the source files used
    to build the loadable plugin containing normalizers, passes and backends.
    One of the source files must export the `thorin_get_dialect_info` function.
    `add_thorin_dialect` creates a new target called `thorin_<name>` that builds
    the dialect plugin.
    Custom properties can be specified in the using `CMakeLists.txt` file,
    e.g. adding include paths is done with `target_include_directories(thorin_<name> <path>..)`.
- `DEPENDS`: The `DEPENDS` arguments specify the relation between multiple
    dialects. This makes sure that the bootstrapping of the dialect is done
    whenever a depended-upon dialect description is changed.
    E.g. `core` depends on `mem`, therefore whenever `mem.thorin` changes,
    `core.thorin` has to be bootstrapped again as well.
- `HEADER_DEPENDS`: The `HEADER_DEPENDS` arguments specify dependencies
    of a dialect's plugin on the generated header of another dialect.
    E.g. `mem.thorin` does not import `core.thorin` but the plugin relies
    on the `%core.conv` axiom. Therefore `mem` requires `core`'s autogenerated
    header to be up-to-date.
- `INSTALL`: Specify, if the dialect description, plugin and headers shall
    be installed with `make install`.
    To export the targets, the export name `install_exports` has to be
    exported accordingly (see [install(EXPORT ..)](https://cmake.org/cmake/help/latest/command/install.html#export))


## Note: a copy of this text is in `docs/coding.md`. Please update!
#]=======================================================================]
function(add_thorin_dialect)
    set(DIALECT ${ARGV0})
    list(SUBLIST ARGV 1 -1 UNPARSED)
    cmake_parse_arguments(
        PARSED              # prefix of output variables
        "INSTALL"           # list of names of the boolean arguments (only defined ones will be true)
        "DIALECT"           # list of names of mono-valued arguments
        "SOURCES;DEPENDS;HEADER_DEPENDS"   # list of names of multi-valued arguments (output variables are lists)
        ${UNPARSED}         # arguments of the function to parse, here we take the all original ones
    )

    set(THORIN_LIB_DIR ${CMAKE_BINARY_DIR}/lib/thorin)

    list(TRANSFORM PARSED_DEPENDS        PREPEND ${THORIN_LIB_DIR}/ OUTPUT_VARIABLE DEPENDS_THORIN_FILES)
    list(TRANSFORM DEPENDS_THORIN_FILES   APPEND .thorin)
    list(TRANSFORM PARSED_DEPENDS        PREPEND ${CMAKE_BINARY_DIR}/include/dialects/ OUTPUT_VARIABLE DEPENDS_HEADER_FILES)
    list(TRANSFORM DEPENDS_HEADER_FILES   APPEND /autogen.h)
    list(TRANSFORM PARSED_HEADER_DEPENDS PREPEND ${CMAKE_BINARY_DIR}/include/dialects/ OUTPUT_VARIABLE PARSED_HEADER_DEPENDS)
    list(TRANSFORM PARSED_HEADER_DEPENDS  APPEND /autogen.h)
    list(APPEND DEPENDS_HEADER_FILES ${PARSED_HEADER_DEPENDS})

    set(THORIN_FILE     ${CMAKE_CURRENT_SOURCE_DIR}/${DIALECT}/${DIALECT}.thorin)
    set(THORIN_FILE_LIB_DIR ${THORIN_LIB_DIR}/${DIALECT}.thorin)
    set(DIALECT_H       ${CMAKE_BINARY_DIR}/include/dialects/${DIALECT}/autogen.h)
    set(DIALECT_MD      ${CMAKE_BINARY_DIR}/docs/dialects/${DIALECT}.md)

    list(APPEND THORIN_DIALECT_LIST "${DIALECT}")
    string(APPEND THORIN_DIALECT_LAYOUT "<tab type=\"user\" url=\"@ref ${DIALECT}\" title=\"${DIALECT}\"/>")

    # populate to globals
    SET(THORIN_DIALECT_LIST   "${THORIN_DIALECT_LIST}"   CACHE INTERNAL "THORIN_DIALECT_LIST")
    SET(THORIN_DIALECT_LAYOUT "${THORIN_DIALECT_LAYOUT}" CACHE INTERNAL "THORIN_DIALECT_LAYOUT")

    # copy dialect thorin file to lib/thorin/${DIALECT}.thorin
    add_custom_command(
        OUTPUT  ${THORIN_FILE_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${THORIN_FILE} ${THORIN_FILE_LIB_DIR}
        DEPENDS ${THORIN_FILE} ${DEPENDS_THORIN_FILES}
    )

    add_custom_command(
        OUTPUT ${DIALECT_MD} ${DIALECT_H}
        COMMAND $<TARGET_FILE:${THORIN_TARGET_NAMESPACE}thorin> -e md -e h ${THORIN_FILE_LIB_DIR} -D ${THORIN_LIB_DIR} --output-h ${DIALECT_H} --output-md ${DIALECT_MD}
        DEPENDS ${THORIN_TARGET_NAMESPACE}thorin ${THORIN_FILE_LIB_DIR}
        COMMENT "Bootstrapping Thorin dialect '${DIALECT}' from '${THORIN_FILE}'"
    )
    add_custom_target(${DIALECT} ALL DEPENDS ${DIALECT_MD} ${DIALECT_H})

    add_library(thorin_${DIALECT}
        MODULE
            ${PARSED_SOURCES}       # original sources passed to add_thorin_dialect
            ${DIALECT_H}            # the generated header of this dialect
            ${DEPENDS_HEADER_FILES} # the generated headers of the dialects we depend on
    )

    set_target_properties(thorin_${DIALECT}
        PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN 1
            WINDOWS_EXPORT_ALL_SYMBOLS OFF
            LIBRARY_OUTPUT_DIRECTORY ${THORIN_LIB_DIR}
    )

    target_link_libraries(thorin_${DIALECT} ${THORIN_TARGET_NAMESPACE}libthorin)

    target_include_directories(thorin_${DIALECT}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include> # dialects/${DIALECT}/autogen.h
            $<INSTALL_INTERFACE:include>
    )

    if(${PARSED_INSTALL})
        install(TARGETS thorin_${DIALECT} EXPORT install_exports LIBRARY DESTINATION lib/thorin RUNTIME DESTINATION lib/thorin INCLUDES DESTINATION include)
        install(FILES ${THORIN_FILE_LIB_DIR} DESTINATION lib/thorin)
        install(FILES ${DIALECT_H} DESTINATION include/dialects/${DIALECT})
        install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DIALECT} DESTINATION include/dialects FILES_MATCHING PATTERN *.h)
    endif()
endfunction()
