require(library ip03.click)

Idle() -> [1] router1
Idle() -> [1] router2
Idle() -> [1] router3
Idle() -> [1] router4
Idle() -> [1] router5
Idle() -> [1] router6

Script(TYPE ACTIVE,
    wait 1,
    // shutdown router4
    write front4.power 0,
    write back4.power 0,
    print "shutdown router4",
    wait 20,
	// poweron router4
    write front4.power 1,
    write back4.power 1,
	print "poweron router4"
)
