./linuxdeploy-x86_64.AppImage --appdir AppDir \
  --exclude-library "libcuda.so*" \
  --exclude-library "libcudart.so*" \
  --exclude-library "libcublas.so*" \
  --exclude-library "libcublasLt.so*"

./appimagetool-x86_64.AppImage AppDir
