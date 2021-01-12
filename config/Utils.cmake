# TODO: add module header info

function(multicomp_install)
    list(FIND ARGN "COMPONENTS" KEYWORD_INDEX)
    list(SUBLIST ARGN 0 ${KEYWORD_INDEX} INSTALL_ARGN)
    math(EXPR COMPS_INDEX "${KEYWORD_INDEX}+1")
    list(SUBLIST ARGN ${COMPS_INDEX} ${ARGC} COMPONENTS)
    foreach(COMP IN LISTS COMPONENTS)
        install(${INSTALL_ARGN} COMPONENT ${COMP})
    endforeach()
endfunction()
