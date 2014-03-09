CURRENT_DIR=`readlink -f .`

echo "CURRENT_DIR = " $CURRENT_DIR

TARGET_DIR=$CURRENT_DIR/bin

rm -rf $TARGET_DIR

mkdir $TARGET_DIR

echo "TARGET_DIR = " $TARGET_DIR

cd src

echo "Building src..."

cc -o $TARGET_DIR/UUGearDaemon UUGearDaemon.c serial.c -lrt
cc -o $TARGET_DIR/UUGear.o -c -Wall -Werror -fPIC UUGear.c 
cc -shared -o $TARGET_DIR/libUUGear.so $TARGET_DIR/UUGear.o -lrt

cc -o $TARGET_DIR/lsuu lsuu.c serial.c

echo "Building example..."

EXAMPLE_DIR=$CURRENT_DIR/example

cd $EXAMPLE_DIR

cc -L$TARGET_DIR -Wall test.c -o $TARGET_DIR/test -lUUGear -lrt

cc -L$TARGET_DIR -Wall AnalogWrite.c -o $TARGET_DIR/AnalogWrite -lUUGear -lrt

cc -L$TARGET_DIR -Wall VoltageMeasurement.c -o $TARGET_DIR/VoltageMeasurement -lUUGear -lrt

cd $CURRENT_DIR

echo "Build End"