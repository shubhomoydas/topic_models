# compile script
mkdir Release
cd Release
mkdir math
mkdir topicmodel
mkdir common
mkdir data
mkdir test
g++ -O0 -g3 -Wall -c -fmessage-length=0 -otest/testinference.o ../src/test/testinference.cpp
g++ -O0 -g3 -Wall -c -fmessage-length=0 -ocommon/Timer.o ../src/common/Timer.cpp
g++ -O0 -g3 -Wall -c -fmessage-length=0 -ocommon/DataArchiver.o ../src/common/DataArchiver.cpp
g++ -O0 -g3 -Wall -c -fmessage-length=0 -otopicmodel/simpleLDA.o ../src/topicmodel/simpleLDA.cpp
g++ -O0 -g3 -Wall -c -fmessage-length=0 -odata/csvdata.o ../src/data/csvdata.cpp
g++ -O0 -g3 -Wall -c -fmessage-length=0 -otest/RankAggregator.o ../src/test/RankAggregator.cpp
g++ -O0 -g3 -Wall -c -fmessage-length=0 -otest/RankAggregatorPL.o ../src/test/RankAggregatorPL.cpp
g++ -O0 -g3 -Wall -c -fmessage-length=0 -otopicmodel/multinomialNBLDA.o ../src/topicmodel/multinomialNBLDA.cpp
g++ -O0 -g3 -Wall -c -fmessage-length=0 -otopicmodel/groupedMultinomialNBLDA.o ../src/topicmodel/groupedMultinomialNBLDA.cpp
g++ -O0 -g3 -Wall -c -fmessage-length=0 -ocommon/utils.o ../src/common/utils.cpp
g++ -O0 -g3 -Wall -c -fmessage-length=0 -omath/math.o ../src/math/math.cpp
g++ -otopicmodel.exe topicmodel/simpleLDA.o topicmodel/multinomialNBLDA.o topicmodel/groupedMultinomialNBLDA.o test/testinference.o test/RankAggregatorPL.o test/RankAggregator.o math/math.o data/csvdata.o common/utils.o common/Timer.o common/DataArchiver.o

