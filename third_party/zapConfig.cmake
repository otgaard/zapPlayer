set(zap_INCLUDE_DIRS ${zap_DIR}/include/zap)
set(zap_LIBRARY_DIR ${zap_DIR}/lib)
set(zap_LIBRARIES ${zap_LIBRARIES} ${zap_LIBRARY_DIR}/libzapMaths.dylib)
set(zap_LIBRARIES ${zap_LIBRARIES} ${zap_LIBRARY_DIR}/libzapEngine.dylib)