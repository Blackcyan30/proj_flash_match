import numpy as np
import pandas as pd


def generate_order_book_csv(num_rows, filename):
    order_ids = np.arange(1, num_rows + 1, dtype=np.uint64)
    symbols = np.random.choice(["AAPL", "GOOG", "MSFT", "TSLA"], size=num_rows)
    sides = np.random.choice(["BUY", "SELL"], size=num_rows)
    prices = np.round(np.random.uniform(9.50, 10.50, size=num_rows), 2)
    quantities = np.random.randint(1, 101, size=num_rows)
    types = np.random.choice(["LIMIT", "IOC"], size=num_rows)

    df = pd.DataFrame(
        {
            "order_id": order_ids,
            "symbol": symbols,
            "side": sides,
            "price": prices,
            "quantity": quantities,
            "type": types,
        }
    )

    df.to_csv(filename, index=False)


# Example usage
generate_order_book_csv(100_000_000, "synthetic_order_book_100M.csv")
generate_order_book_csv(1_000_000_000, "synthetic_order_book_1B.csv")
