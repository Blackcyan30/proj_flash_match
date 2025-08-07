#include <grpcpp/grpcpp.h>

#include <iostream>

#include "order_gateway.grpc.pb.h"

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
  auto stub = flashmatch::OrderGateway::NewStub(channel);

  flashmatch::Order order;
  order.set_symbol("AAPL");
  order.set_id(1);
  order.set_side(flashmatch::Side::BUY);
  order.set_price(10.0);
  order.set_quantity(5);
  order.set_type(flashmatch::OrderType::LIMIT);

  flashmatch::Ack ack;
  grpc::ClientContext ctx;
  grpc::Status status = stub->SubmitOrder(&ctx, order, &ack);

  if (status.ok() && ack.ok()) {
    std::cout << "Order submitted" << std::endl;
  } else {
    std::cout << "Order failed: " << status.error_message() << std::endl;
  }
  return 0;
}
