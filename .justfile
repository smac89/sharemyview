build:
    xmake f --mode=release
    xmake build capture
build-dbg:
    xmake f --mode=debug
    xmake build capture
run:
    xmake run capture
run-dbg:
    xmake run --debug capture
