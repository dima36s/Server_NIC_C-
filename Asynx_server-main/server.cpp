#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <fstream>
#include <string> 
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <fstream>
#include "proto/protobuf.pb.h"

using namespace TestTask::Messages;
using boost::asio::ip::tcp;
int * count_connectios_  = new int;
int * id_connection = new int;
std::ofstream log_file{"log_file.log"};



class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket, boost::asio::deadline_timer timer)
    : socket_(std::move(socket)), timer_(std::move(timer))
  {
    (*count_connectios_)++;
    auto start = std::chrono::system_clock::now();
    std::time_t start_time = std::chrono::system_clock::to_time_t(start);
    log_file << "Start : id - "<< id << " "<<std::ctime(&start_time) << std::endl;
  }
  ~session()
  {
    (*count_connectios_)--;
    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    log_file << "End : id - "<< id << " "<<std::ctime(&end_time) << std::endl;
  }

  void start()
  {
    do_read_size();
  }

private:

  void do_read_size(){
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_len, 4), 
    [this, self](boost::system::error_code ec, std::size_t length)
    {
      if (!ec){
          v = (uint32_t*)data_len;
          do_read();
        }});
  }

  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, *v), 
    [this, self](boost::system::error_code ec, std::size_t length)
    {
      if (!ec){
        WrapperMessage *from = new WrapperMessage();
        from->ParseFromString(data_);

        if (from->has_request_for_slow_response()){
          slow_reponse(from);
        }
        if (from->has_request_for_fast_response()){
          fust_response(from);
        }
    }});
  }

  void fust_response(WrapperMessage *from){
    TestTask::Messages::WrapperMessage *to = new TestTask::Messages::WrapperMessage();
    TestTask::Messages::FastResponse *fast_msg = new TestTask::Messages::FastResponse();
    boost::posix_time::ptime t = boost::posix_time::microsec_clock::universal_time();
    fast_msg->set_current_date_time(boost::posix_time::to_iso_string(t));
    to->set_allocated_fast_response(std::move(fast_msg));
    delete from;
    do_write(to);
  }

  void slow_reponse(WrapperMessage *from){
    auto self(shared_from_this());
    TestTask::Messages::WrapperMessage *to = new TestTask::Messages::WrapperMessage();
    TestTask::Messages::SlowResponse *slow_msg = new TestTask::Messages::SlowResponse();
    slow_msg->set_connected_client_count(*count_connectios_);
    to->set_allocated_slow_response(std::move(slow_msg));
    timer_.expires_from_now(boost::posix_time::seconds(from->request_for_slow_response().time_in_seconds_to_sleep()));;
    timer_.async_wait(
    [this, self, to,from](const boost::system::error_code &error)
    {
      if (!error)
        {
          delete from;
          do_write(to);
        }
    });
  }

  void do_write(WrapperMessage *from)
  {
    auto self(shared_from_this());
    std::string response;
    from->SerializeToString(&response);
    delete from;
    uint32_t value = response.size();
    uint32_t* size = &value;
    char* arr = (char*)size;
    std::string str2(arr,4);
    std::string strf = str2+response;
    boost::asio::async_write(socket_, boost::asio::buffer(strf, strf.size()), 
    [this, self, strf](boost::system::error_code ec, std::size_t length)
    {
      if (!ec)
      {
        do_read_size();
      }
    });
  }

  tcp::socket socket_;
  boost::asio::deadline_timer timer_;
  enum { max_length = 1024 };
  char data_[max_length];
  char data_len[4] = {};
  uint32_t *v;
  int id = *id_connection;
};

class server
{
public:
  server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), timer_(io_context)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
    [this](boost::system::error_code ec, tcp::socket socket)
        {
          if (!ec){
            std::make_shared<session>(std::move(socket),std::move(timer_))->start();
            (*id_connection)++;
          }
          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  boost::asio::deadline_timer timer_;
};

int main(){
     std::ifstream inf{"config"};
     *count_connectios_ = 0;
     *id_connection = 1;
     if(!inf){
          std::cerr << "Ohh, I do'nt see config file";
          return 1;
     } 
     std::string str_port;

     if(!log_file){
       std::cerr << "Ohh, I do'nt see config file";
     }
     inf >> str_port;

     short int port = std::stoi(str_port);
     try{
          boost::asio::io_context io_context;
          server s(io_context, port);

          io_context.run();
     }
     catch (std::exception& e){
          std::cerr << "Exception: " << e.what() << "\n";
     }

     return 0;
}

