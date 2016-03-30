// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "Example.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::td::test;

class ExampleHandler : virtual public ExampleIf {
 public:
  ExampleHandler() {
    // Your initialization goes here
  }

  void NoParamsNoReturn() {
    // Your implementation goes here
    printf("NoParamsNoReturn\n");
  }

  void HasParamsNoReturn(const std::string& thing, const Xtruct& xtruct, const Insanity& insanity) {
    // Your implementation goes here
    printf("HasParamsNoReturn\n");
  }

  void NoParamsHasReturn(Xtruct& _return) {
    // Your implementation goes here
    printf("NoParamsHasReturn\n");
  }

  void HasParamsHasReturn(Insanity& _return, const std::string& thing, const Xtruct& xtruct, const Insanity& insanity) {
    // Your implementation goes here
    printf("HasParamsHasReturn\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<ExampleHandler> handler(new ExampleHandler());
  shared_ptr<TProcessor> processor(new ExampleProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

