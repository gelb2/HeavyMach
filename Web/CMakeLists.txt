# HeavyMach WebAssembly build (normal app, HM_VALIDATION=0).
#   emcmake cmake -S Web -B build/web -DCMAKE_BUILD_TYPE=Release
#   cmake --build build/web
# Output: build/web/dist/{index.html,index.js,index.wasm,hm-assets.js,hm-assets-<n>.data}
cmake_minimum_required(VERSION 3.20)
project(HeavyMachWeb LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "Configure with emcmake (Emscripten toolchain).")
endif()
get_filename_component(HM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

# Same core sources as the root CMakeLists HeavyMachCore target; no web copy of game rules.
set(HM_CORE_SOURCES SaveData GlobalInfo Geometry Motion TitleState GameData VTOL Fighters GroundAA Combat Enemy Visuals Player World ItemBox GameSession BulletVisual Trail TrailRaster SpriteRaster CameraShake LoadingTransition)
list(TRANSFORM HM_CORE_SOURCES PREPEND "${HM_ROOT}/Sources/Core/")
list(TRANSFORM HM_CORE_SOURCES APPEND ".cpp")
set(HM_WEB_SOURCES WebAudio WebGLRenderer WebMedia WebMenus WebGameHUD WebGameView WebMain)
list(TRANSFORM HM_WEB_SOURCES PREPEND "${HM_ROOT}/Sources/Web/")
list(TRANSFORM HM_WEB_SOURCES APPEND ".cpp")

set(HM_COMMON_FLAGS -fwasm-exceptions -ffp-contract=off)
add_executable(index ${HM_CORE_SOURCES} ${HM_WEB_SOURCES})
target_include_directories(index PRIVATE "${HM_ROOT}/Sources/Core" "${HM_ROOT}/Sources/Web" "${HM_ROOT}/Sources/iOS")
target_compile_definitions(index PRIVATE HM_VALIDATION=0)
target_compile_options(index PRIVATE ${HM_COMMON_FLAGS} -Wall -Wextra)
set_source_files_properties("${HM_ROOT}/Sources/Core/SpriteRaster.cpp" "${HM_ROOT}/Sources/Core/TrailRaster.cpp" PROPERTIES COMPILE_OPTIONS "-O2")
set_target_properties(index PROPERTIES SUFFIX ".html" RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/dist")

# Packaged assets keep original file names. Only product-consumed roots are included.
# Web/tools/package_assets.py splits them into data files under 20 MiB, because
# static hosts such as Cloudflare Pages reject files over 25 MiB. The virtual
# paths (/assets/...) and file contents are the same as one --preload-file package.
set(HM_ASSETS
  "${HM_ROOT}/Resources/OriginalImages@/assets/OriginalImages"
  "${HM_ROOT}/Resources/Audio@/assets/Audio"
  "${HM_ROOT}/Resources/RawData@/assets/RawData"
  "${HM_ROOT}/Art/Extensions@/assets/Extensions")
file(GLOB_RECURSE HM_ASSET_FILES CONFIGURE_DEPENDS
  "${HM_ROOT}/Resources/OriginalImages/*" "${HM_ROOT}/Resources/Audio/*"
  "${HM_ROOT}/Resources/RawData/*" "${HM_ROOT}/Art/Extensions/*")
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(HM_PACKAGER "${EMSCRIPTEN_ROOT_PATH}/tools/file_packager.py")
add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/dist/hm-assets.js" "${CMAKE_BINARY_DIR}/dist/hm-assets.json"
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/package_assets.py"
    --packager "${HM_PACKAGER}" --out "${CMAKE_BINARY_DIR}/dist" --cap-mib 20 ${HM_ASSETS}
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tools/package_assets.py" ${HM_ASSET_FILES}
  COMMENT "Packaging HeavyMach web assets"
  VERBATIM)
add_custom_target(hm_assets ALL DEPENDS "${CMAKE_BINARY_DIR}/dist/hm-assets.js")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/manifest.webmanifest" "${CMAKE_BINARY_DIR}/dist/manifest.webmanifest" COPYONLY)
add_dependencies(index hm_assets)
target_link_options(index PRIVATE ${HM_COMMON_FLAGS}
  "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
  -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
  -sALLOW_MEMORY_GROWTH=1 -sINITIAL_MEMORY=134217728
  -sFORCE_FILESYSTEM=1 -lidbfs.js -lopenal
  -sEXPORTED_FUNCTIONS=_main,_hm_resize,_hm_pointer,_hm_key,_hm_keyboard_blur,_hm_motion,_hm_active,_hm_hud_control,_hm_debug_state,_hm_context_lost,_hm_context_restored,_hm_hud_save_result
  -sEXPORTED_RUNTIME_METHODS=FS,IDBFS,UTF8ToString,addRunDependency,removeRunDependency,FS_createPath,FS_createDataFile
  -sENVIRONMENT=web)
set_property(TARGET index APPEND PROPERTY LINK_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shell.html")
set_source_files_properties("${HM_ROOT}/Sources/Web/WebGameView.cpp" PROPERTIES OBJECT_DEPENDS "${HM_ROOT}/Sources/Web/WebHUDEditor.inc")
