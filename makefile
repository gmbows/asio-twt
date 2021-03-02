twt.exe: bin/Main.o bin/Utility.o bin/Network.o bin/Common.o bin/Thread.o bin/NetworkUtils.o
	g++ bin/Main.o bin/Utility.o bin/Network.o bin/Common.o bin/Thread.o bin/NetworkUtils.o -lws2_32 -lwsock32 -D_WIN32_WINNT=0x0601 -o twt.exe

bin/Main.o: src/Main.cpp
	g++ src/Main.cpp -c -o bin/Main.o

bin/Command.o: src/Command.cpp src/Command.h
	g++ src/Command.cpp -c -o bin/Command.o


bin/Utility.o: src/Utility.cpp src/Utility.h
	g++ src/Utility.cpp -c -o bin/Utility.o

bin/Thread.o: src/Thread.cpp src/Thread.h
	g++ src/Thread.cpp -c -o bin/Thread.o

bin/Common.o: src/Common.cpp src/Common.h
	g++ src/Common.cpp -c -D_WIN32_WINNT=0x0601 -o bin/Common.o

bin/NetworkUtils.o: src/NetworkUtils.cpp src/NetworkUtils.h
	g++ src/NetworkUtils.cpp -c -lws2_32 -lwsock32  -o bin/NetworkUtils.o

bin/Network.o: src/Network.cpp src/Network.h
	g++ src/Network.cpp -c -lws2_32 -lwsock32  -o bin/Network.o