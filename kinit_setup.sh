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

            if [ "$1" == "" ]; then
                echo -e "setup_kinit.sh:${LINENO} |  \t    No kerberos user found, please kinit."
                return;
            fi
            kcacheUser=$1
        fi
        echo -e "setup_kinit.sh:${LINENO} |  \t    Saving local kcache for ${kcacheUser}..."

        mkdir $SCRIPT_DIR/tmp &>/dev/null

        #do not overwite existing keberos cache
		if [ ! -e "$SCRIPT_DIR/tmp/krb5cc_${UGROUP_ID}_${kcacheUser}" ]; then
            cp $kcachePath $SCRIPT_DIR/tmp/krb5cc_${UGROUP_ID}_${kcacheUser}
        fi
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

    EMAIL_ADDR=`klist 2>/dev/null|grep "Default principal"|cut -d: -f2|sed 's/ //g'`

    if [[ "x$EMAIL_ADDR" == "x" ]]; then
        echo -e "setup_kinit.sh:${LINENO} |  \t  Attempt to setup git author failed - no kerberos principal found; to setup git author please kinit and then re-setup."
        echo -e "setup_kinit.sh:${LINENO} |  \t "
    else
        AUTHOR_NAME=`timeout 5s ${PWD}/otsdaq-mu2e-config/tele_lookup.sh ${EMAIL_ADDR}`

        # save 'cache' git info to .gitconfig
        #git config --global user.email "${EMAIL_ADDR}" #NOTE -- git config changes for all users of shared username (so other users should use the setup_ots.sh approach)
        #git config --global user.name "${AUTHOR_NAME}" #NOTE -- git config changes for all users of shared username (so other users should use the setup_ots.sh approach)
        export GIT_AUTHOR_EMAIL=${EMAIL_ADDR-} # NULL is OK
        export GIT_AUTHOR_NAME=${AUTHOR_NAME}
        export GIT_COMMITTER_NAME="$GIT_AUTHOR_NAME on $USER@$HOSTNAME"
        export GIT_COMMITTER_EMAIL=$GIT_AUTHOR_EMAIL
        export GIT_SSH_COMMAND="ssh -i ~/.ssh/id_${EMAIL_ADDR:+${EMAIL_ADDR%@*}_}rsa"

        echo -e "setup_kinit.sh:${LINENO} |  \t     GIT_AUTHOR_EMAIL=${GIT_AUTHOR_EMAIL}"
        echo -e "setup_kinit.sh:${LINENO} |  \t     GIT_AUTHOR_NAME=${GIT_AUTHOR_NAME}"
        echo -e "setup_kinit.sh:${LINENO} |  \t     GIT_COMMITTER_NAME=${GIT_COMMITTER_NAME}"
        echo -e "setup_kinit.sh:${LINENO} |  \t     GIT_COMMITTER_EMAIL=${GIT_COMMITTER_EMAIL}"
        echo -e "setup_kinit.sh:${LINENO} |  \t "
    fi




    # Function to renew Kerberos ticket in the background
renew_kerberos_ticket() {
  while true; do
    kinit -R &>/dev/null
    sleep 100
  done
}

# Start the function in the background
renew_kerberos_ticket &
