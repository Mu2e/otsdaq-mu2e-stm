### Setup DQM
# To setup the DQM, you require the following packages. The ones used 
# during development and verified working are presented in parenthesis.
# If you are experiencing any software limitations please force use of
# these packages.

# Verify the installed version numbers
export REQUIRED_EXPRESS_VERSION=`cat package.json | grep "express" | cut -d ':' -f 2 | cut -d '"' -f 2`
export REQUIRED_ZEROMQ_VERSION=`cat package.json | grep "zeromq" | cut -d ':' -f 2 | cut -d '"' -f 2`
export REQUIRED_HTTP_SERVER_VERSION=`cat package.json | grep "http-server" | cut -d ':' -f 2 | cut -d '"' -f 2`
export REQUIRED_CHARTJS_VERSION=`cat package.json | grep "chart.js" | cut -d ':' -f 2 | cut -d '"' -f 2`

# check zeromq
IFS=@ read _ zeromq_version <<< `npm list | grep zeromq`
if
    [[ $zeromq_version != $REQUIRED_ZEROMQ_VERSION ]] ; then 
        echo "ZeroMQ version not correct, required version ${REQUIRED_ZEROMQ_VERSION}, you are using ${zeromq_version}. Proceed with caution - the file may not work as intended with the installed version."
fi
#check chart.js
IFS=@ read _ chartjs_version <<< `npm list | grep chart.js`
if
    [[ $chartjs_version != $REQUIRED_CHARTJS_VERSION ]] ; then 
    echo "Chart.js version not correct, required version ${REQUIRED_CHARTJS_VERSION}, you are using ${chartjs_version}. Proceed with caution - the file may not work as intended with the installed version."
fi
# check express
IFS=@ read _ express_version <<< `npm list | grep express`
if
    [[ $express_version != $REQUIRED_EXPRESS_VERSION ]] ; then 
    echo "Express version not correct, required version ${REUQIRED_EXPRESS_VERSION}, you are using ${express_version} Proceed with caution - the file may not work as intended with the installed version."
fi
# check http-server
IFS=@ read _ http_server <<< `npm list | grep http-server`
if
    [[ $http_server != $REQUIRED_HTTP_SERVER_VERSION ]] ; then 
    echo "ZeroMQ version not correct, required version ${REQUIRED_HTTP_SERVER_VERSION}, you are using ${http_server_version}. Proceed with caution - the file may not work as intended with the installed version."
fi



# Declare data flow addresses
export DQM_HOST_ADDRESS="2050"
export DQM_ZMQ_ADDRESS="2000"

# Declare plotting vairables
export DQM_DISPLAY_TIME="10"
export DQM_SAMPLING_RATE="100"
export DQM_DISPLAYING_SAMPLES=$(($DQM_DISPLAY_TIME * $DQM_SAMPLING_RATE))

# Run the script
node ELBE_DQM.js