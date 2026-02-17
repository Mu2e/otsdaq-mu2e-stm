import sys
import os
import BinaryFile as bf
import xml.etree.ElementTree as XML

def test_xml():
	# test code to read xml file
	xml = {}
	STMDAQ_ROOT = os.getenv('STMDAQ_ROOT')
	xml_file = STMDAQ_ROOT+"/config/stmdaq.xml"
	xroot = XML.parse(xml_file).getroot()
	for value in xroot.iter():
		if (value.tag != 'stm') and ('fmc144' in value.tag):
		    xml[value.tag] = value.text.strip()

	print("Setting Register values using xml file = %s" % (xml_file))
	print(xml)
	for key in xml:
	    ivalue = int (xml[key], 16)
	    print(key)
	    print(" %s = 0x%x (%d)" % (key, ivalue, ivalue)) 

	print(int (xml["fmc144_ext_ADC_offset"], 16))
	print(int (xml["fmc144_Start_reset_stop_clock"],16))
	print(int (xml["fmc144_Test_beam_10g_readout"],16))
	print(int (xml["fmc144_ext_slice_length"],16))
	print(int (xml["fmc144_ext_slice_num"],16))
	print(int (xml["fmc144_ext_int_delay"],16))
	print(int (xml["fmc144_int_ADC_offset"],16))
	print(int (xml["fmc144_int_slice_length"],16))
	print(int (xml["fmc144_int_slice_num"],16))
	print(int (xml["fmc144_ext_trig_timeout"],16))



def debug(data):
	i = 0
	for d in data:
		print("[%d] = %d" % (i,d))
		i = i + 1

#test_xml()

bff = bf.BinaryFile("data/stm_hzdr_0000012_00026.bin")

data = bff.read_data() # read whole file
#debug(data)

read_from_byte_offset = 2*512
num_int16_to_read = 512
data = bff.read_data(read_from_byte_offset, num_int16_to_read) # read part of file
#debug(data)

#data = bff.read_data(read_from_byte_offset, 1000*num_int16_to_read) # check exits if attempt to read too many bytes
#data = bff.read_data(-1) # check rogue argument error



bff.close_file()
