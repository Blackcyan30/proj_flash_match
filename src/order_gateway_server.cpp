#include <grpcpp/grpcpp.h>

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

static lfq::Atomic_Queue<::Order> g_queue(64);

void RunServer(const std::string& server_address) {
  OrderGatewayServiceImpl service(g_queue);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server->Wait();
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  RunServer("0.0.0.0:50051");
  return 0;
}
