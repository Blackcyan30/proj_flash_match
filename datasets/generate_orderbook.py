import argparse
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


def generate_with_pandas(
    filename, total_rows, batch_size=100_000, compress=False, file_format="csv"
):
    header_written = False
    order_id = 1

    with open_output_file(filename, compress) as file:
        with tqdm(total=total_rows, unit=" rows") as pbar:
            while order_id <= total_rows:
                end_id = min(order_id + batch_size, total_rows + 1)
                size = end_id - order_id

                ids = np.arange(order_id, end_id, dtype=np.uint64)
                sides = np.random.choice(["BUY", "SELL"], size=size)
                prices = np.round(np.random.uniform(9.50, 10.50, size=size), 2)
                quantities = np.random.randint(1, 101, size=size)
                types = np.random.choice(["LIMIT", "IOC"], size=size)

                df = pd.DataFrame(
                    {
                        "order_id": ids,
                        "side": sides,
                        "price": prices,
                        "quantity": quantities,
                        "type": types,
                    }
                )

                if file_format == "csv":
                    df.to_csv(file, header=not header_written, index=False)
                elif file_format == "json":
                    df.to_json(file, orient="records", lines=True)

                header_written = True
                pbar.update(size)
                order_id += batch_size


def main():
    parser = argparse.ArgumentParser(
        description="Generate synthetic limit-order book data."
    )
    parser.add_argument("num_rows", type=int, help="Number of rows to generate")
    parser.add_argument(
        "--batch-size", type=int, default=None, help="Custom batch size (default: auto)"
    )
    parser.add_argument("--output", type=str, default=None, help="Output filename")
    parser.add_argument(
        "--compress", action="store_true", help="Compress output to .gz"
    )
    parser.add_argument(
        "--format",
        type=str,
        choices=["csv", "json"],
        default="csv",
        help="Output format (csv or json)",
    )

    args = parser.parse_args()
    num_rows = args.num_rows
    ram_gb = available_ram_gb()

    print(f"🧠 Available RAM: {ram_gb:.2f} GB")
    batch_size = args.batch_size or auto_batch_size(num_rows, ram_gb)
    print(f"📦 Batch size: {batch_size}")

    # Determine default file name
    ext = ".json" if args.format == "json" else ".csv"
    default_name = f"synthetic_order_book_{num_rows}{ext}"
    if args.compress:
        default_name += ".gz"
    output_file = args.output or default_name

    print(f"📁 Output file: {output_file}")
    print(f"🗂️ Format: {args.format.upper()} {'(compressed)' if args.compress else ''}")
    print("🚀 Starting generation...")

    generate_with_pandas(
        output_file,
        num_rows,
        batch_size=batch_size,
        compress=args.compress,
        file_format=args.format,
    )

    print(f"\n✅ Done! File written: {output_file}")


if __name__ == "__main__":
    main()
