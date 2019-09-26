include_directories( ${CMAKE_PROJECT_NAME}

	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}"
)

include_directories( ${PROJECT_NAME}

	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}"
)

target_sources( ${PROJECT_NAME}

	PRIVATE
		"${CMAKE_CURRENT_LIST_DIR}/ApplicationMessages.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/ThreadsModel.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/widgets/ThreadItem.cpp"
		#"${CMAKE_CURRENT_LIST_DIR}/widgets/MessagesListView.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/windows/MessagesMainWindow.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/windows/ThreadViewWindow.cpp"
	PUBLIC
		"${CMAKE_CURRENT_LIST_DIR}/ThreadsModel.hpp"
		"${CMAKE_CURRENT_LIST_DIR}/ApplicationMessages.hpp"
		#"${CMAKE_CURRENT_LIST_DIR}/widgets/MessagesListView.hpp"
		"${CMAKE_CURRENT_LIST_DIR}/widgets/Threadtem.hpp"
        #"${CMAKE_CURRENT_LIST_DIR}/windows/MessagesNewContact.hpp"
)

