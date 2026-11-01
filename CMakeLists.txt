cmake_minimum_required(VERSION 3.20)
project(HeavyMachRestoredCore LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(HeavyMachCore Sources/Core/SaveData.cpp Sources/Core/GlobalInfo.cpp Sources/Core/Geometry.cpp Sources/Core/Motion.cpp Sources/Core/TitleState.cpp Sources/Core/GameData.cpp Sources/Core/VTOL.cpp Sources/Core/Fighters.cpp Sources/Core/GroundAA.cpp Sources/Core/Combat.cpp Sources/Core/Enemy.cpp Sources/Core/Visuals.cpp Sources/Core/Player.cpp Sources/Core/World.cpp Sources/Core/ItemBox.cpp Sources/Core/GameSession.cpp Sources/Core/BulletVisual.cpp Sources/Core/Trail.cpp Sources/Core/TrailRaster.cpp Sources/Core/SpriteRaster.cpp Sources/Core/CameraShake.cpp Sources/Core/LoadingTransition.cpp)
# Keep software pixel loops interactive in Debug without optimizing gameplay
# or disabling assertions. Native Xcode targets use the same per-file -O2.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  set_source_files_properties(Sources/Core/SpriteRaster.cpp Sources/Core/TrailRaster.cpp PROPERTIES COMPILE_OPTIONS "-O2")
endif()
target_include_directories(HeavyMachCore PUBLIC Sources/Core)
target_compile_options(HeavyMachCore PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)
add_executable(HeavyMachCoreTests Tests/CoreTests.cpp Tests/GeometryTests.cpp Tests/MotionTests.cpp Tests/GameDataTests.cpp Tests/EnemyTests.cpp Tests/PlayerTests.cpp Tests/ItemBoxTests.cpp Tests/BulletVisualTests.cpp Tests/TrailTests.cpp Tests/TrailRasterTests.cpp Tests/CameraShakeTests.cpp Tests/LoadingTransitionTests.cpp Tests/TitleStateTests.cpp)
target_link_libraries(HeavyMachCoreTests PRIVATE HeavyMachCore)
enable_testing()
add_test(NAME HeavyMachCoreTests COMMAND HeavyMachCoreTests)

foreach(domain VTOL Fighters GroundAA GroundAARuntime Combat Visuals World StageTransition GameSession AmmoSync AudioContract PlayerGauge EnemyGauge PassiveContinuity Survival CameraIntegration TextureFormat SpriteRaster HUDLayout)
  add_executable(HeavyMach${domain}Tests Tests/${domain}Tests.cpp)
  target_link_libraries(HeavyMach${domain}Tests PRIVATE HeavyMachCore)
  add_test(NAME HeavyMach${domain}Tests COMMAND HeavyMach${domain}Tests)
endforeach()

add_executable(HeavyMachHMButtonStateTests Tests/HMButtonStateTests.cpp)
target_compile_options(HeavyMachHMButtonStateTests PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)
add_test(NAME HeavyMachHMButtonStateTests COMMAND HeavyMachHMButtonStateTests)

add_executable(HeavyMachGameButtonsTests Tests/GameButtonsTests.cpp)
target_compile_options(HeavyMachGameButtonsTests PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)
add_test(NAME HeavyMachGameButtonsTests COMMAND HeavyMachGameButtonsTests)


# Separate copy of the real core: validation entry points never enter HeavyMachCore.
option(HM_BUILD_VALIDATION_TESTS "Build isolated validation scenario runner" ON)
if(HM_BUILD_VALIDATION_TESTS)
  get_target_property(HM_CORE_SOURCES HeavyMachCore SOURCES)
  add_library(HeavyMachValidationCore ${HM_CORE_SOURCES} Sources/Core/ValidationScenario.cpp)
  target_include_directories(HeavyMachValidationCore PUBLIC Sources/Core)
  target_compile_definitions(HeavyMachValidationCore PUBLIC HM_VALIDATION=1)
  target_compile_options(HeavyMachValidationCore PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)
  add_executable(HeavyMachValidationScenarioTests Tests/ValidationScenarioTests.cpp)
  target_link_libraries(HeavyMachValidationScenarioTests PRIVATE HeavyMachValidationCore)
  add_test(NAME HeavyMachValidationScenarioTests COMMAND HeavyMachValidationScenarioTests)
  add_executable(HeavyMachValidationBossTests Tests/ValidationBossTests.cpp)
  target_link_libraries(HeavyMachValidationBossTests PRIVATE HeavyMachValidationCore)
  add_executable(HeavyMachValidationJourneyTests Tests/ValidationJourneyTests.cpp)
  target_link_libraries(HeavyMachValidationJourneyTests PRIVATE HeavyMachValidationCore)
  # Extended gameplay runner: explicit execution retains per-boss results.
  # It is not a smoke-test substitute for completing every encounter.
endif()

add_executable(HeavyMachKeyboardInputTests Tests/KeyboardInputTests.cpp)
target_link_libraries(HeavyMachKeyboardInputTests PRIVATE HeavyMachCore)
add_test(NAME HeavyMachKeyboardInputTests COMMAND HeavyMachKeyboardInputTests)

add_executable(HeavyMachAutoAimTests Tests/AutoAimTests.cpp)
target_link_libraries(HeavyMachAutoAimTests PRIVATE HeavyMachCore)
add_test(NAME HeavyMachAutoAimTests COMMAND HeavyMachAutoAimTests ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(HeavyMachManualCombatTrace Tests/ManualCombatTrace.cpp)
target_link_libraries(HeavyMachManualCombatTrace PRIVATE HeavyMachCore)
