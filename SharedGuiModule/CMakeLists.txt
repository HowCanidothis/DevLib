project(SharedGuiModule)

includeAll()

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRC})
