#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

###################################################################################################################

# Select your developer board by removing the # tag from the beginning of the line

# The default board selected is the AVNET Starter Kit.

# If you are NOT using the AVNET board be sure to comment out the AVNET board

set(AVNET TRUE "AVNET Azure Sphere Starter Kit")                
# set(SEEED_STUDIO_RDB TRUE "Seeed Studio Azure Sphere MT3620 Development Kit (aka Reference Design Board or rdb)")
# set(SEEED_STUDIO_MINI TRUE "Seeed Studio Azure Sphere MT3620 Mini Dev Board")

###################################################################################################################

cmake_minimum_required (VERSION 3.10)
project (AzureSphereIoTCentral C)

azsphere_configure_tools(TOOLS_REVISION "20.07")
azsphere_configure_api(TARGET_API_SET "7")


add_subdirectory("../LearningPathLibrary" out)

set(Source
    "main.c"
)
source_group("Source" FILES ${Source})

if(AVNET)

    set(Oem

        "../LearningPathLibrary/AVNET/board.c"
    )
    source_group("Oem" FILES ${Oem})    

    add_definitions( -DOEM_AVNET=TRUE )

endif(AVNET)

if(SEEED_STUDIO_RDB OR SEEED_STUDIO_MINI)

    set(Oem
        "../LearningPathLibrary/SEEED_STUDIO/board.c"
    )
    source_group("Oem" FILES ${Oem})

    add_definitions( -DOEM_SEEED_STUDIO=TRUE )

endif(SEEED_STUDIO_RDB OR SEEED_STUDIO_MINI)

set(ALL_FILES
    ${Source}
    ${Oem}
)

# Create executable
add_executable(${PROJECT_NAME} ${ALL_FILES})

target_compile_definitions(${PROJECT_NAME} PUBLIC AZURE_IOT_HUB_CONFIGURED)
target_link_libraries(${PROJECT_NAME} applibs pthread gcc_s c azsphere_libs )

target_include_directories(${PROJECT_NAME} PUBLIC ../LearningPathLibrary )

if(AVNET)
    add_subdirectory("../Drivers/AVNET_SK/HighLevel" drivers)
    target_link_libraries(${PROJECT_NAME} avnet_drivers)
    target_include_directories(${PROJECT_NAME} PUBLIC 
                            ../LearningPathLibrary/AVNET
                            ../Drivers/AVNET_SK/HighLevel
                            ../Drivers/AVNET_SK/Common 
                            )
    azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../HardwareDefinitions/avnet_mt3620_sk" TARGET_DEFINITION "azure_sphere_learning_path.json")
endif(AVNET)

if (SEEED_STUDIO_RDB)
    target_include_directories(${PROJECT_NAME} PUBLIC ../LearningPathLibrary/SEEED_STUDIO)
    azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../HardwareDefinitions/mt3620_rdb" TARGET_DEFINITION "azure_sphere_learning_path.json")
endif()

if (SEEED_STUDIO_MINI)
    target_include_directories(${PROJECT_NAME} PUBLIC ../LearningPathLibrary/SEEED_STUDIO)
    azsphere_target_hardware_definition(${PROJECT_NAME} TARGET_DIRECTORY "../HardwareDefinitions/seeed_mt3620_mdb" TARGET_DEFINITION "azure_sphere_learning_path.json")
endif(SEEED_STUDIO_MINI)

azsphere_target_add_image_package(${PROJECT_NAME})