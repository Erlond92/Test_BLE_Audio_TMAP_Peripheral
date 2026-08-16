cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME BLE_Audio_TMAP_Peripheral)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        STM32_WPAN/Target/codec_if.c
        STM32_WPAN/Target/ble_codec.c
        STM32_WPAN/App/tmap_app.c
        STM32_WPAN/App/app_menu_cfg.c
        STM32_WPAN/App/AudioUseCases/usecase_dev_mgmt.c
        STM32_WPAN/App/AudioUseCases/tmap/tmap.c
        STM32_WPAN/App/AudioUseCases/tmap/tmap_db.c
        STM32_WPAN/App/app_menu.c
        STM32_WPAN/App/AudioUseCases/tmap/tmas.c
        STM32_WPAN/App/AudioUseCases/tmap/tmap_alloc.c
        STM32_WPAN/Target/ble_audio_plat.c
        Core/Src/image.c)

# Создаем интерфейсную библиотеку для ваших путей
add_library(user_includes INTERFACE)
target_include_directories(user_includes INTERFACE
        "${PROJECT_SOURCE_DIR}/STM32_WPAN/App/AudioUseCases/tmap"
        "${PROJECT_SOURCE_DIR}/Drivers/BSP/STM32WBAxx_Nucleo"
        "${PROJECT_SOURCE_DIR}/STM32_WPAN/App/AudioUseCases"
)

# Подключаем генерацию CubeMX
add_subdirectory(cmake/stm32cubemx)

# Добавляем пути к сгенерированной библиотеке (используем правильный стиль)
target_link_libraries(stm32cubemx INTERFACE user_includes)
# --------------------------------------------------------

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
        # Add user sources here\
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries (Используем чистый plain-стиль, как требует CubeMX)
target_link_libraries(${CMAKE_PROJECT_NAME}
        stm32cubemx
        user_includes
        "${PROJECT_SOURCE_DIR}/Middlewares/ST/STM32_WPAN/ble/codec/codec_manager/lib/codec_mngr.a"
        "${PROJECT_SOURCE_DIR}/Middlewares/ST/STM32_WPAN/ble/audio/lib/ble_audio.a"
        "${PROJECT_SOURCE_DIR}/Middlewares/ST/STM32_WPAN/ble/codec/lc3/lib/LC3.a"
        "${PROJECT_SOURCE_DIR}/Middlewares/ST/STM32_WPAN/link_layer/ll_cmd_lib/lib/WBA5_LinkLayer_BLE_Full_lib.a"
)
