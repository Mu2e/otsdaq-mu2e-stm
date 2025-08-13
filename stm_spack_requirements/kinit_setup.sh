#!/bin/sh
echo # This script is intended to be sourced.

sh -c "[ `ps $$ | grep bash | wc -l` -gt 0 ] || { echo 'Please switch to the bash shell before running ots.'; exit; }" || exit

export HOSTNAME="$(hostname -f)"

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
otsdir=$SCRIPT_DIR

echo -e "setup_kinit.sh:${LINENO} |  \t SCRIPT_DIR=$SCRIPT_DIR"

            
    kcacheType=`echo $KRB5CCNAME | cut -d ':' -f1`    
    kcachePath=`echo $KRB5CCNAME | cut -d ':' -f2`  
    
    UGROUP_ID=`id | cut -d '(' -f1 | cut -d '=' -f2`; #for example 55443 for mu2ehwdev  
    kcacheUser=`klist 2>/dev/null|grep "Default principal"|cut -d: -f2|sed 's/ //g' | cut -d '@' -f1`

    echo -e "setup_kinit.sh:${LINENO} |  \t kcacheType=$kcacheType"
    if [ "$kcacheType" == "FILE" ]; then

        if [ "$kcacheUser" == "" ]; then
            echo -e "setup_ots.sh:${LINENO} |  \t    No kerberos user found, please kinit."
            return;
        fi
        echo -e "setup_ots.sh:${LINENO} |  \t    Saving local kcache for ${kcacheUser}..."

        mkdir $SCRIPT_DIR/tmp &>/dev/null

        cp $kcachePath $SCRIPT_DIR/tmp/krb5cc_${UGROUP_ID}_${kcacheUser}
        export KRB5CCNAME=$SCRIPT_DIR/tmp/krb5cc_${UGROUP_ID}_${kcacheUser}
    fi

    echo -e "setup_kinit.sh:${LINENO} |  \t kcacheUser=$kcacheUser"
    export GIT_SSH_COMMAND="ssh -i ~/.ssh/id_${kcacheUser}_rsa"

    kinitCache=`klist 2>&1  | grep cache`
    
    # echo "kinitCache $kinitCache"
    # if the klist cache is based only on group account of mu2e* --> UGROUP_ID is the suffix, and people inadvertently share kerberos certificates
    kinitCacheCut=`echo $kinitCache | cut -d ':' -f3`    
    if [[ "x$kinitCacheCut" == "x${UGROUP_ID}" ]]; then
        kdestroy >/dev/null 2>&1
        randomSequence=`tr -dc A-Za-z0-9 </dev/urandom | head -c 13 ; echo ''`
        export KRB5CCNAME=/tmp/krb5cc_${UGROUP_ID}_${randomSequence}
        echo "klist cache was associated with group account, changing for user - now ${KRB5CCNAME}; any kerberos tickets were destroyed"
    fi
    #try 4th position (after AL9?)
    kinitCacheCut=`echo $kinitCache | cut -d ':' -f4`    
    if [[ "x$kinitCacheCut" == "x${UGROUP_ID}" ]]; then
        kdestroy >/dev/null 2>&1
        randomSequence=`tr -dc A-Za-z0-9 </dev/urandom | head -c 13 ; echo ''`
        export KRB5CCNAME=/tmp/krb5cc_${UGROUP_ID}_${randomSequence}
        echo "klist cache was associated with group account, changing for user - now ${KRB5CCNAME}; any kerberos tickets were destroyed"
    fi


    if [[ "x$kinitCache" == "xTicket cache: FILE:/tmp/krb5cc_${UGROUP_ID}" ]]; then
        kdestroy >/dev/null 2>&1
        randomSequence=`tr -dc A-Za-z0-9 </dev/urandom | head -c 13 ; echo ''`
        export KRB5CCNAME=/tmp/krb5cc_${UGROUP_ID}_${randomSequence}
        echo "klist cache was associated with group account, changing for user - now ${KRB5CCNAME}; any kerberos tickets were destroyed"
    fi
    if [[ "x$kinitCache" == "xklist: No credentials cache found (filename: /tmp/krb5cc_${UGROUP_ID})" ]]; then
        kdestroy >/dev/null 2>&1
        randomSequence=`tr -dc A-Za-z0-9 </dev/urandom | head -c 13 ; echo ''`
        export KRB5CCNAME=/tmp/krb5cc_${UGROUP_ID}_${randomSequence}
        echo "klist cache was associated with group account, changing for user - now ${KRB5CCNAME}; any kerberos tickets were destroyed"
    fi
    klist


    # Function to renew Kerberos ticket in the background
renew_kerberos_ticket() {
  while true; do
    kinit -R &>/dev/null
    sleep 100
  done
}

# Start the function in the background
renew_kerberos_ticket &
