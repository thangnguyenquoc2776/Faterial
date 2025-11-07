cmake_minimum_required(VERSION 3.20)

# ================== Project meta ==================
set(APP_NAME Faterial)
project(${APP_NAME} C CXX)

# Chuẩn C/C++
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ================== MSVC runtime & common defs ==================
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX)
    add_compile_options(/MP /utf-8)
endif()

# ================== Cocos2d-x engine ==================
if(NOT DEFINED BUILD_ENGINE_DONE)
    set(COCOS2DX_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cocos2d)
    set(CMAKE_MODULE_PATH ${COCOS2DX_ROOT_PATH}/cmake/Modules/)
    include(CocosBuildSet)
    add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos ${ENGINE_BINARY_PATH}/cocos/core)
endif()

# ================== Resources ==================
set(GAME_RES_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
if(APPLE OR WINDOWS)
    cocos_mark_multi_resources(common_res_files RES_TO "Resources" FOLDERS ${GAME_RES_FOLDER})
endif()

# ===== Helper: chỉ append file nếu tồn tại (cho optional scenes) =====
function(append_if_exists outvar)
    foreach(f IN LISTS ARGN)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${f}")
            list(APPEND ${outvar} ${f})
        endif()
    endforeach()
    set(${outvar} "${${outvar}}" PARENT_SCOPE)
endfunction()

# ================== Game headers & sources ==================
set(GAME_HEADER
    # core
    Classes/core/AppDelegate.h
    Classes/core/CrashGuard.h

    # physics
    Classes/physics/PhysicsDefs.h

    # ui
    Classes/ui/HUDLayer.h
    Classes/ui/PauseLayer.h            # << thêm

    # scenes
    Classes/scenes/MenuScene.h
    Classes/scenes/GameScene.h
    # optional scenes (tự động nếu có)
)
append_if_exists(GAME_HEADER
    Classes/scenes/SettingsScene.h
    Classes/scenes/CreditsScene.h
    Classes/scenes/EndScene.h
)

set(GAME_SRC
    # core
    Classes/core/AppDelegate.cpp

    # ui
    Classes/ui/HUDLayer.cpp
    Classes/ui/PauseLayer.cpp          # << thêm (QUAN TRỌNG)

    # scenes
    Classes/scenes/MenuScene.cpp
    Classes/scenes/GameScene.cpp
    # optional scenes (tự động nếu có)
)
append_if_exists(GAME_SRC
    Classes/scenes/SettingsScene.cpp
    Classes/scenes/CreditsScene.cpp
    Classes/scenes/EndScene.cpp
)

# game base
list(APPEND GAME_HEADER
    Classes/game/Entity.h
    Classes/game/Player.h
    Classes/game/Enemy.h
    Classes/audio/Sound.h
)
list(APPEND GAME_SRC
    Classes/game/Entity.cpp
    Classes/game/Player.cpp
    Classes/game/Enemy.cpp
    Classes/audio/Sound.cpp
)

# enemies / bosses
list(APPEND GAME_HEADER
    Classes/game/enemies/Goomba.h
    Classes/game/enemies/Spiker.h
    Classes/game/bosses/BossGolem.h
)
list(APPEND GAME_SRC
    Classes/game/enemies/Goomba.cpp
    Classes/game/enemies/Spiker.cpp
    Classes/game/bosses/BossGolem.cpp
)

# objects
list(APPEND GAME_HEADER
    Classes/game/objects/Coin.h
    Classes/game/objects/Star.h
    Classes/game/objects/Upgrade.h
    Classes/game/objects/Crate.h
    Classes/game/objects/Gate.h
    Classes/game/objects/PressurePlate.h
    Classes/game/objects/Chest.h
)
list(APPEND GAME_SRC
    Classes/game/objects/Coin.cpp
    Classes/game/objects/Star.cpp
    Classes/game/objects/Upgrade.cpp
    Classes/game/objects/Crate.cpp
    Classes/game/objects/Gate.cpp
    Classes/game/objects/PressurePlate.cpp
    Classes/game/objects/Chest.cpp
)

# weapon
list(APPEND GAME_HEADER
    Classes/game/weapon/Bullet.h
    Classes/game/weapon/Slash.h
)
list(APPEND GAME_SRC
    Classes/game/weapon/Bullet.cpp
    Classes/game/weapon/Slash.cpp
)

# loot & map
list(APPEND GAME_HEADER
    Classes/game/loot/LootTable.h
    Classes/game/map/LevelBuilder.h
)
list(APPEND GAME_SRC
    Classes/game/loot/LootTable.cpp
    Classes/game/map/LevelBuilder.cpp
)

# ================== Platform glue ==================
if(ANDROID)
    list(APPEND GAME_SRC proj.android/app/jni/hellocpp/main.cpp)

elseif(LINUX)
    list(APPEND GAME_SRC proj.linux/main.cpp)

elseif(WINDOWS)
    list(APPEND GAME_HEADER
        proj.win32/main.h
        proj.win32/resource.h
    )
    list(APPEND GAME_SRC
        proj.win32/main.cpp
        proj.win32/game.rc
        ${common_res_files}
    )

elseif(APPLE)
    if(IOS)
        list(APPEND GAME_HEADER
            proj.ios_mac/ios/AppController.h
            proj.ios_mac/ios/RootViewController.h
        )
        set(APP_UI_RES
            proj.ios_mac/ios/LaunchScreen.storyboard
            proj.ios_mac/ios/LaunchScreenBackground.png
            proj.ios_mac/ios/Images.xcassets
        )
        list(APPEND GAME_SRC
            proj.ios_mac/ios/main.m
            proj.ios_mac/ios/AppController.mm
            proj.ios_mac/ios/RootViewController.mm
            proj.ios_mac/ios/Prefix.pch
            ${APP_UI_RES}
        )
    elseif(MACOSX)
        set(APP_UI_RES
            proj.ios_mac/mac/Icon.icns
            proj.ios_mac/mac/Info.plist
        )
        list(APPEND GAME_SRC
            proj.ios_mac/mac/main.cpp
            proj.ios_mac/mac/Prefix.pch
            ${APP_UI_RES}
        )
    endif()
    list(APPEND GAME_SRC ${common_res_files})
endif()

# ================== Target ==================
set(all_code_files ${GAME_HEADER} ${GAME_SRC})

if(NOT ANDROID)
    add_executable(${APP_NAME} ${all_code_files})
else()
    add_library(${APP_NAME} SHARED ${all_code_files})
    add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos/platform/android ${ENGINE_BINARY_PATH}/cocos/platform)
    target_link_libraries(${APP_NAME} PRIVATE -Wl,--whole-archive cpp_android_spec -Wl,--no-whole-archive)
endif()

# ================== Include paths ==================
target_include_directories(${APP_NAME}
    PRIVATE Classes
    PRIVATE Classes/core
    PRIVATE Classes/physics
    PRIVATE Classes/ui
    PRIVATE Classes/scenes
    PRIVATE Classes/game
    PRIVATE Classes/game/enemies
    PRIVATE Classes/game/bosses
    PRIVATE Classes/game/objects
    PRIVATE Classes/game/weapon
    PRIVATE Classes/game/loot
    PRIVATE Classes/game/map
    PRIVATE ${COCOS2DX_ROOT_PATH}/cocos/audio/include/  # AudioEngine
)

# ================== Link engine + system libs ==================
target_link_libraries(${APP_NAME} PRIVATE cocos2d)
if (MSVC)
    target_link_options(${APP_NAME} PRIVATE /NODEFAULTLIB:LIBCMT /IGNORE:4098)
    target_link_libraries(${APP_NAME} PRIVATE winmm DbgHelp legacy_stdio_definitions)
endif()

# ================== App config & copy resources/DLL ==================
setup_cocos_app_config(${APP_NAME})

# Phát hiện optional scenes để bật macro cho MenuScene
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Classes/scenes/SettingsScene.h")
    target_compile_definitions(${APP_NAME} PRIVATE USE_SETTINGS_SCENE=1)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Classes/scenes/CreditsScene.h")
    target_compile_definitions(${APP_NAME} PRIVATE USE_CREDITS_SCENE=1)
endif()

# Ép exe & dll vào build/bin (không tạo Debug/Release con)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
    set_target_properties(${APP_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/bin
        LIBRARY_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/bin
        ARCHIVE_OUTPUT_DIRECTORY_${cfg} ${CMAKE_BINARY_DIR}/lib
    )
endforeach()

# Windows: copy DLL engine (glew, glfw, openal, …) về cạnh .exe
if(WINDOWS)
    cocos_copy_target_dll(${APP_NAME})
endif()

# Copy Resources vào cạnh .exe (Windows/Linux/macOS)
if(LINUX OR WINDOWS)
    cocos_get_resource_path(APP_RES_DIR ${APP_NAME})
    cocos_copy_target_res(${APP_NAME} LINK_TO ${APP_RES_DIR} FOLDERS ${GAME_RES_FOLDER})
endif()

# (macOS/iOS) gán tài nguyên UI
if(APPLE)
    set_target_properties(${APP_NAME} PROPERTIES RESOURCE "${APP_UI_RES}")
    if(MACOSX)
        set_xcode_property(${APP_NAME} INFOPLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/mac/Info.plist")
    elseif(IOS)
        set_xcode_property(${APP_NAME} INFOPLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/ios/Info.plist")
        set_xcode_property(${APP_NAME} ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")
    endif()
endif()
