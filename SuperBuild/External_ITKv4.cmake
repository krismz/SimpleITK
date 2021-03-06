
#-----------------------------------------------------------------------------
# Get and build itk

set(ITK_LANGUAGES_VARS
      PYTHON_EXECUTABLE
      PYTHON_INCLUDE_DIR
      )

VariableListToCache( ITK_LANGUAGES_VARS  ep_languages_cache )
VariableListToArgs( ITK_LANGUAGES_VARS  ep_languages_args )


set(proj ITK)  ## Use ITK convention of calling it ITK
set(ITK_REPOSITORY git://itk.org/ITK.git)

# NOTE: it is very important to update the ITK_DIR path with the
# current version of ITK
set(ITK_TAG_COMMAND GIT_TAG v4.1.0 )

if( ${ITK_WRAPPING} OR ${BUILD_SHARED_LIBS} )
  set( ITK_BUILD_SHARED_LIBS ON )
else()
  set( ITK_BUILD_SHARED_LIBS OFF )
endif()


file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${proj}-build/CMakeCacheInit.txt" "${ep_languages_cache}\n${ep_common_cache}" )

ExternalProject_Add(${proj}
  GIT_REPOSITORY ${ITK_REPOSITORY}
  ${ITK_TAG_COMMAND}
  UPDATE_COMMAND ""
  SOURCE_DIR ${proj}
  BINARY_DIR ${proj}-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
  --no-warn-unused-cli
  -C "${CMAKE_CURRENT_BINARY_DIR}/${proj}-build/CMakeCacheInit.txt"
  ${ep_common_args}
  ${ep_languages_args}
  -DBUILD_EXAMPLES:BOOL=OFF
  -DBUILD_TESTING:BOOL=OFF
  -DBUILD_SHARED_LIBS:BOOL=${ITK_BUILD_SHARED_LIBS}
  -DCMAKE_SKIP_RPATH:BOOL=ON
  -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  -DITK_LEGACY_REMOVE:BOOL=ON
  -DITK_BUILD_ALL_MODULES:BOOL=ON
  -DITK_USE_REVIEW:BOOL=ON
  -DUSE_WRAP_ITK:BOOL=${ITK_WRAPPING}
  -DINSTALL_WRAP_ITK_COMPATIBILITY:BOOL=OFF
  -DWRAP_float:BOOL=ON
  -DWRAP_unsigned_char:BOOL=ON
  -DWRAP_signed_short:BOOL=ON
  -DWRAP_unsigned_short:BOOL=ON
  -DWRAP_complex_float:BOOL=ON
  -DWRAP_vector_float:BOOL=ON
  -DWRAP_covariant_vector_float:BOOL=ON
  -DWRAP_rgb_signed_short:BOOL=ON
  -DWRAP_rgb_unsigned_char:BOOL=ON
  -DWRAP_rgb_unsigned_short:BOOL=ON
  -DWRAP_ITK_TCL:BOOL=OFF
  -DWRAP_ITK_JAVA:BOOL=OFF
  -DWRAP_ITK_PYTHON:BOOL=ON
  ${ITK_PYTHON_ARGS}
  ${FFTW_FLAGS}
  BUILD_COMMAND ${BUILD_COMMAND_STRING}
  DEPENDS
  ${ITK_DEPENDENCIES}
  )


ExternalProject_Get_Property(ITK install_dir)
set(ITK_DIR "${install_dir}/lib/cmake/ITK-4.1" )
set(WrapITK_DIR "${install_dir}/lib/cmake/ITK-4.1/WrapITK")
