### Install the DQM
# Check for required infrastructure and install the required packages. In case of
# failure, the required packages and versions are:
# node 12.22.12
# npm 8.6.0
# express 4.17.3
# zeromq 5.2.8
# http-server 14.1.0
# chartjs @2.5.0
# If you cannot install using `npm install`, use `npm install <package>@<version>`
# and this will be able to run.

# Check if nodejs and npm are installed. Exit either are not.
export REQUIRED_NODE_VERSION="v12.22.12"
NODE_VERSION=`node -v 2>/dev/null`
if
    [[ "$NODE_VERSION" = "" ]] ; then
    echo "No version of node detected. Install node using `sudo dnf module install nodejs:12.22.12`."
    return 1
    elif [[ $NODE_VERSION != $REQUIRED_NODE_VERSION ]] ; then
    echo "Detected node ${NODE_VERSION}, reuqires version ${REQUIRED_NODE_VERSION}."
    echo "Proceed with caution - the file may not work as intended with the installed version."
    else 
    echo "Detected node ${NODE_VERSION}."
fi

export REQUIRED_NPM_VERSION="8.6.0"
NPM_VERSION=`npm -v 2>/dev/null`
if
    [[ "$NPM_VERSION" = "" ]] ; then
    echo "No version of node detected. Install npm v8.6.0."
    return 1
    elif [[ $NPM_VERSION != $REQUIRED_NPM_VERSION ]] ; then
    echo "Detected npm v${NPM_VERSION}, requires version ${REQUIRED_NPM_VERSION}."
    echo "Proceed with caution - the file may not work as intended with the installed version."
    else 
    echo "Detected npm v${NPM_VERSION}"
    
fi

# Install the dependency modules
npm install 1>/dev/null


echo "Installed successfully. Run 'source runDQM.sh' to start the DQM."
