if [[ $LD_ENV_PATH ]]
then
    source $LD_ENV_PATH
else
    source env_ssh
fi

read rp

if [[ "$rp" == "y" ]] || [[ "$rp" == "Y" ]]
then
    ./init_ssh.sh
fi

echo
echo "Select a destination :"
echo "1 : PLT1 HLS"
echo "2 : PLT2 HLS"
echo "3 : PLT1 PM"
echo "4 : PLT2 PM"
echo "[1/2/3/4] : "

read choice

destination=""
user=""

case $choice in
    "1")
	destination=$PLT1_HLS
	user=$USER_HLS
	;;
    "2")
	destination=$PLT2_HLS
	user=$USER_HLS
	;;
    "3")
	destination=$PLT1_PM
	user=$USER_PM
	;;
    "4")
	destination=$PLT2_PM
	user=$USER_PM
	;;
    *)
	echo "Select [1/2/3/4] only !"
	exit 1
	;;
esac

find_openport=0
tunnelpid=0

minport=42000
maxport=45000

# Find an open port
port=0
openports=$(netstat -lnt 2>&1 | grep tcp | grep -v tcp6 | awk '{print $4}' | awk -F: '{print $2}' | xargs echo -n)

for port in $(seq ${minport} ${maxport})
do
  if ! [[ "${openports}" =~ "${port}" ]]
  then
	find_openport=1
    break
  fi
done

ssh -N -L localhost:${port}:${destination}:22 ${GATEWAY} &
tunnelpid=$!

# Give the tunnel some time
sleep 1

read rp
if [[ "$rp" == "y" ]] || [[ "$rp" == "Y" ]]
then
  read source

  if [[ $source == "" ]]
  then
      echo "Null source !"
  else
      echo
      delivery=delivery$(date '+%Y-%m-%d_%H%M%S').tar
      tar -cvf $delivery $source
      res_tar=`echo $?`

      if [[ $res_tar == 0 ]]
      then
	  scp -P ${port} -o StrictHostKeyChecking=no -o HostKeyAlias=${destination} $delivery $user@localhost:/tmp
      else
	  echo "Fail to tar !"
      fi
      rm $delivery
  fi
fi

read rp
if [[ "$rp" == "y" ]] || [[ "$rp" == "Y" ]]
then
    ssh -X -p ${port} -o StrictHostKeyChecking=no -o HostKeyAlias=${destination} $user@localhost
else
    echo "If you keep ssh tunnel alive, you can connect to $destination with command :"
    echo "    ==> ssh -X -p ${port} -o StrictHostKeyChecking=no -o HostKeyAlias=${destination} $user@localhost"
fi

read rp
if [[ "$rp" == "n" ]] || [[ "$rp" == "N" ]] || [[ "$rp" == "" ]]
then
    kill $tunnelpid
else
    echo "To kill ssh tunnel, use :"
    echo "    ==> kill $tunnelpid"
fi
