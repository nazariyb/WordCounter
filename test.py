# coding=utf-8
import os, sys
import csv

# Якщо потрібно тестити послідовну і паралельну або лише паралельну прогу (див. конфігурацію нижче),
# то виклик буде виглядати так

# python test.py n t argv
# n - кількість запусків кожної проги
# t - кількість потоків із якими буде запускатися програма
# t = 0 - означає тестовий мод, прожене прогу з різною кількістю потоків

# Якщо потрібно затестити лише послідовну, то це можна зробити так
# python test.py n argv

# argv - аргументи для програми, якщо такі потрібні, як ні - то не вказувати

# ========== CONFIGURATIONS STARTS ==========
# наприклад, python/java, якщо потрібно запускати просту .exe, то залишити пустим
RUNNER = ''

# вказати повний шлях до файлу, якщо не потрібно тестити паралельну/послідовну програму,
# то залишити пустим
PARALLEL_PROG = './cmake-build-debug/lab4'
SEQUENTIAL_PROG = ''

# файл, куди буде записано скільки працювала програма 
# (з тою чи іншою кількістю потоків, якщо це паралельна)
# не може бути пустим, розширення має бути .csv
RESULT_FILE = 'time_results_test_mode.csv'
# =========== CONFIGURATIONS ENDS ===========


# arguments_number = len(sys.argv)
# assert arguments_number == 3, 'Incorrect input. Not enough arguments'

if PARALLEL_PROG:
    runs = int(sys.argv[1])
    threads = int(sys.argv[2])
    argv = sys.argv[3:]
    TEST_MODE = True if threads == 0 else False

if SEQUENTIAL_PROG:
    runs = int(sys.argv[1])
    argv = sys.argv[2:]


print('Running...')


def find_exe(progname):
    src_fold = ''
    possible_names = [progname, progname + '.exe']
    for root, dirs, files in os.walk('.'):
        for dir in dirs:
            # print('DIR is', dir, 'IN', dirs)
            if dir == 'CMakeFiles':
                continue
            os.chdir(dir)
            for root1, dirs1, files1 in os.walk('.'):
                for file in files1:
                    if file in possible_names:
                        progname = './' + file if file == progname else file
                        src_fold = dir
                        break
                if src_fold:
                    break
            if src_fold:
                break
            os.chdir('../')
        if src_fold:
            break
    # os.chdir('../')
    return progname

def read_result(filename):
     with open(filename, mode='r', encoding='utf-8') as f:
        lines = f.readlines()
        for line in lines:
            l = line.split()
            yield l[0], l[2]



def test_prog(progname, runs, results, threads=''):
    times = []
    # progname = find_exe(progname)

    print('Testing {}...'.format(progname))

    for _ in range(runs):
        command = "{} {} {} {} > temp_time_result.txt"\
            .format(RUNNER, progname, ' '.join(argv), threads)
        print(command)
        os.system(command)
        with open("temp_time_result.txt") as time:
            line = time.readline()
            while not line.startswith("Total time:"):
                line = time.readline()
            times.append(float(line.split()[-1]))
        print('executed {} time(s)'.format(_ + 1))
    print('Storing results.')
    for r in read_result("res_alphabet.txt"):
        # print(r)
        results.add(r)
    for r in read_result("res_numbers.txt"):
        results.add(r)
    # with open("res.txt") as result:
    #     results.add(result.readline())

    print('{} is tested.'.format(progname))
    return min(times)


if __name__ == '__main__':
    results = set()
    if SEQUENTIAL_PROG:
        seq_time = test_prog(SEQUENTIAL_PROG, runs, results)
    if TEST_MODE and PARALLEL_PROG:
        paral_time = []
        for threads in range(1, 17):
            # os.chdir('../')
            paral_time.append(
                test_prog(PARALLEL_PROG, runs, results, str(threads)))
    elif PARALLEL_PROG:
        # os.chdir('../')
        paral_time = test_prog(PARALLEL_PROG, runs, results, str(threads))

    with open("res_alphabet.txt", mode='r', encoding='utf-8') as f:
        # print("Length:", len(f.readlines()), len(results))
        print('Results are equal' if len(results) == len(f.readlines()) else 'Results differs from each other')
    # for w in sorted(results):
    #     print(w)

    if TEST_MODE:
        with open(RESULT_FILE, mode='w', encoding='utf-8') as t_r:
            writer = csv.writer(t_r)
            writer.writerow(["Threads number", "Time μs"])
            if SEQUENTIAL_PROG:
                writer.writerow([0, seq_time])
            if PARALLEL_PROG:
                for t in range(len(paral_time)):
                    writer.writerow([t + 1, paral_time[t]])

        print("Time results are saved to " + RESULT_FILE)
    else:
        if SEQUENTIAL_PROG:
            print("Sequential program's minimal execution time: {}μs".format(
                cons_time))
        if PARALLEL_PROG:
            print("Parallel program's minimal execution time: {}μs".format(
                paral_time))
