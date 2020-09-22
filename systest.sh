if [[ $1 == "" ]]; then # multitheaded, latest only, no record
python systest.py 1
exit
fi

if [[ $1 == "time" ]]; then # singlethreaded, record latest
python systest.py 0
exit
fi

if [[ $1 == "timeall" ]]; then # singlethreaded, record all
python systest.py 2
exit
fi

echo "Usage: bash systest.sh [ | time | timeall]"
