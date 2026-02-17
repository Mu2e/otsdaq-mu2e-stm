from global_vars import *

def create_reg_table(dqm):
    headers = ['Register', 'Ready?']
    data = []
    ids = []
    for i in range(dqm.reg_data_len):
        data.append({headers[0]:dqm.info_description[i], headers[1]:'No','':dqm.info_id[i]})
        ids.append(dqm.info_id[i])
    return headers,data,ids

def create_stm_table(dqm):
    headers = ['Parameter', 'Value']
    data = []
    ids = []
    for i in range(dqm.reg_data_len,dqm.reg_data_len+dqm.stm_data_len):
        data.append({headers[0]:dqm.info_description[i], headers[1]:' - ','':dqm.info_id[i]})
        ids.append(dqm.info_id[i])
    return headers,data,ids

def create_chan_table(dqm,chan):
    headers = ['Parameter', 'Value']
    data = []
    ids = []
    low = 0
    high = 0
    if (chan == 0): 
        low = dqm.reg_data_len + dqm.stm_data_len
        high = dqm.reg_data_len + dqm.stm_data_len + dqm.ch_data_len
    if (chan == 1): 
        low = dqm.reg_data_len + dqm.stm_data_len + dqm.ch_data_len
        high = dqm.reg_data_len + dqm.stm_data_len + CHNUM*dqm.ch_data_len
    for i in range(low, high):
        data.append({headers[0]:dqm.info_description[i], headers[1]:' - ','':dqm.info_id[i]})
        ids.append(dqm.info_id[i])
    return headers,data,ids



