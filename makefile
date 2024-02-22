CC=g++
FLAGS=-O3
HEADERS=$(wildcard **/*.hpp)


build/main: build/main.o build/custom_types.o build/errors.o build/hashing.o build/names.o \
	build/namespaces.o build/objects.o build/parser.o build/predefined.o build/tokenizer.o
	$(CC) $(FLAGS) build/main.o build/custom_types.o build/errors.o build/hashing.o build/names.o \
	build/namespaces.o build/objects.o build/parser.o build/predefined.o build/tokenizer.o \
	-o build/main

build/main.o: src/main.cpp $(HEADERS)
	$(CC) $(FLAGS) -c src/main.cpp -o build/main.o

build/custom_types.o: src/custom_types.cpp $(HEADERS)
	$(CC) $(FLAGS) -c src/custom_types.cpp -o build/custom_types.o

build/errors.o: src/errors.cpp $(HEADERS)
	$(CC) $(FLAGS) -c src/errors.cpp -o build/errors.o

build/hashing.o: src/hashing.cpp $(HEADERS)
	$(CC) $(FLAGS) -c src/hashing.cpp -o build/hashing.o

build/names.o: src/names.cpp $(HEADERS)
	$(CC) $(FLAGS) -c src/names.cpp -o build/names.o

build/namespaces.o: src/namespaces.cpp $(HEADERS)
	$(CC) $(FLAGS) -c src/namespaces.cpp -o build/namespaces.o

build/objects.o: src/objects.cpp $(HEADERS)
	$(CC) $(FLAGS) -c src/objects.cpp -o build/objects.o

build/parser.o: src/parser.cpp $(HEADERS)
	$(CC) $(FLAGS) -c src/parser.cpp -o build/parser.o

build/predefined.o: src/predefined.cpp $(HEADERS)
	$(CC) $(FLAGS) -c src/predefined.cpp -o build/predefined.o

build/tokenizer.o: src/tokenizer.cpp $(HEADERS)
	$(CC) $(FLAGS) -c src/tokenizer.cpp -o build/tokenizer.o


