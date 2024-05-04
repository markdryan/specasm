dump_test_names() {
    if [[ $OSTYPE == 'darwin'* ]]; then
	local size=`stat -f "%z" $1`
    else
	local size=`stat -c "%s" $1`
    fi
    local num_tests_off=$(( size - 1 ))
    local num_tests=`od -An -j $num_tests_off -tu -N1 $1 | xargs`
    local table_start_off=$(( size - 3 ))
    local table_start=`od -An -j $table_start_off -tu -N2 $1 | xargs`
    local ch
    table_start=$(( table_start - $2 + 2))
    for ((i = 1 ; i <= $num_tests ; i++ )); do
	  ch=`od -An -j $table_start -tu -N1 $1 | xargs`
	  while [ $ch -gt 0 ]; do
	      ch=`od -An -j $table_start -tc -N1 $1 | xargs`
	      echo -n $ch
	      table_start=$(( table_start + 1))
	      ch=`od -An -j $table_start -tu -N1 $1 | xargs`
	  done
	  table_start=$(( table_start + 3))
	  if [ $i -lt $num_tests ]; then
	     echo -n " "
	  fi
    done
    echo ""
}
