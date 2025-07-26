import sys
import time
import datetime # Optional: for including a timestamp in the output

def main():
    counter = 0
    while counter < 3:
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        output_message = f"[{timestamp}] Sensor reading update: {counter}"
        print(output_message, flush=True)
        counter += 1

        #   print("this is an error", file=sys.stderr, flush=True)

        time.sleep(1)

if __name__ == "__main__":
    main()