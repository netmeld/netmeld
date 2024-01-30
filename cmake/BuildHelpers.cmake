# =============================================================================
# Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
# (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# =============================================================================
# Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
# =============================================================================

# Used/Useful programs
find_program(HELP2MAN help2man)
if (NOT HELP2MAN)
  message(WARNING "help2man not found, docs will not generate.")
endif()

find_program(PANDOC pandoc)
if (NOT PANDOC)
  message(WARNING "pandoc not found, docs will not fully generate.")
endif()

find_program(SED sed)
if (NOT SED)
  message(WARNING "sed not found, docs may leak build environment values.")
endif()

find_program(GZIP gzip)
if (NOT GZIP)
  message(WARNING "gzip not found, docs will not generate.")
endif()

find_program(GIT git)
if (NOT GIT)
  message(WARNING "git not found, versioning will be wrong.")
endif()

find_program(DATE date)
if (NOT DATE)
  message(WARNING "date not found, versioning will be wrong.")
endif()

# Target related helpers
function(nm_add_generic target)
  set(full_path "${CMAKE_CURRENT_SOURCE_DIR}/${target}")
  if(EXISTS "${full_path}" AND EXISTS "${full_path}/CMakeLists.txt")
    add_subdirectory("${target}")
  endif()
endfunction()
function(target_as_module target)
  if(NOT TGT_MODULE)
    set(prior_tgt_module "${TOOL_SUITE}")
    set(TGT_MODULE "${target}")
    set(prior_tgt_module_test "${TEST_ALL}")
    set(TGT_MODULE_TEST "Test.${target}")
  else()
    set(prior_tgt_module "${TGT_MODULE}")
    set(TGT_MODULE "${TGT_MODULE}-${target}")
    set(prior_tgt_module_test "${TGT_MODULE_TEST}")
    set(TGT_MODULE_TEST "${TGT_MODULE_TEST}.${target}")
  endif()
  add_custom_target("${TGT_MODULE}" DEPENDS ${prior_tgt_module})
  add_custom_target("${TGT_MODULE_TEST}" DEPENDS ${TGT_MODULE})
  add_dependencies(${prior_tgt_module_test} ${TGT_MODULE_TEST})

  get_property(tmp GLOBAL PROPERTY RAW_MODULE_LIST)
  list(APPEND tmp ${target})
  set_property(GLOBAL PROPERTY RAW_MODULE_LIST ${tmp})

  nm_add_generic("${target}")
  #cpack_add_component(${TGT_MODULE})
endfunction()
function(target_as_tool target)
  set(TGT_TOOL "${target}")
  set(TGT_TOOL_TEST "${TGT_MODULE_TEST}.${target}")
  get_timestamp(${target} TGT_VERSION)
  add_dependencies("${TGT_MODULE}" "${TGT_TOOL}")

  get_property(tmp GLOBAL PROPERTY RAW_TOOL_LIST)
  list(APPEND tmp ${target})
  set_property(GLOBAL PROPERTY RAW_TOOL_LIST ${tmp})

  nm_add_generic("${target}")
endfunction()
function(target_as_library target)
  set(TGT_LIBRARY "${TOOL_SUITE}-${TGT_MODULE}")
  if(NOT TGT_LIBRARY_TEST)
    set(TGT_LIBRARY_TEST "${TGT_MODULE_TEST}")
  else()
    set(TGT_LIBRARY_TEST "${TGT_LIBRARY_TEST}.${target}")
  endif()
  nm_add_generic("${target}")
endfunction()

# Testing related helpers
function(nm_add_test target)
  if(TGT_TOOL_TEST)
    set(test_target "${TGT_TOOL_TEST}.${target}")
  elseif(TGT_LIBRARY_TEST)
    set(test_target "${TGT_LIBRARY_TEST}.${target}")
  else()
    set(test_target "${TGT_MODULE_TEST}.${target}")
  endif()
  set(TGT_TEST "${test_target}" PARENT_SCOPE)
  add_executable(${test_target}
      EXCLUDE_FROM_ALL
      ${target}.test.cpp
    )
  target_link_libraries(${test_target}
      ${Boost_LIBRARIES}
    )
  set(test_name "${test_target}")
  add_test(${test_name} ${test_target})
  set_tests_properties(${test_name}
      PROPERTIES LABELS "${TGT_MODULE_TEST}"
    )
  add_dependencies(${TGT_MODULE_TEST} ${test_target})
endfunction()

# Install related helpers
function(nm_install_bin target_file)
  target_compile_definitions(${target_file}
    PUBLIC
      -DPROGRAM_NAME="${target_file}"
      -DPROGRAM_VERSION="${MODULE_VERSION}.${TGT_VERSION}"
    )
  install(
    TARGETS ${target_file}
    OPTIONAL
    COMPONENT ${TGT_MODULE}
    DESTINATION bin
  )
  create_man_from_help(${target_file})
  nm_install_man(${target_file})
endfunction()
function(nm_install_program target_file)
  install(
    PROGRAMS ${target_file}
    OPTIONAL
    COMPONENT ${TGT_MODULE}
    DESTINATION bin
  )
  create_man_from_help(${target_file})
  nm_install_man(${target_file})
endfunction()
function(nm_install_lib target_file)
  install(
    TARGETS ${target_file}
    OPTIONAL
    COMPONENT ${TGT_MODULE}
    DESTINATION lib
  )
  set_target_properties(${target_file}
      PROPERTIES
        VERSION "${MODULE_VERSION}.${TGT_VERSION}"
    )
endfunction()
function(nm_install_include target_dir target_path)
  install(
    DIRECTORY ${target_dir}
    OPTIONAL
    COMPONENT ${TGT_MODULE}
    DESTINATION include/netmeld/${target_path}
    FILES_MATCHING PATTERN "*.[hi]pp"
  )
  add_subdirectory(${target_dir})
endfunction()
function(nm_install_man target_file)
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${target_file}.1.gz"
    OPTIONAL
    COMPONENT ${TGT_MODULE}
    DESTINATION share/man/man1
  )
endfunction()
function(nm_install_conf target_file target_path)
  install(
      FILES ${target_file}
      OPTIONAL
      COMPONENT ${TGT_MODULE}
      DESTINATION ${NETMELD_CONF_DIR}/${target_path}
    )
endfunction()

# Packaging related helpers
function(nm_add_deb_generic cpack_target cpack_value)
  set(${cpack_target} ${cpack_value} CACHE INTERNAL ${cpack_target})
endfunction()
function(nm_add_deb_description tgt_value)
  string(TOUPPER CPACK_DEBIAN_${TGT_MODULE}_DESCRIPTION tgt_var)
  nm_add_deb_generic(${tgt_var} ${tgt_value})
endfunction()
function(nm_add_deb_depends tgt_value)
  string(TOUPPER CPACK_DEBIAN_${TGT_MODULE}_PACKAGE_DEPENDS tgt_var)
  nm_add_deb_generic(${tgt_var} ${tgt_value})
endfunction()
function(nm_add_deb_recommends tgt_value)
  string(TOUPPER CPACK_DEBIAN_${TGT_MODULE}_PACKAGE_RECOMMENDS tgt_var)
  nm_add_deb_generic(${tgt_var} ${tgt_value})
endfunction()
function(nm_add_deb_suggests tgt_value)
  string(TOUPPER CPACK_DEBIAN_${TGT_MODULE}_PACKAGE_SUGGESTS tgt_var)
  nm_add_deb_generic(${tgt_var} ${tgt_value})
endfunction()
function(nm_add_deb_control_data tgt_dir)
  file(GLOB tgt_files CONFIGURE_DEPENDS
       "${CMAKE_CURRENT_SOURCE_DIR}/${tgt_dir}/*")
  string(TOUPPER CPACK_DEBIAN_${TGT_MODULE}_PACKAGE_CONTROL_EXTRA tgt_var)
  nm_add_deb_generic(${tgt_var} "${tgt_files}")
  nm_add_deb_generic(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)
endfunction()

# General purpose helpers
function(get_basename cwd_var result_var)
  execute_process(
      COMMAND
        basename "${cwd_var}"
      OUTPUT_VARIABLE RESULT_STRIPPED
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
  )
  set(${result_var} ${RESULT_STRIPPED} PARENT_SCOPE)
endfunction()

function(get_timestamp target_file result_var)
  execute_process(
      COMMAND
          date +%Y%m%d
      OUTPUT_VARIABLE RESULT_DATE
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
  )
  execute_process(
      COMMAND
          git log -1 --format=%ad --date=format:%Y%m%d
            -- ${CMAKE_CURRENT_SOURCE_DIR}/${target_file}
      OUTPUT_VARIABLE RESULT_GIT
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
  )
  if(RESULT_GIT)
    set(RESULT ${RESULT_GIT})
  elseif(RESULT_DATE)
    set(RESULT ${RESULT_DATE})
  else()
    set(RESULT 0)
  endif()
  set(${result_var} ${RESULT} PARENT_SCOPE)
endfunction()

function(create_man_from_help tool_name)
  if(HELP2MAN AND GZIP)
    set(man_file "${CMAKE_CURRENT_BINARY_DIR}/${tool_name}.1")
    set(readme_file "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(h2m_file "${CMAKE_CURRENT_BINARY_DIR}/${tool_name}.h2m")
    if(PANDOC AND SED AND EXISTS "${readme_file}")
      add_custom_command(
        TARGET ${tool_name}
        COMMAND
          bash -c "sed -e 's@!\\[.*](.*)@```\\nImage at associated markdown page on https://github.com/netmeld/netmeld\\n```@g' ${readme_file}" |
          ${PANDOC} -f gfm -t man
            --output ${h2m_file}
        COMMAND
          bash -c "sed -i -e 's@\\.SH \\(.*\\)@[=\\1]@g' ${h2m_file}"
        VERBATIM
      )
    endif()
    add_custom_command(
      TARGET ${tool_name}
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${tool_name}
      COMMAND
        ${HELP2MAN} ${CMAKE_CURRENT_BINARY_DIR}/${tool_name}
          -I ${h2m_file}
          --no-discard-stderr -N
          -o ${man_file}
    )
    if(SED)
      add_custom_command(
        TARGET ${tool_name}
        DEPENDS ${man_file}
        COMMAND
          bash -c "sed -i -e 's@\\(arg (=\\)$ENV{HOME}@\\1\\$HOME@g' ${man_file}"
        VERBATIM
      )
    endif()
    add_custom_command(
      TARGET ${tool_name}
      DEPENDS ${man_file}
      COMMAND
        ${GZIP} -f ${man_file}
    )
  endif()
endfunction()

function(add_tool_suite_man target_comp)
  if(PANDOC AND GZIP)
    set(man_file "${CMAKE_CURRENT_BINARY_DIR}/${TOOL_SUITE}.7")
    add_custom_command(
      TARGET ${target_comp}
      COMMAND
        ${PANDOC} --standalone -f gfm -t man
          --variable=title:${TOOL_SUITE}
          --variable=header:"Netmeld Tool Suite"
          --variable=section:7
          --variable=footer:${TOOL_SUITE}
          --output ${man_file}
          ${CMAKE_SOURCE_DIR}/README.md
          ${CMAKE_SOURCE_DIR}/LICENSE
      COMMAND
        ${GZIP} -f ${man_file}
    )
    install(
      FILES "${man_file}.gz"
      OPTIONAL
      COMPONENT ${target_comp}
      DESTINATION share/man/man7
    )
  endif()
endfunction()


# Define a function to extract categories
function(extract_categories tool_names)
set(dbtmp "")
set(dltmp "")
set(modtmp "")
set(tooltmp "")
set(fetchtmp "")
set(importtmp "")
    foreach(tool_name ${tool_names})
        # Split the tool name into parts using hyphen as a delimiter
        string(REGEX MATCHALL "[^\\-]+" parts ${tool_name})
        list(LENGTH parts num_parts)

        #tools in root directory
        if(NOT tool_name MATCHES "nm")
            list(APPEND modtmp "tool")
            list(APPEND tooltmp "${tool_name}")
            message("Other Tools")
        elseif (num_parts GREATER_EQUAL 3)
          list(GET parts 0 module)  # The module is the tool
          list(GET parts 1 action)  # The category is the second part
          
          #----- If we want to branch to sub sub sub commands here is wehere we have to do the logic
          #----- For now I concatinate everything after name including name
          list(SUBLIST parts 2 -1 name_parts)  # Exclude the first two parts (module abbreviation and category)
          list(JOIN name_parts "-" name)  # Join the remaining parts into a string named "name"
          
          #Setting globals
          list(APPEND modtmp "${module}")
          if(${module} STREQUAL "nmdb")
            list(APPEND dbtmp "${action}")
          endif()
          if(${module} STREQUAL "nmdl")
            list(APPEND dltmp "${action}")
          endif()
          if(${module} STREQUAL "nmfetch")
            list(APPEND fetchtmp "${action}")
          endif()

          # left off here need to figure out each name for each action aka apk for importers and so on

          if(${action} STREQUAL "import")
            list(APPEND importtmp "${name}") 
          endif()
          
          message("[3 Part] Tool: ${tool_name}, Module: ${module}, Action: ${action}, Name: ${name}")
        elseif(num_parts EQUAL 2)
          list(GET parts 0 module)  # The module of the tool
          list(GET parts 1 name)  # The name is the second part
          message("[2 Part] Tool: ${tool_name}, Module: ${module}, Name: ${name}")

          #Storage of lists
          list(APPEND modtmp "${module}")
          if(${module} STREQUAL "nmdb")
            list(APPEND dbtmp "${name}")
          endif()
          if(${module} STREQUAL "nmdl")
            list(APPEND dltmp "${name}")
          endif()
          if(${module} STREQUAL "nmfetch")
            list(APPEND fetchtmp "${name}")
          endif()

        else()
          message("Tool: ${tool_name}, Unable to extract category")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES modtmp)
    list(REMOVE_DUPLICATES dbtmp)
    list(REMOVE_DUPLICATES dltmp)
    list(REMOVE_DUPLICATES tooltmp)
    list(REMOVE_DUPLICATES fetchtmp)
    list(REMOVE_DUPLICATES importtmp)
    string(REPLACE ";" " " modules "${modtmp}") # Clean further (nm before db and dl )But also need to change nm to nmf but also need to figure out tool-
    string(REPLACE "netmeld" " " modules "${modules}")
    string(REPLACE "nm" " " modules "${modules}")
    string(REPLACE ";" " " dbActions "${dbtmp}")
    string(REPLACE ";" " " dlActions "${dltmp}")
    string(REPLACE ";" " " toolActions "${tooltmp}")
    string(REPLACE ";" " " fetchActions "${fetchtmp}")
    string(REPLACE ";" " " importNames "${importtmp}")
    set(MODULES_LIST "${modules}" PARENT_SCOPE)
    set(dbActions "${dbActions}" PARENT_SCOPE)
    set(dlActions "${dlActions}" PARENT_SCOPE)
    set(toolActions "${toolActions}" PARENT_SCOPE)
    set(fetchActions "${fetchActions}" PARENT_SCOPE)
    set(importNames "${importNames}" PARENT_SCOPE)
endfunction()


# A Function that generates the bash completion dynammically
function(generate_bash_completion_script OUTPUT_FILE MODULES dbActions dlActions toolActions fetchActions importNames)
    set(completion_script_content "# _netmeld_completion Bash completion\n\n")
    
    # Generate subcommands based on modules
    set(completion_script_content "${completion_script_content}_netmeld_completion() {\n")
    set(completion_script_content "${completion_script_content}    local cur prev\n")
    set(completion_script_content "${completion_script_content}    cur=\"\${COMP_WORDS[COMP_CWORD]}\"\n")
    set(completion_script_content "${completion_script_content}    prev=\"\${COMP_WORDS[COMP_CWORD-1]}\"\n")
    set(completion_script_content "${completion_script_content}    if [[ \${prev} == *\"netmeld-tool\"* ]]; then\n")
    set(completion_script_content "${completion_script_content}        modules=\"${MODULES}\"\n")
    set(completion_script_content "${completion_script_content}        COMPREPLY=(\$(compgen -W \"\$modules\" -- \"\${cur}\"))\n")
    set(completion_script_content "${completion_script_content}        return 0\n")
    set(completion_script_content "${completion_script_content}    fi\n")
    set(completion_script_content "${completion_script_content}\n")
    
    # Iterate through modules and for each module place the correct options
    # I can still make this dynamic with a foreach loop over the modules
    set(completion_script_content "${completion_script_content}    if [[ \${prev} == *\"db\"* ]]; then\n")
    set(completion_script_content "${completion_script_content}        sub_opts=\"${dbActions}\"\n")
    set(completion_script_content "${completion_script_content}        COMPREPLY=(\$(compgen -W \"\$sub_opts\" -- \${cur}))\n")
    set(completion_script_content "${completion_script_content}        return 0\n")
    set(completion_script_content "${completion_script_content}    fi\n")

    set(completion_script_content "${completion_script_content}    if [[ \${prev} == *\"dl\"* ]]; then\n")
    set(completion_script_content "${completion_script_content}        sub_opts=\"${dlActions}\"\n")
    set(completion_script_content "${completion_script_content}        COMPREPLY=(\$(compgen -W \"\$sub_opts\" -- \${cur}))\n")
    set(completion_script_content "${completion_script_content}        return 0\n")
    set(completion_script_content "${completion_script_content}    fi\n")

    set(completion_script_content "${completion_script_content}    if [[ \${prev} == *\"tool\"* ]]; then\n")
    set(completion_script_content "${completion_script_content}        sub_opts=\"${toolActions}\"\n")
    set(completion_script_content "${completion_script_content}        COMPREPLY=(\$(compgen -W \"\$sub_opts\" -- \${cur}))\n")
    set(completion_script_content "${completion_script_content}        return 0\n")
    set(completion_script_content "${completion_script_content}    fi\n")

    set(completion_script_content "${completion_script_content}    if [[ \${prev} == *\"fetch\"* ]]; then\n")
    set(completion_script_content "${completion_script_content}        sub_opts=\"${fetchActions}\"\n")
    set(completion_script_content "${completion_script_content}        COMPREPLY=(\$(compgen -W \"\$sub_opts\" -- \${cur}))\n")
    set(completion_script_content "${completion_script_content}        return 0\n")
    set(completion_script_content "${completion_script_content}    fi\n")

    set(completion_script_content "${completion_script_content}    if [[ \${prev} == *\"import\"* ]]; then\n")
    set(completion_script_content "${completion_script_content}        sub_opts=\"${importNames}\"\n")
    set(completion_script_content "${completion_script_content}        COMPREPLY=(\$(compgen -W \"\$sub_opts\" -- \${cur}))\n")
    set(completion_script_content "${completion_script_content}        return 0\n")
    set(completion_script_content "${completion_script_content}    fi\n")


    # Add main completion function
    set(completion_script_content "${completion_script_content}}\n\n")
    set(completion_script_content "${completion_script_content}complete -F _netmeld_completion netmeld-tool\n\n")
    
    # Write content to the output file
    file(WRITE ${OUTPUT_FILE} "${completion_script_content}")
endfunction()
