#!/bin/zsh
echo "Select optimization level:"
echo "1. Maximum optimization for speed (Release)"
echo "2. Maximum optimization for speed, debug info included (RelWithDebInfo)"
echo "3. Maximum optimization for size (MinSizeRel)"
echo "4. Max debug info (Debug)"

echo "Enter the number of the desired optimization level:"
read OPT_LEVEL

case $OPT_LEVEL in
    1)
        echo "Selected: Maximum optimization for speed"
        BUILD_TYPE="Release"
        ;;
    2)
        echo "Selected: Maximum optimization for speed, debug info included"
        BUILD_TYPE="RelWithDebInfo"
        ;;
    3)
        echo "Selected: Maximum optimization for size"
        BUILD_TYPE="MinSizeRel"
        ;;
    4)
        echo "Selected: Max debug info"
        BUILD_TYPE="Debug"
        ;;
esac

if [[ "$1" != "" ]]; then
  BUILD_TYPE="$1"
fi


MODULES_DIR="./modules"


MODULES=($(ls ${MODULES_DIR}))


NUM_MODULES=${#MODULES[@]}

BUILD_DIR="build_${BUILD_TYPE}"

if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "${BUILD_DIR} directory not found. Creating '${BUILD_DIR}' directory..."
  mkdir ${BUILD_DIR}
fi

cd ${BUILD_DIR}


echo "Available modules to build:"
for i in {1..$((NUM_MODULES))}; do
    MODULE_NAME=$(basename "${MODULES[$i]}")
    echo "$((i-1)). $MODULE_NAME"

done

echo "Enter the number of the module to build:"
read MODULE_TO_BUILD


if [[ $MODULE_TO_BUILD -ge 0 && $MODULE_TO_BUILD -lt $NUM_MODULES ]]; then
    echo "Building module: $SELECTED_MODULE_NAME"

    cmake -D CMAKE_BUILD_TYPE="$BUILD_TYPE" -D MODULE_TO_BUILD="$MODULE_TO_BUILD" ..

    make -j 4
else
    echo "Invalid selection, no module selected."
    exit 1
fi
