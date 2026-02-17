import sys
import os.path
import numpy as np

# Binary File Header Structure Per Trigger (temporary for testing)
#[0] [dead]
#[1] [beef]
#[2] 16-bit trigger number
#[3] 16-bit trigger type
#[4,5,6,7]] uint64_t trigger time
#[8,9] uint32_t payload size (header+data)

# Actual binary file will be
#uint32_t trigger number (external or internal)
#uint16_t mode + channel + data type
#uint64_t trigger time
#uint32_t ADC offset 
#uint16_t number of dropped packets
#uint16_t number of slices


class BinaryFile:
    def __init__(self, filename):
        if (os.path.isfile(filename)):
            self.filename = filename
            self.fptr = open(filename,"rb")
            self.fsize = os.path.getsize(filename)
            print("Filename = %s, Size = %d bytes" %(self.filename, self.fsize))
        else:
            print("Requested binary file : %s does not exist : EXITING..." % (filename))
            sys.exit(-1)

# ----- Need to add some code to extract header and also just to return array of time and ADC value

    def read_data(self,*args):  # args[0]: offset in bytes from start of file to read from; args[1]: # int16 to read
        saved_args = locals()
        dt = np.dtype('<i2') 
        if (len(args) == 0):
            read_from_byte_offset = 0
            num_bytes_to_read = self.fsize
        elif (len(args) == 2):
            read_from_byte_offset = int(args[0])
            num_bytes_to_read = 2*int(args[1])
        else:
            print("Unknown arguments to read_data ... EXITING", saved_args)
            sys.exit(-1)

        if (read_from_byte_offset+num_bytes_to_read <= self.fsize):
            self.fptr.seek(read_from_byte_offset,0) 
            b = self.fptr.read(num_bytes_to_read) 
            return np.frombuffer(b,dt)
        else:
            print("Requested to read subset of file that goes beyond EOF, offset = %d bytes and asked to read %d bytes and filesize = %d : EXITING..." % \
                (read_from_byte_offset,num_bytes_to_read,self.fsize))
            sys.exit()

    def close_file(self):
        self.fptr.close()

