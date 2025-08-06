import csv
import random


def generate_order_book_batch(filename, total_rows, batch_size=100_000):
    with open(filename, mode="w", newline="") as file:
        writer = csv.writer(file)
        # Write header including a symbol column
        writer.writerow(["order_id", "symbol", "side", "price", "quantity", "type"])

        order_id = 1
        while order_id <= total_rows:
            batch_end = min(order_id + batch_size, total_rows + 1)
            batch = []

            symbols = ["AAPL", "GOOG", "MSFT", "TSLA"]
            for i in range(order_id, batch_end):
                symbol = random.choice(symbols)
                side = random.choice(["BUY", "SELL"])
                price = round(random.uniform(9.50, 10.50), 2)
                quantity = random.randint(1, 100)
                order_type = random.choice(["LIMIT", "IOC"])
                batch.append([i, symbol, side, price, quantity, order_type])

            writer.writerows(batch)
            print(f"Wrote rows {order_id} to {batch_end - 1}")
            order_id += batch_size


# Example: Generate 100 million rows, writing in 100k-row batches
generate_order_book_batch("synthetic_order_book_100M.csv", total_rows=100_000_000)
