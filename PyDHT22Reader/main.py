import sys
import signal
import board
import adafruit_dht
import time


# A dedicated function to assemble the pin object from a string.
# This encapsulates the mapping logic for better readability.
def get_pin_from_string(pin_str: str):
    """
    Assembles a board pin object from a BCM pin number string.

    Args:
        pin_str: The BCM pin number as a string (e.g., '23').

    Returns:
        The corresponding board pin object, or None if invalid.
    """
    pin_mapping = {
        '4': board.D4, '5': board.D5, '6': board.D6,
        '12': board.D12, '13': board.D13, '16': board.D16,
        '17': board.D17, '18': board.D18, '19': board.D19,
        '20': board.D20, '21': board.D21, '22': board.D22,
        '23': board.D23, '24': board.D24, '25': board.D25,
        '26': board.D26, '27': board.D27,
    }
    return pin_mapping.get(pin_str, None)


def parse_arguments():
    """
    Parses command-line arguments and validates their values.

    Returns:
        A tuple containing the polling interval (float) and the pin object.
        Exits the script with an error message if arguments are invalid.
    """
    if len(sys.argv) != 3:
        print(f"Error: Usage: {sys.argv[0]} <polling_interval_sec> <gpio_pin_bcm_number>", file=sys.stderr, flush=True)
        sys.exit(1)

    # Polling Interval
    try:
        log_interval_sec = float(sys.argv[1])
        if log_interval_sec <= 0:
            print("Error: Polling interval must be a positive number.", file=sys.stderr, flush=True)
            sys.exit(1)
        print(f"Log: Using polling interval: {log_interval_sec} seconds", flush=True)
    except ValueError:
        print(f"Error: Invalid polling interval '{sys.argv[1]}' provided. Must be a number.", file=sys.stderr,
              flush=True)
        sys.exit(1)

    # GPIO Pin
    pin_str = sys.argv[2]
    dht_pin = get_pin_from_string(pin_str)
    if dht_pin is None:
        # A more user-friendly error message is generated in the `get_pin_from_string` function.
        # But we still need to provide an error and exit here.
        print(f"Error: Invalid GPIO pin '{pin_str}' provided.", file=sys.stderr, flush=True)
        sys.exit(1)
    print(f"Log: Using data pin: BCM {pin_str} ({dht_pin})", flush=True)

    return log_interval_sec, dht_pin


def run_sensor_loop(dht_device, log_interval_sec):
    """
    The main execution loop for reading sensor data.

    Args:
        dht_device: The initialized adafruit_dht device object.
        log_interval_sec: The interval between readings in seconds.
    """
    while True:
        try:
            temperature_c = dht_device.temperature
            humidity = dht_device.humidity

            if humidity is not None and temperature_c is not None:
                print(f"DATA: {temperature_c:.1f},{humidity:.1f}", flush=True)
            else:
                print("Log: Failed to retrieve data from sensor. Retrying...", flush=True)

        except RuntimeError as error:
            print(f"Error: Reading error: {error.args[0]}", file=sys.stderr, flush=True)
        except Exception as error:
            print(f"Error: An unexpected error occurred: {error}", file=sys.stderr, flush=True)
            break

        time.sleep(log_interval_sec)


def signal_handler(sig, frame):
    """
    Signal handler for graceful shutdown.
    Raises KeyboardInterrupt to reuse the main cleanup logic.
    """
    print(f"Log: Caught signal {sig}. Initiating cleanup...", flush=True)
    raise KeyboardInterrupt


def main():
    """
    Main orchestrator function for the DHT sensor script.
    """
    dht_device = None
    try:
        log_interval_sec, dht_pin = parse_arguments()

        dht_device = adafruit_dht.DHT22(dht_pin)
        print("Log: Initialized AM2302 sensor.", flush=True)

        # Register signal handlers for graceful exit
        signal.signal(signal.SIGINT, signal_handler)
        signal.signal(signal.SIGTERM, signal_handler)

        run_sensor_loop(dht_device, log_interval_sec)

    except KeyboardInterrupt:
        print("Log: Script aborted by user.", flush=True)
    except Exception as error:
        print(f"Error: An unexpected error occurred during main execution: {error}", file=sys.stderr, flush=True)
    finally:
        if dht_device:
            print("Log: Releasing DHT sensor resources...", flush=True)
            dht_device.exit()
        print("Log: Script finished.", flush=True)


if __name__ == "__main__":
    main()