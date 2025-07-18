
include(ExternalProject)

if (ENABLE_NNDEPLOY_INFERENCE_SNPE STREQUAL "OFF")
elseif (ENABLE_NNDEPLOY_INFERENCE_SNPE STREQUAL "ON")
else()
  include_directories(${ENABLE_NNDEPLOY_INFERENCE_SNPE}/include/zdl)
  include_directories(${ENABLE_NNDEPLOY_INFERENCE_SNPE}/include/zdl/DiagLog)
  include_directories(${ENABLE_NNDEPLOY_INFERENCE_SNPE}/include/zdl/DlContainer)
  include_directories(${ENABLE_NNDEPLOY_INFERENCE_SNPE}/include/zdl/DlSystem)
  include_directories(${ENABLE_NNDEPLOY_INFERENCE_SNPE}/include/zdl/PlatformValidator)
  include_directories(${ENABLE_NNDEPLOY_INFERENCE_SNPE}/include/zdl/SNPE)
  include_directories(${ENABLE_NNDEPLOY_INFERENCE_SNPE}/include/zdl/SnpeUdo)
  set(LIB_PATH ${ENABLE_NNDEPLOY_INFERENCE_SNPE}/lib)
  set(LIBS "SNPE")
  foreach(LIB ${LIBS})
    set(LIB_NAME ${NNDEPLOY_LIB_PREFIX}${LIB}${NNDEPLOY_LIB_SUFFIX})
    set(FULL_LIB_NAME ${LIB_PATH}/${LIB_NAME})
    set(NNDEPLOY_THIRD_PARTY_LIBRARY ${NNDEPLOY_THIRD_PARTY_LIBRARY} ${FULL_LIB_NAME})
  endforeach()
  # file(GLOB_RECURSE INSTALL_LIBS "${LIB_PATH}/*")
  # foreach(INSTALL_LIB ${INSTALL_LIBS})
  #   install(FILES ${INSTALL_LIB} DESTINATION ${NNDEPLOY_INSTALL_LIB_PATH})
  # endforeach()
  install(DIRECTORY ${ENABLE_NNDEPLOY_INFERENCE_SNPE} DESTINATION ${NNDEPLOY_INSTALL_THIRD_PARTY_PATH})
endif()