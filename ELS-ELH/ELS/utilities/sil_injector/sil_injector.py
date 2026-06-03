import csv
import struct
import time
import serial
import argparse

BAUD_RATE = 115200

SYNC_1         = 0xAA
SYNC_2         = 0x55
ID_CTR_TP      = 0x10
PAYLOAD_LEN    = 53
FRAME_SIZE     = 2 + 1 + 1 + PAYLOAD_LEN + 2   # 59 bytes
INTERVAL_S     = 0.010                           # 10 ms


def crc16_ccitt(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) if (crc & 0x8000) else (crc << 1)
        crc &= 0xFFFF
    return crc


def build_frame(timestamp_ms: int, altitude: float, vel_z: float,
                ax: float, ay: float, az: float,
                gx: float, gy: float, gz: float,
                qw: float, qx: float, qy: float, qz: float,
                state: int) -> bytes:
    payload = struct.pack(
        "<IffffffffffffB",
        timestamp_ms,
        altitude, vel_z,
        ax, ay, az,
        gx, gy, gz,
        qw, qx, qy, qz,
        state,
    )
    header = bytes([SYNC_1, SYNC_2, ID_CTR_TP, PAYLOAD_LEN])
    body   = header + payload
    return body + struct.pack("<H", crc16_ccitt(body))


def load_csv(filepath: str) -> list[dict]:
    with open(filepath, encoding="utf-8-sig", errors="replace") as f:
        lines = [l.rstrip("\n") for l in f]

    # Header line: first line that contains "tiempo" (ES) or "time" (EN)
    header_idx = next(
        (i for i, l in enumerate(lines)
         if "tiempo" in l.lower() or "time" in l.lower()),
        None,
    )
    if header_idx is None:
        raise ValueError("Header line not found — expected 'Tiempo' or 'Time' column")

    sep     = "," if "," in lines[header_idx] else "\t"
    headers = [h.strip().lstrip("#").strip().lower() for h in lines[header_idx].split(sep)]

    def col(*searches: list[str]) -> int | None:
        for fragments in searches:
            for i, h in enumerate(headers):
                if all(f in h for f in fragments):
                    return i
        return None

    # Spanish (OpenRocket ES) and English (OpenRocket EN) fragments
    c = {
        "time":    col(["tiempo"],               ["time"]),
        "alt":     col(["altitud"],              ["altitude"]),
        "vel_z":   col(["velocidad", "vertical"],   ["vertical", "velocity"]),
        "accel_z": col(["acel", "vertical"],        ["vertical", "acceleration"]),
        "accel_x": col(["acel", "horizontal"],      ["lateral",  "acceleration"]),
        "roll":    col(["rotaci"],               ["roll"]),
        "pitch":   col(["cabeceo"],              ["pitch"]),
        "yaw":     col(["gui"],                  ["yaw"]),
    }

    print(f"[CSV] Columnas detectadas:")
    for k, v in c.items():
        print(f"       {k:8s} -> col {v}  ({headers[v] if v is not None else 'NOT FOUND'})")

    DEG2RAD = 3.14159265358979 / 180.0

    def get(vals: list[str], key: str) -> float:
        i = c.get(key)
        if i is None or i >= len(vals):
            return 0.0
        try:
            return float(vals[i].strip())
        except ValueError:
            return 0.0

    rows = []
    for line in lines[header_idx + 1:]:
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):   # skip blanks and event comments
            continue
        vals = stripped.split(sep)
        rows.append({
            "t":   get(vals, "time"),
            "alt": get(vals, "alt"),
            "vz":  get(vals, "vel_z"),
            "ax":  get(vals, "accel_x"),
            "ay":  0.0,
            "az":  get(vals, "accel_z"),
            "gx":  get(vals, "roll")  * DEG2RAD,
            "gy":  get(vals, "pitch") * DEG2RAD,
            "gz":  get(vals, "yaw")   * DEG2RAD,
        })
    return rows


def inject(port: str, baud: int, rows: list[dict]) -> None:
    ser = serial.Serial(port, baud, timeout=0)
    print(f"[SIL] {port} @ {baud} baud — {len(rows)} rows — sending every 10 ms")

    t0_sim  = rows[0]["t"]
    t0_wall = time.monotonic()
    idx     = 0
    sent    = 0

    try:
        while idx < len(rows):
            now_wall = time.monotonic()
            now_sim  = t0_sim + (now_wall - t0_wall)

            while idx < len(rows) - 1 and rows[idx + 1]["t"] <= now_sim:
                idx += 1

            r     = rows[idx]
            frame = build_frame(
                int(r["t"] * 1000),
                r["alt"], r["vz"],
                r["ax"],  r["ay"], r["az"],
                r["gx"],  r["gy"], r["gz"],
                1.0, 0.0, 0.0, 0.0,   # quaternion identity
                2,                      # STATE_PAD default; update as needed
            )
            ser.write(frame)
            sent += 1

            if idx >= len(rows) - 1:
                break

            time.sleep(INTERVAL_S)

    except KeyboardInterrupt:
        pass

    print(f"[SIL] Done — {sent} frames sent")
    ser.close()


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", required=True, help="Serial port (e.g. COM3)")
    ap.add_argument("--file", required=True, help="OpenRocket CSV file")
    ap.add_argument("--baud", type=int, default=BAUD_RATE)
    args = ap.parse_args()

    rows = load_csv(args.file)
    inject(args.port, args.baud, rows)


if __name__ == "__main__":
    main()
