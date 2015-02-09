#! /bin/bash
clear
echo INIT-ECA of many nodes
echo can be used to configure many nodes based on the config file 

#allowed protocols are dev or udp
proto=dev

echo using $proto as protocol

#echo appending paths - I think we don't need this any more
#LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/common/export/etherbone/i386/lib; export LD_LIBRARY_PATH
#PATH=$PATH:/common/export/etherbone/i386/bin:/home/france/timing/mcs-bin; export PATH

st=IDLE	
finaldev=$(
cat node-eca.conf | {
		while read line; do
			if [[ $line == $proto* ]] && [ $st == IDLE ]; then
				st=GETPARAMS
				dev=$line
				
				eca-table $dev flush
				if [ $? -gt 0 ]; then
					echo ERROR					
					echo $dev						
					break
				else 				
					echo $dev
				fi
				
			elif [[ $line == 0x* ]] && [ $st == GETPARAMS ]; then
				IFS=" "
				array=($line)
	
				id=${array[0]}
				offs=${array[1]}
				chn=${array[2]}
				outp=${array[3]}
		
				eca-table $dev add $id $offs $chn $outp
				if [ $? -gt 0 ]; then
   	 			echo ERROR
					break
				fi
				eca-ctl $dev -c $chn activate
				if [ $? -gt 0 ]; then
   	 				echo ERROR
					break
				fi		
			else
				if [[ $line != \#* ]]; then 
					st=IDLE
				fi
			fi
			
		done 
	}
)

IFS=$'\n'
array=($finaldev)

st=0
cnt=0

for dev in "${array[@]}";
do
	if [[ $dev == $proto* ]]; then	
		if [ $st == ERROR ]; then		
			echo -ne "$dev\r"
			break
		else
			echo Configured $dev
			eca-table $dev flip-active
			eca-ctl $dev enable
			cnt=`expr $cnt + 1`
#			simple-display $dev -s "CNFG_OK" > /dev/null //not sure if we continue supporting the display
		fi
	elif [[ $dev == ERROR* ]]; then
		st=ERROR		
		echo "ERROR: Config was aborted, could not reach device"  
	else
		echo wtf
		echo $dev	
	fi
done
echo Done. Initialised $cnt Endpoints



