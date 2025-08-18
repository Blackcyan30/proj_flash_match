#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "order_gateway.grpc.pb.h"

int main() {
  const std::string target_str{"localhost:50051"};
  auto channel = grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials());
  std::unique_ptr<flashmatch::OrderGateway::Stub> stub = flashmatch::OrderGateway::NewStub(channel);

  flashmatch::Order order;
  order.set_id(1);
  order.set_symbol("AAPL");
  order.set_side(flashmatch::BUY);
  order.set_price(100.0);
  order.set_quantity(10);
  order.set_type(flashmatch::LIMIT);

  flashmatch::Ack ack;
  grpc::ClientContext context;
  grpc::Status status = stub->SubmitOrder(&context, order, &ack);

  if (status.ok()) {
    std::cout << "Ack ok: " << ack.ok() << std::endl;
  } else {
    std::cerr << "RPC failed: " << status.error_message() << std::endl;
  }

  return 0;
}
