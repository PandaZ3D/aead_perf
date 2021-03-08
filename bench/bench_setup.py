'''
  Author: Allen Aboytes

  Setup date to be encrypted during benchmarking tests
'''
import os

def generate_random_block(block_size):
    # do stuff to make random block of data
    random_block = b'hi' # /dev/random
    return random_block

def main():
  # data sizes to generate
  block_sizes = [0, 1]

  # create directory where test data goes
  data_dir = 'inputs'
  if not os.path.exists(data_dir):
    os.mkdir(data_dir)

  # generate random amount of data
  for b in block_sizes:
    # open file handle where data will go
    test_data_file = open(f"{data_dir}/input_{b}.dat", 'wb')
    # generate random block of data
    random_data = generate_random_block(b)
    # write output data to file
    test_data_file.write(random_data)
    # close file reference (save)
    test_data_file.close()

if __name__ == '__main__':
  main()
