#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include "proto/protobuf.pb.h"

using namespace TestTask::Messages;
using boost::asio::ip::tcp;
using namespace std;
using namespace boost::asio;

enum { max_length = 1024 };


void Slow_msg(){
    WrapperMessage msg;
    RequestForSlowResponse slow_msg;
    slow_msg.set_time_in_seconds_to_sleep(5);
    *msg.mutable_request_for_slow_response() = slow_msg;
    string request;
    msg.SerializeToString(&request);

    cout << "Message size: "<< request.size() << endl;


    //size

    uint32_t value = request.size();
    uint32_t* size = &value;
    char* arr = (char*)size;
    std::string str2(arr,4);
    cout << "Message size int32 in char: "<<str2 << endl;


    //

    ip::tcp::endpoint ep(ip::tcp::endpoint(ip::tcp::v4(), 1234));
    io_service service;
    ip::tcp::socket sock(service);
    sock.connect(ep);

    std::string f_msg = str2 + request;

    std::cout << "Финальное сообщение: " << f_msg << endl;

    std::cout << "Message: ";
    std::cout << request << endl;
    size_t request_length = request.length();
    write(sock, buffer(str2, str2.size()));
    write(sock, buffer(request, request.size()));

    char reply[max_length];
    char msg_size[4];
    read(sock, buffer(msg_size, 4));
    uint32_t* v;
    v = (uint32_t*)msg_size;
    std::cout << "Msg size: " << *v<<endl;
    size_t reply_length = read(sock, buffer(reply, *v));
    std::cout << "Reply is: ";
    std::cout.write(reply, (*v));
    


    WrapperMessage *from = new WrapperMessage();
    from->ParseFromString(reply);
    std::cout << std::endl << "Result : "<< from->slow_response().connected_client_count()<< std::endl;
}

void Fast_msg(){
    WrapperMessage msg;
    RequestForFastResponse fast_msg;
    *msg.mutable_request_for_fast_response() = fast_msg;
    string request;
    msg.SerializeToString(&request);

    cout << "Message size: "<< request.size() << endl;


    //size

    uint32_t value = request.size();
    uint32_t* size = &value;
    char* arr = (char*)size;
    std::string str2(arr,4);
    cout << "Message size int32 in char: "<<str2 << endl;


    //

    

    ip::tcp::endpoint ep(ip::tcp::endpoint(ip::tcp::v4(), 1234));
    io_service service;
    ip::tcp::socket sock(service);
    sock.connect(ep);

    std::string f_msg = str2 + request;

    std::cout << "Финальное сообщение: " << f_msg << endl;

    std::cout << "Message: ";
    std::cout << request << endl;
    size_t request_length = request.length();
    write(sock, buffer(str2, str2.size()));
    write(sock, buffer(request, request.size()));

    char reply[max_length];
    char msg_size[4];
    read(sock, buffer(msg_size, 4));
    uint32_t* v;
    v = (uint32_t*)msg_size;
    std::cout << "Msg size: " << *v<<endl;
    size_t reply_length = read(sock, buffer(reply, *v));
    std::cout << "Reply is: ";
    std::cout.write(reply, *v);
    

    std::cout << endl;
    WrapperMessage *from = new WrapperMessage();
    from->ParseFromString(reply);
    std::cout << std::endl << "Result : " <<std::string(from->fast_response().current_date_time())<< endl;
}

int main(int argc, char* argv[])
{
  try
  {
    Slow_msg();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
 