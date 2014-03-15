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

cp UUGear.py $TARGET_DIR/UUGear.py

echo "Building examples in C..."

EXAMPLE_DIR=$CURRENT_DIR/example/c

cd $EXAMPLE_DIR

cc -L$TARGET_DIR -Wall DigitalReadWrite.c -o $TARGET_DIR/DigitalReadWrite -lUUGear -lrt

cc -L$TARGET_DIR -Wall AnalogWrite.c -o $TARGET_DIR/AnalogWrite -lUUGear -lrt

cc -L$TARGET_DIR -Wall VoltageMeasurement.c -o $TARGET_DIR/VoltageMeasurement -lUUGear -lrt

cc -L$TARGET_DIR -Wall ReadSpeed.c -o $TARGET_DIR/ReadSpeed -lUUGear -lrt

cc -L$TARGET_DIR -Wall ReadDHT11.c -o $TARGET_DIR/ReadDHT11 -lUUGear -lrt

cd $CURRENT_DIR

echo "Copying examples in Python..."

cp $CURRENT_DIR/example/python/*.py $TARGET_DIR

echo "Build End"