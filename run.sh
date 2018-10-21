if [ "$#" -lt "1" ]; then
   echo "\$1 must be one of (" `find ./out -maxdepth 1 -type f` ")";
   exit 1;
fi
fname=`basename $1`
commitid=`git rev-parse HEAD`
export RUN_NAME=$fname-$commitid-`date | sed 's/ /_/g' | sed 's/:/-/g'`
echo "Running: $RUN_NAME"

./$1 $2 $3 | gzip --best > /logs/$RUN_NAME.log
