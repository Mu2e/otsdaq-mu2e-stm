
echo -e "ecl_setup_ots.sh:${LINENO} |  \t  Setting up ECL Mu2e Logbook credentials"
echo -e "ecl_setup_ots.sh:${LINENO} |  \t    Note: If the user input prompt is interrupted, type 'reset' to return normal character display to your console."
export ECL_URL="http://dbweb0.fnal.gov/ECL/mu2e" # will reroute to something like this https://dbweb8.fnal.gov:8443/ECL/mu2e"
export ECL_CATEGORY="Global Run"
# do not put username/pw in saved/committed text files!!

if [ "x$ECL_USER_NAME" == "x" ]; then #when RUNINFO password is not setup, prompt user
       stty -echo
       printf "(Enter your ECL username, or leave blank to disable ECL logging):\n"
       read runinfopass
       stty echo
       export ECL_USER_NAME=$runinfopass
else
       echo -e "ecl_setup_ots.sh:${LINENO} |  \t    unset ECL_USER_NAME # to disable ECL logging"
fi

if [[ "x$ECL_USER_NAME" != "x" && "x$ECL_PASSWORD" == "x" ]]; then #when RUNINFO password is not setup, prompt user
       stty -echo
       printf "(Enter the Services ECL Password for ${ECL_USER_NAME}):\n"
       read runinfopass
       stty echo
       export ECL_PASSWORD=$runinfopass
fi