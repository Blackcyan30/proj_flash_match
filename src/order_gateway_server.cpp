#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "flashmatch/order_queue.hpp"
#include "proto/order_gateway.grpc.pb.h"
#include "types/ordertype.hpp"
#include "types/side.hpp"

class OrderGatewayService final : public flashmatch::OrderGateway::Service {
public:
  grpc::Status SubmitOrder(grpc::ServerContext *context,
                           const flashmatch::Order *request,
                           flashmatch::Ack *response) override {
    Order order{
        request->id(),
        request->symbol(),
        request->side() == flashmatch::BUY ? Side::BUY : Side::SELL,
        request->price(),
        request->quantity(),
        request->type() == flashmatch::LIMIT ? OrderType::LIMIT : OrderType::IOC};

    bool pushed = g_order_queue.push(order);
    response->set_ok(pushed);
    return grpc::Status::OK;
  }
};

int main() {
  const std::string server_address{"0.0.0.0:50051"};
  OrderGatewayService service;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
  return 0;
}
