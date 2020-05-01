from read_logs import read_logs

def main():
    print("Start Plotting Script")
    measurements, resolutions = read_logs(6)
    print(measurements)

    print(resolutions)

if __name__ == '__main__':
    main()