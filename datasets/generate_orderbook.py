import csv
import gzip
import os
import random
import sys

import numpy as np
import pandas as pd
from tqdm import tqdm

import psutil  # type: ignore


def available_ram_gb():
    return psutil.virtual_memory().available / (1024**3)


def auto_batch_size(num_rows, ram_gb):
    est_rows_per_gb = (1024 / 50) * 100_000  # ~2M rows per GB
    safe_batch = int(min(num_rows, ram_gb * est_rows_per_gb))
    return max(10_000, min(safe_batch, 5_000_000))


def open_output_file(filename, compress):
    if compress:
        return gzip.open(filename, "wt", newline="")
    else:
        return open(filename, "a", newline="")


def generate_orders(start_id, num_orders, order_type="mixed"):
    ids = np.arange(start_id, start_id + num_orders, dtype=np.uint64)
    symbols = np.random.choice(["AAPL", "GOOG", "MSFT", "TSLA"], size=num_orders)
    sides = np.random.choice(["BUY", "SELL"], size=num_orders)
    prices = np.round(np.random.uniform(9.50, 10.50, size=num_orders), 2)
    quantities = np.random.randint(1, 101, size=num_orders)

    if order_type == "limit":
        types = np.full(num_orders, "LIMIT")
    elif order_type == "mixed":
        types = np.random.choice(["LIMIT", "IOC"], size=num_orders)
    else:
        raise ValueError("Invalid order_type")

    df = pd.DataFrame(
        {
            "id": ids,
            "symbol": symbols,
            "side": sides,
            "price": prices,
            "quantity": quantities,
            "type": types,
        }
    )
    return df


def generate_with_pandas(
    filename, warmup_rows, total_rows, batch_size, compress, file_format
):
    current_id = 1

    with open_output_file(filename, compress) as file:
        file.write(f"{total_rows},{warmup_rows}\n")
        with tqdm(total=total_rows, unit=" rows") as pbar:

            # WARMUP PHASE
            if warmup_rows > 0:
                for start in range(0, warmup_rows, batch_size):
                    size = min(batch_size, warmup_rows - start)
                    df = generate_orders(current_id, size, order_type="limit")

                    if file_format == "csv":
                        df.to_csv(file, header=False, index=False)
                    elif file_format == "json":
                        df.to_json(file, orient="records", lines=True)
                    current_id += size
                    pbar.update(size)

            # MAIN RANDOM PHASE
            remaining_rows = total_rows - warmup_rows
            for start in range(0, remaining_rows, batch_size):
                size = min(batch_size, remaining_rows - start)
                df = generate_orders(current_id, size, order_type="mixed")

                if file_format == "csv":
                    df.to_csv(file, header=False, index=False)
                elif file_format == "json":
                    df.to_json(file, orient="records", lines=True)

                current_id += size
                pbar.update(size)


def main():
    # Set fixed values for specific benchmark dataset
    num_rows = 120_000_000
    warmup_rows = 20_000_000
    compress = False
    file_format = "csv"
    filename = "ob_100mil_bench_20mil_warm.csv"
    
    # Uncomment below lines if you want interactive input instead
    """
    # Collect user input
    num_rows = int(input("Enter total number of rows to generate: "))
    warmup_rows = int(input("Enter number of warmup (LIMIT-only) rows: "))
    compress_input = input("Compress output file? (yes/no): ").strip().lower()
    compress = compress_input in ("yes", "y", "true", "1")
    format_input = input("Choose file format (csv/json): ").strip().lower()
    file_format = format_input if format_input in ("csv", "json") else "csv"
    filename = input("Enter output filename (or leave blank for default): ").strip()
    """
    

    ram_gb = available_ram_gb()
    batch_size = auto_batch_size(num_rows, ram_gb)

    print(f"🧠 Available RAM: {ram_gb:.2f} GB")
    print(f"📦 Batch size: {batch_size}")

    # Use the provided filename directly
    output_file = filename
    
    # Only add extension if not already included in filename
    if file_format == "json" and not filename.endswith(".json"):
        output_file += ".json"
    elif file_format == "csv" and not filename.endswith(".csv"):
        output_file += ".csv"
    
    # Add compression extension if needed
    if compress and not output_file.endswith(".gz"):
        output_file += ".gz"

    print(f"📁 Output file: {output_file}")
    print(f"🗂️ Format: {file_format.upper()} {'(compressed)' if compress else ''}")
    print("🚀 Starting generation...")

    generate_with_pandas(
        output_file,
        warmup_rows=warmup_rows,
        total_rows=num_rows,
        batch_size=batch_size,
        compress=compress,
        file_format=file_format,
    )

    file_size_bytes = os.path.getsize(output_file)
    print(f"📏 File size: {file_size_bytes / (1024**2):.2f} MB")
    print(f"✅ Done! File written: {output_file}")


if __name__ == "__main__":
    main()
