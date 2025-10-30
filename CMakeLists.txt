cmake_minimum_required(VERSION 3.20)

# ================== Project meta ==================
set(APP_NAME Faterial)
project(${APP_NAME} C CXX)

# ================== MSVC runtime & common defs ==================
if (MSVC)
    # /MD (Release) và /MDd (Debug)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX)
    add_compile_options(/MP)
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

# ================== Game headers & sources ==================
set(GAME_HEADER
    # core
    Classes/core/AppDelegate.h
    Classes/core/CrashGuard.h

    # physics
    Classes/physics/PhysicsDefs.h

    # ui
    Classes/ui/HUDLayer.h

    # scenes
    Classes/scenes/MenuScene.h
    Classes/scenes/GameScene.h

    # game base
    Classes/game/Entity.h
    Classes/game/Player.h
    Classes/game/Enemy.h

    # enemies / bosses / objects
    Classes/game/enemies/Goomba.h
    Classes/game/enemies/Spiker.h
    Classes/game/bosses/BossGolem.h
    Classes/game/objects/Coin.h
    Classes/game/objects/Star.h
    Classes/game/objects/Upgrade.h
    Classes/game/objects/Crate.h
    Classes/game/objects/Gate.h
    Classes/game/objects/PressurePlate.h

    # weapon (NEW)
    Classes/game/weapon/Bullet.h
    Classes/game/weapon/Slash.h
)

set(GAME_SRC
    # core
    Classes/core/AppDelegate.cpp

    # ui
    Classes/ui/HUDLayer.cpp

    # scenes
    Classes/scenes/MenuScene.cpp
    Classes/scenes/GameScene.cpp

    # game base
    Classes/game/Entity.cpp
    Classes/game/Player.cpp
    Classes/game/Enemy.cpp

    # enemies / bosses / objects
    Classes/game/enemies/Goomba.cpp
    Classes/game/enemies/Spiker.cpp
    Classes/game/bosses/BossGolem.cpp
    Classes/game/objects/Coin.cpp
    Classes/game/objects/Star.cpp
    Classes/game/objects/Upgrade.cpp
    Classes/game/objects/Crate.cpp
    Classes/game/objects/Gate.cpp
    Classes/game/objects/PressurePlate.cpp

    # weapon (NEW)
    Classes/game/weapon/Bullet.cpp
    Classes/game/weapon/Slash.cpp
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

# ================== Target (tạo trước, rồi mới target_*) ==================
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
    PRIVATE Classes/game/weapon        # << NEW
    PRIVATE ${COCOS2DX_ROOT_PATH}/cocos/audio/include/  # AudioEngine
)

# ================== Link engine + system libs ==================
target_link_libraries(${APP_NAME} PRIVATE cocos2d)

if (MSVC)
    target_link_options(${APP_NAME} PRIVATE /NODEFAULTLIB:LIBCMT /IGNORE:4098)
    target_link_libraries(${APP_NAME} PRIVATE winmm DbgHelp legacy_stdio_definitions)
endif()

# ================== App config & copy resources ==================
setup_cocos_app_config(${APP_NAME})

if(APPLE)
    set_target_properties(${APP_NAME} PROPERTIES RESOURCE "${APP_UI_RES}")
    if(MACOSX)
        set_xcode_property(${APP_NAME} INFOPLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/mac/Info.plist")
    elseif(IOS)
        set_xcode_property(${APP_NAME} INFOPLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/ios/Info.plist")
        set_xcode_property(${APP_NAME} ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")
    endif()
elseif(WINDOWS)
    cocos_copy_target_dll(${APP_NAME})
endif()

if(LINUX OR WINDOWS)
    cocos_get_resource_path(APP_RES_DIR ${APP_NAME})
    cocos_copy_target_res(${APP_NAME} LINK_TO ${APP_RES_DIR} FOLDERS ${GAME_RES_FOLDER})
endif()

# Tuỳ chọn:
# set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 17)
# set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
