#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include "lock_free_queue/lock_free_queue.hpp"
#include "order_gateway.grpc.pb.h"
#include "types/order.hpp"
#include "types/ordertype.hpp"
#include "types/side.hpp"

using ProtoAck = flashmatch::Ack;
using ProtoOrder = flashmatch::Order;
using ProtoOrderType = flashmatch::OrderType;
using ProtoSide = flashmatch::Side;
using OrderGatewayService = flashmatch::OrderGateway::Service;
using OrderGatewayStub = flashmatch::OrderGateway;

class OrderGatewayServiceImpl final : public OrderGatewayService {
 public:
  explicit OrderGatewayServiceImpl(lfq::Atomic_Queue<::Order>& queue) : queue_(queue) {}
  grpc::Status SubmitOrder(grpc::ServerContext*,
                           const ProtoOrder* request,
                           ProtoAck* response) override {
    ::Order order;
    order.symbol = request->symbol();
    order.id = request->id();
    order.side = request->side() == ProtoSide::BUY ? Side::BUY : Side::SELL;
    order.price = request->price();
    order.quantity = request->quantity();
    order.type = request->type() == ProtoOrderType::LIMIT ? OrderType::LIMIT : OrderType::IOC;
    bool ok = queue_.push(order);
    response->set_ok(ok);
    return ok ? grpc::Status::OK : grpc::Status(grpc::StatusCode::RESOURCE_EXHAUSTED, "queue full");
  }

 private:
  lfq::Atomic_Queue<::Order>& queue_;
};

TEST(OrderGateway, RoundTrip) {
  lfq::Atomic_Queue<::Order> queue(4);
  OrderGatewayServiceImpl service(queue);

  std::string server_address("localhost:50052");
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

  auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
  auto stub = flashmatch::OrderGateway::NewStub(channel);

  ProtoOrder order;
  order.set_symbol("AAPL");
  order.set_id(42);
  order.set_side(ProtoSide::BUY);
  order.set_price(12.5);
  order.set_quantity(10);
  order.set_type(ProtoOrderType::LIMIT);

  ProtoAck ack;
  grpc::ClientContext ctx;
  grpc::Status status = stub->SubmitOrder(&ctx, order, &ack);

  EXPECT_TRUE(status.ok());
  EXPECT_TRUE(ack.ok());

  auto stored = queue.pop();
  EXPECT_EQ(stored.id, 42u);
  EXPECT_EQ(stored.symbol, "AAPL");
  EXPECT_EQ(stored.price, 12.5);
  EXPECT_EQ(stored.quantity, 10u);
  EXPECT_EQ(stored.side, Side::BUY);
  EXPECT_EQ(stored.type, OrderType::LIMIT);

  server->Shutdown();
}
