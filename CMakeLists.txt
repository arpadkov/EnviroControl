cmake_minimum_required(VERSION 3.16)

project("EnviroControl")

# --- Python Script Copying ---
# Define the base source directory for your Python scripts
set(PYTHON_SCRIPTS_BASE_DIR "${CMAKE_SOURCE_DIR}/PyDHT22Reader")

# Define a list of Python script files to copy (relative to PYTHON_SCRIPTS_BASE_DIR)
set(PYTHON_SCRIPTS_TO_COPY
    "main.py"
)

# Add the subdirectory first, so that the executable target is defined.
add_subdirectory("EnviroControl")

# --- Python Script Copying ---
# Create a custom target that depends on your main executable
# This ensures that the main executable is built *before* this custom target runs.
add_custom_target(CopyPythonScripts ALL
    DEPENDS EnviroControl # This target depends on the main C++ executable
    COMMENT "Copying Python scripts..."
)

# Loop through the list of Python scripts and add commands to the custom target.
foreach(script_file IN LISTS PYTHON_SCRIPTS_TO_COPY)
    # Define source and destination paths for each script
    set(SOURCE_PATH "${PYTHON_SCRIPTS_BASE_DIR}/${script_file}")
    set(DEST_PATH "$<TARGET_FILE_DIR:EnviroControl>/${script_file}")

    # Add each copy command to the custom target
    add_custom_command(
        TARGET CopyPythonScripts # This command belongs to the custom target
        POST_BUILD # This specific POST_BUILD means it runs after CopyPythonScripts itself is built (which is after EnviroControl)
        COMMAND ${CMAKE_COMMAND} -E copy
                "${SOURCE_PATH}"
                "${DEST_PATH}"
        COMMENT "Copying Python script ${script_file} to build directory"
    )
endforeach()