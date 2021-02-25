twt.exe: bin/Main.o bin/Utility.o bin/Network.o bin/Common.o
	g++ bin/Main.o bin/Utility.o bin/Network.o bin/Common.o -lws2_32 -lwsock32 -D_WIN32_WINNT=0x0601 -o twt.exe

bin/Main.o: src/Main.cpp
	g++ src/Main.cpp -c -o bin/Main.o

bin/Utility.o: src/Utility.cpp src/Utility.h
	g++ src/Utility.cpp -c -o bin/Utility.o

bin/Common.o: src/Common.cpp src/Common.h
	g++ src/Common.cpp -c -D_WIN32_WINNT=0x0601 -o bin/Common.o

bin/Network.o: src/Network.cpp src/Network.h
	g++ src/Network.cpp -c -lws2_32 -lwsock32  -o bin/Network.o