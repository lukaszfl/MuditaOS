include_directories( ${CMAKE_PROJECT_NAME}

	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}"
)

include_directories( ${PROJECT_NAME}

	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}"
)

message( "${PROJECT_NAME}  ${CMAKE_CURRENT_LIST_DIR}" )

target_sources( ${PROJECT_NAME}

	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/ServicePresage.cpp"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/ServicePresage.hpp"
		"${CMAKE_CURRENT_LIST_DIR}/messages/PresageMessage.hpp"
)

