import csv
import gzip
import os
import random
import sys

import numpy as np
import pandas as pd
import psutil
from tqdm import tqdm


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
    header_written = False
    current_id = 1

    with open_output_file(filename, compress) as file:
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

                    header_written = False
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
    # Collect user input
    num_rows = int(input("Enter total number of rows to generate: "))
    warmup_rows = int(input("Enter number of warmup (LIMIT-only) rows: "))
    compress_input = input("Compress output file? (yes/no): ").strip().lower()
    compress = compress_input in ("yes", "y", "true", "1")
    format_input = input("Choose file format (csv/json): ").strip().lower()
    file_format = format_input if format_input in ("csv", "json") else "csv"
    filename = input("Enter output filename (or leave blank for default): ").strip()

    ram_gb = available_ram_gb()
    batch_size = auto_batch_size(num_rows, ram_gb)

    print(f"🧠 Available RAM: {ram_gb:.2f} GB")
    print(f"📦 Batch size: {batch_size}")

    # Set default filename if not provided
    ext = ".json" if file_format == "json" else ".csv"
    output_file = filename or f"synthetic_order_book_{num_rows}{ext}"
    if compress:
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
