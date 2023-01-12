# Asynx_server






1. to start the server : g++ -std=c++17 server.cpp proto/protobuf.pb.cc -o s.o `pkg-config --cflags --libs protobuf` -lboost_system -lprotobuf
2. to start te test_client : g++ -std=c++17 client.cpp proto/protobuf.pb.cc -o client.o `pkg-config --cflags --libs protobuf`
