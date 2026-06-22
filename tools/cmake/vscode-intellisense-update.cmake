# Ensure .vscode directory exists
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/.vscode")

# Generate c_cpp_properties.json automatically
set(C_CPP_PROPERTIES_FILE "${CMAKE_SOURCE_DIR}/.vscode/c_cpp_properties.json")

file(WRITE "${C_CPP_PROPERTIES_FILE}"
"{
    \"configurations\": [
        {
            \"name\": \"${CMAKE_SYSTEM_NAME}\",
            \"compileCommands\": \"${CMAKE_BINARY_DIR}/compile_commands.json\",
            \"intelliSenseMode\": \"linux-gcc-x64\",
            \"compilerPath\": \"/usr/bin/gcc-14\",
            \"cppStandard\": \"c++23\",
            \"cStandard\": \"c23\",
            \"compilerArgs\": [\"-D_GNU_SOURCE\"],
            \"includePath\": [
                \"${CMAKE_SOURCE_DIR}/**\"
            ]
        }
    ],
    \"version\": 4
}")