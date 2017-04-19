source env_ssh

ssh-keygen -t dsa -f $HOME/.ssh/id_dsa

cmd_grep=`ssh w553458 "cat ~/.ssh/authorized_keys2" | grep -c $DEV_NAME`
if [ $cmd_grep == "0" ]
then
    cat ~/.ssh/id_dsa.pub | ssh $GATEWAY "cat - >> .ssh/authorized_keys2"

    ssh-add
fi
