import sys
import time
import datetime # Optional: for including a timestamp in the output

def main():
    counter = 0
    while True:
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        output_message = f"[{timestamp}] Sensor reading update: {counter}"
        print(output_message, flush=True)
        counter += 1

        if counter == 3:
            print("this is an error", file=sys.stderr, flush=True)

        time.sleep(3) # Wait for 3 seconds before the next iteration

if __name__ == "__main__":
    main()