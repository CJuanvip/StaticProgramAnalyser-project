# Same as prod
if [[ $1 == "all" ]] || [[ $1 == "prod" ]] ; then
rm -rf ./build
mkdir ./build
cd build
cmake ..
make -j8
exit
fi

if [[ $1 == "debug" ]]; then
rm -rf ./build
mkdir ./build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j8
exit
fi

if [[ $1 == "answergen" ]]; then
rm -rf ./build
mkdir ./build
cd build
cmake .. -DCMAKE_BUILD_TYPE=ANSWER_GEN
make -j8
exit
fi

if [[ $1 == "clean" ]]; then
cd build
make clean
cd ..
rm -rf ./build
find ./../Tests00/ -name "*Answer.out" -type f -delete # delete answer files
exit
fi

if [[ $1 == "refresh" ]]; then
cd build
cmake ..
make -j8
exit
fi

if [[ $1 == "test" ]]; then
cd build/src/unit_testing
time ./unit_testing
cd ../integration_testing
time ./integration_testing
exit
fi

echo "Usage: bash build.sh [ all | prod | debug | answergen | clean | refresh | test ]"
exit
