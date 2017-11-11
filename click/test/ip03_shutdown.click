require(library ip03.click)

Script(TYPE ACTIVE,
    wait 1,
    // shutdown router4
    write front4.power 0,
    write back4.power 0,
    print "shutdown router4"
    //
)
