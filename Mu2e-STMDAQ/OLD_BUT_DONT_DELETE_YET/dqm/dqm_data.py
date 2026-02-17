import numpy as np

from global_vars import *

# DQM data class
class dqm_data:

    # Initliase the dqm
    def __init__(self):
            
        # Define variables
        self.info_num = 0
        self.info_id = []
        self.info_len = []
        self.info_description = []
        self.info_map = []
        self.reg_data_len = 0
        self.stm_data_len = 0
        self.ch_data_len = 0
        self.tot_info_len = 0
        self.tot_data_len = 0
                
        # Open the dqm data map file
        fin = open(stmdaq_dir + "/dqm/dqm_map.txt")
        
        # Get all lines in file
        lines = fin.readlines()
                
        # If file found
        if fin:
            # Initialise counters
            map_num = 0
            count = 0
            # Loop through file lines
            for line in lines:
                count += 1
                # Strip the line
                line = line.strip()
                # Split the line into parts by space
                parts = line.split(' ')
                # Store id, length, map and table descrption
                self.info_id.append(str(parts[0]))
                self.info_len.append(int(parts[1]))
                self.info_map.append(map_num)
                self.info_description.append(' '.join(parts[2:]))
                # Store number of register parameters
                if (str(parts[0]) == 'dtc_clk_marker_sync_off'):
                    self.reg_data_len = count
                    count = 0
                # Store number of stm parameters
                if (str(parts[0]) == 'ewt'):
                    self.stm_data_len = count
                    count = 0
                # Increase map number
                map_num += int(parts[1])
            # Store number of channel parameters                
            self.ch_data_len = int(count/CHNUM)
            # Store number of info ids
            self.info_num = len(lines)
            
        # Throw error if dqm file not found
        else:
            print("ERROR: can't find dqm data map file! Exiting...")
            exit(0)

        # Get the total stm info array length 
        self.tot_info_len = int(np.sum(self.info_len))
        # Calculate the total data array length
        self.tot_data_len = int(self.tot_info_len + 2 * total_peak_max)
        # Calculate the total data array length  
        self.dqm_data = np.zeros(self.tot_data_len, dtype=np.int16)
        self.stm_info = [0] * self.info_num
        self.info_dict = {}

    # Build the dictionary of stm info
    def build_start_dict(self):

        # Get the info keys and values
        keys = self.info_id[:self.info_num]

        # Create dictionary of data
        self.info_dict = {k: [] for k in keys}    
        for i in range(self.info_num):
            if (i < self.reg_data_len):
                self.info_dict[keys[i]].append({'description':self.info_description[i],'value':'No'})
            else:
                self.info_dict[keys[i]].append({'description':self.info_description[i],'value':' - '})

    # Build the dictionary of stm info
    def build_info_dict(self,data):

        # Get the data array
        self.build_dqm_array(data)

        # Get the info keys and values
        keys = self.info_id[:self.info_num]
        values = self.stm_info

        # Create dictionary of data
        for i in range(self.info_num):
            if (keys[i] == "receiving_data" or i >= self.reg_data_len):
                self.info_dict[keys[i]] = []
                self.info_dict[keys[i]].append({'description':self.info_description[i],'value':values[i]})

    # Build the dqm data array
    def build_dqm_array(self,data):
        # Loop over STM infos
        for i in range(self.info_num):
            # Get id map and length
            datamap = self.info_map[i]
            length = self.info_len[i] 
            # Set hardware checks
            if (i < self.reg_data_len):
                # Convert from nupmy int16_t to int
                if (data[datamap] == 1):
                    self.stm_info[i] = 'Yes'
                else:
                    self.stm_info[i] = 'No'
            elif (self.info_id[i] in {"prescale_0","prescale_1",
                                      "b_mean_0","b_mean_1",        
                                      "b_rms_0","b_rms_1"}):       
                # Convert from nupmy int16_t to int
                self.stm_info[i] = int(data[datamap].item(0))

            # Get the data directory
            elif (self.info_id[i] == "data_dir"):
                self.stm_info[i] = self.get_data_dir(data[datamap:datamap+length])
            # Get the total data size and rate
            elif (self.info_id[i] in {"tot_data_size","tot_data_rate",
                                      "zs_factor_0","zs_factor_1",
                                      "pulse_rate_0","pulse_rate_1",
                                      "data_rate_0","data_rate_1"}):
                self.stm_info[i] = self.to_float(data[datamap:datamap+length])
            # Get the adc temperature
            elif (self.info_id[i] == "adc_temp"):
                self.stm_info[i] = self.get_adc_temp(data[datamap:datamap+length])
            # Get run, subrun and event numbers
            elif (self.info_id[i] in {"run_num","subrun_num","ewt",
                                      "events_0","on_0","off_0",
                                      "events_1","on_1","off_1",
                                      "packets_0","packets_1"}):
                self.stm_info[i] = make_uint64_t(data[datamap:datamap+length])
                #if (self.info_id[i]="ewt"):
                 #   print( make_uint64_t(data[datamap:datamap+length]) )
            # Get baseline values
            elif (self.info_id[i] in {"baseline_0","baseline_1"}):
                self.stm_info[i] = data[i]
                self.stm_info[i+1] = data[i]
            else:
                print("ERROR: could not find DQM map ID:", self.info_id[i], ". Exiting...")
                exit(0)

    # Get the data directory
    def get_data_dir(self,data):
        # Define data directory string
        ddir = str()
        # Loop over string integers
        for i in range(len(data)):
            # If non-zero, append string
            if (data[i] != 0): ddir += chr(data[i])
        # Return data directory
        return ddir

    # Convert to float
    def to_float(self,data):
        # Convert to uint32_t --> float --> return precision
        value = make_uint32_t(data)
        value = float(value)/1e4
        # Return value
        return value

    # Get the adc temperature
    def get_adc_temp(self,data):
        # Make uint64_t
        temp = make_uint64_t(data)
        adc_temp = float((temp&0xFFF) / 4.0)
        # Return value
        return adc_temp



        
