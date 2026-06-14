@echo off
set SOLUTION_DIR=%~dp0

if not exist "%SOLUTION_DIR%compiledShaders" mkdir "%SOLUTION_DIR%compiledShaders"

for %%f in ("%SOLUTION_DIR%shaders\*.vert") do (
    echo Compiling %%f
    "%VULKAN_SDK%\Bin\glslc.exe" "%%f" -o "%SOLUTION_DIR%compiledShaders\%%~nf_vert.spv"
)

for %%f in ("%SOLUTION_DIR%shaders\*.frag") do (
    echo Compiling %%f
    "%VULKAN_SDK%\Bin\glslc.exe" "%%f" -o "%SOLUTION_DIR%compiledShaders\%%~nf_frag.spv"
)

pause