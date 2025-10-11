if(WIN32)
  # "npx" interrupts current shell script execution without the "call"
  set(npx cmd /c npx)
  set(yarn cmd /c yarn)
else()
  set(npx "npx")
  set(yarn "yarn")
endif()

# copy JS Tools package files
add_custom_command(
  OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/babel.config.js
    ${CMAKE_CURRENT_BINARY_DIR}/package.json
    ${CMAKE_CURRENT_BINARY_DIR}/yarn.lock
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/babel.config.js
    ${CMAKE_CURRENT_SOURCE_DIR}/package.json
    ${CMAKE_CURRENT_SOURCE_DIR}/yarn.lock
    ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(copyNodeApiJSToolsFiles)

# run "yarn install"
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/node_modules.sha1
  DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/package.json
    ${CMAKE_CURRENT_BINARY_DIR}/yarn.lock
  COMMAND ${yarn} install --frozen-lockfile
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(installNodeApiTestJsTools
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/node_modules.sha1
)
add_dependencies(installNodeApiTestJsTools copyNodeApiJSToolsFiles)

# add the Babel transform commands for each test JS file
set(testJSRootDir ${CMAKE_CURRENT_SOURCE_DIR})

# Collect all .js files recursively
file(GLOB_RECURSE basicsTestJSFiles "basics/*.js")
file(GLOB_RECURSE commonTestJSFiles "common/*.js")
file(GLOB_RECURSE jsNativeApiTestJSFiles "js-native-api/*.js")
set(testJSFiles
  ${basicsTestJSFiles}
  ${commonTestJSFiles}
  ${jsNativeApiTestJSFiles})

foreach(absoluteTestJSFile ${testJSFiles})
  # create target directory
  file(RELATIVE_PATH testJSFile ${testJSRootDir} ${absoluteTestJSFile})
  get_filename_component(testJSDir ${testJSFile} DIRECTORY)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${testJSDir})

  # generate Hermes-compatible JavaScript code
  add_custom_command(
    OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/${testJSFile}
      ${CMAKE_CURRENT_BINARY_DIR}/${testJSFile}.map
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${testJSFile}
    COMMAND
      ${npx}
      "babel"
      "--retain-lines"
      "--source-maps"
      "true"
      "--out-file"
      "${CMAKE_CURRENT_BINARY_DIR}/${testJSFile}"
      "--source-map-target"
      "${CMAKE_CURRENT_BINARY_DIR}/${testJSFile}.map"
      "${CMAKE_CURRENT_SOURCE_DIR}/${testJSFile}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

  # build a list of all outputs
  list(APPEND transformedJSFiles
    ${CMAKE_CURRENT_BINARY_DIR}/${testJSFile}
    ${CMAKE_CURRENT_BINARY_DIR}/${testJSFile}.map
  )
endforeach()

# run the Babel transforms for all required output files
add_custom_target(transformJSFiles
  DEPENDS ${transformedJSFiles}
)
add_dependencies(transformJSFiles installNodeApiTestJsTools)

add_subdirectory(js-native-api)
