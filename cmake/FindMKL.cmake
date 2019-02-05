

set(MKLLIB "C:/Program Files (x86)/IntelSWTools/compilers_and_libraries_2019.2.190/windows/mkl")

    find_path(MKL_INCLUDE_DIR
        mkl.h
        PATHS
            /usr/local
			${MKLLIB}/include
    )
	
find_library(MKL_LIBRARIES_CORE
  mkl_core
  PATHS
  ${MKLLIB}/lib/intel64
)

find_library(MKL_LIBRARIES_LP64
  mkl_intel_lp64
  PATHS
  ${MKLLIB}/lib/intel64
)

find_library(MKL_LIBRARIES_SEQUENTIAL
  mkl_sequential
  PATHS
  ${MKLLIB}/lib/intel64
)

set(MKL_LIBRARIES ${MKL_LIBRARIES_CORE} ${MKL_LIBRARIES_LP64} ${MKL_LIBRARIES_SEQUENTIAL})



include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MKL DEFAULT_MSG MKL_INCLUDE_DIR MKL_LIBRARIES)

mark_as_advanced(MKL_LIBRARIES)
