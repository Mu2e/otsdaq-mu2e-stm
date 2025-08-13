#!/bin/bash

WORKAREA=$1
KERBUSER=$2
if [[ "${WORKAREA}" == "" ]]; then
    WORKAREA="ots_v4"
fi

COMMANDS="cd ~/${WORKAREA}; source kinit_setup.sh ${KERBUSER}"
xfce4-terminal --tab --title="TRIGGER"   --command "bash -c \"${COMMANDS}; ssh -t mu2e-calo-01 \\\"${COMMANDS}; source setup_ots.sh trigger; bash -l\\\"\"" --geometry 160x30
xfce4-terminal --tab --title="TOP LEVEL" --command "bash -c \"${COMMANDS}; ssh -t mu2e-calo-01 \\\"emacs -bg honedew -T CALO-01 &; ${COMMANDS}; source setup_ots.sh shift  ; bash -l\\\"\"" --geometry 160x30
xfce4-terminal --tab --title="CALO"      --command "bash -c \"${COMMANDS}; ssh -t mu2e-calo-02 \\\"${COMMANDS}; emacs -bg mistyrose -T CALO-02 &; source setup_ots.sh calo   ; bash -l\\\"\"" --geometry 160x30
xfce4-terminal --tab --title="CFO"       --command "bash -c \"${COMMANDS}; ssh -t mu2e-cfo-01  \\\"${COMMANDS}; source setup_ots.sh cfo    ; bash -l\\\"\"" --geometry 160x30
xfce4-terminal --tab --title="DQM"       --command "bash -c \"${COMMANDS}; ssh -t mu2e-dl-01   \\\"${COMMANDS}; source setup_ots.sh dqm    ; bash -l\\\"\"" --geometry 160x30
xfce4-terminal --tab --title="DL"        --command "bash -c \"${COMMANDS}; ssh -t mu2e-dl-01   \\\"${COMMANDS}; emacs -bg linen -T DL-01 &; source setup_ots.sh trigger; bash -l\\\"\"" --geometry 160x30
