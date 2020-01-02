all: a.out

a.out: main.cpp pch.cpp bitcask.cpp bcFile.cpp Utils/*.cpp  hashTable.cpp messageQ.cpp
    g++ -W -Wall main.cpp pch.cpp bitcask.cpp bcFile.cpp Utils/*.cpp  hashTable.cpp messageQ.cpp -lpthread -std=c++11 -o a.out -mmacosx-version-min=10.12


clean:
    rm a.out 
