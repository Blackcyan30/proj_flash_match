#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

#include "flashmatch/order_queue.hpp"
#include "proto/order_gateway.grpc.pb.h"
#include "types/ordertype.hpp"
#include "types/side.hpp"

class TestOrderGatewayService final : public flashmatch::OrderGateway::Service {
 public:
  grpc::Status SubmitOrder(grpc::ServerContext *context,
                           const flashmatch::Order *request,
                           flashmatch::Ack *response) override {
    Order order{request->id(),
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

TEST(OrderGatewayTest, HandlesConcurrentClients) {
  const std::string server_address{"127.0.0.1:50052"};
  TestOrderGatewayService service;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::thread server_thread([&] { server->Wait(); });

  const int kClientCount = 10;
  std::atomic<int> success{0};
  std::vector<std::thread> clients;
  clients.reserve(kClientCount);

  for (int i = 0; i < kClientCount; ++i) {
    clients.emplace_back([&, i] {
      auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
      std::unique_ptr<flashmatch::OrderGateway::Stub> stub =
          flashmatch::OrderGateway::NewStub(channel);

      flashmatch::Order order;
      order.set_id(i);
      order.set_symbol("AAPL");
      order.set_side(flashmatch::BUY);
      order.set_price(100.0 + i);
      order.set_quantity(10);
      order.set_type(flashmatch::LIMIT);

      flashmatch::Ack ack;
      grpc::ClientContext context;
      grpc::Status status = stub->SubmitOrder(&context, order, &ack);
      if (status.ok() && ack.ok()) {
        ++success;
      }
    });
  }

  for (auto &c : clients) {
    c.join();
  }

  server->Shutdown();
  server_thread.join();

  EXPECT_EQ(success.load(), kClientCount);
}
