download_if_missing() {
    local filename="$1"
    local url="$2"

    if [ ! -f "./$filename" ]; then
        echo "$filename not found. Downloading..."

        # Check for wget or curl
        if command -v wget &> /dev/null; then
            wget -O "$filename" "$url" || { echo "Error: Failed to download $filename"; exit 1; }
        elif command -v curl &> /dev/null; then
            curl -L -o "$filename" "$url" || { echo "Error: Failed to download $filename"; exit 1; }
        else
            echo "Error: Neither wget nor curl is installed. Please install one of them."
            exit 1
        fi

        chmod +x "$filename"
        echo "Downloaded $filename"
    else
        echo "Found $filename"
    fi
}

LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
APPIMAGETOOL_URL="https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage"

download_if_missing "linuxdeploy-x86_64.AppImage" "$LINUXDEPLOY_URL"
download_if_missing "appimagetool-x86_64.AppImage" "$APPIMAGETOOL_URL"

cp ./cmake-build-release/suboptimal ./AppDir/usr/bin/suboptimal

./linuxdeploy-x86_64.AppImage --appdir AppDir \
  --exclude-library "libcuda.so*" \
  --exclude-library "libcudart.so*" \
  --exclude-library "libcublas.so*" \
  --exclude-library "libcublasLt.so*"

./appimagetool-x86_64.AppImage AppDir
