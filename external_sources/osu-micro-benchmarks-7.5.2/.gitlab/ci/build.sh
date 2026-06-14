#!/bin/bash
echo "Gitlab Runner OMB Builder"
MAKE_NP=8

if [[ -d $LATEST_DIR ]]; then
    echo "Build already exists for:"
    echo $LATEST_DIR
else
    echo "Compiling latest branch..."
    mkdir -p $LATEST_DIR
    date > $LATEST_DIR/time_start
    autoreconf -vif >>$LATEST_DIR/autoreconf.log 2>&1


    ./configure --prefix=$LATEST_DIR \
                CC=$MVP_PATH/bin/mpicc \
                CXX=$MVP_PATH/bin/mpicxx \
                LDFLAGS=-L$MVP_PATH/lib \
                CFLAGS=-I$MVP_PATH/include \
                >>$LATEST_DIR/configure.log 2>&1

    make -j $MAKE_NP install >>$LATEST_DIR/install.log 2>&1

    if [[ $? -eq 0 ]]; then
        echo "Success. Installed here: $LATEST_DIR"
        date > $LATEST_DIR/time_end
    else
        echo "Error building..."
        rm -rf $LATEST_DIR
        exit 1
    fi
    ln -s $MVP_PATH/bin $LATEST_DIR/bin
    ln -s $MVP_PATH/include $LATEST_DIR/include
    ln -s $MVP_PATH/lib $LATEST_DIR/lib
fi

if [[ -d $MASTER_DIR ]]; then
    echo "Build already exists for:"
    echo $MASTER_DIR
else
    git clean -dfx >> /dev/null 2>&1
    git checkout -- . >> /dev/null 2>&1
    git checkout master
    echo "Compiling master branch..."
    mkdir -p $MASTER_DIR
    date > $MASTER_DIR/time_start
    autoreconf -vif >>$MASTER_DIR/autoreconf.log 2>&1


    ./configure --prefix=$MASTER_DIR \
                CC=$MVP_PATH/bin/mpicc \
                CXX=$MVP_PATH/bin/mpicxx \
                LDFLAGS=-L$MVP_PATH/lib \
                CFLAGS=-I$MVP_PATH/include \
                >>$MASTER_DIR/configure.log 2>&1


    make -j $MAKE_NP install >>$MASTER_DIR/install.log 2>&1

    if [[ $? -eq 0 ]]; then
        echo "Success. Installed here: $MASTER_DIR"
        date > $MASTER_DIR/time_end
    else
        echo "Error building..."
        exit 1
    fi
    ln -s $MVP_PATH/bin $MASTER_DIR/bin
    ln -s $MVP_PATH/include $MASTER_DIR/include
    ln -s $MVP_PATH/lib $MASTER_DIR/lib
fi

echo "Latest Build: $LATEST_DIR"
echo "Master Build: $MASTER_DIR"
