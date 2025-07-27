Absolutely — here's a clean, developer-friendly `README.md` for your **synthetic limit-order book data generator**.

---

# 📘 Synthetic Limit Order Book Generator

Generate large-scale synthetic limit-order book (LOB) data for benchmarking, testing, or simulations.
Fast, memory-safe, and CLI-friendly — supports CSV, JSON, batching, compression, and automatic tuning based on available RAM.

---

## 🔧 Features

✅ Fast data generation using **NumPy + Pandas batching**
✅ Auto-selects **optimal batch size** based on available RAM
✅ Supports **CSV** and **JSON (NDJSON)** output formats
✅ Optional **Gzip compression** for large files
✅ Live **progress bar and ETA** via `tqdm`
✅ CLI flags for full control

---

## 🚀 Installation

### 1. Install dependencies:

```bash
pip install pandas numpy psutil tqdm
```

> For compressed output, Python’s built-in `gzip` is used (no extra setup needed).

---

## 🧪 Usage

### Basic (1 million rows to CSV):

```bash
python generate_order_book.py 1000000
```

### JSON output:

```bash
python generate_order_book.py 1000000 --format json
```

### Compressed CSV:

```bash
python generate_order_book.py 1000000 --compress
```

### Custom output filename and batch size:

```bash
python generate_order_book.py 1000000 \
  --output lob_data.json.gz \
  --format json \
  --compress \
  --batch-size 500000
```

---

## ⚙️ CLI Options

| Flag                    | Description                                                        |
| ----------------------- | ------------------------------------------------------------------ |
| `num_rows` (positional) | Number of rows to generate (e.g. `1000000`)                        |
| `--batch-size`          | Optional: Manually set the batch size (default: auto based on RAM) |
| `--output`              | Optional: Output filename (default is based on row count + format) |
| `--compress`            | Save file as `.gz` using Gzip                                      |
| `--format`              | Output format: `csv` (default) or `json`                           |

---

## 📦 Output Formats

### CSV (default)

* Standard comma-separated format
* One row per line
* Includes header

### JSON (newline-delimited)

* Each row is a separate JSON object
* One row per line (NDJSON)
* Compatible with tools like Elasticsearch, Kafka, Spark

---

## 🧠 How It Works

* Generates LOB fields:

  * `order_id`: integer (sequential)
  * `side`: `"BUY"` or `"SELL"`
  * `price`: float between `9.50` and `10.50`
  * `quantity`: integer from `1` to `100`
  * `type`: `"LIMIT"` or `"IOC"`
* Uses **NumPy** for fast vectorized generation
* Uses **Pandas** for efficient writing
* Automatically determines safe **batch size** based on system memory
* Streams each batch to file to prevent memory overload

---

## 📊 Example Output (CSV)

```csv
order_id,side,price,quantity,type
1,BUY,9.95,20,LIMIT
2,SELL,10.12,75,IOC
3,BUY,9.88,54,LIMIT
...
```

---

## 🔥 Performance Tips

* Run in a terminal with SSD storage for best results
* Use larger `--batch-size` on systems with more than 8 GB RAM
* Use `.gz` compression to save disk space on very large datasets

---

## 🧩 Future Add-ons (Optional)

* Multiprocessing for even faster generation
* Support for Parquet / Feather / SQLite
* Statistical control over distributions (e.g. 80% LIMIT, 20% IOC)

---

## 📄 License

MIT — free to use, modify, and distribute.

---

Would you like me to:

* Package this as a pip-installable CLI tool?
* Add Docker support or a GUI wrapper?

Let me know!
