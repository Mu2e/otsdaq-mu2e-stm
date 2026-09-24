import os

# Function to get the dictionary for a header from dataVars.hh
def getHeaderDict(hdrStr):
    # Get STMDAQ_ROOT path to utils/dataVars.hh
    stmdaq_root = os.environ["STMDAQ_ROOT"]
    # Get filename dataVars.hh
    filename = "dataVars.hh"
    # Get path to utils/dataVars.hh
    filepath = stmdaq_root + "/utils/" + filename

    # Try to open and read the file
    try:
        # Open the file
        f = open(filepath)
        # Notify the user
        print("readBFHeader: opening file = ",filepath)

        # Initialise the header dictionary
        hdrDict = {}

        # Loop over all lines in the file
        for line in f:
            # If the header string and "uint" are in that line...
            if hdrStr in line and "uint" in line:
                # Store the line stripped of the line break
                l = line.strip("\n")

                # Get the location in the line where hdrStr ends
                # index90 returns an exception when not found, whereas find returns -1
                hdrStrLoc = l.index(hdrStr) + len(hdrStr)
                # Get the location in the line where "=" is
                eqLoc = l.index("=")
                # Get the location in the line where ";" is
                coLoc = l.index(";")
            
                # Get the dictionary entry key
                # Remove whitespace, and underscore (+1)
                key = l[hdrStrLoc+1:eqLoc].strip()            
                # Get the dictionary entry value
                value = l[eqLoc+1:coLoc].strip()
            
                # Print the key and the value
                #print(f"k:{key}, v:{value}.")
                # If the legnth of the key is > 0
                if (len(key) > 0):
                    # Try to store the key as an integer value in the dictionary
                    try:
                        hdrDict[key] = int(value)
                    # If this fails, it probably relies on another dictionary entry
                    except ValueError:
                        # Look for "/" for division, e.g. fw_pHdr_Len = fw_pHdr_Size/2
                        divLoc = value.index("/")
                    
                        # Get key of the other dictionary entry it depends on
                        key2 = value[len(hdrStr)+1:divLoc]
                        # ... and the value
                        val2 = hdrDict[key2]
                        # Get the value to divide by
                        v = value[divLoc+1:]
                        # Do the division
                        theint = int(val2) / int(v)

                        # Now we have the right entry, add it to the dict
                        hdrDict[key] = int(theint)
        
        # Return the header dictionary
        return hdrDict

    # If trying opening the file failed
    except IOError:
        # Notify the user and return
        print(f"File {filepath} not accessible, returning empty Dict")
        return {}
    # Finally, close the file
    finally:
        f.close()
