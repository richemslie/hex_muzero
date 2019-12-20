if [ -z "$K273_PATH" ]; then
    echo "Please set \$K273_PATH"
    return 1
fi

if [ -z "$MUZERO_PATH" ]; then
    export MUZERO_PATH=`python2 -c "import os.path as p; print p.dirname(p.dirname(p.abspath('$BASH_SOURCE')))"`
    echo "Automatically setting \$MUZERO_PATH to" $MUZERO_PATH
else
    echo "MUZERO_PATH set to" $MUZERO_PATH
fi

export LD_LIBRARY_PATH=$MUZERO_PATH/src/cpp:$LD_LIBRARY_PATH
